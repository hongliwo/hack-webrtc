/*
 * Copyright (c) 2023 The WebRTC project authors. All Rights Reserved.
 *
 * Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 * tree. An additional intellectual property rights grant can be found
 * in the file PATENTS.  All contributing project authors may
 * be found in the AUTHORS file in the root of the source tree.
 */

#include "modules/video_coding/codecs/h265/h265_color_space.h"

#include "api/video/color_space.h"
#include "rtc_base/checks.h"

namespace webrtc {

ColorSpace ExtractH265ColorSpace(
		const H265VuiParameters* vui_params,
		const H265VuiParameters::H265MasteringDisplayColourVolume* mdcv,
		const H265VuiParameters::H265ContentLightLevel* cll) {
	if (vui_params == nullptr) {
		return ColorSpace();
	}

	ColorSpace::PrimaryID primaries = ColorSpace::PrimaryID::kUnspecified;
	ColorSpace::TransferID transfer = ColorSpace::TransferID::kUnspecified;
	ColorSpace::MatrixID matrix = ColorSpace::MatrixID::kUnspecified;
	switch (vui_params->colour_primaries) {
		case 1:
			primaries = ColorSpace::PrimaryID::kBT709;
			break;
		case 2:
			primaries = ColorSpace::PrimaryID::kUnspecified;
			break;
		case 4:
			primaries = ColorSpace::PrimaryID::kBT470M;
			break;
		case 5:
			primaries = ColorSpace::PrimaryID::kBT470BG;
			break;
		case 6:
			primaries = ColorSpace::PrimaryID::kSMPTE170M;
			break;
		case 7:
			primaries = ColorSpace::PrimaryID::kSMPTE240M;
			break;
		case 8:
			primaries = ColorSpace::PrimaryID::kFILM;
			break;
		case 9:
			primaries = ColorSpace::PrimaryID::kBT2020;
			break;
		case 10:
			primaries = ColorSpace::PrimaryID::kSMPTEST428;
			break;
		case 11:
			primaries = ColorSpace::PrimaryID::kSMPTEST431;
			break;
		case 12:
			primaries = ColorSpace::PrimaryID::kSMPTEST432;
			break;
		case 22:
			primaries = ColorSpace::PrimaryID::kJEDEC_P22;
			break;
	}

	switch (vui_params->transfer_characteristics) {
		case 1:
			transfer = ColorSpace::TransferID::kBT709;
			break;
		case 2:
			transfer = ColorSpace::TransferID::kUnspecified;
			break;
		case 4:
			transfer = ColorSpace::TransferID::kGAMMA22;
			break;
		case 5:
			transfer = ColorSpace::TransferID::kGAMMA28;
			break;
		case 6:
			transfer = ColorSpace::TransferID::kSMPTE170M;
			break;
		case 7:
			transfer = ColorSpace::TransferID::kSMPTE240M;
			break;
		case 8:
			transfer = ColorSpace::TransferID::kLINEAR;
			break;
		case 9:
			transfer = ColorSpace::TransferID::kLOG;
			break;
		case 10:
			transfer = ColorSpace::TransferID::kLOG_SQRT;
			break;
		case 11:
			transfer = ColorSpace::TransferID::kIEC61966_2_4;
			break;
		case 12:
			transfer = ColorSpace::TransferID::kBT1361_ECG;
			break;
		case 13:
			transfer = ColorSpace::TransferID::kIEC61966_2_1;
			break;
		case 14:
			transfer = ColorSpace::TransferID::kBT2020_10;
			break;
		case 15:
			transfer = ColorSpace::TransferID::kBT2020_12;
			break;
		case 16:
			transfer = ColorSpace::TransferID::kSMPTEST2084;
			break;
		case 17:
			transfer = ColorSpace::TransferID::kSMPTEST428;
			break;
		case 18:
			transfer = ColorSpace::TransferID::kARIB_STD_B67;
			break;
	}

	switch (vui_params->matrix_coefficients) {
		case 0:
			matrix = ColorSpace::MatrixID::kRGB;
			break;
		case 1:
			matrix = ColorSpace::MatrixID::kBT709;
			break;
		case 2:
			matrix = ColorSpace::MatrixID::kUnspecified;
			break;
		case 4:
			matrix = ColorSpace::MatrixID::kFCC;
			break;
		case 5:
			matrix = ColorSpace::MatrixID::kBT470BG;
			break;
		case 6:
			matrix = ColorSpace::MatrixID::kSMPTE170M;
			break;
		case 7:
			matrix = ColorSpace::MatrixID::kSMPTE240M;
			break;
		case 8:
			matrix = ColorSpace::MatrixID::kYCOCG;
			break;
		case 9:
			matrix = ColorSpace::MatrixID::kBT2020_NCL;
			break;
		case 10:
			matrix = ColorSpace::MatrixID::kBT2020_CL;
			break;
		case 11:
			matrix = ColorSpace::MatrixID::kSMPTE2085;
			break;
		case 12:
			matrix = ColorSpace::MatrixID::kCHROMATICITY_DERIVED_NCL;
			break;
		case 13:
			matrix = ColorSpace::MatrixID::kCHROMATICITY_DERIVED_CL;
			break;
		case 14:
			matrix = ColorSpace::MatrixID::kICTCP;
			break;
	}

	ColorSpace::RangeID range = vui_params->video_full_range_flag
		? ColorSpace::RangeID::kFull
		: ColorSpace::RangeID::kLimited;

	ColorSpace::ChromaSiting chroma_siting_horz =
		ColorSpace::ChromaSiting::kUnspecified;
	ColorSpace::ChromaSiting chroma_siting_vert =
		ColorSpace::ChromaSiting::kUnspecified;
	switch (vui_params->chroma_sample_loc_type_top_field) {
		case 0:
			chroma_siting_horz = ColorSpace::ChromaSiting::kCollocated;
			chroma_siting_vert = ColorSpace::ChromaSiting::kCollocated;
			break;
		case 1:
			chroma_siting_horz = ColorSpace::ChromaSiting::kHalf;
			chroma_siting_vert = ColorSpace::ChromaSiting::kCollocated;
			break;
		case 2:
			chroma_siting_horz = ColorSpace::ChromaSiting::kCollocated;
			chroma_siting_vert = ColorSpace::ChromaSiting::kHalf;
			break;
		case 3:
			chroma_siting_horz = ColorSpace::ChromaSiting::kHalf;
			chroma_siting_vert = ColorSpace::ChromaSiting::kHalf;
			break;
		case 4:
			chroma_siting_horz = ColorSpace::ChromaSiting::kCollocated;
			chroma_siting_vert = ColorSpace::ChromaSiting::kCollocated;
			break;
		case 5:
			chroma_siting_horz = ColorSpace::ChromaSiting::kCollocated;
			chroma_siting_vert = ColorSpace::ChromaSiting::kHalf;
			break;
	}

	HdrMetadata hdr_metadata;
	if (mdcv != nullptr) {
		hdr_metadata.mastering_metadata.primary_r.x = mdcv->display_primaries_x[0];
		hdr_metadata.mastering_metadata.primary_r.y = mdcv->display_primaries_y[0];
		hdr_metadata.mastering_metadata.primary_g.x = mdcv->display_primaries_x[1];
		hdr_metadata.mastering_metadata.primary_g.y = mdcv->display_primaries_y[1];
		hdr_metadata.mastering_metadata.primary_b.x = mdcv->display_primaries_x[2];
		hdr_metadata.mastering_metadata.primary_b.y = mdcv->display_primaries_y[2];
		hdr_metadata.mastering_metadata.white_point.x = mdcv->white_point_x;
		hdr_metadata.mastering_metadata.white_point.y = mdcv->white_point_y;
		hdr_metadata.mastering_metadata.luminance_max =
			mdcv->max_display_mastering_luminance;
		hdr_metadata.mastering_metadata.luminance_min =
			mdcv->min_display_mastering_luminance;
	}
	if (cll != nullptr) {
		hdr_metadata.max_cll = cll->max_content_light_level;
		hdr_metadata.max_fall = cll->max_pic_average_light_level;
	}

	return ColorSpace(primaries, transfer, matrix, range, chroma_siting_horz,
			chroma_siting_vert, std::move(hdr_metadata));
}

}  // namespace webrtc

