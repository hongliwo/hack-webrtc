/*
 *  Copyright (c) 2016 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "modules/audio_coding/codecs/legacy_encoded_audio_frame.h"

#include <algorithm>
#include <memory>
#include <utility>

#include "rtc_base/checks.h"
#include "rtc_base/logging.h"

namespace webrtc {

LegacyEncodedAudioFrame::LegacyEncodedAudioFrame(AudioDecoder* decoder,
                                                 rtc::Buffer&& payload)
    : decoder_(decoder), payload_(std::move(payload)) {}

LegacyEncodedAudioFrame::~LegacyEncodedAudioFrame() = default;

size_t LegacyEncodedAudioFrame::Duration() const {
  const int ret = decoder_->PacketDuration(payload_.data(), payload_.size());
  return (ret < 0) ? 0 : static_cast<size_t>(ret);
}

absl::optional<AudioDecoder::EncodedAudioFrame::DecodeResult>
LegacyEncodedAudioFrame::Decode(rtc::ArrayView<int16_t> decoded) const {
  AudioDecoder::SpeechType speech_type = AudioDecoder::kSpeech;
  RTC_LOG(LS_INFO) << "### LegacyEncodedAudioFrame::Decode befor";
  RTC_LOG(LS_INFO) << "### payload_.size():"<<payload_.size()<<",decoder_->SampleRateHz()"<<decoder_->SampleRateHz();
  const int ret = decoder_->Decode(
      payload_.data(), payload_.size(), decoder_->SampleRateHz(),
      decoded.size() * sizeof(int16_t), decoded.data(), &speech_type);

  if (ret < 0)
    return absl::nullopt;

  return DecodeResult{static_cast<size_t>(ret), speech_type};
}

std::vector<AudioDecoder::ParseResult> LegacyEncodedAudioFrame::SplitBySamples(
    AudioDecoder* decoder,
    rtc::Buffer&& payload,
    uint32_t timestamp,
    size_t bytes_per_ms,
    uint32_t timestamps_per_ms) {
	RTC_LOG(LS_INFO) << "### LegacyEncodedAudioFrame::SplitBySamples payload.size():"<<payload.size();
  RTC_DCHECK(payload.data());
  std::vector<AudioDecoder::ParseResult> results;
  size_t split_size_bytes = payload.size();

  // Find a "chunk size" >= 20 ms and < 40 ms.
  const size_t min_chunk_size = bytes_per_ms * 96;//64;	//20;//60;//20;
  
  if (min_chunk_size >= payload.size()) {
	RTC_LOG(LS_INFO) << "### 1 LegacyEncodedAudioFrame::SplitBySamples min_chunk_size:"<<min_chunk_size<<" >= payload.size():"<<payload.size();
    std::unique_ptr<LegacyEncodedAudioFrame> frame(
        new LegacyEncodedAudioFrame(decoder, std::move(payload)));
	//RTC_LOG(LS_INFO) << "### 2 LegacyEncodedAudioFrame::SplitBySamples min_chunk_size:"<<min_chunk_size<<" >= payload.size():"<<payload.size();
    results.emplace_back(timestamp, 0, std::move(frame));
	//RTC_LOG(LS_INFO) << "### 3 LegacyEncodedAudioFrame::SplitBySamples min_chunk_size:"<<min_chunk_size<<" >= payload.size():"<<payload.size();
  } else {
	RTC_LOG(LS_INFO) << "### LegacyEncodedAudioFrame::SplitBySamples min_chunk_size:"<<min_chunk_size<<" < payload.size():"<<payload.size();
    // Reduce the split size by half as long as `split_size_bytes` is at least
    // twice the minimum chunk size (so that the resulting size is at least as
    // large as the minimum chunk size).
	RTC_LOG(LS_INFO) << "### LegacyEncodedAudioFrame::SplitBySamples,line:" << __LINE__;
    
#if 1
    while (split_size_bytes >= 2 * min_chunk_size) {
	RTC_LOG(LS_INFO) << "### LegacyEncodedAudioFrame::SplitBySamples,line:" << __LINE__;
      split_size_bytes /= 2;
    }
#endif

    const uint32_t timestamps_per_chunk = static_cast<uint32_t>(
        split_size_bytes * timestamps_per_ms / bytes_per_ms);
	RTC_LOG(LS_INFO) << "### LegacyEncodedAudioFrame::SplitBySamples,bytes_per_ms:"<<bytes_per_ms<<",timestamps_per_ms:"<<timestamps_per_ms<<",split_size_bytes:"<<split_size_bytes<<",timestamps_per_chunk:"<<timestamps_per_chunk;
    size_t byte_offset;
    uint32_t timestamp_offset;
	RTC_LOG(LS_INFO) << "### LegacyEncodedAudioFrame::SplitBySamples,line:" << __LINE__;
    for (byte_offset = 0, timestamp_offset = 0; byte_offset < payload.size();
         byte_offset += split_size_bytes,
        timestamp_offset += timestamps_per_chunk) {
	RTC_LOG(LS_INFO) << "### LegacyEncodedAudioFrame::SplitBySamples,split_size_bytes:"<<split_size_bytes<<",payload.size():"<<payload.size()<<",byte_offset:"<<byte_offset;
      split_size_bytes = std::min(split_size_bytes, payload.size() - byte_offset);
	RTC_LOG(LS_INFO) << "### LegacyEncodedAudioFrame::SplitBySamples,split_size_bytes:"<<split_size_bytes;
      rtc::Buffer new_payload(payload.data() + byte_offset, split_size_bytes);
	RTC_LOG(LS_INFO) << "### LegacyEncodedAudioFrame::SplitBySamples,line:" << __LINE__;
	  //RTC_LOG(LS_INFO) << "### LegacyEncodedAudioFrame::SplitBySamples new_payload:"<<new_payload.size();
      std::unique_ptr<LegacyEncodedAudioFrame> frame(
          new LegacyEncodedAudioFrame(decoder, std::move(new_payload)));
	RTC_LOG(LS_INFO) << "### LegacyEncodedAudioFrame::SplitBySamples,line:" << __LINE__;
      results.emplace_back(timestamp + timestamp_offset, 0, std::move(frame));
	RTC_LOG(LS_INFO) << "### LegacyEncodedAudioFrame::SplitBySamples,line:" << __LINE__;
    }
  }

  return results;
}

}  // namespace webrtc
