/*
 *  Copyright (c) 2017 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "api/audio_codecs/aac/audio_decoder_aac.h"

#include <memory>
#include <vector>

#include "absl/strings/match.h"
#include "modules/audio_coding/codecs/aac/audio_decoder_aac.h"
#include "rtc_base/numerics/safe_conversions.h"
#include "rtc_base/logging.h"

namespace webrtc {

absl::optional<AudioDecoderAAC::Config> AudioDecoderAAC::SdpToConfig(
    const SdpAudioFormat& format) {
	RTC_LOG(LS_INFO) << "### SdpToConfig";
  return absl::EqualsIgnoreCase(format.name, "AAC") &&
                 format.clockrate_hz == 16000 &&
                 (format.num_channels == 1 || format.num_channels == 2)
             ? absl::optional<Config>(
                   Config{rtc::dchecked_cast<int>(format.num_channels)})
             : absl::nullopt;
}

void AudioDecoderAAC::AppendSupportedDecoders(
    std::vector<AudioCodecSpec>* specs) {

	RTC_LOG(LS_INFO) << "### AppendSupportedDecoders";
  specs->push_back({{"AAC", 16000, 1}, {16000, 1, 64000}});
}

std::unique_ptr<AudioDecoder> AudioDecoderAAC::MakeAudioDecoder(
    Config config,
    absl::optional<AudioCodecPairId> /*codec_pair_id*/) {
	
	RTC_LOG(LS_INFO) << "### MakeAudioDecoder, config.num_channels:" << config.num_channels;

  switch (config.num_channels) {
    case 1:
      return std::make_unique<AudioDecoderAACImpl>();
    case 2:
      return std::make_unique<AudioDecoderAACStereoImpl>();
    default:
      return nullptr;
  }
}

}  // namespace webrtc
