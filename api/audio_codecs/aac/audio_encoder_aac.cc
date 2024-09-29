/*
 *  Copyright (c) 2017 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "api/audio_codecs/aac/audio_encoder_aac.h"

#include <memory>
#include <vector>

#include "absl/strings/match.h"
#include "modules/audio_coding/codecs/aac/audio_encoder_aac.h"
#include "rtc_base/numerics/safe_conversions.h"
#include "rtc_base/numerics/safe_minmax.h"
#include "rtc_base/string_to_number.h"

namespace webrtc {

absl::optional<AudioEncoderAACConfig> AudioEncoderAAC::SdpToConfig(
    const SdpAudioFormat& format) {
  if (!absl::EqualsIgnoreCase(format.name, "aac") ||
      format.clockrate_hz != 16000) {
    return absl::nullopt;
  }

  AudioEncoderAACConfig config;
  config.num_channels = rtc::checked_cast<int>(format.num_channels);
  auto ptime_iter = format.parameters.find("ptime");
  if (ptime_iter != format.parameters.end()) {
    auto ptime = rtc::StringToNumber<int>(ptime_iter->second);
    if (ptime && *ptime > 0) {
      const int whole_packets = *ptime / 10;
      config.frame_size_ms = rtc::SafeClamp<int>(whole_packets * 10, 10, 60);
    }
  }
  if (!config.IsOk()) {
    RTC_DCHECK_NOTREACHED();
    return absl::nullopt;
  }
  return config;
}

void AudioEncoderAAC::AppendSupportedEncoders(
    std::vector<AudioCodecSpec>* specs) {
	RTC_LOG(LS_INFO) << "### AppendSupportedEncoders";
  const SdpAudioFormat fmt = {"AAC", 16000, 1};
  const AudioCodecInfo info = QueryAudioEncoder(*SdpToConfig(fmt));
  specs->push_back({fmt, info});
}

AudioCodecInfo AudioEncoderAAC::QueryAudioEncoder(
    const AudioEncoderAACConfig& config) {
  RTC_DCHECK(config.IsOk());
  return {16000, rtc::dchecked_cast<size_t>(config.num_channels),
          64000 * config.num_channels};
}

std::unique_ptr<AudioEncoder> AudioEncoderAAC::MakeAudioEncoder(
    const AudioEncoderAACConfig& config,
    int payload_type,
    absl::optional<AudioCodecPairId> /*codec_pair_id*/,
    const FieldTrialsView* field_trials) {
  if (!config.IsOk()) {
    RTC_DCHECK_NOTREACHED();
    return nullptr;
  }
  return std::make_unique<AudioEncoderAACImpl>(config, payload_type);
}

}  // namespace webrtc
