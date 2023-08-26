/*
 * Copyright (c) 2012-2014,2016 The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *     * Neither the name of The Linux Foundation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _QDMETADATA_H
#define _QDMETADATA_H

#ifdef USE_COLOR_METADATA
#include <color_metadata.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_IGC_LUT_ENTRIES 256

enum ColorSpace_t{
    ITU_R_601,
    ITU_R_601_FR,
    ITU_R_709,
};

struct HSICData_t {
    int32_t hue;
    float   saturation;
    int32_t intensity;
    float   contrast;
};

struct Sharp2Data_t {
    int32_t strength;
    uint32_t edge_thr;
    uint32_t smooth_thr;
    uint32_t noise_thr;
};

struct IGCData_t{
    uint16_t c0[MAX_IGC_LUT_ENTRIES];
    uint16_t c1[MAX_IGC_LUT_ENTRIES];
    uint16_t c2[MAX_IGC_LUT_ENTRIES];
};

struct BufferDim_t {
    int32_t sliceWidth;
    int32_t sliceHeight;
};

struct MetaData_t {
    int32_t operation;
    int32_t interlaced;
    struct BufferDim_t bufferDim;
    struct HSICData_t hsicData;
    int32_t sharpness;
    int32_t video_interface;
    struct IGCData_t igcData;
    struct Sharp2Data_t Sharp2Data;
    int64_t timestamp;
    uint32_t refreshrate;
    enum ColorSpace_t colorSpace;
     /* Gralloc sets PRIV_SECURE_BUFFER flag to inform that the buffers are from
      * ION_SECURE. which should not be mapped. However, for GPU post proc
      * feature, GFX needs to map this buffer, in the client context and in SF
      * context, it should not. Hence to differentiate, add this metadata field
      * for clients to set, and GPU will to read and know when to map the
      * SECURE_BUFFER(ION) */
    int32_t mapSecureBuffer;
#ifdef USE_COLOR_METADATA
   /* Color Aspects + HDR info */
   ColorMetaData color;
#endif
};

enum DispParamType {
    PP_PARAM_HSIC       = 0x0001,
    PP_PARAM_SHARPNESS  = 0x0002,
    PP_PARAM_INTERLACED = 0x0004,
    PP_PARAM_VID_INTFC  = 0x0008,
    PP_PARAM_IGC        = 0x0010,
    PP_PARAM_SHARP2     = 0x0020,
    PP_PARAM_TIMESTAMP  = 0x0040,
    UPDATE_BUFFER_GEOMETRY = 0x0080,
    UPDATE_REFRESH_RATE = 0x0100,
    UPDATE_COLOR_SPACE = 0x0200,
    MAP_SECURE_BUFFER = 0x400,
    COLOR_METADATA = 0x800,
};

enum DispFetchParamType {
    GET_PP_PARAM_INTERLACED = 0x0004,
    GET_BUFFER_GEOMETRY = 0x0080,
    GET_REFRESH_RATE = 0x0100,
    GET_COLOR_SPACE = 0x0200,
    GET_MAP_SECURE_BUFFER = 0x400,
    GET_COLOR_METADATA = 0x800,
};

struct private_handle_t;
int setMetaData(struct private_handle_t *handle, enum DispParamType paramType,
        void *param);

int getMetaData(struct private_handle_t *handle, enum DispFetchParamType paramType,
        void *param);

int copyMetaData(struct private_handle_t *src, struct private_handle_t *dst);
#ifdef __cplusplus
}
#endif

#endif /* _QDMETADATA_H */

