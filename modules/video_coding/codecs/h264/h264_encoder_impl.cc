/*
 *  Copyright (c) 2024 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 *
 */

// Everything declared/defined in this header is only required when WebRTC is
// build with H264 support, please do not move anything out of the
// #ifdef unless needed and tested.
#ifdef WEBRTC_USE_H264

#include "modules/video_coding/codecs/h264/h264_encoder_impl.h"

#include <algorithm>
#include <limits>
#include <string>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
}

#include "absl/strings/match.h"
#include "common_video/libyuv/include/webrtc_libyuv.h"
#include "modules/video_coding/svc/create_scalability_structure.h"
#include "modules/video_coding/utility/simulcast_rate_allocator.h"
#include "modules/video_coding/utility/simulcast_utility.h"
#include "rtc_base/checks.h"
#include "rtc_base/logging.h"
#include "rtc_base/time_utils.h"
#include "system_wrappers/include/metrics.h"
#include "third_party/libyuv/include/libyuv/convert.h"
#include "third_party/libyuv/include/libyuv/scale.h"

namespace webrtc {

namespace {

// QP scaling thresholds.
static const int kLowH264QpThreshold = 24;
static const int kHighH264QpThreshold = 37;

// Used by histograms. Values of entries should not be changed.
enum H264EncoderImplEvent {
  kH264EncoderEventInit = 0,
  kH264EncoderEventError = 1,
  kH264EncoderEventMax = 16,
};

VideoFrameType ConvertToVideoFrameType(int flags) {
	if (flags & AV_PICTURE_TYPE_I)
	  return VideoFrameType::kVideoFrameKey;
  return VideoFrameType::kVideoFrameDelta;
}

}  // namespace

// Helper method used by H264EncoderImpl::Encode.
// Copies the encoded bytes from `pkt` to `encoded_image`.
static void RtpFragmentize(EncodedImage* encoded_image, AVPacket* pkt) {
  size_t required_capacity = pkt->size;
  auto buffer = EncodedImageBuffer::Create(required_capacity);
  encoded_image->SetEncodedData(buffer);

	memcpy(buffer->data(), pkt->data, pkt->size);
	encoded_image->set_size(pkt->size);
}

H264EncoderImpl::H264EncoderImpl(const cricket::VideoCodec& codec)
    : packetization_mode_(H264PacketizationMode::SingleNalUnit),
      max_payload_size_(0),
      number_of_cores_(0),
      encoded_image_callback_(nullptr),
      has_reported_init_(false),
      has_reported_error_(false) {
		  RTC_LOG(LS_INFO) << "### H264EncoderImpl::H264EncoderImpl";
  RTC_CHECK(absl::EqualsIgnoreCase(codec.name, cricket::kH264CodecName));
  std::string packetization_mode_string;
  if (codec.GetParam(cricket::kH264FmtpPacketizationMode,
                     &packetization_mode_string) &&
      packetization_mode_string == "1") {
    packetization_mode_ = H264PacketizationMode::NonInterleaved;
  }
  downscaled_buffers_.reserve(kMaxSimulcastStreams - 1);
  encoded_images_.reserve(kMaxSimulcastStreams);
  encoders_.reserve(kMaxSimulcastStreams);
  configurations_.reserve(kMaxSimulcastStreams);
  tl0sync_limit_.reserve(kMaxSimulcastStreams);
  svc_controllers_.reserve(kMaxSimulcastStreams);
}

H264EncoderImpl::~H264EncoderImpl() {
  Release();
}

int32_t H264EncoderImpl::InitEncode(const VideoCodec* inst,
                                    const VideoEncoder::Settings& settings) {
	RTC_LOG(LS_INFO) << "### H264EncoderImpl::InitEncode";
  ReportInit();
  if (!inst || inst->codecType != kVideoCodecH264) {
    ReportError();
    return WEBRTC_VIDEO_CODEC_ERR_PARAMETER;
  }
  if (inst->maxFramerate == 0) {
    ReportError();
    return WEBRTC_VIDEO_CODEC_ERR_PARAMETER;
  }
  if (inst->width < 1 || inst->height < 1) {
    ReportError();
    return WEBRTC_VIDEO_CODEC_ERR_PARAMETER;
  }

  int32_t release_ret = Release();
  if (release_ret != WEBRTC_VIDEO_CODEC_OK) {
    ReportError();
    return release_ret;
  }

  int number_of_streams = SimulcastUtility::NumberOfSimulcastStreams(*inst);
  bool doing_simulcast = (number_of_streams > 1);

  if (doing_simulcast &&
      !SimulcastUtility::ValidSimulcastParameters(*inst, number_of_streams)) {
    return WEBRTC_VIDEO_CODEC_ERR_SIMULCAST_PARAMETERS_NOT_SUPPORTED;
  }
  downscaled_buffers_.resize(number_of_streams - 1);
  encoded_images_.resize(number_of_streams);
  encoders_.resize(number_of_streams);
  svc_controllers_.resize(number_of_streams);
  configurations_.resize(number_of_streams);
  tl0sync_limit_.resize(number_of_streams);

  number_of_cores_ = settings.number_of_cores;
  max_payload_size_ = settings.max_payload_size;
  codec_ = *inst;

  // Code expects simulcastStream resolutions to be correct, make sure they are
  // filled even when there are no simulcast layers.
  if (codec_.numberOfSimulcastStreams == 0) {
    codec_.simulcastStream[0].width = codec_.width;
    codec_.simulcastStream[0].height = codec_.height;
  }

  for (int i = 0, idx = number_of_streams - 1; i < number_of_streams;
		  ++i, --idx) {
	  const AVCodec* codec = avcodec_find_encoder(AV_CODEC_ID_H264);
	  if (!codec) {
		  RTC_LOG(LS_ERROR) << "Can't find H.264 encoder";
		  Release();
		  ReportError();
		  return WEBRTC_VIDEO_CODEC_ERROR;
	  }

	  AVCodecContext* codec_context = avcodec_alloc_context3(codec);
	  if (!codec_context) {
		  RTC_LOG(LS_ERROR) << "Failed to allocate codec context";
		  Release();
		  ReportError();
		  return WEBRTC_VIDEO_CODEC_ERROR;
	  }

		// Store ffmpeg encoder.
		encoders_[i] = codec_context;

    // Set internal settings from codec_settings
    configurations_[i].simulcast_idx = idx;
    configurations_[i].sending = false;
    configurations_[i].width = codec_.simulcastStream[idx].width;
    configurations_[i].height = codec_.simulcastStream[idx].height;
    configurations_[i].max_frame_rate = static_cast<float>(codec_.maxFramerate);
    configurations_[i].frame_dropping_on = codec_.GetFrameDropEnabled();
    configurations_[i].key_frame_interval = codec_.H264()->keyFrameInterval;
    configurations_[i].num_temporal_layers =
        std::max(codec_.H264()->numberOfTemporalLayers,
                 codec_.simulcastStream[idx].numberOfTemporalLayers);

    // Create downscaled image buffers.
    if (i > 0) {
      downscaled_buffers_[i - 1] = I420Buffer::Create(
          configurations_[i].width, configurations_[i].height,
          configurations_[i].width, configurations_[i].width / 2,
          configurations_[i].width / 2);
    }

    // Codec_settings uses kbits/second; encoder uses bits/second.
    configurations_[i].max_bps = codec_.maxBitrate * 1000;
    configurations_[i].target_bps = codec_.startBitrate * 1000;

    // Initialize encoded image. Default buffer size: size of unencoded data.
    const size_t new_capacity =
        CalcBufferSize(VideoType::kI420, codec_.simulcastStream[idx].width,
                       codec_.simulcastStream[idx].height);
    encoded_images_[i].SetEncodedData(EncodedImageBuffer::Create(new_capacity));
    encoded_images_[i]._encodedWidth = codec_.simulcastStream[idx].width;
    encoded_images_[i]._encodedHeight = codec_.simulcastStream[idx].height;
    encoded_images_[i].set_size(0);

    tl0sync_limit_[i] = configurations_[i].num_temporal_layers;
    absl::optional<ScalabilityMode> scalability_mode;
    switch (configurations_[i].num_temporal_layers) {
      case 0:
        break;
      case 1:
        scalability_mode = ScalabilityMode::kL1T1;
        break;
      case 2:
        scalability_mode = ScalabilityMode::kL1T2;
        break;
      case 3:
        scalability_mode = ScalabilityMode::kL1T3;
        break;
      default:
        RTC_DCHECK_NOTREACHED();
    }
    if (scalability_mode.has_value()) {
      svc_controllers_[i] =
          CreateScalabilityStructure(scalability_mode.value());
      if (svc_controllers_[i] == nullptr) {
        RTC_LOG(LS_ERROR) << "Failed to create scalability structure";
        Release();
        ReportError();
        return WEBRTC_VIDEO_CODEC_ERROR;
      }
    }

		// Configure FFmpeg
		codec_context->width = configurations_[i].width;
		codec_context->height = configurations_[i].height;
		codec_context->time_base = (AVRational){1, 90000};
		codec_context->framerate = (AVRational){(int)configurations_[i].max_frame_rate, 1};
		codec_context->pix_fmt = AV_PIX_FMT_YUV420P;
		codec_context->bit_rate = configurations_[i].target_bps;

		if (avcodec_open2(codec_context, codec, NULL) < 0) {
			RTC_LOG(LS_ERROR) << "Failed to open codec";
			Release();
			ReportError();
			return WEBRTC_VIDEO_CODEC_ERROR;
		}
  }

  SimulcastRateAllocator init_allocator(codec_);
  VideoBitrateAllocation allocation =
      init_allocator.Allocate(VideoBitrateAllocationParameters(
          DataRate::KilobitsPerSec(codec_.startBitrate), codec_.maxFramerate));
  SetRates(RateControlParameters(allocation, codec_.maxFramerate));
  return WEBRTC_VIDEO_CODEC_OK;
}

int32_t H264EncoderImpl::Release() {
  while (!encoders_.empty()) {
		AVCodecContext* codec_context = encoders_.back();
		if (codec_context) {
			avcodec_free_context(&codec_context);
    }
    encoders_.pop_back();
  }
  downscaled_buffers_.clear();
  configurations_.clear();
  encoded_images_.clear();
  tl0sync_limit_.clear();
  svc_controllers_.clear();
  return WEBRTC_VIDEO_CODEC_OK;
}

int32_t H264EncoderImpl::RegisterEncodeCompleteCallback(
    EncodedImageCallback* callback) {
  encoded_image_callback_ = callback;
  return WEBRTC_VIDEO_CODEC_OK;
}

void H264EncoderImpl::SetRates(const RateControlParameters& parameters) {
  if (encoders_.empty()) {
    RTC_LOG(LS_WARNING) << "SetRates() while uninitialized.";
    return;
  }

  if (parameters.framerate_fps < 1.0) {
    RTC_LOG(LS_WARNING) << "Invalid frame rate: " << parameters.framerate_fps;
    return;
  }

  if (parameters.bitrate.get_sum_bps() == 0) {
    // Encoder paused, turn off all encoding.
    for (size_t i = 0; i < configurations_.size(); ++i) {
      configurations_[i].SetStreamState(false);
    }
    return;
  }

  codec_.maxFramerate = static_cast<uint32_t>(parameters.framerate_fps);

  size_t stream_idx = encoders_.size() - 1;
  for (size_t i = 0; i < encoders_.size(); ++i, --stream_idx) {
    configurations_[i].target_bps =
        parameters.bitrate.GetSpatialLayerSum(stream_idx);
    configurations_[i].max_frame_rate = parameters.framerate_fps;

    if (configurations_[i].target_bps) {
      configurations_[i].SetStreamState(true);

			// Update FFmpeg encoder settings
			AVCodecContext* codec_context = encoders_[i];
			codec_context->bit_rate = configurations_[i].target_bps;
			codec_context->framerate = 
				(AVRational){(int)configurations_[i].max_frame_rate, 1};
    } else {
      configurations_[i].SetStreamState(false);
    }
  }
}

int32_t H264EncoderImpl::Encode(
    const VideoFrame& input_frame,
    const std::vector<VideoFrameType>* frame_types) {
	RTC_LOG(LS_INFO) << "### H264EncoderImpl::Encode";
  if (encoders_.empty()) {
    ReportError();
    return WEBRTC_VIDEO_CODEC_UNINITIALIZED;
  }
  if (!encoded_image_callback_) {
    RTC_LOG(LS_WARNING)
        << "InitEncode() has been called, but a callback function "
           "has not been set with RegisterEncodeCompleteCallback()";
    ReportError();
    return WEBRTC_VIDEO_CODEC_UNINITIALIZED;
  }

  rtc::scoped_refptr<I420BufferInterface> frame_buffer =
      input_frame.video_frame_buffer()->ToI420();
  if (!frame_buffer) {
    RTC_LOG(LS_ERROR) << "Failed to convert "
                      << VideoFrameBufferTypeToString(
                             input_frame.video_frame_buffer()->type())
                      << " image to I420. Can't encode frame.";
    return WEBRTC_VIDEO_CODEC_ENCODER_FAILURE;
  }

  // Encode image for each layer.
  for (size_t i = 0; i < encoders_.size(); ++i) {
    if (!configurations_[i].sending) {
      continue;
    }

		AVCodecContext* codec_context = encoders_[i];

		// Prepare AVFrame
		AVFrame* frame = av_frame_alloc();
		if (!frame) {
			RTC_LOG(LS_ERROR) << "Failed to allocate video frame";
			ReportError();
			return WEBRTC_VIDEO_CODEC_ERROR;
      }

		frame->format = codec_context->pix_fmt;
		frame->width = codec_context->width;
		frame->height = codec_context->height;
		frame->pts = input_frame.timestamp();

		// Allocate frame buffer
		int ret = av_frame_get_buffer(frame, 32);
		if (ret < 0) {
			RTC_LOG(LS_ERROR) << "Failed to allocate frame data";
			av_frame_free(&frame);
			ReportError();
			return WEBRTC_VIDEO_CODEC_ERROR;
    }

		// Copy frame data
		if (i == 0) {
			libyuv::I420Copy(frame_buffer->DataY(), frame_buffer->StrideY(),
					frame_buffer->DataU(), frame_buffer->StrideU(),
					frame_buffer->DataV(), frame_buffer->StrideV(),
					frame->data[0], frame->linesize[0],
					frame->data[1], frame->linesize[1],
					frame->data[2], frame->linesize[2],
					frame->width, frame->height);
		} else {
			libyuv::I420Copy(downscaled_buffers_[i - 1]->DataY(), downscaled_buffers_[i - 1]->StrideY(),
					downscaled_buffers_[i - 1]->DataU(), downscaled_buffers_[i - 1]->StrideU(),
					downscaled_buffers_[i - 1]->DataV(), downscaled_buffers_[i - 1]->StrideV(),
					frame->data[0], frame->linesize[0],
					frame->data[1], frame->linesize[1],
					frame->data[2], frame->linesize[2],
					frame->width, frame->height);
		}

		// Encode
		ret = avcodec_send_frame(codec_context, frame);
		if (ret < 0) {
			RTC_LOG(LS_ERROR) << "Failed to send frame for encoding";
			av_frame_free(&frame);
			ReportError();
			return WEBRTC_VIDEO_CODEC_ERROR;
		}

		AVPacket* pkt = av_packet_alloc();
		if (!pkt) {
			RTC_LOG(LS_ERROR) << "Failed to allocate packet";
			av_frame_free(&frame);
      ReportError();
      return WEBRTC_VIDEO_CODEC_ERROR;
    }

		ret = avcodec_receive_packet(codec_context, pkt);
		if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
			av_packet_free(&pkt);
			av_frame_free(&frame);
			continue;
		} else if (ret < 0) {
			RTC_LOG(LS_ERROR) << "Failed to encode frame";
			av_packet_free(&pkt);
			av_frame_free(&frame);
			ReportError();
			return WEBRTC_VIDEO_CODEC_ERROR;
		}

		// Set encoded image parameters
    encoded_images_[i]._encodedWidth = configurations_[i].width;
    encoded_images_[i]._encodedHeight = configurations_[i].height;
    encoded_images_[i].SetTimestamp(input_frame.timestamp());
    encoded_images_[i].SetColorSpace(input_frame.color_space());
		encoded_images_[i]._frameType = ConvertToVideoFrameType(pkt->flags);
    encoded_images_[i].SetSpatialIndex(configurations_[i].simulcast_idx);

		// Copy encoded data
		RtpFragmentize(&encoded_images_[i], pkt);

		// Clean up
		av_packet_free(&pkt);
		av_frame_free(&frame);


    // Encoder can skip frames to save bandwidth in which case
    // `encoded_images_[i]._length` == 0.
		if (encoded_images_[i].size() > 0) {
			// Parse QP.
			h264_bitstream_parser_.ParseBitstream(encoded_images_[i]);
			encoded_images_[i].qp_ =
				h264_bitstream_parser_.GetLastSliceQp().value_or(-1);

      // Deliver encoded image.
			CodecSpecificInfo codec_specific;
			codec_specific.codecType = kVideoCodecH264;
			codec_specific.codecSpecific.H264.packetization_mode = packetization_mode_;
			codec_specific.codecSpecific.H264.temporal_idx = kNoTemporalIdx;
			codec_specific.codecSpecific.H264.idr_frame = (pkt->flags & AV_PKT_FLAG_KEY);
			codec_specific.codecSpecific.H264.base_layer_sync = false;

			encoded_image_callback_->OnEncodedImage(encoded_images_[i], &codec_specific);
		}
  }

	return WEBRTC_VIDEO_CODEC_OK;
}

void H264EncoderImpl::ReportInit() {
  if (has_reported_init_)
    return;
  RTC_HISTOGRAM_ENUMERATION("WebRTC.Video.H264EncoderImpl.Event",
                            kH264EncoderEventInit, kH264EncoderEventMax);
  has_reported_init_ = true;
}

void H264EncoderImpl::ReportError() {
  if (has_reported_error_)
    return;
  RTC_HISTOGRAM_ENUMERATION("WebRTC.Video.H264EncoderImpl.Event",
                            kH264EncoderEventError, kH264EncoderEventMax);
  has_reported_error_ = true;
}

VideoEncoder::EncoderInfo H264EncoderImpl::GetEncoderInfo() const {
	RTC_LOG(LS_INFO) << "### H264EncoderImpl::GetEncoderInfo";
  EncoderInfo info;
  info.supports_native_handle = false;
	info.implementation_name = "FFmpeg";
  info.scaling_settings =
      VideoEncoder::ScalingSettings(kLowH264QpThreshold, kHighH264QpThreshold);
  info.is_hardware_accelerated = false;
  info.supports_simulcast = true;
  info.preferred_pixel_formats = {VideoFrameBuffer::Type::kI420};
  return info;
}

void H264EncoderImpl::LayerConfig::SetStreamState(bool send_stream) {
  if (send_stream && !sending) {
    // Need a key frame if we have not sent this stream before.
    key_frame_request = true;
  }
  sending = send_stream;
}

}  // namespace webrtc

#endif  // WEBRTC_USE_H264
