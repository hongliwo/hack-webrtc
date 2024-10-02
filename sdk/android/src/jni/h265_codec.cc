/*
 *  Copyright 2017 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <jni.h>
#include "rtc_base/logging.h"

#include "modules/video_coding/codecs/h265/include/h265.h"
#include "sdk/android/generated_libh265_jni/LibH265Decoder_jni.h"
#include "sdk/android/generated_libh265_jni/LibH265Encoder_jni.h"
#include "sdk/android/src/jni/jni_helpers.h"

namespace webrtc {
namespace jni {

static jlong JNI_LibH265Encoder_CreateEncoder(JNIEnv* jni) {
  return jlongFromPointer(H265Encoder::Create().release());
}

static jlong JNI_LibH265Decoder_CreateDecoder(JNIEnv* jni) {
	RTC_LOG(LS_WARNING) << "### JNI_LibH265Decoder_CreateDecoder";
  return jlongFromPointer(H265Decoder::Create().release());
}

}  // namespace jni
}  // namespace webrtc
