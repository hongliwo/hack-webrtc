/*
 *  Copyright (c) 2017 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef API_AUDIO_CODECS_AAC_AUDIO_ENCODER_AAC_H_
#define API_AUDIO_CODECS_AAC_AUDIO_ENCODER_AAC_H_

#include <memory>
#include <vector>

#include "absl/types/optional.h"
#include "api/audio_codecs/audio_codec_pair_id.h"
#include "api/audio_codecs/audio_encoder.h"
#include "api/audio_codecs/audio_format.h"
#include "api/audio_codecs/aac/audio_encoder_aac_config.h"
#include "api/field_trials_view.h"
#include "rtc_base/system/rtc_export.h"
#include "rtc_base/logging.h"

namespace webrtc {

// AAC encoder API for use as a template parameter to
// CreateAudioEncoderFactory<...>().
struct RTC_EXPORT AudioEncoderAAC {
  using Config = AudioEncoderAACConfig;
  static absl::optional<AudioEncoderAACConfig> SdpToConfig(
      const SdpAudioFormat& audio_format);
  static void AppendSupportedEncoders(std::vector<AudioCodecSpec>* specs);
  static AudioCodecInfo QueryAudioEncoder(const AudioEncoderAACConfig& config);
  static std::unique_ptr<AudioEncoder> MakeAudioEncoder(
      const AudioEncoderAACConfig& config,
      int payload_type,
      absl::optional<AudioCodecPairId> codec_pair_id = absl::nullopt,
      const FieldTrialsView* field_trials = nullptr);
};

}  // namespace webrtc

#endif  // API_AUDIO_CODECS_AAC_AUDIO_ENCODER_AAC_H_
