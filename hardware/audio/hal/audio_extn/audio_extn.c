/*
 * Copyright (c) 2013-2017, The Linux Foundation. All rights reserved.
 * Not a Contribution.
 *
 * Copyright (C) 2013 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * This file was modified by DTS, Inc. The portions of the
 * code modified by DTS, Inc are copyrighted and
 * licensed separately, as follows:
 *
 * (C) 2014 DTS, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "audio_hw_extn"
/*#define LOG_NDEBUG 0*/
#define LOG_NDDEBUG 0

#include <stdlib.h>
#include <errno.h>
#include <dlfcn.h>
#include <cutils/properties.h>
#include <cutils/log.h>

#include "audio_hw.h"
#include "audio_extn.h"
#include "platform.h"
#include "platform_api.h"
#include "edid.h"

#include "sound/compress_params.h"

#define MAX_SLEEP_RETRY 100
#define WIFI_INIT_WAIT_SLEEP 50

struct audio_extn_module {
    bool anc_enabled;
    bool aanc_enabled;
    bool custom_stereo_enabled;
    uint32_t proxy_channel_num;
    bool hpx_enabled;
    bool vbat_enabled;
};

static struct audio_extn_module aextnmod = {
    .anc_enabled = 0,
    .aanc_enabled = 0,
    .custom_stereo_enabled = 0,
    .proxy_channel_num = 2,
    .hpx_enabled = 0,
    .vbat_enabled = 0,
};

#define AUDIO_PARAMETER_KEY_ANC        "anc_enabled"
#define AUDIO_PARAMETER_KEY_WFD        "wfd_channel_cap"
#define AUDIO_PARAMETER_CAN_OPEN_PROXY "can_open_proxy"
#define AUDIO_PARAMETER_CUSTOM_STEREO  "stereo_as_dual_mono"
/* Query offload playback instances count */
#define AUDIO_PARAMETER_OFFLOAD_NUM_ACTIVE "offload_num_active"
#define AUDIO_PARAMETER_HPX            "HPX"

#ifndef HFP_ENABLED
#define audio_extn_hfp_set_parameters(adev, parms) (0)
#else
void audio_extn_hfp_set_parameters(struct audio_device *adev,
                                           struct str_parms *parms);
#endif

#ifndef SOURCE_TRACKING_ENABLED
#define audio_extn_source_track_set_parameters(adev, parms) (0)
#define audio_extn_source_track_get_parameters(adev, query, reply) (0)
#else
void audio_extn_source_track_set_parameters(struct audio_device *adev,
                                            struct str_parms *parms);
void audio_extn_source_track_get_parameters(const struct audio_device *adev,
                                            struct str_parms *query,
                                            struct str_parms *reply);
#endif

#ifndef CUSTOM_STEREO_ENABLED
#define audio_extn_customstereo_set_parameters(adev, parms)         (0)
#else
void audio_extn_customstereo_set_parameters(struct audio_device *adev,
                                           struct str_parms *parms)
{
    int ret = 0;
    char value[32]={0};
    bool custom_stereo_state = false;
    const char *mixer_ctl_name = "Set Custom Stereo OnOff";
    struct mixer_ctl *ctl;

    ALOGV("%s", __func__);
    ret = str_parms_get_str(parms, AUDIO_PARAMETER_CUSTOM_STEREO, value,
                            sizeof(value));
    if (ret >= 0) {
        if (!strncmp("true", value, sizeof("true")) || atoi(value))
            custom_stereo_state = true;

        if (custom_stereo_state == aextnmod.custom_stereo_enabled)
            return;

        ctl = mixer_get_ctl_by_name(adev->mixer, mixer_ctl_name);
        if (!ctl) {
            ALOGE("%s: Could not get ctl for mixer cmd - %s",
                  __func__, mixer_ctl_name);
            return;
        }
        if (mixer_ctl_set_value(ctl, 0, custom_stereo_state) < 0) {
            ALOGE("%s: Could not set custom stereo state %d",
                  __func__, custom_stereo_state);
            return;
        }
        aextnmod.custom_stereo_enabled = custom_stereo_state;
        ALOGV("%s: Setting custom stereo state success", __func__);
    }
}
#endif /* CUSTOM_STEREO_ENABLED */

#ifndef DTS_EAGLE
#define audio_extn_hpx_set_parameters(adev, parms)         (0)
#define audio_extn_hpx_get_parameters(query, reply)  (0)
#define audio_extn_check_and_set_dts_hpx_state(adev)       (0)
#else
void audio_extn_hpx_set_parameters(struct audio_device *adev,
                                   struct str_parms *parms)
{
    int ret = 0;
    char value[32]={0};
    char prop[PROPERTY_VALUE_MAX] = "false";
    bool hpx_state = false;
    const char *mixer_ctl_name = "Set HPX OnOff";
    struct mixer_ctl *ctl = NULL;
    ALOGV("%s", __func__);

    property_get("vendor.audio.use.dts_eagle", prop, "0");
    if (strncmp("true", prop, sizeof("true")))
        return;

    ret = str_parms_get_str(parms, AUDIO_PARAMETER_HPX, value,
                            sizeof(value));
    if (ret >= 0) {
        if (!strncmp("ON", value, sizeof("ON")))
            hpx_state = true;

        if (hpx_state == aextnmod.hpx_enabled)
            return;

        aextnmod.hpx_enabled = hpx_state;
        /* set HPX state on stream pp */
        if (adev->offload_effects_set_hpx_state != NULL)
            adev->offload_effects_set_hpx_state(hpx_state);

        audio_extn_dts_eagle_fade(adev, aextnmod.hpx_enabled, NULL);
        /* set HPX state on device pp */
        ctl = mixer_get_ctl_by_name(adev->mixer, mixer_ctl_name);
        if (ctl)
            mixer_ctl_set_value(ctl, 0, aextnmod.hpx_enabled);
    }
}

static int audio_extn_hpx_get_parameters(struct str_parms *query,
                                       struct str_parms *reply)
{
    int ret;
    char value[32]={0};

    ALOGV("%s: hpx %d", __func__, aextnmod.hpx_enabled);
    ret = str_parms_get_str(query, AUDIO_PARAMETER_HPX, value,
                            sizeof(value));
    if (ret >= 0) {
        if (aextnmod.hpx_enabled)
            str_parms_add_str(reply, AUDIO_PARAMETER_HPX, "ON");
        else
            str_parms_add_str(reply, AUDIO_PARAMETER_HPX, "OFF");
    }
    return ret;
}

void audio_extn_check_and_set_dts_hpx_state(const struct audio_device *adev)
{
    char prop[PROPERTY_VALUE_MAX];
    property_get("vendor.audio.use.dts_eagle", prop, "0");
    if (strncmp("true", prop, sizeof("true")))
        return;
    if (adev->offload_effects_set_hpx_state)
        adev->offload_effects_set_hpx_state(aextnmod.hpx_enabled);
}
#endif

#ifdef VBAT_MONITOR_ENABLED
bool audio_extn_is_vbat_enabled(void)
{
    ALOGD("%s: status: %d", __func__, aextnmod.vbat_enabled);
    return (aextnmod.vbat_enabled ? true: false);
}

bool audio_extn_can_use_vbat(void)
{
    char prop_vbat_enabled[PROPERTY_VALUE_MAX] = "false";

    property_get("persist.vendor.audio.vbat.enabled", prop_vbat_enabled, "0");
    if (!strncmp("true", prop_vbat_enabled, 4)) {
        aextnmod.vbat_enabled = 1;
    }

    ALOGD("%s: vbat.enabled property is set to %s", __func__, prop_vbat_enabled);
    return (aextnmod.vbat_enabled ? true: false);
}
#endif

#ifndef ANC_HEADSET_ENABLED
#define audio_extn_set_anc_parameters(adev, parms)       (0)
#else
bool audio_extn_get_anc_enabled(void)
{
    ALOGD("%s: anc_enabled:%d", __func__, aextnmod.anc_enabled);
    return (aextnmod.anc_enabled ? true: false);
}

bool audio_extn_should_use_handset_anc(int in_channels)
{
    char prop_aanc[PROPERTY_VALUE_MAX] = "false";

    property_get("persist.vendor.audio.aanc.enable", prop_aanc, "0");
    if (!strncmp("true", prop_aanc, 4)) {
        ALOGD("%s: AANC enabled in the property", __func__);
        aextnmod.aanc_enabled = 1;
    }

    return (aextnmod.aanc_enabled && aextnmod.anc_enabled
            && (in_channels == 1));
}

bool audio_extn_should_use_fb_anc(void)
{
  char prop_anc[PROPERTY_VALUE_MAX] = "feedforward";

  property_get("persist.vendor.audio.headset.anc.type", prop_anc, "0");
  if (!strncmp("feedback", prop_anc, sizeof("feedback"))) {
    ALOGD("%s: FB ANC headset type enabled\n", __func__);
    return true;
  }
  return false;
}

void audio_extn_set_anc_parameters(struct audio_device *adev,
                                   struct str_parms *parms)
{
    int ret;
    char value[32] ={0};
    struct listnode *node;
    struct audio_usecase *usecase;
    struct str_parms *query_44_1;
    struct str_parms *reply_44_1;
    struct str_parms *parms_disable_44_1;

    ret = str_parms_get_str(parms, AUDIO_PARAMETER_KEY_ANC, value,
                            sizeof(value));
    if (ret >= 0) {
        if (strcmp(value, "true") == 0)
            aextnmod.anc_enabled = true;
        else
            aextnmod.anc_enabled = false;

        /* Store current 44.1 configuration and disable it temporarily before
         * changing ANC state.
         * Since 44.1 playback is not allowed with anc on.
         * If ANC switch is done when 44.1 is active three devices would need
         * sequencing 1. "headphones-44.1", 2. "headphones-anc" and
         * 3. "headphones".
         * Note: Enable/diable of anc would affect other two device's state.
         */
        query_44_1 = str_parms_create_str(AUDIO_PARAMETER_KEY_NATIVE_AUDIO);
        reply_44_1 = str_parms_create();
        if (!query_44_1 || !reply_44_1) {
            if (query_44_1) {
                str_parms_destroy(query_44_1);
            }
            if (reply_44_1) {
                str_parms_destroy(reply_44_1);
            }

            ALOGE("%s: param creation failed", __func__);
            return;
        }

        platform_get_parameters(adev->platform, query_44_1, reply_44_1);

        parms_disable_44_1 = str_parms_create();
        if (!parms_disable_44_1) {
            str_parms_destroy(query_44_1);
            str_parms_destroy(reply_44_1);
            ALOGE("%s: param creation failed for parms_disable_44_1", __func__);
            return;
        }

        str_parms_add_str(parms_disable_44_1, AUDIO_PARAMETER_KEY_NATIVE_AUDIO, "false");
        platform_set_parameters(adev->platform, parms_disable_44_1);
        str_parms_destroy(parms_disable_44_1);

        // Refresh device selection for anc playback
        list_for_each(node, &adev->usecase_list) {
            usecase = node_to_item(node, struct audio_usecase, list);
            if (usecase->type == PCM_PLAYBACK) {
                if (usecase->stream.out->devices == \
                    AUDIO_DEVICE_OUT_WIRED_HEADPHONE ||
                    usecase->stream.out->devices ==  \
                    AUDIO_DEVICE_OUT_WIRED_HEADSET) {
                        select_devices(adev, usecase->id);
                        ALOGV("%s: switching device completed", __func__);
                        break;
                }
            }
        }

        // Restore 44.1 configuration on top of updated anc state
        platform_set_parameters(adev->platform, reply_44_1);
        str_parms_destroy(query_44_1);
        str_parms_destroy(reply_44_1);
    }

    ALOGD("%s: anc_enabled:%d", __func__, aextnmod.anc_enabled);
}
#endif /* ANC_HEADSET_ENABLED */

#ifndef FLUENCE_ENABLED
#define audio_extn_set_fluence_parameters(adev, parms) (0)
#define audio_extn_get_fluence_parameters(adev, query, reply) (0)
#else
void audio_extn_set_fluence_parameters(struct audio_device *adev,
                                            struct str_parms *parms)
{
    int ret = 0, err;
    char value[32];
    struct listnode *node;
    struct audio_usecase *usecase;

    err = str_parms_get_str(parms, AUDIO_PARAMETER_KEY_FLUENCE,
                                 value, sizeof(value));
    ALOGV_IF(err >= 0, "%s: Set Fluence Type to %s", __func__, value);
    if (err >= 0) {
        ret = platform_set_fluence_type(adev->platform, value);
        if (ret != 0) {
            ALOGE("platform_set_fluence_type returned error: %d", ret);
        } else {
            /*
             *If the fluence is manually set/reset, devices
             *need to get updated for all the usecases
             *i.e. audio and voice.
             */
             list_for_each(node, &adev->usecase_list) {
                 usecase = node_to_item(node, struct audio_usecase, list);
                 select_devices(adev, usecase->id);
             }
        }
    }
}

int audio_extn_get_fluence_parameters(const struct audio_device *adev,
                       struct str_parms *query, struct str_parms *reply)
{
    int ret = 0, err;
    char value[256] = {0};

    err = str_parms_get_str(query, AUDIO_PARAMETER_KEY_FLUENCE, value,
                                                          sizeof(value));
    if (err >= 0) {
        ret = platform_get_fluence_type(adev->platform, value, sizeof(value));
        if (ret >= 0) {
            ALOGV("%s: Fluence Type is %s", __func__, value);
            str_parms_add_str(reply, AUDIO_PARAMETER_KEY_FLUENCE, value);
        } else
            goto done;
    }
done:
    return ret;
}
#endif /* FLUENCE_ENABLED */

#ifndef AFE_PROXY_ENABLED
#define audio_extn_set_afe_proxy_parameters(adev, parms)  (0)
#define audio_extn_get_afe_proxy_parameters(query, reply) (0)
#else
static int32_t afe_proxy_set_channel_mapping(struct audio_device *adev,
                                                     int channel_count)
{
    struct mixer_ctl *ctl;
    const char *mixer_ctl_name = "Playback Device Channel Map";
    int set_values[8] = {0};
    int ret;
    ALOGV("%s channel_count:%d",__func__, channel_count);

    switch (channel_count) {
    case 2:
        set_values[0] = PCM_CHANNEL_FL;
        set_values[1] = PCM_CHANNEL_FR;
        break;
    case 6:
        set_values[0] = PCM_CHANNEL_FL;
        set_values[1] = PCM_CHANNEL_FR;
        set_values[2] = PCM_CHANNEL_FC;
        set_values[3] = PCM_CHANNEL_LFE;
        set_values[4] = PCM_CHANNEL_LS;
        set_values[5] = PCM_CHANNEL_RS;
        break;
    case 8:
        set_values[0] = PCM_CHANNEL_FL;
        set_values[1] = PCM_CHANNEL_FR;
        set_values[2] = PCM_CHANNEL_FC;
        set_values[3] = PCM_CHANNEL_LFE;
        set_values[4] = PCM_CHANNEL_LS;
        set_values[5] = PCM_CHANNEL_RS;
        set_values[6] = PCM_CHANNEL_LB;
        set_values[7] = PCM_CHANNEL_RB;
        break;
    default:
        ALOGE("unsupported channels(%d) for setting channel map",
                                                    channel_count);
        return -EINVAL;
    }

    ctl = mixer_get_ctl_by_name(adev->mixer, mixer_ctl_name);
    if (!ctl) {
        ALOGE("%s: Could not get ctl for mixer cmd - %s",
              __func__, mixer_ctl_name);
        return -EINVAL;
    }
    ALOGV("AFE: set mapping(%d %d %d %d %d %d %d %d) for channel:%d",
        set_values[0], set_values[1], set_values[2], set_values[3], set_values[4],
        set_values[5], set_values[6], set_values[7], channel_count);
    ret = mixer_ctl_set_array(ctl, set_values, channel_count);
    return ret;
}

int32_t audio_extn_set_afe_proxy_channel_mixer(struct audio_device *adev,
                                    int channel_count)
{
    int32_t ret = 0;
    const char *channel_cnt_str = NULL;
    struct mixer_ctl *ctl = NULL;
    const char *mixer_ctl_name = "PROXY_RX Channels";

    ALOGD("%s: entry", __func__);
    /* use the existing channel count set by hardware params to
    configure the back end for stereo as usb/a2dp would be
    stereo by default */
    ALOGD("%s: channels = %d", __func__, channel_count);
    switch (channel_count) {
    case 8: channel_cnt_str = "Eight"; break;
    case 7: channel_cnt_str = "Seven"; break;
    case 6: channel_cnt_str = "Six"; break;
    case 5: channel_cnt_str = "Five"; break;
    case 4: channel_cnt_str = "Four"; break;
    case 3: channel_cnt_str = "Three"; break;
    default: channel_cnt_str = "Two"; break;
    }

    if(channel_count >= 2 && channel_count <= 8) {
       ctl = mixer_get_ctl_by_name(adev->mixer, mixer_ctl_name);
       if (!ctl) {
            ALOGE("%s: could not get ctl for mixer cmd - %s",
                  __func__, mixer_ctl_name);
        return -EINVAL;
       }
    }
    mixer_ctl_set_enum_by_string(ctl, channel_cnt_str);

    if (channel_count == 6 || channel_count == 8 || channel_count == 2) {
        ret = afe_proxy_set_channel_mapping(adev, channel_count);
    } else {
        ALOGE("%s: set unsupported channel count(%d)",  __func__, channel_count);
        ret = -EINVAL;
    }

    ALOGD("%s: exit", __func__);
    return ret;
}

void audio_extn_set_afe_proxy_parameters(struct audio_device *adev,
                                         struct str_parms *parms)
{
    int ret, val;
    char value[32]={0};

    ret = str_parms_get_str(parms, AUDIO_PARAMETER_KEY_WFD, value,
                            sizeof(value));
    if (ret >= 0) {
        val = atoi(value);
        aextnmod.proxy_channel_num = val;
        adev->cur_wfd_channels = val;
        ALOGD("%s: channel capability set to: %d", __func__,
               aextnmod.proxy_channel_num);
    }
}

int audio_extn_get_afe_proxy_parameters(struct str_parms *query,
                                        struct str_parms *reply)
{
    int ret, val;
    char value[32]={0};

    ret = str_parms_get_str(query, AUDIO_PARAMETER_CAN_OPEN_PROXY, value,
                            sizeof(value));
    if (ret >= 0) {
        if (audio_extn_usb_is_proxy_inuse())
            val = 0;
        else
            val = 1;
        str_parms_add_int(reply, AUDIO_PARAMETER_CAN_OPEN_PROXY, val);
    }

    return 0;
}

/* must be called with hw device mutex locked */
int32_t audio_extn_read_afe_proxy_channel_masks(struct stream_out *out)
{
    int ret = 0;
    int channels = aextnmod.proxy_channel_num;

    switch (channels) {
        /*
         * Do not handle stereo output in Multi-channel cases
         * Stereo case is handled in normal playback path
         */
    case 6:
        ALOGV("%s: AFE PROXY supports 5.1", __func__);
        out->supported_channel_masks[0] = AUDIO_CHANNEL_OUT_5POINT1;
        break;
    case 8:
        ALOGV("%s: AFE PROXY supports 5.1 and 7.1 channels", __func__);
        out->supported_channel_masks[0] = AUDIO_CHANNEL_OUT_5POINT1;
        out->supported_channel_masks[1] = AUDIO_CHANNEL_OUT_7POINT1;
        break;
    default:
        ALOGE("AFE PROXY does not support multi channel playback");
        ret = -ENOSYS;
        break;
    }
    return ret;
}

int32_t audio_extn_get_afe_proxy_channel_count()
{
    return aextnmod.proxy_channel_num;
}

#endif /* AFE_PROXY_ENABLED */

static int get_active_offload_usecases(const struct audio_device *adev,
                                       struct str_parms *query,
                                       struct str_parms *reply)
{
    int ret, count = 0;
    char value[32]={0};
    struct listnode *node;
    struct audio_usecase *usecase;

    ALOGV("%s", __func__);
    ret = str_parms_get_str(query, AUDIO_PARAMETER_OFFLOAD_NUM_ACTIVE, value,
                            sizeof(value));
    if (ret >= 0) {
        list_for_each(node, &adev->usecase_list) {
            usecase = node_to_item(node, struct audio_usecase, list);
            if (is_offload_usecase(usecase->id))
                count++;
        }
        ALOGV("%s, number of active offload usecases: %d", __func__, count);
        str_parms_add_int(reply, AUDIO_PARAMETER_OFFLOAD_NUM_ACTIVE, count);
    }
    return ret;
}

void audio_extn_set_parameters(struct audio_device *adev,
                               struct str_parms *parms)
{
   audio_extn_set_anc_parameters(adev, parms);
   audio_extn_set_fluence_parameters(adev, parms);
   audio_extn_set_afe_proxy_parameters(adev, parms);
   audio_extn_fm_set_parameters(adev, parms);
   audio_extn_sound_trigger_set_parameters(adev, parms);
   audio_extn_listen_set_parameters(adev, parms);
   audio_extn_ssr_set_parameters(adev, parms);
   audio_extn_hfp_set_parameters(adev, parms);
   audio_extn_dts_eagle_set_parameters(adev, parms);
   audio_extn_ddp_set_parameters(adev, parms);
   audio_extn_ds2_set_parameters(adev, parms);
   audio_extn_customstereo_set_parameters(adev, parms);
   audio_extn_hpx_set_parameters(adev, parms);
   audio_extn_pm_set_parameters(parms);
   audio_extn_source_track_set_parameters(adev, parms);
   if (adev->offload_effects_set_parameters != NULL)
       adev->offload_effects_set_parameters(parms);
}

void audio_extn_get_parameters(const struct audio_device *adev,
                              struct str_parms *query,
                              struct str_parms *reply)
{
    char *kv_pairs = NULL;
    audio_extn_get_afe_proxy_parameters(query, reply);
    audio_extn_get_fluence_parameters(adev, query, reply);
    audio_extn_ssr_get_parameters(adev, query, reply);
    get_active_offload_usecases(adev, query, reply);
    audio_extn_dts_eagle_get_parameters(adev, query, reply);
    audio_extn_hpx_get_parameters(query, reply);
    audio_extn_source_track_get_parameters(adev, query, reply);
    if (adev->offload_effects_get_parameters != NULL)
        adev->offload_effects_get_parameters(query, reply);

    kv_pairs = str_parms_to_str(reply);
    ALOGD_IF(kv_pairs != NULL, "%s: returns %s", __func__, kv_pairs);
    free(kv_pairs);
}

#ifndef COMPRESS_METADATA_NEEDED
#define audio_extn_parse_compress_metadata(out, parms) (0)
#else
int audio_extn_parse_compress_metadata(struct stream_out *out,
                                       struct str_parms *parms)
{
    int ret = 0;
    char value[32];

    if (out->format == AUDIO_FORMAT_FLAC) {
        ret = str_parms_get_str(parms, AUDIO_OFFLOAD_CODEC_FLAC_MIN_BLK_SIZE, value, sizeof(value));
        if (ret >= 0) {
            out->compr_config.codec->options.flac_dec.min_blk_size = atoi(value);
            out->is_compr_metadata_avail = true;
        }
        ret = str_parms_get_str(parms, AUDIO_OFFLOAD_CODEC_FLAC_MAX_BLK_SIZE, value, sizeof(value));
        if (ret >= 0) {
            out->compr_config.codec->options.flac_dec.max_blk_size = atoi(value);
            out->is_compr_metadata_avail = true;
        }
        ret = str_parms_get_str(parms, AUDIO_OFFLOAD_CODEC_FLAC_MIN_FRAME_SIZE, value, sizeof(value));
        if (ret >= 0) {
            out->compr_config.codec->options.flac_dec.min_frame_size = atoi(value);
            out->is_compr_metadata_avail = true;
        }
        ret = str_parms_get_str(parms, AUDIO_OFFLOAD_CODEC_FLAC_MAX_FRAME_SIZE, value, sizeof(value));
        if (ret >= 0) {
            out->compr_config.codec->options.flac_dec.max_frame_size = atoi(value);
            out->is_compr_metadata_avail = true;
        }
        ALOGV("FLAC metadata: min_blk_size %d, max_blk_size %d min_frame_size %d max_frame_size %d",
              out->compr_config.codec->options.flac_dec.min_blk_size,
              out->compr_config.codec->options.flac_dec.max_blk_size,
              out->compr_config.codec->options.flac_dec.min_frame_size,
              out->compr_config.codec->options.flac_dec.max_frame_size);
    }

    else if (out->format == AUDIO_FORMAT_ALAC) {
        ret = str_parms_get_str(parms, AUDIO_OFFLOAD_CODEC_ALAC_FRAME_LENGTH, value, sizeof(value));
        if (ret >= 0) {
            out->compr_config.codec->options.alac.frame_length = atoi(value);
            out->is_compr_metadata_avail = true;
        }
        ret = str_parms_get_str(parms, AUDIO_OFFLOAD_CODEC_ALAC_COMPATIBLE_VERSION, value, sizeof(value));
        if (ret >= 0) {
            out->compr_config.codec->options.alac.compatible_version = atoi(value);
            out->is_compr_metadata_avail = true;
        }
        ret = str_parms_get_str(parms, AUDIO_OFFLOAD_CODEC_ALAC_BIT_DEPTH, value, sizeof(value));
        if (ret >= 0) {
            out->compr_config.codec->options.alac.bit_depth = atoi(value);
            out->is_compr_metadata_avail = true;
        }
        ret = str_parms_get_str(parms, AUDIO_OFFLOAD_CODEC_ALAC_PB, value, sizeof(value));
        if (ret >= 0) {
            out->compr_config.codec->options.alac.pb = atoi(value);
            out->is_compr_metadata_avail = true;
        }
        ret = str_parms_get_str(parms, AUDIO_OFFLOAD_CODEC_ALAC_MB, value, sizeof(value));
        if (ret >= 0) {
            out->compr_config.codec->options.alac.mb = atoi(value);
            out->is_compr_metadata_avail = true;
        }

        ret = str_parms_get_str(parms, AUDIO_OFFLOAD_CODEC_ALAC_KB, value, sizeof(value));
        if (ret >= 0) {
            out->compr_config.codec->options.alac.kb = atoi(value);
            out->is_compr_metadata_avail = true;
        }
        ret = str_parms_get_str(parms, AUDIO_OFFLOAD_CODEC_ALAC_NUM_CHANNELS, value, sizeof(value));
        if (ret >= 0) {
            out->compr_config.codec->options.alac.num_channels = atoi(value);
            out->is_compr_metadata_avail = true;
        }
        ret = str_parms_get_str(parms, AUDIO_OFFLOAD_CODEC_ALAC_MAX_RUN, value, sizeof(value));
        if (ret >= 0) {
            out->compr_config.codec->options.alac.max_run = atoi(value);
            out->is_compr_metadata_avail = true;
        }
        ret = str_parms_get_str(parms, AUDIO_OFFLOAD_CODEC_ALAC_MAX_FRAME_BYTES, value, sizeof(value));
        if (ret >= 0) {
            out->compr_config.codec->options.alac.max_frame_bytes = atoi(value);
            out->is_compr_metadata_avail = true;
        }
        ret = str_parms_get_str(parms, AUDIO_OFFLOAD_CODEC_ALAC_AVG_BIT_RATE, value, sizeof(value));
        if (ret >= 0) {
            out->compr_config.codec->options.alac.avg_bit_rate = atoi(value);
            out->is_compr_metadata_avail = true;
        }
        ret = str_parms_get_str(parms, AUDIO_OFFLOAD_CODEC_ALAC_SAMPLING_RATE, value, sizeof(value));
        if (ret >= 0) {
            out->compr_config.codec->options.alac.sample_rate = atoi(value);
            out->is_compr_metadata_avail = true;
        }
        ret = str_parms_get_str(parms, AUDIO_OFFLOAD_CODEC_ALAC_CHANNEL_LAYOUT_TAG, value, sizeof(value));
        if (ret >= 0) {
            out->compr_config.codec->options.alac.channel_layout_tag = atoi(value);
            out->is_compr_metadata_avail = true;
        }
        ALOGV("ALAC CSD values: frameLength %d bitDepth %d numChannels %d"
                " maxFrameBytes %d, avgBitRate %d, sampleRate %d",
                out->compr_config.codec->options.alac.frame_length,
                out->compr_config.codec->options.alac.bit_depth,
                out->compr_config.codec->options.alac.num_channels,
                out->compr_config.codec->options.alac.max_frame_bytes,
                out->compr_config.codec->options.alac.avg_bit_rate,
                out->compr_config.codec->options.alac.sample_rate);
    }

    else if (out->format == AUDIO_FORMAT_APE) {
        ret = str_parms_get_str(parms, AUDIO_OFFLOAD_CODEC_APE_COMPATIBLE_VERSION, value, sizeof(value));
        if (ret >= 0) {
            out->compr_config.codec->options.ape.compatible_version = atoi(value);
            out->is_compr_metadata_avail = true;
        }
        ret = str_parms_get_str(parms, AUDIO_OFFLOAD_CODEC_APE_COMPRESSION_LEVEL, value, sizeof(value));
        if (ret >= 0) {
            out->compr_config.codec->options.ape.compression_level = atoi(value);
            out->is_compr_metadata_avail = true;
        }
        ret = str_parms_get_str(parms, AUDIO_OFFLOAD_CODEC_APE_FORMAT_FLAGS, value, sizeof(value));
        if (ret >= 0) {
            out->compr_config.codec->options.ape.format_flags = atoi(value);
            out->is_compr_metadata_avail = true;
        }
        ret = str_parms_get_str(parms, AUDIO_OFFLOAD_CODEC_APE_BLOCKS_PER_FRAME, value, sizeof(value));
        if (ret >= 0) {
            out->compr_config.codec->options.ape.blocks_per_frame = atoi(value);
            out->is_compr_metadata_avail = true;
        }
        ret = str_parms_get_str(parms, AUDIO_OFFLOAD_CODEC_APE_FINAL_FRAME_BLOCKS, value, sizeof(value));
        if (ret >= 0) {
            out->compr_config.codec->options.ape.final_frame_blocks = atoi(value);
            out->is_compr_metadata_avail = true;
        }
        ret = str_parms_get_str(parms, AUDIO_OFFLOAD_CODEC_APE_TOTAL_FRAMES, value, sizeof(value));
        if (ret >= 0) {
            out->compr_config.codec->options.ape.total_frames = atoi(value);
            out->is_compr_metadata_avail = true;
        }
        ret = str_parms_get_str(parms, AUDIO_OFFLOAD_CODEC_APE_BITS_PER_SAMPLE, value, sizeof(value));
        if (ret >= 0) {
            out->compr_config.codec->options.ape.bits_per_sample = atoi(value);
            out->is_compr_metadata_avail = true;
        }
        ret = str_parms_get_str(parms, AUDIO_OFFLOAD_CODEC_APE_NUM_CHANNELS, value, sizeof(value));
        if (ret >= 0) {
            out->compr_config.codec->options.ape.num_channels = atoi(value);
            out->is_compr_metadata_avail = true;
        }
        ret = str_parms_get_str(parms, AUDIO_OFFLOAD_CODEC_APE_SAMPLE_RATE, value, sizeof(value));
        if (ret >= 0) {
            out->compr_config.codec->options.ape.sample_rate = atoi(value);
            out->is_compr_metadata_avail = true;
        }
        ret = str_parms_get_str(parms, AUDIO_OFFLOAD_CODEC_APE_SEEK_TABLE_PRESENT, value, sizeof(value));
        if (ret >= 0) {
            out->compr_config.codec->options.ape.seek_table_present = atoi(value);
            out->is_compr_metadata_avail = true;
        }
        ALOGV("APE CSD values: compatibleVersion %d compressionLevel %d"
                " formatFlags %d blocksPerFrame %d finalFrameBlocks %d"
                " totalFrames %d bitsPerSample %d numChannels %d"
                " sampleRate %d seekTablePresent %d",
                out->compr_config.codec->options.ape.compatible_version,
                out->compr_config.codec->options.ape.compression_level,
                out->compr_config.codec->options.ape.format_flags,
                out->compr_config.codec->options.ape.blocks_per_frame,
                out->compr_config.codec->options.ape.final_frame_blocks,
                out->compr_config.codec->options.ape.total_frames,
                out->compr_config.codec->options.ape.bits_per_sample,
                out->compr_config.codec->options.ape.num_channels,
                out->compr_config.codec->options.ape.sample_rate,
                out->compr_config.codec->options.ape.seek_table_present);
    }

    else if (out->format == AUDIO_FORMAT_VORBIS) {
        ret = str_parms_get_str(parms, AUDIO_OFFLOAD_CODEC_VORBIS_BITSTREAM_FMT, value, sizeof(value));
        if (ret >= 0) {
        // transcoded bitstream mode
            out->compr_config.codec->options.vorbis_dec.bit_stream_fmt = (atoi(value) > 0) ? 1 : 0;
            out->is_compr_metadata_avail = true;
        }
    }

    else if (out->format == AUDIO_FORMAT_WMA || out->format == AUDIO_FORMAT_WMA_PRO) {
        ret = str_parms_get_str(parms, AUDIO_OFFLOAD_CODEC_WMA_FORMAT_TAG, value, sizeof(value));
        if (ret >= 0) {
            out->compr_config.codec->format = atoi(value);
            out->is_compr_metadata_avail = true;
        }
        ret = str_parms_get_str(parms, AUDIO_OFFLOAD_CODEC_AVG_BIT_RATE, value, sizeof(value));
        if (ret >= 0) {
            out->compr_config.codec->options.wma.avg_bit_rate = atoi(value);
            out->is_compr_metadata_avail = true;
        }
        ret = str_parms_get_str(parms, AUDIO_OFFLOAD_CODEC_WMA_BLOCK_ALIGN, value, sizeof(value));
        if (ret >= 0) {
            out->compr_config.codec->options.wma.super_block_align = atoi(value);
            out->is_compr_metadata_avail = true;
        }
        ret = str_parms_get_str(parms, AUDIO_OFFLOAD_CODEC_WMA_BIT_PER_SAMPLE, value, sizeof(value));
        if (ret >= 0) {
            out->compr_config.codec->options.wma.bits_per_sample = atoi(value);
            out->is_compr_metadata_avail = true;
        }
        ret = str_parms_get_str(parms, AUDIO_OFFLOAD_CODEC_WMA_CHANNEL_MASK, value, sizeof(value));
        if (ret >= 0) {
            out->compr_config.codec->options.wma.channelmask = atoi(value);
            out->is_compr_metadata_avail = true;
        }
        ret = str_parms_get_str(parms, AUDIO_OFFLOAD_CODEC_WMA_ENCODE_OPTION, value, sizeof(value));
        if (ret >= 0) {
            out->compr_config.codec->options.wma.encodeopt = atoi(value);
            out->is_compr_metadata_avail = true;
        }
        ret = str_parms_get_str(parms, AUDIO_OFFLOAD_CODEC_WMA_ENCODE_OPTION1, value, sizeof(value));
        if (ret >= 0) {
            out->compr_config.codec->options.wma.encodeopt1 = atoi(value);
            out->is_compr_metadata_avail = true;
        }
        ret = str_parms_get_str(parms, AUDIO_OFFLOAD_CODEC_WMA_ENCODE_OPTION2, value, sizeof(value));
        if (ret >= 0) {
            out->compr_config.codec->options.wma.encodeopt2 = atoi(value);
            out->is_compr_metadata_avail = true;
        }
        ALOGV("WMA params: fmt %x, bit rate %x, balgn %x, sr %d, chmsk %x"
                " encop %x, op1 %x, op2 %x",
                out->compr_config.codec->format,
                out->compr_config.codec->options.wma.avg_bit_rate,
                out->compr_config.codec->options.wma.super_block_align,
                out->compr_config.codec->options.wma.bits_per_sample,
                out->compr_config.codec->options.wma.channelmask,
                out->compr_config.codec->options.wma.encodeopt,
                out->compr_config.codec->options.wma.encodeopt1,
                out->compr_config.codec->options.wma.encodeopt2);
    }

    return ret;
}
#endif

#ifdef AUXPCM_BT_ENABLED
int32_t audio_extn_read_xml(struct audio_device *adev, uint32_t mixer_card,
                            const char* mixer_xml_path,
                            const char* mixer_xml_path_auxpcm)
{
    char bt_soc[128];
    bool wifi_init_complete = false;
    int sleep_retry = 0;

    while (!wifi_init_complete && sleep_retry < MAX_SLEEP_RETRY) {
        property_get("qcom.bluetooth.soc", bt_soc, NULL);
        if (strncmp(bt_soc, "unknown", sizeof("unknown"))) {
            wifi_init_complete = true;
        } else {
            usleep(WIFI_INIT_WAIT_SLEEP*1000);
            sleep_retry++;
        }
    }

    if (!strncmp(bt_soc, "ath3k", sizeof("ath3k")))
        adev->audio_route = audio_route_init(mixer_card, mixer_xml_path_auxpcm);
    else
        adev->audio_route = audio_route_init(mixer_card, mixer_xml_path);

    return 0;
}
#endif /* AUXPCM_BT_ENABLED */

#ifdef KPI_OPTIMIZE_ENABLED
typedef int (*perf_lock_acquire_t)(int, int, int*, int);
typedef int (*perf_lock_release_t)(int);

static void *qcopt_handle;
static perf_lock_acquire_t perf_lock_acq;
static perf_lock_release_t perf_lock_rel;

static int perf_lock_handle;
char opt_lib_path[512] = {0};
int perf_lock_opts[1] = {0x20E};

int audio_extn_perf_lock_init(void)
{
    int ret = 0;
    if (qcopt_handle == NULL) {
        if (property_get("ro.vendor.extension_library",
                         opt_lib_path, NULL) <= 0) {
            ALOGE("%s: Failed getting perf property \n", __func__);
            ret = -EINVAL;
            goto err;
        }
        if ((qcopt_handle = dlopen(opt_lib_path, RTLD_NOW)) == NULL) {
            ALOGE("%s: Failed to open perf handle \n", __func__);
            ret = -EINVAL;
            goto err;
        } else {
            perf_lock_acq = (perf_lock_acquire_t)dlsym(qcopt_handle,
                                                       "perf_lock_acq");
            if (perf_lock_acq == NULL) {
                ALOGE("%s: Perf lock Acquire NULL \n", __func__);
                ret = -EINVAL;
                goto err;
            }
            perf_lock_rel = (perf_lock_release_t)dlsym(qcopt_handle,
                                                       "perf_lock_rel");
            if (perf_lock_rel == NULL) {
                ALOGE("%s: Perf lock Release NULL \n", __func__);
                ret = -EINVAL;
                goto err;
            }
            ALOGE("%s: Perf lock handles Success \n", __func__);
        }
    }
err:
    return ret;
}

void audio_extn_perf_lock_acquire(void)
{
    if (perf_lock_acq)
        perf_lock_handle = perf_lock_acq(perf_lock_handle, 0, perf_lock_opts, 1);
    else
        ALOGE("%s: Perf lock acquire error \n", __func__);
}

void audio_extn_perf_lock_release(void)
{
    if (perf_lock_rel && perf_lock_handle)
        perf_lock_rel(perf_lock_handle);
    else
        ALOGE("%s: Perf lock release error \n", __func__);
}
#endif /* KPI_OPTIMIZE_ENABLED */

static int audio_extn_set_multichannel_usecase(struct audio_device *adev,
                                               struct stream_in *in,
                                               struct audio_config *config,
                                               bool *update_params)
{
    int ret = -EINVAL;
    int channel_count = audio_channel_count_from_in_mask(in->channel_mask);
    *update_params = false;

    int max_mic_count = platform_get_max_mic_count(adev->platform);
    /* validate input params*/
    if ((channel_count == 6) &&
        (in->format == AUDIO_FORMAT_PCM_16_BIT)) {

        switch (max_mic_count) {
            case 4:
                config->channel_mask = AUDIO_CHANNEL_INDEX_MASK_4;
                break;
            case 3:
                config->channel_mask = AUDIO_CHANNEL_INDEX_MASK_3;
                break;
            case 2:
                config->channel_mask = AUDIO_CHANNEL_IN_STEREO;
                break;
            default:
                config->channel_mask = AUDIO_CHANNEL_IN_STEREO;
                break;
        }
        ret = 0;
        *update_params = true;
    }
    return ret;
}

int audio_extn_check_and_set_multichannel_usecase(struct audio_device *adev,
                                                  struct stream_in *in,
                                                  struct audio_config *config,
                                                  bool *update_params)
{
    bool ssr_supported = false;
    ssr_supported = audio_extn_ssr_check_usecase(in);
    if (ssr_supported) {
        int ret;
        pthread_mutex_lock(&adev->lock);
        ret = audio_extn_ssr_set_usecase(in, config, update_params);
        pthread_mutex_unlock(&adev->lock);
        return ret;
    } else {
        return audio_extn_set_multichannel_usecase(adev, in, config, update_params);
    }
}
