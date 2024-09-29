/*
 *  Copyright (c) 2014 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "modules/audio_coding/codecs/aac/audio_encoder_aac.h"

#include <cstdint>

#include "modules/audio_coding/codecs/aac/aac_interface.h"
#include "rtc_base/checks.h"
#include "rtc_base/numerics/safe_conversions.h"
#include "rtc_base/logging.h"

namespace webrtc {

namespace {

const size_t kSampleRateHz = 16000;

}  // namespace

AudioEncoderAACImpl::AudioEncoderAACImpl(const AudioEncoderAACConfig& config,
                                           int payload_type)
    : num_channels_(config.num_channels),
      payload_type_(payload_type),
      num_10ms_frames_per_packet_(
          static_cast<size_t>(config.frame_size_ms / 10)),
      num_10ms_frames_buffered_(0),
      first_timestamp_in_buffer_(0),
      encoders_(new EncoderState[num_channels_]),
      interleave_buffer_(2 * num_channels_)
{
  RTC_CHECK(config.IsOk());
  const size_t samples_per_channel =
      kSampleRateHz / 100 * num_10ms_frames_per_packet_;
  for (size_t i = 0; i < num_channels_; ++i) {
    encoders_[i].speech_buffer.reset(new int16_t[samples_per_channel]);
	encoders_[i].input_buffer_.clear();
    encoders_[i].encoded_buffer.SetSize(768);	// samples_per_channel / 2
  }
  Reset();
}

AudioEncoderAACImpl::~AudioEncoderAACImpl() = default;

int AudioEncoderAACImpl::SampleRateHz() const {
  return kSampleRateHz;
}

size_t AudioEncoderAACImpl::NumChannels() const {
  return num_channels_;
}

int AudioEncoderAACImpl::RtpTimestampRateHz() const {
  // The RTP timestamp rate for G.722 is 8000 Hz, even though it is a 16 kHz
  // codec.
  return kSampleRateHz;	// kSampleRateHz / 2;
}

size_t AudioEncoderAACImpl::Num10MsFramesInNextPacket() const {
  return num_10ms_frames_per_packet_;
}

size_t AudioEncoderAACImpl::Max10MsFramesInAPacket() const {
  return num_10ms_frames_per_packet_;
}

int AudioEncoderAACImpl::GetTargetBitrate() const {
  // 4 bits/sample, 16000 samples/s/channel.
  return static_cast<int>(28000 * NumChannels());
}

void AudioEncoderAACImpl::Reset() {
  num_10ms_frames_buffered_ = 0;
  for (size_t i = 0; i < num_channels_; ++i) {
	  RTC_LOG(LS_INFO) << "### TALK Reset !!!";
	  RTC_CHECK_EQ(0, WebRtcAAC_EncoderInit(encoders_[i].encoder));
  }
}

absl::optional<std::pair<TimeDelta, TimeDelta>>
AudioEncoderAACImpl::GetFrameLengthRange() const {
  return {{TimeDelta::Millis(num_10ms_frames_per_packet_ * 10),
           TimeDelta::Millis(num_10ms_frames_per_packet_ * 10)}};
}

AudioEncoder::EncodedInfo AudioEncoderAACImpl::EncodeImpl(
    uint32_t rtp_timestamp,
    rtc::ArrayView<const int16_t> audio,
    rtc::Buffer* encoded) {
  if (num_10ms_frames_buffered_ == 0)
    first_timestamp_in_buffer_ = rtp_timestamp;

  RTC_LOG(LS_INFO) << "### TALK kSampleRateHz: "<<kSampleRateHz<<",num_10ms_frames_buffered_:"<<num_10ms_frames_buffered_<<",num_channels_:"<<num_channels_<<",audio.size():"<<audio.size();

  // Deinterleave samples and save them in each channel's buffer.
  const size_t start = kSampleRateHz / 100 * num_10ms_frames_buffered_;
  for (size_t i = 0; i < kSampleRateHz / 100; ++i)
    for (size_t j = 0; j < num_channels_; ++j)
      encoders_[j].speech_buffer[start + i] = audio[i * num_channels_ + j];

  // If we don't yet have enough samples for a packet, we're done for now.
  if (++num_10ms_frames_buffered_ < num_10ms_frames_per_packet_) {
    return EncodedInfo();
  }

  // Encode each channel separately.
  RTC_CHECK_EQ(num_10ms_frames_buffered_, num_10ms_frames_per_packet_);
  num_10ms_frames_buffered_ = 0;
  const size_t samples_per_channel = SamplesPerChannel();
  size_t bytes_encoded = 0;
  for (size_t i = 0; i < num_channels_; ++i) {
	  bytes_encoded = WebRtcAAC_Encode(
        encoders_[i].encoder, encoders_[i].speech_buffer.get(),
        samples_per_channel, encoders_[i].encoded_buffer.data());
    //RTC_CHECK_EQ(bytes_encoded, samples_per_channel / 2);
	if (bytes_encoded <= 0) {
		return EncodedInfo();
	}
  }

  //const size_t bytes_to_encode = samples_per_channel / 2 * num_channels_;
  const size_t bytes_to_encode = bytes_encoded;
  EncodedInfo info;
  info.encoded_bytes = encoded->AppendData(
      bytes_to_encode, [&](rtc::ArrayView<uint8_t> encoded) {
        // Interleave the encoded bytes of the different channels. Each separate
        // channel and the interleaved stream encodes two samples per byte, most
        // significant half first.
#if 0
        for (size_t i = 0; i < samples_per_channel / 2; ++i) {
          for (size_t j = 0; j < num_channels_; ++j) {
            uint8_t two_samples = encoders_[j].encoded_buffer.data()[i];
            interleave_buffer_.data()[j] = two_samples >> 4;
            interleave_buffer_.data()[num_channels_ + j] = two_samples & 0xf;
          }
          for (size_t j = 0; j < num_channels_; ++j)
            encoded[i * num_channels_ + j] =
                interleave_buffer_.data()[2 * j] << 4 |
                interleave_buffer_.data()[2 * j + 1];
        }
#else
		for (size_t i = 0; i < bytes_encoded; ++i) {
          for (size_t j = 0; j < num_channels_; ++j) {
            uint8_t two_samples = encoders_[j].encoded_buffer.data()[i];
			encoded[i * num_channels_ + j] = two_samples;
          }
		}
#endif

        return bytes_to_encode;
      });
  info.encoded_timestamp = first_timestamp_in_buffer_;
  info.payload_type = payload_type_;
  info.encoder_type = CodecType::kAac;
  return info;
}

AudioEncoderAACImpl::EncoderState::EncoderState() {
	encoder = NULL;
  RTC_CHECK_EQ(0, WebRtcAAC_CreateEncoder(&encoder));
}

AudioEncoderAACImpl::EncoderState::~EncoderState() {
  RTC_CHECK_EQ(0, WebRtcAAC_FreeEncoder(encoder));
}

size_t AudioEncoderAACImpl::SamplesPerChannel() const {
  return kSampleRateHz / 100 * num_10ms_frames_per_packet_;
}

}  // namespace webrtc
