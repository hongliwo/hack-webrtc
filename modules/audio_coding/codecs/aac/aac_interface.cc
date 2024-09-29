/*
 *  Copyright (c) 2011 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <stdlib.h>
#include <string.h>

#include "modules/audio_coding/codecs/aac/aac_interface.h"
#include "modules/third_party/aac/aac_enc_dec.h"

#include "rtc_base/checks.h"
#include "rtc_base/logging.h"

#define AAC_TT_MP4_ADTS 0

int16_t WebRtcAAC_CreateEncoder(AACEncInst **AACenc_inst)
{
	RTC_LOG(LS_INFO) << "### TALK WebRtcAAC_CreateEncoder";

	*AACenc_inst = (AACEncInst *)malloc(sizeof(AACEncoderState));
	if (*AACenc_inst != NULL) {
		memset(*AACenc_inst, 0x00, sizeof(WebRtcAacEncInst));
		return(0);
	} else {
		return(-1);
	}
}

int16_t WebRtcAAC_EncoderInit(AACEncInst *AACenc_inst)
{
	RTC_LOG(LS_INFO) << "### TALK WebRtcAAC_EncoderInit";

	if (AACenc_inst == NULL) {
		RTC_LOG(LS_INFO) << "### TALK WebRtcAAC_EncoderInit 2";
		AACenc_inst = (AACEncInst *)malloc(sizeof(AACEncoderState));
		if (AACenc_inst != NULL) {
			RTC_LOG(LS_INFO) << "### TALK WebRtcAAC_EncoderInit 3";
			memset(AACenc_inst, 0x00, sizeof(WebRtcAacEncInst));
		} else {
			RTC_LOG(LS_INFO) << "### TALK WebRtcAAC_EncoderInit 4";
			return(-1);
		}
	}
	RTC_LOG(LS_INFO) << "### TALK WebRtcAAC_EncoderInit 5";

	if (AACenc_inst->encoder != NULL) {
		RTC_LOG(LS_INFO) << "### TALK WebRtcAAC_EncoderInit will close encoder";
		aacEncClose(&(AACenc_inst->encoder));
		RTC_LOG(LS_WARNING) << "### TALK WebRtcAAC_EncoderInit aacEncClose";
	}
	RTC_LOG(LS_INFO) << "### TALK WebRtcAAC_EncoderInit 6";

#if 1
	if (AACenc_inst->pcmBuffer != NULL) {
		free(AACenc_inst->pcmBuffer);
		AACenc_inst->pcmBuffer = NULL;
		RTC_LOG(LS_WARNING) << "### TALK WebRtcAAC_EncoderInit pcmBuffer free";
	}
#endif

	int ret = -1;
	int sample_rate = 16000;
	int bitrate = 16000;	// 28000;	// 28000; // 32000;
	int vbr = 0;
	int aot = AOT_AAC_LC;
	int afterburner = 1;
	int channels = 1;
	CHANNEL_MODE mode = MODE_1;

	//int input_size;

	if (aacEncOpen(&(AACenc_inst->encoder), 0, channels) != AACENC_OK) {
		RTC_LOG(LS_ERROR) << "aacEncOpen error, Unable to open encoder";
		goto CleanUp;
	}

	if (aacEncoder_SetParam(AACenc_inst->encoder, AACENC_AOT, aot) != AACENC_OK) {
		RTC_LOG(LS_ERROR) << "aacEncoder_SetParam error, Unable to set the AOT";
		goto CleanUp;
	}

	if (aacEncoder_SetParam(AACenc_inst->encoder, AACENC_SAMPLERATE, sample_rate) != AACENC_OK) {
		RTC_LOG(LS_ERROR) << "aacEncoder_SetParam error, Unable to set the sample rate";
		goto CleanUp;
	}

	if (aacEncoder_SetParam(AACenc_inst->encoder, AACENC_CHANNELMODE, mode) != AACENC_OK) {
		RTC_LOG(LS_ERROR) << "aacEncoder_SetParam error, Unable to set the channel mode";
		goto CleanUp;
	}

	if (aacEncoder_SetParam(AACenc_inst->encoder, AACENC_CHANNELORDER, 1) != AACENC_OK) {
		RTC_LOG(LS_ERROR) << "aacEncoder_SetParam error, Unable to set the wav channel order";
		goto CleanUp;
	}

	if (vbr != 0) {
		if (aacEncoder_SetParam(AACenc_inst->encoder, AACENC_BITRATEMODE, vbr) != AACENC_OK) {
			RTC_LOG(LS_ERROR) << "aacEncoder_SetParam error, Unable to set the VBR bitrate mode";
			goto CleanUp;
		}
	} else {
		if (aacEncoder_SetParam(AACenc_inst->encoder, AACENC_BITRATE, bitrate) != AACENC_OK) {
			RTC_LOG(LS_ERROR) << "aacEncoder_SetParam error, Unable to set the bitrate";
			goto CleanUp;
		}
	}

	// It is usually TT_MP4_ADTS and TT_MP4_RAW
	if (aacEncoder_SetParam(AACenc_inst->encoder, AACENC_TRANSMUX, TT_MP4_RAW) != AACENC_OK) {
		RTC_LOG(LS_ERROR) << "aacEncoder_SetParam error Unable to set the RAW/ADTS transmux";
		goto CleanUp;
	}

	if (aacEncoder_SetParam(AACenc_inst->encoder, AACENC_AFTERBURNER, afterburner) != AACENC_OK) {
		RTC_LOG(LS_ERROR) << "aacEncoder_SetParam error, Unable to set the afterburner mode";
		goto CleanUp;
	}

	if (aacEncEncode(AACenc_inst->encoder, NULL, NULL, NULL, NULL) != AACENC_OK) {
		RTC_LOG(LS_ERROR) << "aacEncEncode error, Unable to initialize the encoder";
		goto CleanUp;
	}

	if (aacEncInfo(AACenc_inst->encoder, &(AACenc_inst->info)) != AACENC_OK) {
		RTC_LOG(LS_ERROR) << "aacEncInfo error, Unable to get the encoder info";
		goto CleanUp;
	}

	ret = 0;

	RTC_LOG(LS_INFO) << "### TALK WebRtcAAC_EncoderInit, AACenc_inst->info.frameLength:" << AACenc_inst->info.frameLength;
#if 1
	AACenc_inst->pcmBuffer = (unsigned char *)malloc(AACenc_inst->info.frameLength * 10);
	if (AACenc_inst->pcmBuffer == NULL) {
		RTC_LOG(LS_ERROR) << "### TALK WebRtcAAC_EncoderInit malloc error";
		return -1; 
	}
	RTC_LOG(LS_INFO) << "### TALK WebRtcAAC_EncoderInit, AACenc_inst->pcmBuffer:" <<AACenc_inst->pcmBuffer;
#endif
	AACenc_inst->pcmBufferLen = 0;

CleanUp:
	if (ret != 0) {
		if (AACenc_inst->encoder != NULL) {
			aacEncClose(&(AACenc_inst->encoder));
			AACenc_inst->encoder = NULL;
			RTC_LOG(LS_ERROR) << "### TALK WebRtcAAC_EncoderInit close";
		}
	}
	
	RTC_LOG(LS_INFO) << "### TALK WebRtcAAC_EncoderInit finish";

	return ret;
}

int WebRtcAAC_FreeEncoder(AACEncInst *AACenc_inst)
{
	RTC_LOG(LS_INFO) << "### TALK WebRtcAAC_CreateEncoder";

	if (AACenc_inst != NULL) {
		if (AACenc_inst->encoder) {
			aacEncClose(&AACenc_inst->encoder);
			AACenc_inst->encoder = NULL;
		}

		if (AACenc_inst->pcmBuffer != NULL) {
			free(AACenc_inst->pcmBuffer);
			AACenc_inst->pcmBuffer = NULL;
		}

		free(AACenc_inst);
	}

	return 0;
}

size_t WebRtcAAC_Encode(AACEncInst *AACenc_inst, const int16_t* speechIn, size_t len, uint8_t* encoded)
{
	RTC_LOG(LS_INFO) << "### TALK WebRtcAAC_Encode";
	
	if (AACenc_inst == NULL || AACenc_inst->encoder == NULL || AACenc_inst->pcmBuffer == NULL
			|| speechIn == NULL || len == 0 || encoded == NULL) {
		RTC_LOG(LS_ERROR) << "### TALK WebRtcAAC_Encode param error";
		return -1;
	}

	//size_t ret = 0;
	int out_size = len;
	size_t out_bytes = 0;

#if 1
	RTC_LOG(LS_INFO) << "### TALK WebRtcAAC_Encode AACenc_inst->pcmBuffer:"<<AACenc_inst->pcmBuffer<<",AACenc_inst->pcmBufferLen:" << AACenc_inst->pcmBufferLen << ",len:"<<len;
	
#if 1
	if (!AACenc_inst->pcmBuffer) {
		RTC_LOG(LS_ERROR) << "### TALK WebRtcAAC_Encode AACenc_inst->pcmBuffer null";
	}
#endif

	RTC_LOG(LS_INFO) << "### TALK WebRtcAAC_Encode speechIn:" << speechIn[0] << "," << speechIn[1];

#if 0
	memcpy((unsigned char *)(AACenc_inst->pcmBuffer + AACenc_inst->pcmBufferLen), (unsigned char *)speechIn, len*2);
#else
	unsigned char *speechInPtr = (unsigned char *)speechIn;
	memcpy((unsigned char *)(AACenc_inst->pcmBuffer + AACenc_inst->pcmBufferLen), speechInPtr, len*2);
#endif
	AACenc_inst->pcmBufferLen += (len*2);  ///缓存解码后pcm数

	if (AACenc_inst->pcmBufferLen < AACenc_inst->info.frameLength * sizeof(INT_PCM)) {
		RTC_LOG(LS_INFO) << "### TALK need more, AACenc_inst->pcmBufferLen:" << AACenc_inst->pcmBufferLen;
		return 0;
	} else {
		RTC_LOG(LS_INFO) << "### TALK buff full, AACenc_inst->pcmBufferLen:" << AACenc_inst->pcmBufferLen;
	}

	unsigned char *pcmBuffer =  AACenc_inst->pcmBuffer;
#else
	//unsigned char *pcmBuffer = (unsigned char *)speechIn;
	const int16_t *pcmBuffer = speechIn;
#endif

#if 1
	AACENC_BufDesc in_buf = { 0 }, out_buf = { 0 };
	AACENC_InArgs in_args = { 0 };
	AACENC_OutArgs out_args = { 0 };
	int in_identifier = IN_AUDIO_DATA;
	int in_elem_size;
	int out_identifier = OUT_BITSTREAM_DATA;
	int out_elem_size;
	AACENC_ERROR err;
	void *in_ptr = pcmBuffer;
	//void *in_ptr = (void *)speechIn;//apcmBuffer;
	//int in_buffer_size = g_pcmBufferLen;
	int in_buffer_size = AACenc_inst->info.frameLength * sizeof(INT_PCM);

	// 2 bytes per sample point
	//in_args.numInSamples = g_pcmBufferLen / 2;
	in_args.numInSamples = AACenc_inst->info.frameLength * sizeof(INT_PCM) / 2;

	in_elem_size = 2;
	in_buf.numBufs = 1;
	in_buf.bufs = &in_ptr;
	in_buf.bufferIdentifiers = &in_identifier;
	in_buf.bufSizes = &in_buffer_size;
	in_buf.bufElSizes = &in_elem_size;

	out_elem_size = 1;
	out_buf.numBufs = 1;
	out_buf.bufs = (void**)&encoded;
	out_buf.bufferIdentifiers = &out_identifier;
	out_buf.bufSizes = &out_size;
	out_buf.bufElSizes = &out_elem_size;

	if ((err = aacEncEncode(AACenc_inst->encoder, &in_buf, &out_buf, &in_args, &out_args)) != AACENC_OK) {
		RTC_LOG(LS_ERROR) << "### TALK Encoding failed, err:"<<err;
		return 0;
	} else {
		RTC_LOG(LS_ERROR) << "### TALK Encoding success";
	}
	
	//RTC_LOG(LS_INFO) << "### TALK WebRtcAAC_Encode 3";

	if (AACenc_inst->pcmBufferLen >= sizeof(INT_PCM) * AACenc_inst->info.frameLength) {
		AACenc_inst->pcmBufferLen -= (sizeof(INT_PCM) * AACenc_inst->info.frameLength);

		RTC_LOG(LS_INFO) << "### TALK memmove, sizeof(INT_PCM) * AACenc_inst->info.frameLength:" << sizeof(INT_PCM) * AACenc_inst->info.frameLength <<",AACenc_inst->pcmBufferLen:" << AACenc_inst->pcmBufferLen;
		memmove(AACenc_inst->pcmBuffer,
				&AACenc_inst->pcmBuffer[sizeof(INT_PCM) * AACenc_inst->info.frameLength],
				AACenc_inst->pcmBufferLen);
	} else {
		AACenc_inst->pcmBufferLen = 0;
	}

	out_bytes = out_args.numOutBytes;
	
	//RTC_LOG(LS_INFO) << "### TALK WebRtcAAC_Encode 4";

#if 0
	AACENC_InfoStruct pInfo;
	err = aacEncInfo(AACenc_inst->encoder, &pInfo);
	if (err != AACENC_OK)
	{
		RTC_LOG(LS_ERROR) << "### TALK aacEncInfo error";
	}
	else
	{
		RTC_LOG(LS_INFO) << "### TALK maxOutBufBytes:"<< pInfo.maxOutBufBytes;
		RTC_LOG(LS_INFO) << "### TALK maxAncBytes:"<< pInfo.maxAncBytes;
		RTC_LOG(LS_INFO) << "### TALK inBufFillLevel:"<< pInfo.inBufFillLevel;
		RTC_LOG(LS_INFO) << "### TALK inputChannels:"<< pInfo.inputChannels;
		RTC_LOG(LS_INFO) << "### TALK frameLength:"<< pInfo.frameLength;
		RTC_LOG(LS_INFO) << "### TALK nDelay:"<< pInfo.nDelay;
		RTC_LOG(LS_INFO) << "### TALK nDelayCore:"<< pInfo.nDelayCore;
		RTC_LOG(LS_INFO) << "### TALK confBuf:"<< pInfo.confBuf;
		RTC_LOG(LS_INFO) << "### TALK confSize:"<< pInfo.confSize;
	}
#endif

#endif
	RTC_LOG(LS_INFO) << "### TALK WebRtcAAC_Encode finish, out_bytes:"<<out_bytes;

	return out_bytes;
}

int16_t WebRtcAAC_CreateDecoder(AACDecInst **AACdec_inst)
{
	RTC_LOG(LS_INFO) << "### AAC WebRtcAAC_CreateDecoder";

	*AACdec_inst = (AACDecInst *)malloc(sizeof(AACDecoderState));
	if (*AACdec_inst != NULL) {
		memset(*AACdec_inst, 0x00, sizeof(WebRtcAacDecInst));
		return(0);
	} else {
		return(-1);
	}
}

void WebRtcAAC_DecoderInit(AACDecInst* inst) {

	RTC_LOG(LS_INFO) << "### AAC WebRtcAAC_DecoderInit";
#if 1	
	if (!inst)
	{
		return;
	}

	if (inst->decoder != NULL) {
		RTC_LOG(LS_INFO) << "### AAC WebRtcAAC_DecoderInit, will close decoder";
		aacDecoder_Close(inst->decoder);
		inst->decoder = NULL;
	}

	if (1 == AAC_TT_MP4_ADTS) {
		inst->decoder = aacDecoder_Open(TT_MP4_ADTS, 1);
	} else {
		inst->decoder = aacDecoder_Open(TT_MP4_RAW, 1);
	}

	if (0 == AAC_TT_MP4_ADTS) {
		UCHAR lc_conf[] = { 0x14, 0x08};	// 16k 1c
		UCHAR *conf[] = { lc_conf };
		static UINT conf_len = sizeof(lc_conf);
		AAC_DECODER_ERROR err = aacDecoder_ConfigRaw(inst->decoder, conf, &conf_len);
		if(err > 0) {
			RTC_LOG(LS_INFO) << "### AAC aacDecoder_ConfigRaw error";
		}
	}

	int conceal_method = 2;//2;//0 muting 1 noise 2 interpolation
	aacDecoder_SetParam(inst->decoder, AAC_CONCEAL_METHOD, conceal_method);
	aacDecoder_SetParam(inst->decoder, AAC_PCM_MAX_OUTPUT_CHANNELS, 1); //MONO:1
	aacDecoder_SetParam(inst->decoder, AAC_PCM_MIN_OUTPUT_CHANNELS, 1); //MONO:1
#else
#endif
}

int WebRtcAAC_FreeDecoder(AACDecInst *AACdec_inst)
{
	RTC_LOG(LS_INFO) << "### AAC WebRtcAAC_FreeDecoder";
#if 1
	if (AACdec_inst) {
		if (AACdec_inst->decoder)
		{
			aacDecoder_Close(AACdec_inst->decoder);
			AACdec_inst->decoder = NULL;
		}
		free(AACdec_inst);
	}
#endif
	return 0;
}

size_t WebRtcAAC_Decode(AACDecInst *AACdec_inst,
                         const uint8_t *encoded,
                         size_t len,
                         int16_t *decoded,
                         int16_t *speechType)
{
	RTC_LOG(LS_INFO) << "### AAC WebRtcAAC_Decode";

	if (!AACdec_inst || AACdec_inst->decoder==NULL || !encoded || !decoded || len <= 0)
	{
		return 0;
	}

#if 1
	unsigned int valid = len;
	unsigned int encoded_len = len;
	uint8_t *pencoded = (uint8_t *)encoded;

	AAC_DECODER_ERROR err;
	err = aacDecoder_Fill(AACdec_inst->decoder, &pencoded, &encoded_len, &valid);
	if (err != AAC_DEC_OK)
	{   
		RTC_LOG(LS_INFO) << "### AAC aacDecoder_Fill failed";
		return -1;
	}
	else
	{
		RTC_LOG(LS_INFO) << "### AAC aacDecoder_Fill, encoded_len:" <<  encoded_len << ", valid:" << valid;
	}

	err = aacDecoder_DecodeFrame(AACdec_inst->decoder, (INT_PCM *)decoded, 2048 * sizeof(INT_PCM)  / sizeof(INT_PCM), 0);
	if (err != AAC_DEC_OK)
	{   
		if (err == AAC_DEC_NOT_ENOUGH_BITS)
		{
			RTC_LOG(LS_INFO) << "### AAC aacDecoder_DecodeFrame AAC_DEC_NOT_ENOUGH_BITS";
			return 0;
		}
		else
		{
			RTC_LOG(LS_INFO) << "### AAC aacDecoder_DecodeFrame failed";
			return -1;
		}
	}
	RTC_LOG(LS_INFO) << "### AAC aacDecoder_DecodeFrame success";

	CStreamInfo* info = aacDecoder_GetStreamInfo(AACdec_inst->decoder);
	RTC_LOG(LS_INFO) << "### AAC aacDecoder_GetStreamInfo,frameSize:" << info->frameSize;
	if (!info)
	{   
		return -1;
	}

	//return info->frameSize * sizeof(INT_PCM);
	return info->frameSize;
#else
	return 0;
#endif
}

int16_t WebRtcAAC_Version(char *versionStr, short len)
{
    // Get version string
    char version[30] = "2.0.2\n";
    if (strlen(version) < (unsigned int)len)
    {
        strcpy(versionStr, version);
        return 0;
    }
    else
    {
        return -1;
    }
}
