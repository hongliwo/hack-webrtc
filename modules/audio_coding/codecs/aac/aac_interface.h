/*
 *  Copyright (c) 2011 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef MODULES_AUDIO_CODING_CODECS_AAC_AAC_INTERFACE_H_
#define MODULES_AUDIO_CODING_CODECS_AAC_AAC_INTERFACE_H_

#include <stddef.h>
#include <stdint.h>
#include "modules/audio_coding/codecs/aac/aac_inst.h"

/*
 * Solution to support multiple instances
 */

typedef struct WebRtcAacEncInst AACEncInst;
typedef struct WebRtcAacDecInst AACDecInst;

/*
 * Comfort noise constants
 */

#define AAC_WEBRTC_SPEECH 1
#define AAC_WEBRTC_CNG 2

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************************
 * WebRtcAAC_CreateEncoder(...)
 *
 * Create memory used for AAC encoder
 *
 * Input:
 *     - AACenc_inst         : AAC instance for encoder
 *
 * Return value               :  0 - Ok
 *                              -1 - Error
 */
int16_t WebRtcAAC_CreateEncoder(AACEncInst** AACenc_inst);

/****************************************************************************
 * WebRtcAAC_EncoderInit(...)
 *
 * This function initializes a AAC instance
 *
 * Input:
 *     - AACenc_inst         : AAC instance, i.e. the user that should receive
 *                             be initialized
 *
 * Return value               :  0 - Ok
 *                              -1 - Error
 */

int16_t WebRtcAAC_EncoderInit(AACEncInst* AACenc_inst);

/****************************************************************************
 * WebRtcAAC_FreeEncoder(...)
 *
 * Free the memory used for AAC encoder
 *
 * Input:
 *     - AACenc_inst         : AAC instance for encoder
 *
 * Return value               :  0 - Ok
 *                              -1 - Error
 */
int WebRtcAAC_FreeEncoder(AACEncInst* AACenc_inst);

/****************************************************************************
 * WebRtcAAC_Encode(...)
 *
 * This function encodes AAC encoded data.
 *
 * Input:
 *     - AACenc_inst         : AAC instance, i.e. the user that should encode
 *                              a packet
 *     - speechIn             : Input speech vector
 *     - len                  : Samples in speechIn
 *
 * Output:
 *        - encoded           : The encoded data vector
 *
 * Return value               : Length (in bytes) of coded data
 */

size_t WebRtcAAC_Encode(AACEncInst* AACenc_inst,
                         const int16_t* speechIn,
                         size_t len,
                         uint8_t* encoded);

/****************************************************************************
 * WebRtcAAC_CreateDecoder(...)
 *
 * Create memory used for AAC encoder
 *
 * Input:
 *     - AACdec_inst         : AAC instance for decoder
 *
 * Return value               :  0 - Ok
 *                              -1 - Error
 */
int16_t WebRtcAAC_CreateDecoder(AACDecInst** AACdec_inst);

/****************************************************************************
 * WebRtcAAC_DecoderInit(...)
 *
 * This function initializes a AAC instance
 *
 * Input:
 *     - inst      : AAC instance
 */

void WebRtcAAC_DecoderInit(AACDecInst* inst);

/****************************************************************************
 * WebRtcAAC_FreeDecoder(...)
 *
 * Free the memory used for AAC decoder
 *
 * Input:
 *     - AACdec_inst         : AAC instance for decoder
 *
 * Return value               :  0 - Ok
 *                              -1 - Error
 */

int WebRtcAAC_FreeDecoder(AACDecInst* AACdec_inst);

/****************************************************************************
 * WebRtcAAC_Decode(...)
 *
 * This function decodes a packet with G729 frame(s). Output speech length
 * will be a multiple of 80 samples (80*frames/packet).
 *
 * Input:
 *     - AACdec_inst       : AAC instance, i.e. the user that should decode
 *                            a packet
 *     - encoded            : Encoded AAC frame(s)
 *     - len                : Bytes in encoded vector
 *
 * Output:
 *        - decoded         : The decoded vector
 *      - speechType        : 1 normal, 2 CNG (Since AAC does not have its own
 *                            DTX/CNG scheme it should always return 1)
 *
 * Return value             : Samples in decoded vector
 */

size_t WebRtcAAC_Decode(AACDecInst* AACdec_inst,
                         const uint8_t* encoded,
                         size_t len,
                         int16_t* decoded,
                         int16_t* speechType);

/****************************************************************************
 * WebRtcAAC_Version(...)
 *
 * Get a string with the current version of the codec
 */

int16_t WebRtcAAC_Version(char* versionStr, short len);

#ifdef __cplusplus
}
#endif

#endif /* MODULES_AUDIO_CODING_CODECS_AAC_AAC_INTERFACE_H_ */
