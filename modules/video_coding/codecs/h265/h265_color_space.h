/*
 
  Copyright (c) 2023 The WebRTC project authors. All Rights Reserved.
  Use of this source code is governed by a BSD-style license
  that can be found in the LICENSE file in the root of the source
  tree. An additional intellectual property rights grant can be found
  in the file PATENTS. All contributing project authors may
  be found in the AUTHORS file in the root of the source tree.
  */
#ifndef MODULES_VIDEO_CODING_CODECS_H265_H265_COLOR_SPACE_H_
#define MODULES_VIDEO_CODING_CODECS_H265_H265_COLOR_SPACE_H_

#include "api/video/color_space.h"

namespace webrtc {

	struct H265VuiParameters {
		struct H265MasteringDisplayColourVolume {
			uint16_t display_primaries_x[3];
			uint16_t display_primaries_y[3];
			uint16_t white_point_x;
			uint16_t white_point_y;
			uint32_t max_display_mastering_luminance;
			uint32_t min_display_mastering_luminance;
		};

		struct H265ContentLightLevel {
			uint16_t max_content_light_level;
			uint16_t max_pic_average_light_level;
		};

		uint8_t colour_primaries;
		uint8_t transfer_characteristics;
		uint8_t matrix_coefficients;
		uint8_t video_full_range_flag;
		uint8_t chroma_sample_loc_type_top_field;
	};

	ColorSpace ExtractH265ColorSpace(
			const H265VuiParameters* vui_params,
			const H265VuiParameters::H265MasteringDisplayColourVolume* mdcv,
			const H265VuiParameters::H265ContentLightLevel* cll);

} // namespace webrtc

#endif // MODULES_VIDEO_CODING_CODECS_H265_H265_COLOR_SPACE_H_
