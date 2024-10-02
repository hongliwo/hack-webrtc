/*
 *  Copyright (c) 2023 The WebRTC project authors. All Rights Reserved.
 * 
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 * 
 */

#include "modules/video_coding/codecs/h265/include/h265.h"

#include <memory>
#include <string>

#include "absl/container/inlined_vector.h"
#include "absl/types/optional.h"
#include "api/video_codecs/sdp_video_format.h"
#include "media/base/media_constants.h"
#include "rtc_base/trace_event.h"

#if defined(WEBRTC_USE_H265)
#include "modules/video_coding/codecs/h265/h265_decoder_impl.h"
#include "modules/video_coding/codecs/h265/h265_encoder_impl.h"
#endif

#include "rtc_base/checks.h"
#include "rtc_base/logging.h"

namespace webrtc {

namespace {

#if defined(WEBRTC_USE_H265)
	bool g_rtc_use_h265 = true;
#endif

	// If H.265 codec is supported.
	bool IsH265CodecSupported() {
#if defined(WEBRTC_USE_H265)
		return g_rtc_use_h265;
#else
		return false;
#endif
	}

	constexpr ScalabilityMode kSupportedScalabilityModes[] = {
		ScalabilityMode::kL1T1, ScalabilityMode::kL1T2, ScalabilityMode::kL1T3};

}  // namespace

SdpVideoFormat CreateH265Format(H265Profile profile,
		H265Level level,
		const std::string& packetization_mode,
		bool add_scalability_modes) {
	const absl::optional<std::string> profile_string =
		H265ProfileLevelIdToString(H265ProfileLevelId(profile, level));
	RTC_CHECK(profile_string);
	absl::InlinedVector<ScalabilityMode, kScalabilityModeCount> scalability_modes;
	if (add_scalability_modes) {
		for (const auto scalability_mode : kSupportedScalabilityModes) {
			scalability_modes.push_back(scalability_mode);
		}
	}
	return SdpVideoFormat(
			cricket::kH265CodecName,
			{{cricket::kH265FmtpProfileSpace, "0"},
			{cricket::kH265FmtpProfileId, std::to_string(static_cast<int>(profile))},
			{cricket::kH265FmtpLevelId, std::to_string(static_cast<int>(level))},
			{cricket::kH265FmtpTierFlag, "0"},
			{cricket::kH265FmtpPacketizationMode, packetization_mode}},
			scalability_modes);
}

void DisableRtcUseH265() {
#if defined(WEBRTC_USE_H265)
	g_rtc_use_h265 = false;
#endif
}

std::vector<SdpVideoFormat> SupportedH265Codecs(bool add_scalability_modes) {
	TRACE_EVENT0("webrtc", __func__);
	if (!IsH265CodecSupported())
		return std::vector<SdpVideoFormat>();

	return {CreateH265Format(H265Profile::kProfileMain, H265Level::kLevel3_1,
			"1", add_scalability_modes),
		   CreateH265Format(H265Profile::kProfileMain, H265Level::kLevel3_1,
				   "0", add_scalability_modes),
		   CreateH265Format(H265Profile::kProfileMain10, H265Level::kLevel3_1,
				   "1", add_scalability_modes),
		   CreateH265Format(H265Profile::kProfileMain10, H265Level::kLevel3_1,
				   "0", add_scalability_modes)};
}

std::vector<SdpVideoFormat> SupportedH265DecoderCodecs() {
	TRACE_EVENT0("webrtc", __func__);
	if (!IsH265CodecSupported())
		return std::vector<SdpVideoFormat>();

	return SupportedH265Codecs();
}

std::unique_ptr<H265Encoder> H265Encoder::Create() {
#if defined(WEBRTC_USE_H265)
	RTC_LOG(LS_INFO) << "Creating H265EncoderImpl.";
	return std::make_unique<H265EncoderImpl>(cricket::VideoCodec("H265"));
#else
	RTC_NOTREACHED();
	return nullptr;
#endif
}

std::unique_ptr<H265Encoder> H265Encoder::Create(
		const cricket::VideoCodec& codec) {
	RTC_DCHECK(H265Encoder::IsSupported());
#if defined(WEBRTC_USE_H265)
	RTC_CHECK(g_rtc_use_h265);
	RTC_LOG(LS_INFO) << "Creating H265EncoderImpl.";
	return std::make_unique<H265EncoderImpl>(codec);
#else
	RTC_DCHECK_NOTREACHED();
	return nullptr;
#endif
}

bool H265Encoder::IsSupported() {
	return IsH265CodecSupported();
}

bool H265Encoder::SupportsScalabilityMode(ScalabilityMode scalability_mode) {
	for (const auto& entry : kSupportedScalabilityModes) {
		if (entry == scalability_mode) {
			return true;
		}
	}
	return false;
}

std::unique_ptr<H265Decoder> H265Decoder::Create() {
	RTC_DCHECK(H265Decoder::IsSupported());
#if defined(WEBRTC_USE_H265)
	RTC_CHECK(g_rtc_use_h265);
	RTC_LOG(LS_INFO) << "Creating H265DecoderImpl.";
	return std::make_unique<H265DecoderImpl>();
#else
	RTC_DCHECK_NOTREACHED();
	return nullptr;
#endif
}

bool H265Decoder::IsSupported() {
	return IsH265CodecSupported();
}

}  // namespace webrtc

