/*
 *  Copyright (c) 2014 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef MODULES_AUDIO_CODING_CODECS_AAC_AUDIO_ENCODER_AAC_H_
#define MODULES_AUDIO_CODING_CODECS_AAC_AUDIO_ENCODER_AAC_H_

#include <memory>
#include <utility>

#include "absl/types/optional.h"
#include "api/audio_codecs/audio_encoder.h"
#include "api/audio_codecs/aac/audio_encoder_aac_config.h"
#include "api/units/time_delta.h"
#include "modules/audio_coding/codecs/aac/aac_interface.h"
#include "rtc_base/buffer.h"
//#include "rtc_base/constructor_magic.h"

namespace webrtc {

class AudioEncoderAACImpl final : public AudioEncoder {
 public:
  AudioEncoderAACImpl(const AudioEncoderAACConfig& config, int payload_type);
 ~AudioEncoderAACImpl() override;
  
  AudioEncoderAACImpl(const AudioEncoderAACImpl&) = delete;
  AudioEncoderAACImpl& operator=(const AudioEncoderAACImpl&) = delete;


  int SampleRateHz() const override;
  size_t NumChannels() const override;
  int RtpTimestampRateHz() const override;
  size_t Num10MsFramesInNextPacket() const override;
  size_t Max10MsFramesInAPacket() const override;
  int GetTargetBitrate() const override;
  void Reset() override;
  absl::optional<std::pair<TimeDelta, TimeDelta>> GetFrameLengthRange()
      const override;

 protected:
  EncodedInfo EncodeImpl(uint32_t rtp_timestamp,
                         rtc::ArrayView<const int16_t> audio,
                         rtc::Buffer* encoded) override;

 private:
  // The encoder state for one channel.
  struct EncoderState {
    AACEncInst* encoder;
    std::unique_ptr<int16_t[]> speech_buffer;  // Queued up for encoding.
	std::vector<int16_t> input_buffer_;
    rtc::Buffer encoded_buffer;                // Already encoded.
    EncoderState();
    ~EncoderState();
  };

  size_t SamplesPerChannel() const;

  const size_t num_channels_;
  const int payload_type_;
  const size_t num_10ms_frames_per_packet_;
  size_t num_10ms_frames_buffered_;
  uint32_t first_timestamp_in_buffer_;
  const std::unique_ptr<EncoderState[]> encoders_;
  rtc::Buffer interleave_buffer_;
};

}  // namespace webrtc
#endif  // MODULES_AUDIO_CODING_CODECS_AAC_AUDIO_ENCODER_AAC_H_
