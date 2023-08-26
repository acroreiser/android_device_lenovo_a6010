/*
 * Copyright (c) 2014-2016, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of The Linux Foundation nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
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

#define LOG_TAG "hardware_info"
/*#define LOG_NDEBUG 0*/
#define LOG_NDDEBUG 0

#include <stdlib.h>
#include <dlfcn.h>
#include <cutils/log.h>
#include <cutils/str_parms.h>
#include "audio_hw.h"
#include "platform.h"
#include "platform_api.h"


struct hardware_info {
    char name[HW_INFO_ARRAY_MAX_SIZE];
    char type[HW_INFO_ARRAY_MAX_SIZE];
    /* variables for handling target variants */
    uint32_t num_snd_devices;
    char dev_extn[HW_INFO_ARRAY_MAX_SIZE];
    snd_device_t  *snd_devices;
};

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

static int update_hardware_info_8x16(struct hardware_info *hw_info, const char *snd_card_name)
{
    if (!strcmp(snd_card_name, "msm8x16-snd-card")) {
        strlcpy(hw_info->type, "", sizeof(hw_info->type));
        strlcpy(hw_info->name, "msm8x16", sizeof(hw_info->name));
        hw_info->snd_devices = NULL;
        hw_info->num_snd_devices = 0;
        strlcpy(hw_info->dev_extn, "", sizeof(hw_info->dev_extn));
    } else if (!strcmp(snd_card_name, "msm8x16-snd-card-mtp")) {
        strlcpy(hw_info->type, "", sizeof(hw_info->type));
        strlcpy(hw_info->name, "msm8x16", sizeof(hw_info->name));
        hw_info->snd_devices = NULL;
        hw_info->num_snd_devices = 0;
        strlcpy(hw_info->dev_extn, "", sizeof(hw_info->dev_extn));
    } else if (!strcmp(snd_card_name, "msm8x16-snd-card-sbc")) {
        strlcpy(hw_info->type, "sbc", sizeof(hw_info->type));
        strlcpy(hw_info->name, "msm8x16", sizeof(hw_info->name));
        hw_info->snd_devices = NULL;
        hw_info->num_snd_devices = 0;
    } else if (!strcmp(snd_card_name, "msm8x16-skuh-snd-card")) {
        strlcpy(hw_info->type, "skuh", sizeof(hw_info->type));
        strlcpy(hw_info->name, "msm8x16", sizeof(hw_info->name));
        hw_info->snd_devices = NULL;
        hw_info->num_snd_devices = 0;
        strlcpy(hw_info->dev_extn, "", sizeof(hw_info->dev_extn));
    } else if (!strcmp(snd_card_name, "msm8x16-skui-snd-card")) {
        strlcpy(hw_info->type, "skui", sizeof(hw_info->type));
        strlcpy(hw_info->name, "msm8x16", sizeof(hw_info->name));
        hw_info->snd_devices = NULL;
        hw_info->num_snd_devices = 0;
        strlcpy(hw_info->dev_extn, "", sizeof(hw_info->dev_extn));
    } else if (!strcmp(snd_card_name, "msm8x16-skuhf-snd-card")) {
        strlcpy(hw_info->type, "skuhf", sizeof(hw_info->type));
        strlcpy(hw_info->name, "msm8x16", sizeof(hw_info->name));
        hw_info->snd_devices = NULL;
        hw_info->num_snd_devices = 0;
        strlcpy(hw_info->dev_extn, "", sizeof(hw_info->dev_extn));
    } else if (!strcmp(snd_card_name, "msm8939-snd-card")) {
        strlcpy(hw_info->type, "", sizeof(hw_info->type));
        strlcpy(hw_info->name, "msm8939", sizeof(hw_info->name));
        hw_info->snd_devices = NULL;
        hw_info->num_snd_devices = 0;
        strlcpy(hw_info->dev_extn, "", sizeof(hw_info->dev_extn));
    } else if (!strcmp(snd_card_name, "msm8939-snd-card-mtp")) {
        strlcpy(hw_info->type, "", sizeof(hw_info->type));
        strlcpy(hw_info->name, "msm8939", sizeof(hw_info->name));
        hw_info->snd_devices = NULL;
        hw_info->num_snd_devices = 0;
        strlcpy(hw_info->dev_extn, "", sizeof(hw_info->dev_extn));
    } else if (!strcmp(snd_card_name, "msm8939-snd-card-skuk")) {
        strlcpy(hw_info->type, "skuk", sizeof(hw_info->type));
        strlcpy(hw_info->name, "msm8939", sizeof(hw_info->name));
        hw_info->snd_devices = NULL;
        hw_info->num_snd_devices = 0;
        strlcpy(hw_info->dev_extn, "", sizeof(hw_info->dev_extn));
    } else if (!strcmp(snd_card_name, "msm8939-tapan-snd-card") ||
               !strcmp(snd_card_name, "msm8939-tapan9302-snd-card")) {
        strlcpy(hw_info->type, "", sizeof(hw_info->type));
        strlcpy(hw_info->name, "msm8939", sizeof(hw_info->name));
        hw_info->snd_devices = NULL;
        hw_info->num_snd_devices = 0;
        strlcpy(hw_info->dev_extn, "", sizeof(hw_info->dev_extn));
    } else if (!strcmp(snd_card_name, "msm8909-snd-card")) {
        strlcpy(hw_info->type, "", sizeof(hw_info->type));
        strlcpy(hw_info->name, "msm8909", sizeof(hw_info->name));
        hw_info->snd_devices = NULL;
        hw_info->num_snd_devices = 0;
        strlcpy(hw_info->dev_extn, "", sizeof(hw_info->dev_extn));
    } else if (!strcmp(snd_card_name, "msm8909-skua-snd-card")) {
        strlcpy(hw_info->type, "skua", sizeof(hw_info->type));
        strlcpy(hw_info->name, "msm8909", sizeof(hw_info->name));
        hw_info->snd_devices = NULL;
        hw_info->num_snd_devices = 0;
        strlcpy(hw_info->dev_extn, "", sizeof(hw_info->dev_extn));
    } else if (!strcmp(snd_card_name, "msm8909-skuc-snd-card")) {
        strlcpy(hw_info->type, "skuc", sizeof(hw_info->type));
        strlcpy(hw_info->name, "msm8909", sizeof(hw_info->name));
        hw_info->snd_devices = NULL;
        hw_info->num_snd_devices = 0;
        strlcpy(hw_info->dev_extn, "", sizeof(hw_info->dev_extn));
    } else if (!strcmp(snd_card_name, "msm8909-pm8916-snd-card")) {
        strlcpy(hw_info->type, "", sizeof(hw_info->type));
        strlcpy(hw_info->name, "msm8909", sizeof(hw_info->name));
        hw_info->snd_devices = NULL;
        hw_info->num_snd_devices = 0;
        strlcpy(hw_info->dev_extn, "", sizeof(hw_info->dev_extn));
    } else if (!strcmp(snd_card_name, "msm8909-skue-snd-card")) {
        strlcpy(hw_info->type, "skue", sizeof(hw_info->type));
        strlcpy(hw_info->name, "msm8909", sizeof(hw_info->name));
        hw_info->snd_devices = NULL;
        hw_info->num_snd_devices = 0;
        strlcpy(hw_info->dev_extn, "", sizeof(hw_info->dev_extn));
    } else if (!strcmp(snd_card_name, "msm8939-snd-card-skul")) {
        strlcpy(hw_info->type, "skul", sizeof(hw_info->type));
        strlcpy(hw_info->name, "msm8939", sizeof(hw_info->name));
        hw_info->snd_devices = NULL;
        hw_info->num_snd_devices = 0;
        strlcpy(hw_info->dev_extn, "", sizeof(hw_info->dev_extn));
    }  else if (!strcmp(snd_card_name, "msm8952-snd-card")) {
        strlcpy(hw_info->type, "", sizeof(hw_info->type));
        strlcpy(hw_info->name, "msm8952", sizeof(hw_info->name));
        hw_info->snd_devices = NULL;
        hw_info->num_snd_devices = 0;
        strlcpy(hw_info->dev_extn, "", sizeof(hw_info->dev_extn));
    } else if (!strcmp(snd_card_name, "msm8952-snd-card-mtp")) {
        strlcpy(hw_info->type, "", sizeof(hw_info->type));
        strlcpy(hw_info->name, "msm8952", sizeof(hw_info->name));
        hw_info->snd_devices = NULL;
        hw_info->num_snd_devices = 0;
        strlcpy(hw_info->dev_extn, "", sizeof(hw_info->dev_extn));
    } else if (!strcmp(snd_card_name, "msm8952-tomtom-snd-card")) {
        strlcpy(hw_info->type, "", sizeof(hw_info->type));
        strlcpy(hw_info->name, "msm8952", sizeof(hw_info->name));
        hw_info->snd_devices = NULL;
        hw_info->num_snd_devices = 0;
        strlcpy(hw_info->dev_extn, "", sizeof(hw_info->dev_extn));
    } else if (!strcmp(snd_card_name, "msm8976-tasha-snd-card")) {
        strlcpy(hw_info->type, "", sizeof(hw_info->type));
        strlcpy(hw_info->name, "msm8976", sizeof(hw_info->name));
        hw_info->snd_devices = NULL;
        hw_info->num_snd_devices = 0;
        strlcpy(hw_info->dev_extn, "", sizeof(hw_info->dev_extn));
    } else if (!strcmp(snd_card_name, "msm8976-tasha-skun-snd-card")) {
       strlcpy(hw_info->type, "", sizeof(hw_info->type));
       strlcpy(hw_info->name, "msm8976", sizeof(hw_info->name));
       hw_info->snd_devices = NULL;
       hw_info->num_snd_devices = 0;
       strlcpy(hw_info->dev_extn, "", sizeof(hw_info->dev_extn));
    }  else if (!strcmp(snd_card_name, "msm8952-skum-snd-card")) {
        strlcpy(hw_info->type, "", sizeof(hw_info->type));
        strlcpy(hw_info->name, "msm8952", sizeof(hw_info->name));
        hw_info->snd_devices = NULL;
        hw_info->num_snd_devices = 0;
        strlcpy(hw_info->dev_extn, "", sizeof(hw_info->dev_extn));
    }  else if (!strcmp(snd_card_name, "msm8976-skun-snd-card")) {
        strlcpy(hw_info->type, "", sizeof(hw_info->type));
        strlcpy(hw_info->name, "msm8976", sizeof(hw_info->name));
        hw_info->snd_devices = NULL;
        hw_info->num_snd_devices = 0;
        strlcpy(hw_info->dev_extn, "", sizeof(hw_info->dev_extn));
    }  else if (!strcmp(snd_card_name, "msm8976-skup-snd-card")) {
        strlcpy(hw_info->type, "", sizeof(hw_info->type));
        strlcpy(hw_info->name, "msm8976", sizeof(hw_info->name));
        hw_info->snd_devices = NULL;
        hw_info->num_snd_devices = 0;
        strlcpy(hw_info->dev_extn, "", sizeof(hw_info->dev_extn));
    } else if (!strcmp(snd_card_name, "msm8939-florida-snd-card")) {
        strlcpy(hw_info->type, "florida", sizeof(hw_info->type));
        strlcpy(hw_info->name, "msm8939", sizeof(hw_info->name));
        hw_info->snd_devices = NULL;
        hw_info->num_snd_devices = 0;
        strlcpy(hw_info->dev_extn, "", sizeof(hw_info->dev_extn));
    } else {
        ALOGW("%s: Not an  8x16/8939/8909/8952 device", __func__);
        return -ENODEV;
    }
    return 0;
}

void *hw_info_init(const char *snd_card_name)
{
    struct hardware_info *hw_info;

    hw_info = malloc(sizeof(struct hardware_info));
    if (!hw_info) {
        ALOGE("failed to allocate mem for hardware info");
        return NULL;
    }

    if (update_hardware_info_8x16(hw_info, snd_card_name) < 0) {
        ALOGE("%s: Unsupported target %s:",__func__, snd_card_name);
        free(hw_info);
        hw_info = NULL;
    }

    return hw_info;
}

void hw_info_deinit(void *hw_info)
{
    struct hardware_info *my_data = (struct hardware_info*) hw_info;

    if(!my_data)
        free(my_data);
}

void hw_info_append_hw_type(void *hw_info, snd_device_t snd_device,
                            char *device_name)
{
    struct hardware_info *my_data = (struct hardware_info*) hw_info;
    uint32_t i = 0;

    snd_device_t *snd_devices =
            (snd_device_t *) my_data->snd_devices;

    if(snd_devices != NULL) {
        for (i = 0; i <  my_data->num_snd_devices; i++) {
            if (snd_device == (snd_device_t)snd_devices[i]) {
                ALOGV("extract dev_extn device %d, extn = %s",
                        (snd_device_t)snd_devices[i],  my_data->dev_extn);
                CHECK(strlcat(device_name,  my_data->dev_extn,
                        DEVICE_NAME_MAX_SIZE) < DEVICE_NAME_MAX_SIZE);
                break;
            }
        }
    }
    ALOGD("%s : device_name = %s", __func__,device_name);
}
