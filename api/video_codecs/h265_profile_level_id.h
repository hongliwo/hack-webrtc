/*
 *  Copyright (c) 2024 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef API_VIDEO_CODECS_H265_PROFILE_LEVEL_ID_H_
#define API_VIDEO_CODECS_H265_PROFILE_LEVEL_ID_H_

#include <string>

#include "absl/types/optional.h"
#include "api/video_codecs/sdp_video_format.h"
#include "rtc_base/system/rtc_export.h"

namespace webrtc {

enum class H265Profile {
	kProfileMain,
	kProfileMain10,
	kProfileMainStillPicture,
};

// All values are equal to ten times the level number, except level 1b which is
// special.
enum class H265Level {
	kLevel1,
	kLevel2,
	kLevel2_1,
	kLevel3,
	kLevel3_1,
	kLevel4,
	kLevel4_1,
	kLevel5,
	kLevel5_1,
	kLevel5_2,
	kLevel6,
	kLevel6_1,
	kLevel6_2
};

struct H265ProfileLevelId {
  constexpr H265ProfileLevelId(H265Profile profile, H265Level level)
      : profile(profile), level(level) {}
  H265Profile profile;
  H265Level level;
};

// Parse profile level id that is represented as a string of 3 hex bytes.
// Nothing will be returned if the string is not a recognized H265
// profile level id.
absl::optional<H265ProfileLevelId> ParseH265ProfileLevelId(const char* str);

// Parse profile level id that is represented as a string of 3 hex bytes
// contained in an SDP key-value map. A default profile level id will be
// returned if the profile-level-id key is missing. Nothing will be returned if
// the key is present but the string is invalid.
RTC_EXPORT absl::optional<H265ProfileLevelId> ParseSdpForH265ProfileLevelId(
    const SdpVideoFormat::Parameters& params);

// Given that a decoder supports up to a given frame size (in pixels) at up to a
// given number of frames per second, return the highest H.264 level where it
// can guarantee that it will be able to support all valid encoded streams that
// are within that level.
RTC_EXPORT absl::optional<H265Level> H265SupportedLevel(
    int max_frame_pixel_count,
    float max_fps);

// Returns canonical string representation as three hex bytes of the profile
// level id, or returns nothing for invalid profile level ids.
RTC_EXPORT absl::optional<std::string> H265ProfileLevelIdToString(
    const H265ProfileLevelId& profile_level_id);

// Returns true if the parameters have the same H265 profile (Baseline, High,
// etc).
RTC_EXPORT bool H265IsSameProfile(const SdpVideoFormat::Parameters& params1,
                                  const SdpVideoFormat::Parameters& params2);

}  // namespace webrtc

#endif  // API_VIDEO_CODECS_H265_PROFILE_LEVEL_ID_H_
