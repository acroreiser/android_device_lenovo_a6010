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
 */

#define LOG_TAG "msm8974_platform"
/*#define LOG_NDEBUG 0*/
#define LOG_NDDEBUG 0
/*#define VERY_VERY_VERBOSE_LOGGING*/
#ifdef VERY_VERY_VERBOSE_LOGGING
#define ALOGVV ALOGV
#else
#define ALOGVV(a...) do { } while(0)
#endif

#include <stdlib.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <cutils/log.h>
#include <cutils/properties.h>
#include <cutils/str_parms.h>
#include <audio_hw.h>
#include <platform_api.h>
#include "platform.h"
#include "audio_extn.h"
#include "acdb.h"
#include "voice_extn.h"
#include "edid.h"
#include "sound/compress_params.h"
#include "sound/msmcal-hwdep.h"

#define SOUND_TRIGGER_DEVICE_HANDSET_MONO_LOW_POWER_ACDB_ID (100)
#define MIXER_XML_DEFAULT_PATH "/vendor/etc/mixer_paths.xml"
#define MIXER_XML_PATH_AUXPCM "/vendor/etc/mixer_paths_auxpcm.xml"
#define MIXER_XML_PATH_I2S "/vendor/etc/mixer_paths_i2s.xml"
#define MIXER_XML_BASE_STRING "/vendor/etc/mixer_paths"
#define MIXER_FILE_DELIMITER "_"
#define MIXER_FILE_EXT ".xml"


#define PLATFORM_INFO_XML_PATH      "/vendor/etc/audio_platform_info.xml"
#define PLATFORM_INFO_XML_PATH_I2S  "/vendor/etc/audio_platform_info_i2s.xml"

#define LIB_ACDB_LOADER "libacdbloader.so"
#define AUDIO_DATA_BLOCK_MIXER_CTL "HDMI EDID"
#define CVD_VERSION_MIXER_CTL "CVD Version"

#define MAX_COMPRESS_OFFLOAD_FRAGMENT_SIZE (256 * 1024)
#define MIN_COMPRESS_OFFLOAD_FRAGMENT_SIZE (2 * 1024)
#define COMPRESS_OFFLOAD_FRAGMENT_SIZE_FOR_AV_STREAMING (2 * 1024)
#define COMPRESS_OFFLOAD_FRAGMENT_SIZE (32 * 1024)

/* Used in calculating fragment size for pcm offload */
#define PCM_OFFLOAD_BUFFER_DURATION 40 /* 40 millisecs */

/* MAX PCM fragment size cannot be increased  further due
 * to flinger's cblk size of 1mb,and it has to be a multiple of
 * 24 - lcm of channels supported by DSP
 */
#define MAX_PCM_OFFLOAD_FRAGMENT_SIZE (240 * 1024)
#define MIN_PCM_OFFLOAD_FRAGMENT_SIZE 512

/*
 * Offload buffer size for compress passthrough
 */
#define MIN_COMPRESS_PASSTHROUGH_FRAGMENT_SIZE (2 * 1024)
#define MAX_COMPRESS_PASSTHROUGH_FRAGMENT_SIZE (8 * 1024)

#define DIV_ROUND_UP(x, y) (((x) + (y) - 1)/(y))
#define ALIGN(x, y) ((y) * DIV_ROUND_UP((x), (y)))
/*
 * This file will have a maximum of 38 bytes:
 *
 * 4 bytes: number of audio blocks
 * 4 bytes: total length of Short Audio Descriptor (SAD) blocks
 * Maximum 10 * 3 bytes: SAD blocks
 */
#define MAX_SAD_BLOCKS      10
#define SAD_BLOCK_SIZE      3

#define MAX_CVD_VERSION_STRING_SIZE    100

/* EDID format ID for LPCM audio */
#define EDID_FORMAT_LPCM    1

/* fallback app type if the default app type from acdb loader fails */
#define DEFAULT_APP_TYPE  0x11130

#define SAMPLE_RATE_8KHZ  8000
#define SAMPLE_RATE_16KHZ 16000

#define MAX_SET_CAL_BYTE_SIZE 65536

#define AUDIO_PARAMETER_KEY_FLUENCE_TYPE  "fluence"
#define AUDIO_PARAMETER_KEY_SLOWTALK      "st_enable"
#define AUDIO_PARAMETER_KEY_HD_VOICE      "hd_voice"
#define AUDIO_PARAMETER_KEY_VOLUME_BOOST  "volume_boost"
#define AUDIO_PARAMETER_KEY_AUD_CALDATA   "cal_data"
#define AUDIO_PARAMETER_KEY_AUD_CALRESULT "cal_result"

#define AUDIO_PARAMETER_KEY_MONO_SPEAKER "mono_speaker"

/* Query external audio device connection status */
#define AUDIO_PARAMETER_KEY_EXT_AUDIO_DEVICE "ext_audio_device"

#define EVENT_EXTERNAL_SPK_1 "qc_ext_spk_1"
#define EVENT_EXTERNAL_SPK_2 "qc_ext_spk_2"
#define EVENT_EXTERNAL_MIC   "qc_ext_mic"
#define MAX_CAL_NAME 20
#define MAX_MIME_TYPE_LENGTH 30

char cal_name_info[WCD9XXX_MAX_CAL][MAX_CAL_NAME] = {
        [WCD9XXX_ANC_CAL] = "anc_cal",
        [WCD9XXX_MBHC_CAL] = "mbhc_cal",
};

#define  AUDIO_PARAMETER_IS_HW_DECODER_SESSION_ALLOWED  "is_hw_dec_session_allowed"

char dsp_only_decoders_mime[][MAX_MIME_TYPE_LENGTH] = {
    "audio/x-ms-wma" /* wma*/ ,
    "audio/x-ms-wma-lossless" /* wma lossless */ ,
    "audio/x-ms-wma-pro" /* wma prop */ ,
    "audio/amr-wb-plus" /* amr wb plus */ ,
    "audio/alac"  /*alac */ ,
    "audio/x-ape" /*ape */,
};


enum {
	VOICE_FEATURE_SET_DEFAULT,
	VOICE_FEATURE_SET_VOLUME_BOOST
};

struct audio_block_header
{
    int reserved;
    int length;
};

typedef struct acdb_audio_cal_cfg {
    uint32_t             persist;
    uint32_t             snd_dev_id;
    audio_devices_t      dev_id;
    int32_t              acdb_dev_id;
    uint32_t             app_type;
    uint32_t             topo_id;
    uint32_t             sampling_rate;
    uint32_t             cal_type;
    uint32_t             module_id;
    uint32_t             param_id;
} acdb_audio_cal_cfg_t;

enum {
    CAL_MODE_SEND           = 0x1,
    CAL_MODE_PERSIST        = 0x2,
    CAL_MODE_RTAC           = 0x4
};

acdb_loader_get_calibration_t acdb_loader_get_calibration;

typedef struct codec_backend_cfg {
    uint32_t sample_rate;
    uint32_t bit_width;
    char     *bitwidth_mixer_ctl;
    char     *samplerate_mixer_ctl;
} codec_backend_cfg_t;

typedef struct {
    bool platform_na_prop_enabled;
    bool ui_na_prop_enabled;
} native_audio_prop;

static native_audio_prop na_props = {0, 0};
static bool supports_true_32_bit = false;

struct platform_data {
    struct audio_device *adev;
    bool fluence_in_spkr_mode;
    bool fluence_in_voice_call;
    bool fluence_in_voice_rec;
    bool fluence_in_audio_rec;
    bool external_spk_1;
    bool external_spk_2;
    bool external_mic;
    int  fluence_type;
    int  fluence_mode;
    char fluence_cap[PROPERTY_VALUE_MAX];
    bool slowtalk;
    bool hd_voice;
    bool ec_ref_enabled;
    bool is_i2s_ext_modem;
    bool is_acdb_initialized;
    int mono_speaker;
    /* Audio calibration related functions */
    void                       *acdb_handle;
    int                        voice_feature_set;
    acdb_init_t                acdb_init;
    acdb_deallocate_t          acdb_deallocate;
    acdb_send_audio_cal_t      acdb_send_audio_cal;
    acdb_set_audio_cal_t       acdb_set_audio_cal;
    acdb_get_audio_cal_t       acdb_get_audio_cal;
    acdb_send_voice_cal_t      acdb_send_voice_cal;
    acdb_reload_vocvoltable_t  acdb_reload_vocvoltable;
    acdb_get_default_app_type_t acdb_get_default_app_type;

    void *hw_info;
    struct csd_data *csd;
    void *edid_info;
    bool edid_valid;
    char ec_ref_mixer_path[64];
    codec_backend_cfg_t current_backend_cfg[MAX_CODEC_BACKENDS];
    int hw_dep_fd;
    int source_mic_type;
    int max_mic_count
};

static int pcm_device_table[AUDIO_USECASE_MAX][2] = {
    [USECASE_AUDIO_PLAYBACK_DEEP_BUFFER] = {DEEP_BUFFER_PCM_DEVICE,
                                            DEEP_BUFFER_PCM_DEVICE},
    [USECASE_AUDIO_PLAYBACK_LOW_LATENCY] = {LOWLATENCY_PCM_DEVICE,
                                           LOWLATENCY_PCM_DEVICE},
    [USECASE_AUDIO_PLAYBACK_MULTI_CH] = {MULTIMEDIA2_PCM_DEVICE,
                                         MULTIMEDIA2_PCM_DEVICE},
    [USECASE_AUDIO_PLAYBACK_OFFLOAD] =
                     {PLAYBACK_OFFLOAD_DEVICE, PLAYBACK_OFFLOAD_DEVICE},
    [USECASE_AUDIO_PLAYBACK_OFFLOAD2] =
                     {PLAYBACK_OFFLOAD_DEVICE2, PLAYBACK_OFFLOAD_DEVICE2},
#ifdef MULTIPLE_OFFLOAD_ENABLED
    [USECASE_AUDIO_PLAYBACK_OFFLOAD3] =
                     {PLAYBACK_OFFLOAD_DEVICE3, PLAYBACK_OFFLOAD_DEVICE3},
    [USECASE_AUDIO_PLAYBACK_OFFLOAD4] =
                     {PLAYBACK_OFFLOAD_DEVICE4, PLAYBACK_OFFLOAD_DEVICE4},
    [USECASE_AUDIO_PLAYBACK_OFFLOAD5] =
                     {PLAYBACK_OFFLOAD_DEVICE5, PLAYBACK_OFFLOAD_DEVICE5},
    [USECASE_AUDIO_PLAYBACK_OFFLOAD6] =
                     {PLAYBACK_OFFLOAD_DEVICE6, PLAYBACK_OFFLOAD_DEVICE6},
    [USECASE_AUDIO_PLAYBACK_OFFLOAD7] =
                     {PLAYBACK_OFFLOAD_DEVICE7, PLAYBACK_OFFLOAD_DEVICE7},
    [USECASE_AUDIO_PLAYBACK_OFFLOAD8] =
                     {PLAYBACK_OFFLOAD_DEVICE8, PLAYBACK_OFFLOAD_DEVICE8},
    [USECASE_AUDIO_PLAYBACK_OFFLOAD9] =
                     {PLAYBACK_OFFLOAD_DEVICE9, PLAYBACK_OFFLOAD_DEVICE9},
#endif
    [USECASE_AUDIO_PLAYBACK_ULL] = {MULTIMEDIA3_PCM_DEVICE,
                                    MULTIMEDIA3_PCM_DEVICE},

    [USECASE_AUDIO_RECORD] = {AUDIO_RECORD_PCM_DEVICE, AUDIO_RECORD_PCM_DEVICE},
    [USECASE_AUDIO_RECORD_COMPRESS] = {COMPRESS_CAPTURE_DEVICE, COMPRESS_CAPTURE_DEVICE},
    [USECASE_AUDIO_RECORD_LOW_LATENCY] = {LOWLATENCY_PCM_DEVICE,
                                          LOWLATENCY_PCM_DEVICE},
    [USECASE_AUDIO_RECORD_FM_VIRTUAL] = {MULTIMEDIA2_PCM_DEVICE,
                                  MULTIMEDIA2_PCM_DEVICE},
    [USECASE_AUDIO_PLAYBACK_FM] = {FM_PLAYBACK_PCM_DEVICE, FM_CAPTURE_PCM_DEVICE},
    [USECASE_AUDIO_HFP_SCO] = {HFP_PCM_RX, HFP_SCO_RX},
    [USECASE_AUDIO_HFP_SCO_WB] = {HFP_PCM_RX, HFP_SCO_RX},
    [USECASE_VOICE_CALL] = {VOICE_CALL_PCM_DEVICE, VOICE_CALL_PCM_DEVICE},
    [USECASE_VOICE2_CALL] = {VOICE2_CALL_PCM_DEVICE, VOICE2_CALL_PCM_DEVICE},
    [USECASE_VOLTE_CALL] = {VOLTE_CALL_PCM_DEVICE, VOLTE_CALL_PCM_DEVICE},
    [USECASE_QCHAT_CALL] = {QCHAT_CALL_PCM_DEVICE, QCHAT_CALL_PCM_DEVICE},
    [USECASE_VOWLAN_CALL] = {VOWLAN_CALL_PCM_DEVICE, VOWLAN_CALL_PCM_DEVICE},
    [USECASE_VOICEMMODE1_CALL] = {VOICEMMODE1_CALL_PCM_DEVICE,
                                  VOICEMMODE1_CALL_PCM_DEVICE},
    [USECASE_VOICEMMODE2_CALL] = {VOICEMMODE2_CALL_PCM_DEVICE,
                                  VOICEMMODE2_CALL_PCM_DEVICE},
    [USECASE_COMPRESS_VOIP_CALL] = {COMPRESS_VOIP_CALL_PCM_DEVICE, COMPRESS_VOIP_CALL_PCM_DEVICE},
    [USECASE_INCALL_REC_UPLINK] = {AUDIO_RECORD_PCM_DEVICE,
                                   AUDIO_RECORD_PCM_DEVICE},
    [USECASE_INCALL_REC_DOWNLINK] = {AUDIO_RECORD_PCM_DEVICE,
                                     AUDIO_RECORD_PCM_DEVICE},
    [USECASE_INCALL_REC_UPLINK_AND_DOWNLINK] = {AUDIO_RECORD_PCM_DEVICE,
                                                AUDIO_RECORD_PCM_DEVICE},
    [USECASE_INCALL_REC_UPLINK_COMPRESS] = {COMPRESS_CAPTURE_DEVICE,
                                            COMPRESS_CAPTURE_DEVICE},
    [USECASE_INCALL_REC_DOWNLINK_COMPRESS] = {COMPRESS_CAPTURE_DEVICE,
                                              COMPRESS_CAPTURE_DEVICE},
    [USECASE_INCALL_REC_UPLINK_AND_DOWNLINK_COMPRESS] = {COMPRESS_CAPTURE_DEVICE,
                                                         COMPRESS_CAPTURE_DEVICE},
    [USECASE_INCALL_MUSIC_UPLINK] = {INCALL_MUSIC_UPLINK_PCM_DEVICE,
                                     INCALL_MUSIC_UPLINK_PCM_DEVICE},
    [USECASE_INCALL_MUSIC_UPLINK2] = {INCALL_MUSIC_UPLINK2_PCM_DEVICE,
                                      INCALL_MUSIC_UPLINK2_PCM_DEVICE},
    [USECASE_AUDIO_SPKR_CALIB_RX] = {SPKR_PROT_CALIB_RX_PCM_DEVICE, -1},
    [USECASE_AUDIO_SPKR_CALIB_TX] = {-1, SPKR_PROT_CALIB_TX_PCM_DEVICE},

    [USECASE_AUDIO_PLAYBACK_AFE_PROXY] = {AFE_PROXY_PLAYBACK_PCM_DEVICE,
                                          AFE_PROXY_RECORD_PCM_DEVICE},
    [USECASE_AUDIO_RECORD_AFE_PROXY] = {AFE_PROXY_PLAYBACK_PCM_DEVICE,
                                        AFE_PROXY_RECORD_PCM_DEVICE},

};

/* Array to store sound devices */
static char * device_table[SND_DEVICE_MAX] = {
    [SND_DEVICE_NONE] = "none",
    /* Playback sound devices */
    [SND_DEVICE_OUT_HANDSET] = "handset",
    [SND_DEVICE_OUT_SPEAKER] = "speaker",
    [SND_DEVICE_OUT_SPEAKER_EXTERNAL_1] = "speaker-ext-1",
    [SND_DEVICE_OUT_SPEAKER_EXTERNAL_2] = "speaker-ext-2",
    [SND_DEVICE_OUT_SPEAKER_REVERSE] = "speaker-reverse",
    [SND_DEVICE_OUT_HEADPHONES] = "headphones",
    [SND_DEVICE_OUT_HEADPHONES_44_1] = "headphones-44.1",
    [SND_DEVICE_OUT_SPEAKER_AND_HEADPHONES] = "speaker-and-headphones",
    [SND_DEVICE_OUT_SPEAKER_AND_HEADPHONES_EXTERNAL_1] = "speaker-and-headphones-ext-1",
    [SND_DEVICE_OUT_SPEAKER_AND_HEADPHONES_EXTERNAL_2] = "speaker-and-headphones-ext-2",
    [SND_DEVICE_OUT_VOICE_HANDSET] = "voice-handset",
    [SND_DEVICE_OUT_VOICE_SPEAKER] = "voice-speaker",
    [SND_DEVICE_OUT_VOICE_SPEAKER_2] = "voice-speaker-2",
    [SND_DEVICE_OUT_VOICE_HEADPHONES] = "voice-headphones",
    [SND_DEVICE_OUT_HDMI] = "hdmi",
    [SND_DEVICE_OUT_SPEAKER_AND_HDMI] = "speaker-and-hdmi",
    [SND_DEVICE_OUT_BT_SCO] = "bt-sco-headset",
    [SND_DEVICE_OUT_BT_SCO_WB] = "bt-sco-headset-wb",
    [SND_DEVICE_OUT_VOICE_TTY_FULL_HEADPHONES] = "voice-tty-full-headphones",
    [SND_DEVICE_OUT_VOICE_TTY_VCO_HEADPHONES] = "voice-tty-vco-headphones",
    [SND_DEVICE_OUT_VOICE_TTY_HCO_HANDSET] = "voice-tty-hco-handset",
    [SND_DEVICE_OUT_VOICE_TX] = "voice-tx",
    [SND_DEVICE_OUT_AFE_PROXY] = "afe-proxy",
    [SND_DEVICE_OUT_USB_HEADSET] = "usb-headphones",
    [SND_DEVICE_OUT_SPEAKER_AND_USB_HEADSET] = "speaker-and-usb-headphones",
    [SND_DEVICE_OUT_TRANSMISSION_FM] = "transmission-fm",
    [SND_DEVICE_OUT_ANC_HEADSET] = "anc-headphones",
    [SND_DEVICE_OUT_ANC_FB_HEADSET] = "anc-fb-headphones",
    [SND_DEVICE_OUT_VOICE_ANC_HEADSET] = "voice-anc-headphones",
    [SND_DEVICE_OUT_VOICE_ANC_FB_HEADSET] = "voice-anc-fb-headphones",
    [SND_DEVICE_OUT_SPEAKER_AND_ANC_HEADSET] = "speaker-and-anc-headphones",
    [SND_DEVICE_OUT_ANC_HANDSET] = "anc-handset",
    [SND_DEVICE_OUT_SPEAKER_PROTECTED] = "speaker-protected",
    [SND_DEVICE_OUT_VOICE_SPEAKER_PROTECTED] = "voice-speaker-protected",
    [SND_DEVICE_OUT_VOICE_SPEAKER_2_PROTECTED] = "voice-speaker-2-protected",

    /* Capture sound devices */
    [SND_DEVICE_IN_HANDSET_MIC] = "handset-mic",
    [SND_DEVICE_IN_HANDSET_MIC_EXTERNAL] = "handset-mic-ext",
    [SND_DEVICE_IN_HANDSET_MIC_AEC] = "handset-mic",
    [SND_DEVICE_IN_HANDSET_MIC_NS] = "handset-mic",
    [SND_DEVICE_IN_HANDSET_MIC_AEC_NS] = "handset-mic",
    [SND_DEVICE_IN_HANDSET_DMIC] = "dmic-endfire",
    [SND_DEVICE_IN_HANDSET_DMIC_AEC] = "dmic-endfire",
    [SND_DEVICE_IN_HANDSET_DMIC_NS] = "dmic-endfire",
    [SND_DEVICE_IN_HANDSET_DMIC_AEC_NS] = "dmic-endfire",
    [SND_DEVICE_IN_SPEAKER_MIC] = "speaker-mic",
    [SND_DEVICE_IN_SPEAKER_MIC_AEC] = "speaker-mic",
    [SND_DEVICE_IN_SPEAKER_MIC_NS] = "speaker-mic",
    [SND_DEVICE_IN_SPEAKER_MIC_AEC_NS] = "speaker-mic",
    [SND_DEVICE_IN_SPEAKER_DMIC] = "speaker-dmic-endfire",
    [SND_DEVICE_IN_SPEAKER_DMIC_AEC] = "speaker-dmic-endfire",
    [SND_DEVICE_IN_SPEAKER_DMIC_NS] = "speaker-dmic-endfire",
    [SND_DEVICE_IN_SPEAKER_DMIC_AEC_NS] = "speaker-dmic-endfire",
    [SND_DEVICE_IN_HEADSET_MIC] = "headset-mic",
    [SND_DEVICE_IN_HEADSET_MIC_FLUENCE] = "headset-mic",
    [SND_DEVICE_IN_VOICE_SPEAKER_MIC] = "voice-speaker-mic",
    [SND_DEVICE_IN_VOICE_HEADSET_MIC] = "voice-headset-mic",
    [SND_DEVICE_IN_HDMI_MIC] = "hdmi-mic",
    [SND_DEVICE_IN_BT_SCO_MIC] = "bt-sco-mic",
    [SND_DEVICE_IN_BT_SCO_MIC_NREC] = "bt-sco-mic",
    [SND_DEVICE_IN_BT_SCO_MIC_WB] = "bt-sco-mic-wb",
    [SND_DEVICE_IN_BT_SCO_MIC_WB_NREC] = "bt-sco-mic-wb",
    [SND_DEVICE_IN_CAMCORDER_MIC] = "camcorder-mic",
    [SND_DEVICE_IN_VOICE_DMIC] = "voice-dmic-ef",
    [SND_DEVICE_IN_VOICE_SPEAKER_DMIC] = "voice-speaker-dmic-ef",
    [SND_DEVICE_IN_VOICE_SPEAKER_QMIC] = "voice-speaker-qmic",
    [SND_DEVICE_IN_VOICE_TTY_FULL_HEADSET_MIC] = "voice-tty-full-headset-mic",
    [SND_DEVICE_IN_VOICE_TTY_VCO_HANDSET_MIC] = "voice-tty-vco-handset-mic",
    [SND_DEVICE_IN_VOICE_TTY_HCO_HEADSET_MIC] = "voice-tty-hco-headset-mic",
    [SND_DEVICE_IN_VOICE_RX] = "voice-rx",

    [SND_DEVICE_IN_VOICE_REC_MIC] = "voice-rec-mic",
    [SND_DEVICE_IN_VOICE_REC_MIC_NS] = "voice-rec-mic",
    [SND_DEVICE_IN_VOICE_REC_DMIC_STEREO] = "voice-rec-dmic-ef",
    [SND_DEVICE_IN_VOICE_REC_DMIC_FLUENCE] = "voice-rec-dmic-ef-fluence",
    [SND_DEVICE_IN_USB_HEADSET_MIC] = "usb-headset-mic",
    [SND_DEVICE_IN_CAPTURE_FM] = "capture-fm",
    [SND_DEVICE_IN_AANC_HANDSET_MIC] = "aanc-handset-mic",
    [SND_DEVICE_IN_QUAD_MIC] = "quad-mic",
    [SND_DEVICE_IN_HANDSET_STEREO_DMIC] = "handset-stereo-dmic-ef",
    [SND_DEVICE_IN_SPEAKER_STEREO_DMIC] = "speaker-stereo-dmic-ef",
    [SND_DEVICE_IN_CAPTURE_VI_FEEDBACK] = "vi-feedback",
    [SND_DEVICE_IN_VOICE_SPEAKER_DMIC_BROADSIDE] = "voice-speaker-dmic-broadside",
    [SND_DEVICE_IN_SPEAKER_DMIC_BROADSIDE] = "speaker-dmic-broadside",
    [SND_DEVICE_IN_SPEAKER_DMIC_AEC_BROADSIDE] = "speaker-dmic-broadside",
    [SND_DEVICE_IN_SPEAKER_DMIC_NS_BROADSIDE] = "speaker-dmic-broadside",
    [SND_DEVICE_IN_SPEAKER_DMIC_AEC_NS_BROADSIDE] = "speaker-dmic-broadside",
    [SND_DEVICE_IN_HANDSET_QMIC] = "quad-mic",
    [SND_DEVICE_IN_SPEAKER_QMIC_AEC] = "quad-mic",
    [SND_DEVICE_IN_SPEAKER_QMIC_NS] = "quad-mic",
    [SND_DEVICE_IN_SPEAKER_QMIC_AEC_NS] = "quad-mic",
    [SND_DEVICE_IN_THREE_MIC] = "three-mic",
    [SND_DEVICE_IN_HANDSET_TMIC] = "three-mic",
    [SND_DEVICE_IN_UNPROCESSED_MIC] = "unprocessed-mic",
    [SND_DEVICE_IN_UNPROCESSED_STEREO_MIC] = "voice-rec-dmic-ef",
    [SND_DEVICE_IN_UNPROCESSED_THREE_MIC] = "three-mic",
    [SND_DEVICE_IN_UNPROCESSED_QUAD_MIC] = "quad-mic",
    [SND_DEVICE_IN_UNPROCESSED_HEADSET_MIC] = "headset-mic",
};

// Platform specific backend bit width table
static int backend_bit_width_table[SND_DEVICE_MAX] = {0};

/* ACDB IDs (audio DSP path configuration IDs) for each sound device */
static int acdb_device_table[SND_DEVICE_MAX] = {
    [SND_DEVICE_NONE] = -1,
    [SND_DEVICE_OUT_HANDSET] = 7,
    [SND_DEVICE_OUT_SPEAKER] = 14,
    [SND_DEVICE_OUT_SPEAKER_EXTERNAL_1] = 130,
    [SND_DEVICE_OUT_SPEAKER_EXTERNAL_2] = 130,
    [SND_DEVICE_OUT_SPEAKER_REVERSE] = 14,
    [SND_DEVICE_OUT_HEADPHONES] = 10,
    [SND_DEVICE_OUT_HEADPHONES_44_1] = 10,
    [SND_DEVICE_OUT_SPEAKER_AND_HEADPHONES] = 10,
    [SND_DEVICE_OUT_SPEAKER_AND_HEADPHONES_EXTERNAL_1] = 130,
    [SND_DEVICE_OUT_SPEAKER_AND_HEADPHONES_EXTERNAL_2] = 130,
    [SND_DEVICE_OUT_VOICE_HANDSET] = 7,
    [SND_DEVICE_OUT_VOICE_SPEAKER] = 14,
    [SND_DEVICE_OUT_VOICE_SPEAKER_2] = 14,
    [SND_DEVICE_OUT_VOICE_HEADPHONES] = 10,
    [SND_DEVICE_OUT_HDMI] = 18,
    [SND_DEVICE_OUT_SPEAKER_AND_HDMI] = 14,
    [SND_DEVICE_OUT_BT_SCO] = 22,
    [SND_DEVICE_OUT_BT_SCO_WB] = 39,
    [SND_DEVICE_OUT_VOICE_TTY_FULL_HEADPHONES] = 17,
    [SND_DEVICE_OUT_VOICE_TTY_VCO_HEADPHONES] = 17,
    [SND_DEVICE_OUT_VOICE_TTY_HCO_HANDSET] = 37,
    [SND_DEVICE_OUT_VOICE_TX] = 45,
    [SND_DEVICE_OUT_AFE_PROXY] = 0,
    [SND_DEVICE_OUT_USB_HEADSET] = 45,
    [SND_DEVICE_OUT_SPEAKER_AND_USB_HEADSET] = 14,
    [SND_DEVICE_OUT_TRANSMISSION_FM] = 0,
    [SND_DEVICE_OUT_ANC_HEADSET] = 26,
    [SND_DEVICE_OUT_ANC_FB_HEADSET] = 27,
    [SND_DEVICE_OUT_VOICE_ANC_HEADSET] = 26,
    [SND_DEVICE_OUT_VOICE_ANC_FB_HEADSET] = 27,
    [SND_DEVICE_OUT_SPEAKER_AND_ANC_HEADSET] = 26,
    [SND_DEVICE_OUT_ANC_HANDSET] = 103,
    [SND_DEVICE_OUT_SPEAKER_PROTECTED] = 124,
    [SND_DEVICE_OUT_VOICE_SPEAKER_PROTECTED] = 101,
    [SND_DEVICE_OUT_VOICE_SPEAKER_2_PROTECTED] = 101,

    [SND_DEVICE_IN_HANDSET_MIC] = 4,
    [SND_DEVICE_IN_HANDSET_MIC_EXTERNAL] = 4,
    [SND_DEVICE_IN_HANDSET_MIC_AEC] = 106,
    [SND_DEVICE_IN_HANDSET_MIC_NS] = 107,
    [SND_DEVICE_IN_HANDSET_MIC_AEC_NS] = 108,
    [SND_DEVICE_IN_HANDSET_DMIC] = 41,
    [SND_DEVICE_IN_HANDSET_DMIC_AEC] = 109,
    [SND_DEVICE_IN_HANDSET_DMIC_NS] = 110,
    [SND_DEVICE_IN_HANDSET_DMIC_AEC_NS] = 111,
    [SND_DEVICE_IN_SPEAKER_MIC] = 11,
    [SND_DEVICE_IN_SPEAKER_MIC_AEC] = 112,
    [SND_DEVICE_IN_SPEAKER_MIC_NS] = 113,
    [SND_DEVICE_IN_SPEAKER_MIC_AEC_NS] = 114,
    [SND_DEVICE_IN_SPEAKER_DMIC] = 43,
    [SND_DEVICE_IN_SPEAKER_DMIC_AEC] = 115,
    [SND_DEVICE_IN_SPEAKER_DMIC_NS] = 116,
    [SND_DEVICE_IN_SPEAKER_DMIC_AEC_NS] = 117,
    [SND_DEVICE_IN_HEADSET_MIC] = 8,
    [SND_DEVICE_IN_HEADSET_MIC_FLUENCE] = 47,
    [SND_DEVICE_IN_VOICE_SPEAKER_MIC] = 11,
    [SND_DEVICE_IN_VOICE_HEADSET_MIC] = 8,
    [SND_DEVICE_IN_HDMI_MIC] = 4,
    [SND_DEVICE_IN_BT_SCO_MIC] = 21,
    [SND_DEVICE_IN_BT_SCO_MIC_NREC] = 122,
    [SND_DEVICE_IN_BT_SCO_MIC_WB] = 38,
    [SND_DEVICE_IN_BT_SCO_MIC_WB_NREC] = 123,
    [SND_DEVICE_IN_CAMCORDER_MIC] = 4,
    [SND_DEVICE_IN_VOICE_DMIC] = 41,
    [SND_DEVICE_IN_VOICE_SPEAKER_DMIC] = 43,
    [SND_DEVICE_IN_VOICE_SPEAKER_QMIC] = 19,
    [SND_DEVICE_IN_VOICE_TTY_FULL_HEADSET_MIC] = 16,
    [SND_DEVICE_IN_VOICE_TTY_VCO_HANDSET_MIC] = 36,
    [SND_DEVICE_IN_VOICE_TTY_HCO_HEADSET_MIC] = 16,
    [SND_DEVICE_IN_VOICE_RX] = 44,

    [SND_DEVICE_IN_VOICE_REC_MIC] = 4,
    [SND_DEVICE_IN_VOICE_REC_MIC_NS] = 107,
    [SND_DEVICE_IN_VOICE_REC_DMIC_STEREO] = 34,
    [SND_DEVICE_IN_VOICE_REC_DMIC_FLUENCE] = 41,
    [SND_DEVICE_IN_USB_HEADSET_MIC] = 44,
    [SND_DEVICE_IN_CAPTURE_FM] = 0,
    [SND_DEVICE_IN_AANC_HANDSET_MIC] = 104,
    [SND_DEVICE_IN_QUAD_MIC] = 46,
    [SND_DEVICE_IN_HANDSET_STEREO_DMIC] = 34,
    [SND_DEVICE_IN_SPEAKER_STEREO_DMIC] = 35,
    [SND_DEVICE_IN_CAPTURE_VI_FEEDBACK] = 102,
    [SND_DEVICE_IN_CAPTURE_VI_FEEDBACK_MONO_1] = 102,
    [SND_DEVICE_IN_CAPTURE_VI_FEEDBACK_MONO_2] = 102,
    [SND_DEVICE_IN_VOICE_SPEAKER_DMIC_BROADSIDE] = 12,
    [SND_DEVICE_IN_SPEAKER_DMIC_BROADSIDE] = 12,
    [SND_DEVICE_IN_SPEAKER_DMIC_AEC_BROADSIDE] = 119,
    [SND_DEVICE_IN_SPEAKER_DMIC_NS_BROADSIDE] = 121,
    [SND_DEVICE_IN_SPEAKER_DMIC_AEC_NS_BROADSIDE] = 120,
    [SND_DEVICE_IN_HANDSET_QMIC] = 125,
    [SND_DEVICE_IN_SPEAKER_QMIC_AEC] = 126,
    [SND_DEVICE_IN_SPEAKER_QMIC_NS] = 127,
    [SND_DEVICE_IN_SPEAKER_QMIC_AEC_NS] = 129,
    [SND_DEVICE_IN_THREE_MIC] = 46, /* for APSS Surround Sound Recording */
    [SND_DEVICE_IN_HANDSET_TMIC] = 125, /* for 3mic recording with fluence */
    [SND_DEVICE_IN_UNPROCESSED_MIC] = 143,
    [SND_DEVICE_IN_UNPROCESSED_STEREO_MIC] = 144,
    [SND_DEVICE_IN_UNPROCESSED_THREE_MIC] = 145,
    [SND_DEVICE_IN_UNPROCESSED_QUAD_MIC] = 146,
    [SND_DEVICE_IN_UNPROCESSED_HEADSET_MIC] = 147,
};

struct name_to_index {
    char name[100];
    unsigned int index;
};

#define TO_NAME_INDEX(X)   #X, X

/* Used to get index from parsed string */
static struct name_to_index snd_device_name_index[SND_DEVICE_MAX] = {
    {TO_NAME_INDEX(SND_DEVICE_OUT_HANDSET)},
    {TO_NAME_INDEX(SND_DEVICE_OUT_SPEAKER)},
    {TO_NAME_INDEX(SND_DEVICE_OUT_SPEAKER_EXTERNAL_1)},
    {TO_NAME_INDEX(SND_DEVICE_OUT_SPEAKER_EXTERNAL_2)},
    {TO_NAME_INDEX(SND_DEVICE_OUT_SPEAKER_REVERSE)},
    {TO_NAME_INDEX(SND_DEVICE_OUT_HEADPHONES)},
    {TO_NAME_INDEX(SND_DEVICE_OUT_HEADPHONES_44_1)},
    {TO_NAME_INDEX(SND_DEVICE_OUT_SPEAKER_AND_HEADPHONES)},
    {TO_NAME_INDEX(SND_DEVICE_OUT_SPEAKER_AND_HEADPHONES_EXTERNAL_1)},
    {TO_NAME_INDEX(SND_DEVICE_OUT_SPEAKER_AND_HEADPHONES_EXTERNAL_2)},
    {TO_NAME_INDEX(SND_DEVICE_OUT_VOICE_HANDSET)},
    {TO_NAME_INDEX(SND_DEVICE_OUT_VOICE_SPEAKER)},
    {TO_NAME_INDEX(SND_DEVICE_OUT_VOICE_SPEAKER_2)},
    {TO_NAME_INDEX(SND_DEVICE_OUT_VOICE_HEADPHONES)},
    {TO_NAME_INDEX(SND_DEVICE_OUT_HDMI)},
    {TO_NAME_INDEX(SND_DEVICE_OUT_SPEAKER_AND_HDMI)},
    {TO_NAME_INDEX(SND_DEVICE_OUT_BT_SCO)},
    {TO_NAME_INDEX(SND_DEVICE_OUT_BT_SCO_WB)},
    {TO_NAME_INDEX(SND_DEVICE_OUT_VOICE_TTY_FULL_HEADPHONES)},
    {TO_NAME_INDEX(SND_DEVICE_OUT_VOICE_TTY_VCO_HEADPHONES)},
    {TO_NAME_INDEX(SND_DEVICE_OUT_VOICE_TTY_HCO_HANDSET)},
    {TO_NAME_INDEX(SND_DEVICE_OUT_AFE_PROXY)},
    {TO_NAME_INDEX(SND_DEVICE_OUT_USB_HEADSET)},
    {TO_NAME_INDEX(SND_DEVICE_OUT_SPEAKER_AND_USB_HEADSET)},
    {TO_NAME_INDEX(SND_DEVICE_OUT_TRANSMISSION_FM)},
    {TO_NAME_INDEX(SND_DEVICE_OUT_ANC_HEADSET)},
    {TO_NAME_INDEX(SND_DEVICE_OUT_ANC_FB_HEADSET)},
    {TO_NAME_INDEX(SND_DEVICE_OUT_VOICE_ANC_HEADSET)},
    {TO_NAME_INDEX(SND_DEVICE_OUT_VOICE_ANC_FB_HEADSET)},
    {TO_NAME_INDEX(SND_DEVICE_OUT_SPEAKER_AND_ANC_HEADSET)},
    {TO_NAME_INDEX(SND_DEVICE_OUT_ANC_HANDSET)},
    {TO_NAME_INDEX(SND_DEVICE_OUT_SPEAKER_PROTECTED)},
    {TO_NAME_INDEX(SND_DEVICE_OUT_VOICE_SPEAKER_PROTECTED)},
    {TO_NAME_INDEX(SND_DEVICE_OUT_VOICE_SPEAKER_2_PROTECTED)},
    {TO_NAME_INDEX(SND_DEVICE_IN_HANDSET_MIC)},
    {TO_NAME_INDEX(SND_DEVICE_IN_HANDSET_MIC_EXTERNAL)},
    {TO_NAME_INDEX(SND_DEVICE_IN_HANDSET_MIC_AEC)},
    {TO_NAME_INDEX(SND_DEVICE_IN_HANDSET_MIC_NS)},
    {TO_NAME_INDEX(SND_DEVICE_IN_HANDSET_MIC_AEC_NS)},
    {TO_NAME_INDEX(SND_DEVICE_IN_HANDSET_DMIC)},
    {TO_NAME_INDEX(SND_DEVICE_IN_HANDSET_DMIC_AEC)},
    {TO_NAME_INDEX(SND_DEVICE_IN_HANDSET_DMIC_NS)},
    {TO_NAME_INDEX(SND_DEVICE_IN_HANDSET_DMIC_AEC_NS)},
    {TO_NAME_INDEX(SND_DEVICE_IN_SPEAKER_MIC)},
    {TO_NAME_INDEX(SND_DEVICE_IN_SPEAKER_MIC_AEC)},
    {TO_NAME_INDEX(SND_DEVICE_IN_SPEAKER_MIC_NS)},
    {TO_NAME_INDEX(SND_DEVICE_IN_SPEAKER_MIC_AEC_NS)},
    {TO_NAME_INDEX(SND_DEVICE_IN_SPEAKER_DMIC)},
    {TO_NAME_INDEX(SND_DEVICE_IN_SPEAKER_DMIC_AEC)},
    {TO_NAME_INDEX(SND_DEVICE_IN_SPEAKER_DMIC_NS)},
    {TO_NAME_INDEX(SND_DEVICE_IN_SPEAKER_DMIC_AEC_NS)},
    {TO_NAME_INDEX(SND_DEVICE_IN_HEADSET_MIC)},
    {TO_NAME_INDEX(SND_DEVICE_IN_HEADSET_MIC_FLUENCE)},
    {TO_NAME_INDEX(SND_DEVICE_IN_VOICE_SPEAKER_MIC)},
    {TO_NAME_INDEX(SND_DEVICE_IN_VOICE_HEADSET_MIC)},
    {TO_NAME_INDEX(SND_DEVICE_IN_HDMI_MIC)},
    {TO_NAME_INDEX(SND_DEVICE_IN_BT_SCO_MIC)},
    {TO_NAME_INDEX(SND_DEVICE_IN_BT_SCO_MIC_NREC)},
    {TO_NAME_INDEX(SND_DEVICE_IN_BT_SCO_MIC_WB)},
    {TO_NAME_INDEX(SND_DEVICE_IN_BT_SCO_MIC_WB_NREC)},
    {TO_NAME_INDEX(SND_DEVICE_IN_CAMCORDER_MIC)},
    {TO_NAME_INDEX(SND_DEVICE_IN_VOICE_DMIC)},
    {TO_NAME_INDEX(SND_DEVICE_IN_VOICE_SPEAKER_DMIC)},
    {TO_NAME_INDEX(SND_DEVICE_IN_VOICE_SPEAKER_QMIC)},
    {TO_NAME_INDEX(SND_DEVICE_IN_VOICE_TTY_FULL_HEADSET_MIC)},
    {TO_NAME_INDEX(SND_DEVICE_IN_VOICE_TTY_VCO_HANDSET_MIC)},
    {TO_NAME_INDEX(SND_DEVICE_IN_VOICE_TTY_HCO_HEADSET_MIC)},
    {TO_NAME_INDEX(SND_DEVICE_IN_VOICE_REC_MIC)},
    {TO_NAME_INDEX(SND_DEVICE_IN_VOICE_REC_MIC_NS)},
    {TO_NAME_INDEX(SND_DEVICE_IN_VOICE_REC_DMIC_STEREO)},
    {TO_NAME_INDEX(SND_DEVICE_IN_VOICE_REC_DMIC_FLUENCE)},
    {TO_NAME_INDEX(SND_DEVICE_IN_USB_HEADSET_MIC)},
    {TO_NAME_INDEX(SND_DEVICE_IN_CAPTURE_FM)},
    {TO_NAME_INDEX(SND_DEVICE_IN_AANC_HANDSET_MIC)},
    {TO_NAME_INDEX(SND_DEVICE_IN_QUAD_MIC)},
    {TO_NAME_INDEX(SND_DEVICE_IN_HANDSET_STEREO_DMIC)},
    {TO_NAME_INDEX(SND_DEVICE_IN_SPEAKER_STEREO_DMIC)},
    {TO_NAME_INDEX(SND_DEVICE_IN_CAPTURE_VI_FEEDBACK)},
    {TO_NAME_INDEX(SND_DEVICE_IN_CAPTURE_VI_FEEDBACK_MONO_1)},
    {TO_NAME_INDEX(SND_DEVICE_IN_CAPTURE_VI_FEEDBACK_MONO_2)},
    {TO_NAME_INDEX(SND_DEVICE_IN_VOICE_SPEAKER_DMIC_BROADSIDE)},
    {TO_NAME_INDEX(SND_DEVICE_IN_SPEAKER_DMIC_BROADSIDE)},
    {TO_NAME_INDEX(SND_DEVICE_IN_SPEAKER_DMIC_AEC_BROADSIDE)},
    {TO_NAME_INDEX(SND_DEVICE_IN_SPEAKER_DMIC_NS_BROADSIDE)},
    {TO_NAME_INDEX(SND_DEVICE_IN_SPEAKER_DMIC_AEC_NS_BROADSIDE)},
    {TO_NAME_INDEX(SND_DEVICE_IN_HANDSET_QMIC)},
    {TO_NAME_INDEX(SND_DEVICE_IN_SPEAKER_QMIC_AEC)},
    {TO_NAME_INDEX(SND_DEVICE_IN_SPEAKER_QMIC_NS)},
    {TO_NAME_INDEX(SND_DEVICE_IN_SPEAKER_QMIC_AEC_NS)},
    {TO_NAME_INDEX(SND_DEVICE_IN_THREE_MIC)},
    {TO_NAME_INDEX(SND_DEVICE_IN_HANDSET_TMIC)},
    {TO_NAME_INDEX(SND_DEVICE_IN_UNPROCESSED_MIC)},
    {TO_NAME_INDEX(SND_DEVICE_IN_UNPROCESSED_STEREO_MIC)},
    {TO_NAME_INDEX(SND_DEVICE_IN_UNPROCESSED_THREE_MIC)},
    {TO_NAME_INDEX(SND_DEVICE_IN_UNPROCESSED_QUAD_MIC)},
    {TO_NAME_INDEX(SND_DEVICE_IN_UNPROCESSED_HEADSET_MIC)},
};

static char * backend_table[SND_DEVICE_MAX] = {0};

static struct name_to_index usecase_name_index[AUDIO_USECASE_MAX] = {
    {TO_NAME_INDEX(USECASE_AUDIO_PLAYBACK_DEEP_BUFFER)},
    {TO_NAME_INDEX(USECASE_AUDIO_PLAYBACK_LOW_LATENCY)},
    {TO_NAME_INDEX(USECASE_AUDIO_PLAYBACK_MULTI_CH)},
    {TO_NAME_INDEX(USECASE_AUDIO_PLAYBACK_OFFLOAD)},
    {TO_NAME_INDEX(USECASE_AUDIO_PLAYBACK_OFFLOAD2)},
#ifdef MULTIPLE_OFFLOAD_ENABLED
    {TO_NAME_INDEX(USECASE_AUDIO_PLAYBACK_OFFLOAD3)},
    {TO_NAME_INDEX(USECASE_AUDIO_PLAYBACK_OFFLOAD4)},
    {TO_NAME_INDEX(USECASE_AUDIO_PLAYBACK_OFFLOAD5)},
    {TO_NAME_INDEX(USECASE_AUDIO_PLAYBACK_OFFLOAD6)},
    {TO_NAME_INDEX(USECASE_AUDIO_PLAYBACK_OFFLOAD7)},
    {TO_NAME_INDEX(USECASE_AUDIO_PLAYBACK_OFFLOAD8)},
    {TO_NAME_INDEX(USECASE_AUDIO_PLAYBACK_OFFLOAD9)},
#endif
    {TO_NAME_INDEX(USECASE_AUDIO_PLAYBACK_ULL)},
    {TO_NAME_INDEX(USECASE_AUDIO_RECORD)},
    {TO_NAME_INDEX(USECASE_AUDIO_RECORD_LOW_LATENCY)},
    {TO_NAME_INDEX(USECASE_VOICE_CALL)},
    {TO_NAME_INDEX(USECASE_VOICE2_CALL)},
    {TO_NAME_INDEX(USECASE_VOLTE_CALL)},
    {TO_NAME_INDEX(USECASE_QCHAT_CALL)},
    {TO_NAME_INDEX(USECASE_VOWLAN_CALL)},
    {TO_NAME_INDEX(USECASE_INCALL_REC_UPLINK)},
    {TO_NAME_INDEX(USECASE_INCALL_REC_DOWNLINK)},
    {TO_NAME_INDEX(USECASE_INCALL_REC_UPLINK_AND_DOWNLINK)},
    {TO_NAME_INDEX(USECASE_AUDIO_HFP_SCO)},
};

#define NO_COLS 2
#ifdef PLATFORM_APQ8084
static int msm_device_to_be_id [][NO_COLS] = {
       {AUDIO_DEVICE_OUT_EARPIECE                       ,       2},
       {AUDIO_DEVICE_OUT_SPEAKER                        ,       2},
       {AUDIO_DEVICE_OUT_WIRED_HEADSET                  ,       2},
       {AUDIO_DEVICE_OUT_WIRED_HEADPHONE                ,       2},
       {AUDIO_DEVICE_OUT_BLUETOOTH_SCO                  ,       11},
       {AUDIO_DEVICE_OUT_BLUETOOTH_SCO_HEADSET          ,       11},
       {AUDIO_DEVICE_OUT_BLUETOOTH_SCO_CARKIT           ,       11},
       {AUDIO_DEVICE_OUT_BLUETOOTH_A2DP                 ,       -1},
       {AUDIO_DEVICE_OUT_BLUETOOTH_A2DP_HEADPHONES      ,       -1},
       {AUDIO_DEVICE_OUT_BLUETOOTH_A2DP_SPEAKER         ,       -1},
       {AUDIO_DEVICE_OUT_AUX_DIGITAL                    ,       4},
       {AUDIO_DEVICE_OUT_ANLG_DOCK_HEADSET              ,       9},
       {AUDIO_DEVICE_OUT_DGTL_DOCK_HEADSET              ,       9},
       {AUDIO_DEVICE_OUT_USB_ACCESSORY                  ,       -1},
       {AUDIO_DEVICE_OUT_USB_DEVICE                     ,       -1},
       {AUDIO_DEVICE_OUT_REMOTE_SUBMIX                  ,       9},
       {AUDIO_DEVICE_OUT_PROXY                          ,       9},
       {AUDIO_DEVICE_OUT_FM                             ,       7},
       {AUDIO_DEVICE_OUT_FM_TX                          ,       8},
       {AUDIO_DEVICE_OUT_ALL                            ,      -1},
       {AUDIO_DEVICE_NONE                               ,      -1},
       {AUDIO_DEVICE_OUT_DEFAULT                        ,      -1},
};
#elif PLATFORM_MSM8994
static int msm_device_to_be_id [][NO_COLS] = {
       {AUDIO_DEVICE_OUT_EARPIECE                       ,       2},
       {AUDIO_DEVICE_OUT_SPEAKER                        ,       2},
       {AUDIO_DEVICE_OUT_WIRED_HEADSET                  ,       2},
       {AUDIO_DEVICE_OUT_WIRED_HEADPHONE                ,       2},
       {AUDIO_DEVICE_OUT_BLUETOOTH_SCO                  ,       38},
       {AUDIO_DEVICE_OUT_BLUETOOTH_SCO_HEADSET          ,       38},
       {AUDIO_DEVICE_OUT_BLUETOOTH_SCO_CARKIT           ,       38},
       {AUDIO_DEVICE_OUT_BLUETOOTH_A2DP                 ,       -1},
       {AUDIO_DEVICE_OUT_BLUETOOTH_A2DP_HEADPHONES      ,       -1},
       {AUDIO_DEVICE_OUT_BLUETOOTH_A2DP_SPEAKER         ,       -1},
       {AUDIO_DEVICE_OUT_AUX_DIGITAL                    ,       4},
       {AUDIO_DEVICE_OUT_ANLG_DOCK_HEADSET              ,       9},
       {AUDIO_DEVICE_OUT_DGTL_DOCK_HEADSET              ,       9},
       {AUDIO_DEVICE_OUT_USB_ACCESSORY                  ,       -1},
       {AUDIO_DEVICE_OUT_USB_DEVICE                     ,       -1},
       {AUDIO_DEVICE_OUT_REMOTE_SUBMIX                  ,       9},
       {AUDIO_DEVICE_OUT_PROXY                          ,       9},
/* Add the correct be ids */
       {AUDIO_DEVICE_OUT_FM                             ,       7},
       {AUDIO_DEVICE_OUT_FM_TX                          ,       8},
       {AUDIO_DEVICE_OUT_ALL                            ,      -1},
       {AUDIO_DEVICE_NONE                               ,      -1},
       {AUDIO_DEVICE_OUT_DEFAULT                        ,      -1},
};
#elif PLATFORM_MSM8996
static int msm_device_to_be_id [][NO_COLS] = {
       {AUDIO_DEVICE_OUT_EARPIECE                       ,       2},
       {AUDIO_DEVICE_OUT_SPEAKER                        ,       2},
       {AUDIO_DEVICE_OUT_WIRED_HEADSET                  ,       2},
       {AUDIO_DEVICE_OUT_WIRED_HEADPHONE                ,       2},
       {AUDIO_DEVICE_OUT_BLUETOOTH_SCO                  ,       11},
       {AUDIO_DEVICE_OUT_BLUETOOTH_SCO_HEADSET          ,       11},
       {AUDIO_DEVICE_OUT_BLUETOOTH_SCO_CARKIT           ,       11},
       {AUDIO_DEVICE_OUT_BLUETOOTH_A2DP                 ,       -1},
       {AUDIO_DEVICE_OUT_BLUETOOTH_A2DP_HEADPHONES      ,       -1},
       {AUDIO_DEVICE_OUT_BLUETOOTH_A2DP_SPEAKER         ,       -1},
       {AUDIO_DEVICE_OUT_AUX_DIGITAL                    ,       4},
       {AUDIO_DEVICE_OUT_ANLG_DOCK_HEADSET              ,       9},
       {AUDIO_DEVICE_OUT_DGTL_DOCK_HEADSET              ,       9},
       {AUDIO_DEVICE_OUT_USB_ACCESSORY                  ,       -1},
       {AUDIO_DEVICE_OUT_USB_DEVICE                     ,       -1},
       {AUDIO_DEVICE_OUT_REMOTE_SUBMIX                  ,       9},
       {AUDIO_DEVICE_OUT_PROXY                          ,       9},
/* Add the correct be ids */
       {AUDIO_DEVICE_OUT_FM                             ,       7},
       {AUDIO_DEVICE_OUT_FM_TX                          ,       8},
       {AUDIO_DEVICE_OUT_ALL                            ,      -1},
       {AUDIO_DEVICE_NONE                               ,      -1},
       {AUDIO_DEVICE_OUT_DEFAULT                        ,      -1},
};
#else
static int msm_device_to_be_id [][NO_COLS] = {
    {AUDIO_DEVICE_NONE, -1},
};
#endif
static int msm_be_id_array_len  =
    sizeof(msm_device_to_be_id) / sizeof(msm_device_to_be_id[0]);


#define DEEP_BUFFER_PLATFORM_DELAY (29*1000LL)
#define PCM_OFFLOAD_PLATFORM_DELAY (30*1000LL)
#define LOW_LATENCY_PLATFORM_DELAY (13*1000LL)

void platform_set_echo_reference(struct audio_device *adev, bool enable,
    audio_devices_t out_device)
{
    struct platform_data *my_data = (struct platform_data *)adev->platform;
    snd_device_t snd_device = SND_DEVICE_NONE;
    struct stream_out out;

    out.devices = out_device;

    if (strcmp(my_data->ec_ref_mixer_path, "")) {
        ALOGV("%s: disabling %s", __func__, my_data->ec_ref_mixer_path);
        audio_route_reset_and_update_path(adev->audio_route,
            my_data->ec_ref_mixer_path);
    }

    if (enable) {
        strlcpy(my_data->ec_ref_mixer_path, "echo-reference",
            sizeof(my_data->ec_ref_mixer_path));
        snd_device = platform_get_output_snd_device(adev->platform, &out);
        /*
         * If native audio device reference count > 0, then apply codec EC otherwise
         * fallback to headphones if so or default
         */
        if (adev->snd_dev_ref_cnt[SND_DEVICE_OUT_HEADPHONES_44_1] > 0)
            platform_add_backend_name(my_data->ec_ref_mixer_path,
                SND_DEVICE_OUT_HEADPHONES_44_1);
        else
            platform_add_backend_name(my_data->ec_ref_mixer_path, snd_device);

        ALOGD("%s: enabling %s", __func__, my_data->ec_ref_mixer_path);
        audio_route_apply_and_update_path(adev->audio_route,
            my_data->ec_ref_mixer_path);
    }
}

static struct csd_data *open_csd_client(bool i2s_ext_modem)
{
    struct csd_data *csd = calloc(1, sizeof(struct csd_data));

    if (!csd) {
        ALOGE("failed to allocate csd_data mem");
        return NULL;
    }

    csd->csd_client = dlopen(LIB_CSD_CLIENT, RTLD_NOW);
    if (csd->csd_client == NULL) {
        ALOGE("%s: DLOPEN failed for %s", __func__, LIB_CSD_CLIENT);
        goto error;
    } else {
        ALOGV("%s: DLOPEN successful for %s", __func__, LIB_CSD_CLIENT);

        csd->deinit = (deinit_t)dlsym(csd->csd_client,
                                             "csd_client_deinit");
        if (csd->deinit == NULL) {
            ALOGE("%s: dlsym error %s for csd_client_deinit", __func__,
                  dlerror());
            goto error;
        }
        csd->disable_device = (disable_device_t)dlsym(csd->csd_client,
                                             "csd_client_disable_device");
        if (csd->disable_device == NULL) {
            ALOGE("%s: dlsym error %s for csd_client_disable_device",
                  __func__, dlerror());
            goto error;
        }
        csd->enable_device_config = (enable_device_config_t)dlsym(csd->csd_client,
                                               "csd_client_enable_device_config");
        if (csd->enable_device_config == NULL) {
            ALOGE("%s: dlsym error %s for csd_client_enable_device_config",
                  __func__, dlerror());
            goto error;
        }
        csd->enable_device = (enable_device_t)dlsym(csd->csd_client,
                                             "csd_client_enable_device");
        if (csd->enable_device == NULL) {
            ALOGE("%s: dlsym error %s for csd_client_enable_device",
                  __func__, dlerror());
            goto error;
        }
        csd->start_voice = (start_voice_t)dlsym(csd->csd_client,
                                             "csd_client_start_voice");
        if (csd->start_voice == NULL) {
            ALOGE("%s: dlsym error %s for csd_client_start_voice",
                  __func__, dlerror());
            goto error;
        }
        csd->stop_voice = (stop_voice_t)dlsym(csd->csd_client,
                                             "csd_client_stop_voice");
        if (csd->stop_voice == NULL) {
            ALOGE("%s: dlsym error %s for csd_client_stop_voice",
                  __func__, dlerror());
            goto error;
        }
        csd->volume = (volume_t)dlsym(csd->csd_client,
                                             "csd_client_volume");
        if (csd->volume == NULL) {
            ALOGE("%s: dlsym error %s for csd_client_volume",
                  __func__, dlerror());
            goto error;
        }
        csd->mic_mute = (mic_mute_t)dlsym(csd->csd_client,
                                             "csd_client_mic_mute");
        if (csd->mic_mute == NULL) {
            ALOGE("%s: dlsym error %s for csd_client_mic_mute",
                  __func__, dlerror());
            goto error;
        }
        csd->slow_talk = (slow_talk_t)dlsym(csd->csd_client,
                                             "csd_client_slow_talk");
        if (csd->slow_talk == NULL) {
            ALOGE("%s: dlsym error %s for csd_client_slow_talk",
                  __func__, dlerror());
            goto error;
        }
        csd->start_playback = (start_playback_t)dlsym(csd->csd_client,
                                             "csd_client_start_playback");
        if (csd->start_playback == NULL) {
            ALOGE("%s: dlsym error %s for csd_client_start_playback",
                  __func__, dlerror());
            goto error;
        }
        csd->stop_playback = (stop_playback_t)dlsym(csd->csd_client,
                                             "csd_client_stop_playback");
        if (csd->stop_playback == NULL) {
            ALOGE("%s: dlsym error %s for csd_client_stop_playback",
                  __func__, dlerror());
            goto error;
        }
        csd->set_lch = (set_lch_t)dlsym(csd->csd_client, "csd_client_set_lch");
        if (csd->set_lch == NULL) {
            ALOGE("%s: dlsym error %s for csd_client_set_lch",
                  __func__, dlerror());
            /* Ignore the error as this is not mandatory function for
             * basic voice call to work.
             */
        }
        csd->start_record = (start_record_t)dlsym(csd->csd_client,
                                             "csd_client_start_record");
        if (csd->start_record == NULL) {
            ALOGE("%s: dlsym error %s for csd_client_start_record",
                  __func__, dlerror());
            goto error;
        }
        csd->stop_record = (stop_record_t)dlsym(csd->csd_client,
                                             "csd_client_stop_record");
        if (csd->stop_record == NULL) {
            ALOGE("%s: dlsym error %s for csd_client_stop_record",
                  __func__, dlerror());
            goto error;
        }

        csd->get_sample_rate = (get_sample_rate_t)dlsym(csd->csd_client,
                                             "csd_client_get_sample_rate");
        if (csd->get_sample_rate == NULL) {
            ALOGE("%s: dlsym error %s for csd_client_get_sample_rate",
                  __func__, dlerror());

            goto error;
        }

        csd->init = (init_t)dlsym(csd->csd_client, "csd_client_init");

        if (csd->init == NULL) {
            ALOGE("%s: dlsym error %s for csd_client_init",
                  __func__, dlerror());
            goto error;
        } else {
            csd->init(i2s_ext_modem);
        }
    }
    return csd;

error:
    free(csd);
    csd = NULL;
    return csd;
}

void close_csd_client(struct csd_data *csd)
{
    if (csd != NULL) {
        csd->deinit();
        dlclose(csd->csd_client);
        free(csd);
        csd = NULL;
    }
}

static bool platform_is_i2s_ext_modem(const char *snd_card_name,
                                      struct platform_data *plat_data)
{
    plat_data->is_i2s_ext_modem = false;

    if (!strncmp(snd_card_name, "apq8084-taiko-i2s-mtp-snd-card",
                 sizeof("apq8084-taiko-i2s-mtp-snd-card")) ||
        !strncmp(snd_card_name, "apq8084-taiko-i2s-cdp-snd-card",
                 sizeof("apq8084-taiko-i2s-cdp-snd-card"))) {
        plat_data->is_i2s_ext_modem = true;
    }
    ALOGV("%s, is_i2s_ext_modem:%d",__func__, plat_data->is_i2s_ext_modem);

    return plat_data->is_i2s_ext_modem;
}

static void set_platform_defaults(struct platform_data * my_data)
{
    int32_t dev, count = 0;
    char dsp_decoder_property[PROPERTY_VALUE_MAX];
    const char *MEDIA_MIMETYPE_AUDIO_ALAC = "audio/alac";
    const char *MEDIA_MIMETYPE_AUDIO_APE = "audio/x-ape";

    for (dev = 0; dev < SND_DEVICE_MAX; dev++) {
        backend_table[dev] = NULL;
    }
    for (dev = 0; dev < SND_DEVICE_MAX; dev++) {
        backend_bit_width_table[dev] = CODEC_BACKEND_DEFAULT_BIT_WIDTH;
    }

    // TBD - do these go to the platform-info.xml file.
    // will help in avoiding strdups here
    backend_table[SND_DEVICE_IN_BT_SCO_MIC] = strdup("bt-sco");
    backend_table[SND_DEVICE_IN_BT_SCO_MIC_WB] = strdup("bt-sco-wb");
    backend_table[SND_DEVICE_IN_BT_SCO_MIC_NREC] = strdup("bt-sco");
    backend_table[SND_DEVICE_IN_BT_SCO_MIC_WB_NREC] = strdup("bt-sco-wb");
    backend_table[SND_DEVICE_OUT_BT_SCO] = strdup("bt-sco");
    backend_table[SND_DEVICE_OUT_BT_SCO_WB] = strdup("bt-sco-wb");
    backend_table[SND_DEVICE_OUT_HDMI] = strdup("hdmi");
    backend_table[SND_DEVICE_OUT_SPEAKER_AND_HDMI] = strdup("speaker-and-hdmi");
    backend_table[SND_DEVICE_OUT_VOICE_TX] = strdup("afe-proxy");
    backend_table[SND_DEVICE_IN_VOICE_RX] = strdup("afe-proxy");

    backend_table[SND_DEVICE_OUT_AFE_PROXY] = strdup("afe-proxy");
    backend_table[SND_DEVICE_OUT_USB_HEADSET] = strdup("usb-headphones");
    backend_table[SND_DEVICE_OUT_SPEAKER_AND_USB_HEADSET] =
        strdup("speaker-and-usb-headphones");
    backend_table[SND_DEVICE_IN_USB_HEADSET_MIC] = strdup("usb-headset-mic");
    backend_table[SND_DEVICE_IN_CAPTURE_FM] = strdup("capture-fm");
    backend_table[SND_DEVICE_OUT_TRANSMISSION_FM] = strdup("transmission-fm");
    backend_table[SND_DEVICE_OUT_HEADPHONES] = strdup("headphones");
    backend_table[SND_DEVICE_OUT_HEADPHONES_44_1] = strdup("headphones-44.1");

    if(is_external_codec) {
        my_data->max_mic_count = PLATFORM_DEFAULT_EXTERNAL_CODEC_MIC_COUNT;
    } else {
        my_data->max_mic_count = PLATFORM_DEFAULT_MIC_COUNT;
    }
     /*remove ALAC & APE from DSP decoder list based on software decoder availability*/
     for (count = 0; count < sizeof(dsp_only_decoders_mime)/sizeof(dsp_only_decoders_mime[0]);
            count++) {

         if (!strncmp(MEDIA_MIMETYPE_AUDIO_ALAC, dsp_only_decoders_mime[count],
              strlen(dsp_only_decoders_mime[count]))) {

             if(property_get_bool("vendor.audio.use.sw.alac.decoder", false)) {
                 ALOGD("Alac software decoder is available...removing alac from DSP decoder list");
                 strncpy(dsp_only_decoders_mime[count],"none",5);
             }
         } else if (!strncmp(MEDIA_MIMETYPE_AUDIO_APE, dsp_only_decoders_mime[count],
              strlen(dsp_only_decoders_mime[count]))) {

             if(property_get_bool("vendor.audio.use.sw.ape.decoder", false)) {
                 ALOGD("APE software decoder is available...removing ape from DSP decoder list");
                 strncpy(dsp_only_decoders_mime[count],"none",5);
             }
         }
     }
}

void get_cvd_version(char *cvd_version, struct audio_device *adev)
{
    struct mixer_ctl *ctl;
    int count;
    int ret = 0;

    ctl = mixer_get_ctl_by_name(adev->mixer, CVD_VERSION_MIXER_CTL);
    if (!ctl) {
        ALOGE("%s: Could not get ctl for mixer cmd - %s",  __func__, CVD_VERSION_MIXER_CTL);
        goto done;
    }
    mixer_ctl_update(ctl);

    count = mixer_ctl_get_num_values(ctl);
    if (count > MAX_CVD_VERSION_STRING_SIZE)
        count = MAX_CVD_VERSION_STRING_SIZE;

    ret = mixer_ctl_get_array(ctl, cvd_version, count);
    if (ret != 0) {
        ALOGE("%s: ERROR! mixer_ctl_get_array() failed to get CVD Version", __func__);
        goto done;
    }

done:
    return;
}

static int hw_util_open(int card_no)
{
    int fd = -1;
    char dev_name[256];

    snprintf(dev_name, sizeof(dev_name), "/dev/snd/hwC%uD%u",
                               card_no, WCD9XXX_CODEC_HWDEP_NODE);
    ALOGD("%s Opening device %s\n", __func__, dev_name);
    fd = open(dev_name, O_WRONLY);
    if (fd < 0) {
        ALOGE("%s: cannot open device '%s'\n", __func__, dev_name);
        return fd;
    }
    ALOGD("%s success", __func__);
    return fd;
}

struct param_data {
    int    use_case;
    int    acdb_id;
    int    get_size;
    int    buff_size;
    int    data_size;
    void   *buff;
};

static int send_codec_cal(acdb_loader_get_calibration_t acdb_loader_get_calibration, int fd)
{
    int ret = 0, type;

    for (type = WCD9XXX_ANC_CAL; type < WCD9XXX_MAX_CAL; type++) {
        struct wcdcal_ioctl_buffer codec_buffer;
        struct param_data calib;

        /* MAD calibration is handled by sound trigger HAL, skip here */
        if (type == WCD9XXX_MAD_CAL)
            continue;

        calib.get_size = 1;
        ret = acdb_loader_get_calibration(cal_name_info[type], sizeof(struct param_data),
                                                                 &calib);
        if (ret < 0) {
            ALOGE("%s get_calibration failed\n", __func__);
            return ret;
        }
        calib.get_size = 0;
        calib.buff = malloc(calib.buff_size);
        ret = acdb_loader_get_calibration(cal_name_info[type],
                              sizeof(struct param_data), &calib);
        if (ret < 0) {
            ALOGE("%s get_calibration failed\n", __func__);
            free(calib.buff);
            return ret;
        }
        codec_buffer.buffer = calib.buff;
        codec_buffer.size = calib.data_size;
        codec_buffer.cal_type = type;
        if (ioctl(fd, SNDRV_CTL_IOCTL_HWDEP_CAL_TYPE, &codec_buffer) < 0)
            ALOGE("Failed to call ioctl  for %s err=%d",
                                  cal_name_info[type], errno);
        ALOGD("%s cal sent for %s", __func__, cal_name_info[type]);
        free(calib.buff);
    }
    return ret;
}

static void audio_hwdep_send_cal(struct platform_data *plat_data)
{
    int fd = plat_data->hw_dep_fd;

    if (fd < 0)
        fd = hw_util_open(plat_data->adev->snd_card);
    if (fd == -1) {
        ALOGE("%s error open\n", __func__);
        return;
    }

    acdb_loader_get_calibration = (acdb_loader_get_calibration_t)
          dlsym(plat_data->acdb_handle, "acdb_loader_get_calibration");

    if (acdb_loader_get_calibration == NULL) {
        ALOGE("%s: ERROR. dlsym Error:%s acdb_loader_get_calibration", __func__,
           dlerror());
        if (fd >= 0) {
            close(fd);
            plat_data->hw_dep_fd = -1;
        }
        return;
    }
    if (send_codec_cal(acdb_loader_get_calibration, fd) < 0)
        ALOGE("%s: Could not send anc cal", __FUNCTION__);

    send_codec_cal(acdb_loader_get_calibration, plat_data, fd);
    plat_data->hw_dep_fd = fd;
}

int platform_acdb_init(void *platform)
{
    struct platform_data *my_data = (struct platform_data *)platform;
    char *cvd_version = NULL;
    int key = 0;
    const char *snd_card_name;
    int result;
    char value[PROPERTY_VALUE_MAX];
    cvd_version = calloc(1, MAX_CVD_VERSION_STRING_SIZE);
    if (!cvd_version)
        ALOGE("Failed to allocate cvd version");
    else
        get_cvd_version(cvd_version, my_data->adev);

    property_get("audio.ds1.metainfo.key",value,"0");
    key = atoi(value);
    snd_card_name = mixer_get_name(my_data->adev->mixer);
    result = my_data->acdb_init(snd_card_name, cvd_version, key);
    if (cvd_version)
        free(cvd_version);
    if (!result) {
        my_data->is_acdb_initialized = true;
        ALOGD("ACDB initialized");
        audio_hwdep_send_cal(my_data);
    } else {
        my_data->is_acdb_initialized = false;
        ALOGD("ACDB initialization failed");
    }
    return result;
}

static void get_source_mic_type(struct platform_data * my_data)
{
    // support max to mono, example if max count is 3, usecase supports Three, dual and mono mic
    switch (my_data->max_mic_count) {
        case 4:
            my_data->source_mic_type |= SOURCE_QUAD_MIC;
        case 3:
            my_data->source_mic_type |= SOURCE_THREE_MIC;
        case 2:
            my_data->source_mic_type |= SOURCE_DUAL_MIC;
        case 1:
            my_data->source_mic_type |= SOURCE_MONO_MIC;
            break;
        default:
            ALOGE("%s: max_mic_count (%d), is not supported, setting to default",
                   __func__, my_data->max_mic_count);
            my_data->source_mic_type = SOURCE_MONO_MIC | SOURCE_DUAL_MIC;
            break;
    }
}

void *platform_init(struct audio_device *adev)
{
    char platform[PROPERTY_VALUE_MAX];
    char baseband[PROPERTY_VALUE_MAX];
    char value[PROPERTY_VALUE_MAX];
    struct platform_data *my_data = NULL;
    int retry_num = 0, snd_card_num = 0, key = 0, ret = 0;
    const char *snd_card_name;
    char *cvd_version = NULL;
    char *snd_internal_name = NULL;
    char *tmp = NULL;
    char mixer_xml_file[MIXER_PATH_MAX_LENGTH]= {0};
    const char *mixer_ctl_name = "Set HPX ActiveBe";
    struct mixer_ctl *ctl = NULL;
    int idx;

    adev->snd_card = audio_extn_utils_get_snd_card_num();
    if (adev->snd_card < 0) {
        ALOGE("%s: Unable to find correct sound card", __func__);
        return NULL;
    }
    ALOGD("%s: Opened sound card:%d", __func__, adev->snd_card);

    adev->mixer = mixer_open(adev->snd_card);
    if (!adev->mixer) {
        ALOGE("%s: Unable to open the mixer card: %d", __func__,
               adev->snd_card);
        return NULL;
    }

    snd_card_name = strdup(mixer_get_name(adev->mixer));
    if (!snd_card_name) {
        ALOGE("failed to allocate memory for snd_card_name\n");
        mixer_close(adev->mixer);
        return NULL;
    }

    my_data = calloc(1, sizeof(struct platform_data));
    if (!my_data) {
        ALOGE("failed to allocate platform data");
        if (snd_card_name)
            free(snd_card_name);
        mixer_close(adev->mixer);
        return NULL;
    }

    my_data->is_slimbus_interface = true;
    my_data->is_internal_codec = false;

    my_data->hw_info = hw_info_init(snd_card_name);
    if (!my_data->hw_info) {
        ALOGE("failed to init hw_info");
        mixer_close(adev->mixer);
        if (my_data)
            free(my_data);

        if (snd_card_name)
            free(snd_card_name);
        return NULL;
    }

    if (platform_is_i2s_ext_modem(snd_card_name, my_data)) {
        ALOGD("%s: Call MIXER_XML_PATH_I2S", __func__);

        adev->audio_route = audio_route_init(adev->snd_card,
                                             MIXER_XML_PATH_I2S);
    } else {
        /* Get the codec internal name from the sound card name
         * and form the mixer paths file name dynamically. This
         * is generic way of picking any codec name based mixer
         * files in future with no code change. This code
         * assumes mixer files are formed with format as
         * mixer_paths_internalcodecname.xml

         * If this dynamically read mixer files fails to open then it
         * falls back to default mixer file i.e mixer_paths.xml. This is
         * done to preserve backward compatibility but not mandatory as
         * long as the mixer files are named as per above assumption.
        */
        snd_card_name_t = strdup(snd_card_name);
        snd_internal_name = strtok_r(snd_card_name_t, "-", &tmp);

        if (snd_internal_name != NULL) {
            snd_internal_name = strtok_r(NULL, "-", &tmp);
        }
        if (snd_internal_name != NULL) {
            strlcpy(mixer_xml_file, MIXER_XML_BASE_STRING,
                MIXER_PATH_MAX_LENGTH);
            strlcat(mixer_xml_file, MIXER_FILE_DELIMITER,
                MIXER_PATH_MAX_LENGTH);
            strlcat(mixer_xml_file, snd_internal_name,
                MIXER_PATH_MAX_LENGTH);
            strlcat(mixer_xml_file, MIXER_FILE_EXT,
                MIXER_PATH_MAX_LENGTH);
        } else {
            strlcpy(mixer_xml_file, MIXER_XML_DEFAULT_PATH,
                MIXER_PATH_MAX_LENGTH);
        }

        if (F_OK == access(mixer_xml_file, 0)) {
            ALOGD("%s: Loading mixer file: %s", __func__, mixer_xml_file);
            if (audio_extn_read_xml(adev, adev->snd_card, mixer_xml_file,
                            MIXER_XML_PATH_AUXPCM) == -ENOSYS)
                adev->audio_route = audio_route_init(adev->snd_card,
                                               mixer_xml_file);
        } else {
            ALOGD("%s: Loading default mixer file", __func__);
            if (audio_extn_read_xml(adev, adev->snd_card, MIXER_XML_DEFAULT_PATH,
                            MIXER_XML_PATH_AUXPCM) == -ENOSYS)
                adev->audio_route = audio_route_init(adev->snd_card,
                                               MIXER_XML_DEFAULT_PATH);
                update_codec_type_and_interface(my_data, snd_card_name);
        }
    }
    if (!adev->audio_route) {
        ALOGE("%s: Failed to init audio route controls, aborting.",
               __func__);
        if (my_data)
            free(my_data);
        if (snd_card_name)
            free(snd_card_name);
        if (snd_card_name_t)
            free(snd_card_name_t);
        mixer_close(adev->mixer);
        return NULL;
    }

    my_data->adev = adev;
    my_data->fluence_in_spkr_mode = false;
    my_data->fluence_in_voice_call = false;
    my_data->fluence_in_voice_rec = false;
    my_data->fluence_in_audio_rec = false;
    my_data->external_spk_1 = false;
    my_data->external_spk_2 = false;
    my_data->external_mic = false;
    my_data->fluence_type = FLUENCE_NONE;
    my_data->fluence_mode = FLUENCE_ENDFIRE;
    my_data->slowtalk = false;
    my_data->hd_voice = false;
    my_data->edid_info = NULL;
    my_data->hw_dep_fd = -1;
    my_data->mono_speaker = SPKR_1;

    property_get("ro.vendor.audio.sdk.fluencetype", my_data->fluence_cap, "");
    if (!strncmp("fluencepro", my_data->fluence_cap, sizeof("fluencepro"))) {
        my_data->fluence_type = FLUENCE_QUAD_MIC | FLUENCE_DUAL_MIC;
    } else if (!strncmp("fluence", my_data->fluence_cap, sizeof("fluence"))) {
        my_data->fluence_type = FLUENCE_DUAL_MIC;
    } else {
        my_data->fluence_type = FLUENCE_NONE;
    }

    if (my_data->fluence_type != FLUENCE_NONE) {
        property_get("persist.vendor.audio.fluence.voicecall",value,"");
        if (!strncmp("true", value, sizeof("true"))) {
            my_data->fluence_in_voice_call = true;
        }

        property_get("persist.vendor.audio.fluence.voicerec",value,"");
        if (!strncmp("true", value, sizeof("true"))) {
            my_data->fluence_in_voice_rec = true;
        }

        property_get("persist.vendor.audio.fluence.audiorec",value,"");
        if (!strncmp("true", value, sizeof("true"))) {
            my_data->fluence_in_audio_rec = true;
        }

        property_get("persist.vendor.audio.fluence.speaker",value,"");
        if (!strncmp("true", value, sizeof("true"))) {
            my_data->fluence_in_spkr_mode = true;
        }

        property_get("persist.vendor.audio.fluence.mode",value,"");
        if (!strncmp("broadside", value, sizeof("broadside"))) {
            my_data->fluence_mode = FLUENCE_BROADSIDE;
        }
    }
    my_data->voice_feature_set = VOICE_FEATURE_SET_DEFAULT;
    my_data->acdb_handle = dlopen(LIB_ACDB_LOADER, RTLD_NOW);
    if (my_data->acdb_handle == NULL) {
        ALOGE("%s: DLOPEN failed for %s", __func__, LIB_ACDB_LOADER);
    } else {
        ALOGV("%s: DLOPEN successful for %s", __func__, LIB_ACDB_LOADER);
        my_data->acdb_deallocate = (acdb_deallocate_t)dlsym(my_data->acdb_handle,
                                                    "acdb_loader_deallocate_ACDB");
        if (!my_data->acdb_deallocate)
            ALOGE("%s: Could not find the symbol acdb_loader_deallocate_ACDB from %s",
                  __func__, LIB_ACDB_LOADER);

        my_data->acdb_send_audio_cal = (acdb_send_audio_cal_t)dlsym(my_data->acdb_handle,
                                                    "acdb_loader_send_audio_cal_v2");
        if (!my_data->acdb_send_audio_cal)
            ALOGE("%s: Could not find the symbol acdb_send_audio_cal from %s",
                  __func__, LIB_ACDB_LOADER);

        my_data->acdb_set_audio_cal = (acdb_set_audio_cal_t)dlsym(my_data->acdb_handle,
                                                    "acdb_loader_set_audio_cal_v2");
        if (!my_data->acdb_set_audio_cal)
            ALOGE("%s: Could not find the symbol acdb_set_audio_cal_v2 from %s",
                  __func__, LIB_ACDB_LOADER);

        my_data->acdb_get_audio_cal = (acdb_get_audio_cal_t)dlsym(my_data->acdb_handle,
                                                    "acdb_loader_get_audio_cal_v2");
        if (!my_data->acdb_get_audio_cal)
            ALOGE("%s: Could not find the symbol acdb_get_audio_cal_v2 from %s",
                  __func__, LIB_ACDB_LOADER);

        my_data->acdb_send_voice_cal = (acdb_send_voice_cal_t)dlsym(my_data->acdb_handle,
                                                    "acdb_loader_send_voice_cal");
        if (!my_data->acdb_send_voice_cal)
            ALOGE("%s: Could not find the symbol acdb_loader_send_voice_cal from %s",
                  __func__, LIB_ACDB_LOADER);

        my_data->acdb_reload_vocvoltable = (acdb_reload_vocvoltable_t)dlsym(my_data->acdb_handle,
                                                    "acdb_loader_reload_vocvoltable");
        if (!my_data->acdb_reload_vocvoltable)
            ALOGE("%s: Could not find the symbol acdb_loader_reload_vocvoltable from %s",
                  __func__, LIB_ACDB_LOADER);

        my_data->acdb_get_default_app_type = (acdb_get_default_app_type_t)dlsym(
                                                    my_data->acdb_handle,
                                                    "acdb_loader_get_default_app_type");
        if (!my_data->acdb_get_default_app_type)
            ALOGE("%s: Could not find the symbol acdb_get_default_app_type from %s",
                  __func__, LIB_ACDB_LOADER);

        my_data->acdb_init = (acdb_init_t)dlsym(my_data->acdb_handle,
                                                    "acdb_loader_init_v2");
        if (my_data->acdb_init == NULL) {
            ALOGE("%s: dlsym error %s for acdb_loader_init_v2", __func__, dlerror());
            goto acdb_init_fail;
        }

        int result = acdb_init(adev->snd_card);
        if (!result) {
            my_data->is_acdb_initialized = true;
            ALOGD("ACDB initialized");
            audio_hwdep_send_cal(my_data);
        } else {
            my_data->is_acdb_initialized = false;
            ALOGD("ACDB initialization failed");
        }
    }

    /* Configure active back end for HPX*/
    ctl = mixer_get_ctl_by_name(adev->mixer, mixer_ctl_name);
    if (ctl) {
        ALOGI(" sending HPX Active BE information ");
        mixer_ctl_set_value(ctl, 0, true);
    }

acdb_init_fail:

    set_platform_defaults(my_data);

    /* Initialize ACDB ID's */
    if (my_data->is_i2s_ext_modem)
        platform_info_init(PLATFORM_INFO_XML_PATH_I2S);
    else
        platform_info_init(PLATFORM_INFO_XML_PATH);

    /* If platform is apq8084 and baseband is MDM, load CSD Client specific
     * symbols. Voice call is handled by MDM and apps processor talks to
     * MDM through CSD Client
     */
    property_get("ro.board.platform", platform, "");
    property_get("ro.baseband", baseband, "");
    if (!strncmp("apq8084", platform, sizeof("apq8084")) &&
        !strncmp("mdm", baseband, (sizeof("mdm")-1))) {
         my_data->csd = open_csd_client(my_data->is_i2s_ext_modem);
    } else {
         my_data->csd = NULL;
    }

    /* obtain source mic type from max mic count*/
    get_source_mic_type(my_data);
    ALOGD("%s: Fluence_Type(%d) max_mic_count(%d) mic_type(0x%x) fluence_in_voice_call(%d)"
          " fluence_in_voice_rec(%d) fluence_in_spkr_mode(%d) ",
          __func__, my_data->fluence_type, my_data->max_mic_count, my_data->source_mic_type,
          my_data->fluence_in_voice_call, my_data->fluence_in_voice_rec,
          my_data->fluence_in_spkr_mode);

    /* init usb */
    audio_extn_usb_init(adev);
    /* update sound cards appropriately */
    audio_extn_usb_set_proxy_sound_card(adev->snd_card);

    /* init dap hal */
    audio_extn_dap_hal_init(adev->snd_card);

    /* Read one time ssr property */
    audio_extn_ssr_update_enabled();
    audio_extn_spkr_prot_init(adev);

    audio_extn_dolby_set_license(adev);
    audio_hwdep_send_cal(my_data);

    /* init audio device arbitration */
    audio_extn_dev_arbi_init();

    /* initialize backend config */
    for (idx = 0; idx < MAX_CODEC_BACKENDS; idx++) {
        my_data->current_backend_cfg[idx].sample_rate = CODEC_BACKEND_DEFAULT_SAMPLE_RATE;
        if (idx == HEADPHONE_44_1_BACKEND)
            my_data->current_backend_cfg[idx].sample_rate = OUTPUT_SAMPLING_RATE_44100;
        my_data->current_backend_cfg[idx].bit_width = CODEC_BACKEND_DEFAULT_BIT_WIDTH;
    }

    my_data->current_backend_cfg[DEFAULT_CODEC_BACKEND].bitwidth_mixer_ctl =
        strdup("SLIM_0_RX Format");
    my_data->current_backend_cfg[DEFAULT_CODEC_BACKEND].samplerate_mixer_ctl =
        strdup("SLIM_0_RX SampleRate");

    my_data->current_backend_cfg[HEADPHONE_44_1_BACKEND].bitwidth_mixer_ctl =
        strdup("SLIM_5_RX Format");
    my_data->current_backend_cfg[HEADPHONE_44_1_BACKEND].samplerate_mixer_ctl =
        strdup("SLIM_5_RX SampleRate");

    my_data->edid_info = NULL;
    return my_data;
}

void platform_deinit(void *platform)
{
    struct platform_data *my_data = (struct platform_data *)platform;

    if (my_data->edid_info) {
        free(my_data->edid_info);
        my_data->edid_info = NULL;
    }

    if (my_data->hw_dep_fd >= 0) {
        close(my_data->hw_dep_fd);
        my_data->hw_dep_fd = -1;
    }

    hw_info_deinit(my_data->hw_info);
    close_csd_client(my_data->csd);

    int32_t dev;
    for (dev = 0; dev < SND_DEVICE_MAX; dev++) {
        if (backend_table[dev]) {
            free(backend_table[dev]);
            backend_table[dev]= NULL;
        }
    }

    /* deinit audio device arbitration */
    audio_extn_dev_arbi_deinit();

    if (my_data->edid_info) {
        free(my_data->edid_info);
        my_data->edid_info = NULL;
    }

    free(platform);
    /* deinit usb */
    audio_extn_usb_deinit();
    audio_extn_dap_hal_deinit();
}

int platform_is_acdb_initialized(void *platform)
{
    struct platform_data *my_data = (struct platform_data *)platform;
    ALOGD("%s: acdb initialized %d\n", __func__, my_data->is_acdb_initialized);
    return my_data->is_acdb_initialized;
}

void platform_snd_card_update(void *platform, card_status_t card_status)
{
    struct platform_data *my_data = (struct platform_data *)platform;

    if (card_status == CARD_STATUS_ONLINE) {
        if (!platform_is_acdb_initialized(my_data)) {
            if(platform_acdb_init(my_data))
                ALOGE("%s: acdb initialization is failed", __func__);
        } else if (my_data->acdb_send_common_top() < 0) {
                ALOGD("%s: acdb did not set common topology", __func__);
        }
    }
}

const char *platform_get_snd_device_name(snd_device_t snd_device)
{
    if (snd_device >= SND_DEVICE_MIN && snd_device < SND_DEVICE_MAX)
        return device_table[snd_device];
    else
        return "";
}

const char *platform_get_spkr_1_tz_name(snd_device_t snd_device)
{
    if (snd_device >= SND_DEVICE_MIN && snd_device < SND_DEVICE_MAX)
        return speaker_device_tz_names.spkr_1_tz_name;
    else
        return "";
}

const char *platform_get_spkr_2_tz_name(snd_device_t snd_device)
{
    if (snd_device >= SND_DEVICE_MIN && snd_device < SND_DEVICE_MAX)
        return speaker_device_tz_names.spkr_2_tz_name;
    else
        return "";
}

int platform_get_snd_device_name_extn(void *platform, snd_device_t snd_device,
                                      char *device_name)
{
    struct platform_data *my_data = (struct platform_data *)platform;

    if (snd_device >= SND_DEVICE_MIN && snd_device < SND_DEVICE_MAX) {
        strlcpy(device_name, device_table[snd_device], DEVICE_NAME_MAX_SIZE);
        hw_info_append_hw_type(my_data->hw_info, snd_device, device_name);
    } else {
        strlcpy(device_name, "", DEVICE_NAME_MAX_SIZE);
        return -EINVAL;
    }

    return 0;
}

bool platform_can_enable_spkr_prot_on_device(snd_device_t snd_device)
{
    bool ret = false;

    if (snd_device == SND_DEVICE_OUT_SPEAKER ||
        snd_device == SND_DEVICE_OUT_SPEAKER_REVERSE ||
        snd_device == SND_DEVICE_OUT_VOICE_SPEAKER ||
        snd_device == SND_DEVICE_OUT_VOICE_SPEAKER_2 ||
        ret = true;
    }

    return ret;
}

void platform_add_backend_name(char *mixer_path, snd_device_t snd_device, struct audio_usecase *usecase)
{
    if ((snd_device < SND_DEVICE_MIN) || (snd_device >= SND_DEVICE_MAX)) {
        ALOGE("%s: Invalid snd_device = %d", __func__, snd_device);
        return;
    }

    const char * suffix = backend_table[snd_device];

    if (suffix != NULL) {
        strlcat(mixer_path, " ", MIXER_PATH_MAX_LENGTH);
        strlcat(mixer_path, suffix, MIXER_PATH_MAX_LENGTH);
    }
}

int platform_get_pcm_device_id(audio_usecase_t usecase, int device_type)
{
    int device_id;
    if (device_type == PCM_PLAYBACK)
        device_id = pcm_device_table[usecase][0];
    else
        device_id = pcm_device_table[usecase][1];
    return device_id;
}

static int find_index(struct name_to_index * table, int32_t len, const char * name)
{
    int ret = 0;
    int32_t i;

    if (table == NULL) {
        ALOGE("%s: table is NULL", __func__);
        ret = -ENODEV;
        goto done;
    }

    if (name == NULL) {
        ALOGE("null key");
        ret = -ENODEV;
        goto done;
    }

    for (i=0; i < len; i++) {
        const char* tn = table[i].name;
        size_t len = strlen(tn);
        if (strncmp(tn, name, len) == 0) {
            if (strlen(name) != len) {
                continue; // substring
            }
            ret = table[i].index;
            goto done;
        }
    }
    ALOGE("%s: Could not find index for name = %s",
            __func__, name);
    ret = -ENODEV;
done:
    return ret;
}

int platform_set_fluence_type(void *platform, char *value)
{
    int ret = 0;
    int fluence_type = FLUENCE_NONE;
    int fluence_flag = NONE_FLAG;
    struct platform_data *my_data = (struct platform_data *)platform;
    struct audio_device *adev = my_data->adev;

    ALOGV("%s: fluence type:%d", __func__, my_data->fluence_type);

    /* only dual mic turn on and off is supported as of now through setparameters */
    if (!strncmp(AUDIO_PARAMETER_VALUE_DUALMIC,value, sizeof(AUDIO_PARAMETER_VALUE_DUALMIC))) {
        if (!strncmp("fluencepro", my_data->fluence_cap, sizeof("fluencepro")) ||
            !strncmp("fluence", my_data->fluence_cap, sizeof("fluence"))) {
            ALOGV("fluence dualmic feature enabled \n");
            fluence_type = FLUENCE_DUAL_MIC;
            fluence_flag = DMIC_FLAG;
        } else {
            ALOGE("%s: Failed to set DUALMIC", __func__);
            ret = -1;
            goto done;
        }
    } else if (!strncmp(AUDIO_PARAMETER_KEY_NO_FLUENCE, value, sizeof(AUDIO_PARAMETER_KEY_NO_FLUENCE))) {
        ALOGV("fluence disabled");
        fluence_type = FLUENCE_NONE;
    } else {
        ALOGE("Invalid fluence value : %s",value);
        ret = -1;
        goto done;
    }

    if (fluence_type != my_data->fluence_type) {
        ALOGV("%s: Updating fluence_type to :%d", __func__, fluence_type);
        my_data->fluence_type = fluence_type;
        adev->acdb_settings = (adev->acdb_settings & FLUENCE_MODE_CLEAR) | fluence_flag;
    }
done:
    return ret;
}

int platform_get_fluence_type(void *platform, char *value, uint32_t len)
{
    int ret = 0;
    struct platform_data *my_data = (struct platform_data *)platform;

    if (my_data->fluence_type == FLUENCE_QUAD_MIC) {
        strlcpy(value, "quadmic", len);
    } else if (my_data->fluence_type == FLUENCE_DUAL_MIC) {
        strlcpy(value, "dualmic", len);
    } else if (my_data->fluence_type == FLUENCE_NONE) {
        strlcpy(value, "none", len);
    } else
        ret = -1;

    return ret;
}

int platform_get_snd_device_index(char *device_name)
{
    return find_index(snd_device_name_index, SND_DEVICE_MAX, device_name);
}

int platform_get_usecase_index(const char *usecase_name)
{
    return find_index(usecase_name_index, AUDIO_USECASE_MAX, usecase_name);
}

int platform_set_snd_device_acdb_id(snd_device_t snd_device, unsigned int acdb_id)
{
    int ret = 0;

    if ((snd_device < SND_DEVICE_MIN) || (snd_device >= SND_DEVICE_MAX)) {
        ALOGE("%s: Invalid snd_device = %d",
            __func__, snd_device);
        ret = -EINVAL;
        goto done;
    }

    acdb_device_table[snd_device] = acdb_id;
done:
    return ret;
}

int platform_get_default_app_type(void *platform)
{
    struct platform_data *my_data = (struct platform_data *)platform;

    if (my_data->acdb_get_default_app_type)
        return my_data->acdb_get_default_app_type();
    else
        return DEFAULT_APP_TYPE;
}

int platform_get_snd_device_acdb_id(snd_device_t snd_device)
{
    if ((snd_device < SND_DEVICE_MIN) || (snd_device >= SND_DEVICE_MAX)) {
        ALOGE("%s: Invalid snd_device = %d", __func__, snd_device);
        return -EINVAL;
    }
    return acdb_device_table[snd_device];
}

int platform_set_snd_device_bit_width(snd_device_t snd_device, unsigned int bit_width)
{
    int ret = 0;

    if ((snd_device < SND_DEVICE_MIN) || (snd_device >= SND_DEVICE_MAX)) {
        ALOGE("%s: Invalid snd_device = %d",
            __func__, snd_device);
        ret = -EINVAL;
        goto done;
    }

    backend_bit_width_table[snd_device] = bit_width;
done:
    return ret;
}

int platform_get_snd_device_bit_width(snd_device_t snd_device)
{
    if ((snd_device < SND_DEVICE_MIN) || (snd_device >= SND_DEVICE_MAX)) {
        ALOGE("%s: Invalid snd_device = %d", __func__, snd_device);
        return DEFAULT_OUTPUT_SAMPLING_RATE;
    }
    return backend_bit_width_table[snd_device];
}

int platform_set_native_support(bool codec_support)
{
    int ret = -1;
    ret = na_props.platform_na_prop_enabled = na_props.ui_na_prop_enabled
        = codec_support;
    ALOGD("%s: na_props.platform_na_prop_enabled: %d", __func__,
           na_props.platform_na_prop_enabled);
    return ret;
}

int platform_get_native_support()
{
    int ret;
    if (na_props.platform_na_prop_enabled) {
        ret = na_props.ui_na_prop_enabled;
    } else {
        ret = na_props.platform_na_prop_enabled;
    }
    ALOGV("%s: na_props.ui_na_prop_enabled: %d", __func__,
           na_props.ui_na_prop_enabled);
    return ret;
}

void native_audio_get_params(struct str_parms *query,
                             struct str_parms *reply,
                             char *value, int len)
{
    int ret;
    ret = str_parms_get_str(query, AUDIO_PARAMETER_KEY_NATIVE_AUDIO,
                            value, len);
    if (ret >= 0) {
        if (na_props.platform_na_prop_enabled) {
            str_parms_add_str(reply, AUDIO_PARAMETER_KEY_NATIVE_AUDIO,
                          na_props.ui_na_prop_enabled ? "true" : "false");
            ALOGV("%s: na_props.ui_na_prop_enabled: %d", __func__,
                   na_props.ui_na_prop_enabled);
        } else {
            str_parms_add_str(reply, AUDIO_PARAMETER_KEY_NATIVE_AUDIO,
                              "false");
            ALOGV("%s: native audio not supported: %d", __func__,
                   na_props.platform_na_prop_enabled);
        }
    }
}

int native_audio_set_params(struct platform_data *platform,
                            struct str_parms *parms, char *value, int len)
{
    int ret = 0;
    struct audio_usecase *usecase;
    struct listnode *node;

    ret = str_parms_get_str(parms, AUDIO_PARAMETER_KEY_NATIVE_AUDIO,
                             value, len);
    if (ret >= 0) {
        if (na_props.platform_na_prop_enabled) {
            if (!strncmp("true", value, sizeof("true"))) {
                na_props.ui_na_prop_enabled = true;
                ALOGD("%s: native audio feature enabled from UI",__func__);
            }
            else {
                na_props.ui_na_prop_enabled = false;
                ALOGD("%s: native audio feature disabled from UI",__func__);

            }

            str_parms_del(parms, AUDIO_PARAMETER_KEY_NATIVE_AUDIO);

            /*
             * Iterate through the usecase list and trigger device switch for
             * all the appropriate usecases
             */
            list_for_each(node, &(platform->adev)->usecase_list) {
                 usecase = node_to_item(node, struct audio_usecase, list);

                 if (is_offload_usecase(usecase->id) &&
                    (usecase->stream.out->devices & AUDIO_DEVICE_OUT_WIRED_HEADPHONE ||
                    usecase->stream.out->devices & AUDIO_DEVICE_OUT_WIRED_HEADSET) &&
                    OUTPUT_SAMPLING_RATE_44100 == usecase->stream.out->sample_rate) {
                         ALOGD("%s: triggering dynamic device switch for usecase(%d: %s)"
                               " stream(%p), device(%d)", __func__, usecase->id,
                               use_case_table[usecase->id], usecase->stream,
                               usecase->stream.out->devices);
                         select_devices(platform->adev, usecase->id);
                 }
            }
        } else {
              ALOGD("%s: native audio not supported: %d", __func__,
                     na_props.platform_na_prop_enabled);
        }
    }
    return ret;
}

int platform_get_backend_index(snd_device_t snd_device)
{
    int32_t port = DEFAULT_CODEC_BACKEND;

    if (snd_device >= SND_DEVICE_MIN && snd_device < SND_DEVICE_MAX) {
        if (backend_table[snd_device] != NULL &&
            strcmp(backend_table[snd_device], "headphones-44.1") == 0)
            port = HEADPHONE_44_1_BACKEND;
        else
            port = DEFAULT_CODEC_BACKEND;
    } else {
        ALOGV("%s: Invalid device - %d ", __func__, snd_device);
    }

    ALOGV("%s: backend port - %d", __func__, port);
    return port;
}

int platform_send_audio_calibration(void *platform, struct audio_usecase *usecase,
                                    int app_type, int sample_rate)
{
    struct platform_data *my_data = (struct platform_data *)platform;
    int acdb_dev_id, acdb_dev_type;
    struct audio_device *adev = my_data->adev;
    int snd_device = SND_DEVICE_OUT_SPEAKER;

    if (usecase->type == PCM_PLAYBACK)
        snd_device = platform_get_output_snd_device(adev->platform,
                                            usecase->stream.out);
    else if ((usecase->type == PCM_CAPTURE) &&
                   voice_is_in_call_rec_stream(usecase->stream.in))
        snd_device = voice_get_incall_rec_snd_device(usecase->in_snd_device);
    else if ((usecase->type == PCM_HFP_CALL) || (usecase->type == PCM_CAPTURE))
        snd_device = platform_get_input_snd_device(adev->platform,
                                            adev->primary_output->devices);
    acdb_dev_id = acdb_device_table[audio_extn_get_spkr_prot_snd_device(snd_device)];
    if (acdb_dev_id < 0) {
        ALOGE("%s: Could not find acdb id for device(%d)",
              __func__, snd_device);
        return -EINVAL;
    }
    if (my_data->acdb_send_audio_cal) {
        ALOGV("%s: sending audio calibration for snd_device(%d) acdb_id(%d)",
              __func__, snd_device, acdb_dev_id);
        if (snd_device >= SND_DEVICE_OUT_BEGIN &&
                snd_device < SND_DEVICE_OUT_END)
            acdb_dev_type = ACDB_DEV_TYPE_OUT;
        else
            acdb_dev_type = ACDB_DEV_TYPE_IN;
        my_data->acdb_send_audio_cal(acdb_dev_id, acdb_dev_type, app_type,
                                     sample_rate);
    }
    return 0;
}

int platform_switch_voice_call_device_pre(void *platform)
{
    struct platform_data *my_data = (struct platform_data *)platform;
    int ret = 0;

    if (my_data->csd != NULL &&
        voice_is_in_call(my_data->adev)) {
        /* This must be called before disabling mixer controls on APQ side */
        ret = my_data->csd->disable_device();
        if (ret < 0) {
            ALOGE("%s: csd_client_disable_device, failed, error %d",
                  __func__, ret);
        }
    }
    return ret;
}

int platform_switch_voice_call_enable_device_config(void *platform,
                                                    snd_device_t out_snd_device,
                                                    snd_device_t in_snd_device)
{
    struct platform_data *my_data = (struct platform_data *)platform;
    int acdb_rx_id, acdb_tx_id;
    int ret = 0;

    if (my_data->csd == NULL)
        return ret;

    if ((out_snd_device == SND_DEVICE_OUT_VOICE_SPEAKER ||
         out_snd_device == SND_DEVICE_OUT_VOICE_SPEAKER_2) &&
        audio_extn_spkr_prot_is_enabled())
        acdb_rx_id = acdb_device_table[SND_DEVICE_OUT_SPEAKER_PROTECTED];
    else
        acdb_rx_id = acdb_device_table[out_snd_device];

    acdb_tx_id = acdb_device_table[in_snd_device];

    if (acdb_rx_id > 0 && acdb_tx_id > 0) {
        ret = my_data->csd->enable_device_config(acdb_rx_id, acdb_tx_id);
        if (ret < 0) {
            ALOGE("%s: csd_enable_device_config, failed, error %d",
                  __func__, ret);
        }
    } else {
        ALOGE("%s: Incorrect ACDB IDs (rx: %d tx: %d)", __func__,
              acdb_rx_id, acdb_tx_id);
    }

    return ret;
}

int platform_switch_voice_call_device_post(void *platform,
                                           snd_device_t out_snd_device,
                                           snd_device_t in_snd_device)
{
    struct platform_data *my_data = (struct platform_data *)platform;
    int acdb_rx_id, acdb_tx_id;

    if (my_data->acdb_send_voice_cal == NULL) {
        ALOGE("%s: dlsym error for acdb_send_voice_call", __func__);
    } else {
        if (audio_extn_spkr_prot_is_enabled()) {
            if (out_snd_device == SND_DEVICE_OUT_VOICE_SPEAKER)
                out_snd_device = SND_DEVICE_OUT_VOICE_SPEAKER_PROTECTED;
            else
                out_snd_device = SND_DEVICE_OUT_VOICE_SPEAKER_2_PROTECTED;
        }
        acdb_rx_id = acdb_device_table[out_snd_device];
        acdb_tx_id = acdb_device_table[in_snd_device];

        if (acdb_rx_id > 0 && acdb_tx_id > 0)
            my_data->acdb_send_voice_cal(acdb_rx_id, acdb_tx_id);
        else
            ALOGE("%s: Incorrect ACDB IDs (rx: %d tx: %d)", __func__,
                  acdb_rx_id, acdb_tx_id);
    }

    return 0;
}

int platform_switch_voice_call_usecase_route_post(void *platform,
                                                  snd_device_t out_snd_device,
                                                  snd_device_t in_snd_device)
{
    struct platform_data *my_data = (struct platform_data *)platform;
    int acdb_rx_id, acdb_tx_id;
    int ret = 0;

    if (my_data->csd == NULL)
        return ret;

    if ((out_snd_device == SND_DEVICE_OUT_VOICE_SPEAKER ||
        out_snd_device == SND_DEVICE_OUT_VOICE_SPEAKER_2) &&
        audio_extn_spkr_prot_is_enabled())
        acdb_rx_id = acdb_device_table[SND_DEVICE_OUT_SPEAKER_PROTECTED];
    else
        acdb_rx_id = acdb_device_table[out_snd_device];

    acdb_tx_id = acdb_device_table[in_snd_device];

    if (acdb_rx_id > 0 && acdb_tx_id > 0) {
        ret = my_data->csd->enable_device(acdb_rx_id, acdb_tx_id,
                                          my_data->adev->acdb_settings);
        if (ret < 0) {
            ALOGE("%s: csd_enable_device, failed, error %d", __func__, ret);
        }
    } else {
        ALOGE("%s: Incorrect ACDB IDs (rx: %d tx: %d)", __func__,
              acdb_rx_id, acdb_tx_id);
    }

    return ret;
}

int platform_start_voice_call(void *platform, uint32_t vsid)
{
    struct platform_data *my_data = (struct platform_data *)platform;
    int ret = 0;

    if (my_data->csd != NULL) {
        ret = my_data->csd->start_voice(vsid);
        if (ret < 0) {
            ALOGE("%s: csd_start_voice error %d\n", __func__, ret);
        }
    }
    return ret;
}

int platform_stop_voice_call(void *platform, uint32_t vsid)
{
    struct platform_data *my_data = (struct platform_data *)platform;
    int ret = 0;

    if (my_data->csd != NULL) {
        ret = my_data->csd->stop_voice(vsid);
        if (ret < 0) {
            ALOGE("%s: csd_stop_voice error %d\n", __func__, ret);
        }
    }
    return ret;
}

int platform_get_sample_rate(void *platform, uint32_t *rate)
{
    struct platform_data *my_data = (struct platform_data *)platform;
    int ret = 0;

    if ((my_data->csd != NULL) && my_data->is_i2s_ext_modem) {
        ret = my_data->csd->get_sample_rate(rate);
        if (ret < 0) {
            ALOGE("%s: csd_get_sample_rate error %d\n", __func__, ret);
        }
    }
    return ret;
}

int platform_set_voice_volume(void *platform, int volume)
{
    struct platform_data *my_data = (struct platform_data *)platform;
    struct audio_device *adev = my_data->adev;
    struct mixer_ctl *ctl;
    const char *mixer_ctl_name = "Voice Rx Gain";
    int vol_index = 0, ret = 0;
    uint32_t set_values[ ] = {0,
                              ALL_SESSION_VSID,
                              DEFAULT_VOLUME_RAMP_DURATION_MS};

    // Voice volume levels are mapped to adsp volume levels as follows.
    // 100 -> 5, 80 -> 4, 60 -> 3, 40 -> 2, 20 -> 1  0 -> 0
    // But this values don't changed in kernel. So, below change is need.
    vol_index = (int)percent_to_index(volume, MIN_VOL_INDEX, MAX_VOL_INDEX);
    set_values[0] = vol_index;

    ctl = mixer_get_ctl_by_name(adev->mixer, mixer_ctl_name);
    if (!ctl) {
        ALOGE("%s: Could not get ctl for mixer cmd - %s",
              __func__, mixer_ctl_name);
        return -EINVAL;
    }
    ALOGV("Setting voice volume index: %d", set_values[0]);
    mixer_ctl_set_array(ctl, set_values, ARRAY_SIZE(set_values));

    if (my_data->csd != NULL) {
        ret = my_data->csd->volume(ALL_SESSION_VSID, volume,
                                   DEFAULT_VOLUME_RAMP_DURATION_MS);
        if (ret < 0) {
            ALOGE("%s: csd_volume error %d", __func__, ret);
        }
    }
    return ret;
}

int platform_set_mic_mute(void *platform, bool state)
{
    struct platform_data *my_data = (struct platform_data *)platform;
    struct audio_device *adev = my_data->adev;
    struct mixer_ctl *ctl;
    const char *mixer_ctl_name = "Voice Tx Mute";
    int ret = 0;
    uint32_t set_values[ ] = {0,
                              ALL_SESSION_VSID,
                              DEFAULT_MUTE_RAMP_DURATION_MS};

    set_values[0] = state;
    ctl = mixer_get_ctl_by_name(adev->mixer, mixer_ctl_name);
    if (!ctl) {
        ALOGE("%s: Could not get ctl for mixer cmd - %s",
              __func__, mixer_ctl_name);
        return -EINVAL;
    }
    ALOGV("Setting voice mute state: %d", state);
    mixer_ctl_set_array(ctl, set_values, ARRAY_SIZE(set_values));

    if (my_data->csd != NULL) {
        ret = my_data->csd->mic_mute(ALL_SESSION_VSID, state,
                                     DEFAULT_MUTE_RAMP_DURATION_MS);
        if (ret < 0) {
            ALOGE("%s: csd_mic_mute error %d", __func__, ret);
        }
    }
    return ret;
}

int platform_set_device_mute(void *platform, bool state, char *dir)
{
    struct platform_data *my_data = (struct platform_data *)platform;
    struct audio_device *adev = my_data->adev;
    struct mixer_ctl *ctl;
    char *mixer_ctl_name = NULL;
    int ret = 0;
    uint32_t set_values[ ] = {0,
                              ALL_SESSION_VSID,
                              0};
    if(dir == NULL) {
        ALOGE("%s: Invalid direction:%s", __func__, dir);
        return -EINVAL;
    }

    if (!strncmp("rx", dir, sizeof("rx"))) {
        mixer_ctl_name = "Voice Rx Device Mute";
    } else if (!strncmp("tx", dir, sizeof("tx"))) {
        mixer_ctl_name = "Voice Tx Device Mute";
    } else {
        return -EINVAL;
    }

    set_values[0] = state;
    ctl = mixer_get_ctl_by_name(adev->mixer, mixer_ctl_name);
    if (!ctl) {
        ALOGE("%s: Could not get ctl for mixer cmd - %s",
              __func__, mixer_ctl_name);
        return -EINVAL;
    }

    ALOGV("%s: Setting device mute state: %d, mixer ctrl:%s",
          __func__,state, mixer_ctl_name);
    mixer_ctl_set_array(ctl, set_values, ARRAY_SIZE(set_values));

    return ret;
}

snd_device_t platform_get_output_snd_device(void *platform, struct stream_out *out)
{
    struct platform_data *my_data = (struct platform_data *)platform;
    struct audio_device *adev = my_data->adev;
    audio_mode_t mode = adev->mode;
    snd_device_t snd_device = SND_DEVICE_NONE;
    audio_devices_t devices = out->devices;
    unsigned int sample_rate = out->sample_rate;

    audio_channel_mask_t channel_mask = (adev->active_input == NULL) ?
                                AUDIO_CHANNEL_IN_MONO : adev->active_input->channel_mask;
    int channel_count = popcount(channel_mask);

    ALOGV("%s: enter: output devices(%#x)", __func__, devices);
    if (devices == AUDIO_DEVICE_NONE ||
        devices & AUDIO_DEVICE_BIT_IN) {
        ALOGV("%s: Invalid output devices (%#x)", __func__, devices);
        goto exit;
    }

    if (popcount(devices) == 2) {
        if (devices == (AUDIO_DEVICE_OUT_WIRED_HEADPHONE |
                        AUDIO_DEVICE_OUT_SPEAKER)) {
            if (my_data->external_spk_1)
                snd_device = SND_DEVICE_OUT_SPEAKER_AND_HEADPHONES_EXTERNAL_1;
            else if (my_data->external_spk_2)
                snd_device = SND_DEVICE_OUT_SPEAKER_AND_HEADPHONES_EXTERNAL_2;
            else
                snd_device = SND_DEVICE_OUT_SPEAKER_AND_HEADPHONES;
        } else if (devices == (AUDIO_DEVICE_OUT_WIRED_HEADSET |
                               AUDIO_DEVICE_OUT_SPEAKER)) {
            if (audio_extn_get_anc_enabled())
                snd_device = SND_DEVICE_OUT_SPEAKER_AND_ANC_HEADSET;
            else if (my_data->external_spk_1)
                snd_device = SND_DEVICE_OUT_SPEAKER_AND_HEADPHONES_EXTERNAL_1;
            else if (my_data->external_spk_2)
                snd_device = SND_DEVICE_OUT_SPEAKER_AND_HEADPHONES_EXTERNAL_2;
            else
                snd_device = SND_DEVICE_OUT_SPEAKER_AND_HEADPHONES;
        } else if (devices == (AUDIO_DEVICE_OUT_AUX_DIGITAL |
                               AUDIO_DEVICE_OUT_SPEAKER)) {
            snd_device = SND_DEVICE_OUT_SPEAKER_AND_HDMI;
        } else if (devices == (AUDIO_DEVICE_OUT_ANLG_DOCK_HEADSET |
                               AUDIO_DEVICE_OUT_SPEAKER)) {
            snd_device = SND_DEVICE_OUT_SPEAKER_AND_USB_HEADSET;
        } else {
            ALOGE("%s: Invalid combo device(%#x)", __func__, devices);
            goto exit;
        }
        if (snd_device != SND_DEVICE_NONE) {
            goto exit;
        }
    }

    if (popcount(devices) != 1) {
        ALOGE("%s: Invalid output devices(%#x)", __func__, devices);
        goto exit;
    }

    if ((mode == AUDIO_MODE_IN_CALL) ||
        voice_extn_compress_voip_is_active(adev)) {
        if (devices & AUDIO_DEVICE_OUT_WIRED_HEADPHONE ||
            devices & AUDIO_DEVICE_OUT_WIRED_HEADSET) {
            if ((adev->voice.tty_mode != TTY_MODE_OFF) &&
                !voice_extn_compress_voip_is_active(adev)) {
                switch (adev->voice.tty_mode) {
                case TTY_MODE_FULL:
                    snd_device = SND_DEVICE_OUT_VOICE_TTY_FULL_HEADPHONES;
                    break;
                case TTY_MODE_VCO:
                    snd_device = SND_DEVICE_OUT_VOICE_TTY_VCO_HEADPHONES;
                    break;
                case TTY_MODE_HCO:
                    snd_device = SND_DEVICE_OUT_VOICE_TTY_HCO_HANDSET;
                    break;
                default:
                    ALOGE("%s: Invalid TTY mode (%#x)",
                          __func__, adev->voice.tty_mode);
                }
            } else if (audio_extn_get_anc_enabled()) {
                if (audio_extn_should_use_fb_anc())
                    snd_device = SND_DEVICE_OUT_VOICE_ANC_FB_HEADSET;
                else
                    snd_device = SND_DEVICE_OUT_VOICE_ANC_HEADSET;
            } else {
                snd_device = SND_DEVICE_OUT_VOICE_HEADPHONES;
            }
        } else if (devices & AUDIO_DEVICE_OUT_ALL_SCO) {
            if (adev->bt_wb_speech_enabled)
                snd_device = SND_DEVICE_OUT_BT_SCO_WB;
            else
                snd_device = SND_DEVICE_OUT_BT_SCO;
        } else if (devices & AUDIO_DEVICE_OUT_SPEAKER) {
            if (my_data->mono_speaker == SPKR_1)
                snd_device = SND_DEVICE_OUT_VOICE_SPEAKER;
            else
                snd_device = SND_DEVICE_OUT_VOICE_SPEAKER_2;
        } else if (devices & AUDIO_DEVICE_OUT_ANLG_DOCK_HEADSET ||
                   devices & AUDIO_DEVICE_OUT_DGTL_DOCK_HEADSET) {
            snd_device = SND_DEVICE_OUT_USB_HEADSET;
        } else if (devices & AUDIO_DEVICE_OUT_FM_TX) {
            snd_device = SND_DEVICE_OUT_TRANSMISSION_FM;
        } else if (devices & AUDIO_DEVICE_OUT_EARPIECE) {
            if (audio_extn_should_use_handset_anc(channel_count))
                snd_device = SND_DEVICE_OUT_ANC_HANDSET;
            else
                snd_device = SND_DEVICE_OUT_VOICE_HANDSET;
        } else if (devices & AUDIO_DEVICE_OUT_TELEPHONY_TX)
            snd_device = SND_DEVICE_OUT_VOICE_TX;

        if (snd_device != SND_DEVICE_NONE) {
            goto exit;
        }
    }

    if (devices & AUDIO_DEVICE_OUT_WIRED_HEADPHONE ||
        devices & AUDIO_DEVICE_OUT_WIRED_HEADSET) {
        if (devices & AUDIO_DEVICE_OUT_WIRED_HEADSET
            && audio_extn_get_anc_enabled()) {
            if (audio_extn_should_use_fb_anc())
                snd_device = SND_DEVICE_OUT_ANC_FB_HEADSET;
            else
                snd_device = SND_DEVICE_OUT_ANC_HEADSET;
        } else if (platform_get_native_support() &&
                    OUTPUT_SAMPLING_RATE_44100 == sample_rate)
                snd_device = SND_DEVICE_OUT_HEADPHONES_44_1;
          else
                snd_device = SND_DEVICE_OUT_HEADPHONES;
    } else if (devices & AUDIO_DEVICE_OUT_SPEAKER) {
        if (my_data->external_spk_1)
            snd_device = SND_DEVICE_OUT_SPEAKER_EXTERNAL_1;
        else if (my_data->external_spk_2)
            snd_device = SND_DEVICE_OUT_SPEAKER_EXTERNAL_2;
        else if (adev->speaker_lr_swap)
            snd_device = SND_DEVICE_OUT_SPEAKER_REVERSE;
        else
            snd_device = SND_DEVICE_OUT_SPEAKER;
    } else if (devices & AUDIO_DEVICE_OUT_ALL_SCO) {
        if (adev->bt_wb_speech_enabled)
            snd_device = SND_DEVICE_OUT_BT_SCO_WB;
        else
            snd_device = SND_DEVICE_OUT_BT_SCO;
    } else if (devices & AUDIO_DEVICE_OUT_AUX_DIGITAL) {
        snd_device = SND_DEVICE_OUT_HDMI ;
    } else if (devices & AUDIO_DEVICE_OUT_ANLG_DOCK_HEADSET ||
               devices & AUDIO_DEVICE_OUT_DGTL_DOCK_HEADSET) {
        ALOGD("%s: setting USB hadset channel capability(2) for Proxy", __func__);
        audio_extn_set_afe_proxy_channel_mixer(adev, 2);
        snd_device = SND_DEVICE_OUT_USB_HEADSET;
    } else if (devices & AUDIO_DEVICE_OUT_FM_TX) {
        snd_device = SND_DEVICE_OUT_TRANSMISSION_FM;
    } else if (devices & AUDIO_DEVICE_OUT_EARPIECE) {
        snd_device = SND_DEVICE_OUT_HANDSET;
    } else if (devices & AUDIO_DEVICE_OUT_PROXY) {
        channel_count = audio_extn_get_afe_proxy_channel_count();
        ALOGD("%s: setting sink capability(%d) for Proxy", __func__, channel_count);
        audio_extn_set_afe_proxy_channel_mixer(adev, channel_count);
        snd_device = SND_DEVICE_OUT_AFE_PROXY;
    } else {
        ALOGE("%s: Unknown device(s) %#x", __func__, devices);
    }
exit:
    ALOGV("%s: exit: snd_device(%s)", __func__, device_table[snd_device]);
    return snd_device;
}

snd_device_t platform_get_input_snd_device(void *platform, audio_devices_t out_device)
{
    struct platform_data *my_data = (struct platform_data *)platform;
    struct audio_device *adev = my_data->adev;
    audio_source_t  source = (adev->active_input == NULL) ?
                                AUDIO_SOURCE_DEFAULT : adev->active_input->source;

    audio_mode_t    mode   = adev->mode;
    audio_devices_t in_device = ((adev->active_input == NULL) ?
                                    AUDIO_DEVICE_NONE : adev->active_input->device)
                                & ~AUDIO_DEVICE_BIT_IN;
    audio_channel_mask_t channel_mask = (adev->active_input == NULL) ?
                                AUDIO_CHANNEL_IN_MONO : adev->active_input->channel_mask;
    snd_device_t snd_device = SND_DEVICE_NONE;
    int channel_count = popcount(channel_mask);

    ALOGV("%s: enter: out_device(%#x) in_device(%#x) channel_count (%d) channel_mask (0x%x)",
          __func__, out_device, in_device, channel_count, channel_mask);
    if (my_data->external_mic) {
        if ((out_device != AUDIO_DEVICE_NONE) && ((mode == AUDIO_MODE_IN_CALL) ||
            voice_extn_compress_voip_is_active(adev) || audio_extn_hfp_is_active(adev))) {
            if (out_device & AUDIO_DEVICE_OUT_WIRED_HEADPHONE ||
               out_device & AUDIO_DEVICE_OUT_EARPIECE ||
               out_device & AUDIO_DEVICE_OUT_SPEAKER )
                snd_device = SND_DEVICE_IN_HANDSET_MIC_EXTERNAL;
        } else if (in_device & AUDIO_DEVICE_IN_BUILTIN_MIC ||
                   in_device & AUDIO_DEVICE_IN_BACK_MIC) {
            snd_device = SND_DEVICE_IN_HANDSET_MIC_EXTERNAL;
        }
    }

    if (snd_device != AUDIO_DEVICE_NONE)
        goto exit;

    if ((out_device != AUDIO_DEVICE_NONE) && ((mode == AUDIO_MODE_IN_CALL) ||
        voice_extn_compress_voip_is_active(adev) || audio_extn_hfp_is_active(adev))) {
        if ((adev->voice.tty_mode != TTY_MODE_OFF) &&
            !voice_extn_compress_voip_is_active(adev)) {
            if (out_device & AUDIO_DEVICE_OUT_WIRED_HEADPHONE ||
                out_device & AUDIO_DEVICE_OUT_WIRED_HEADSET) {
                switch (adev->voice.tty_mode) {
                case TTY_MODE_FULL:
                    snd_device = SND_DEVICE_IN_VOICE_TTY_FULL_HEADSET_MIC;
                    break;
                case TTY_MODE_VCO:
                    snd_device = SND_DEVICE_IN_VOICE_TTY_VCO_HANDSET_MIC;
                    break;
                case TTY_MODE_HCO:
                    snd_device = SND_DEVICE_IN_VOICE_TTY_HCO_HEADSET_MIC;
                    break;
                default:
                    ALOGE("%s: Invalid TTY mode (%#x)", __func__, adev->voice.tty_mode);
                }
                goto exit;
            }
        }
        if (out_device & AUDIO_DEVICE_OUT_EARPIECE ||
            out_device & AUDIO_DEVICE_OUT_WIRED_HEADPHONE) {
            if (out_device & AUDIO_DEVICE_OUT_EARPIECE &&
                audio_extn_should_use_handset_anc(channel_count)) {
                if ((my_data->fluence_type != FLUENCE_NONE) &&
                    (my_data->source_mic_type & SOURCE_DUAL_MIC)) {
                    snd_device = SND_DEVICE_IN_VOICE_FLUENCE_DMIC_AANC;
                    adev->acdb_settings |= DMIC_FLAG;
                } else {
                    snd_device = SND_DEVICE_IN_AANC_HANDSET_MIC;
                }
                adev->acdb_settings |= ANC_FLAG;
            } else if (my_data->fluence_type == FLUENCE_NONE ||
                my_data->fluence_in_voice_call == false) {
                snd_device = SND_DEVICE_IN_HANDSET_MIC;
                if (audio_extn_hfp_is_active(adev))
                    platform_set_echo_reference(adev, true, out_device);
            } else {
                snd_device = SND_DEVICE_IN_VOICE_DMIC;
            }
        } else if (out_device & AUDIO_DEVICE_OUT_WIRED_HEADSET) {
            snd_device = SND_DEVICE_IN_VOICE_HEADSET_MIC;
            if (audio_extn_hfp_is_active(adev))
                platform_set_echo_reference(adev, true, out_device);
        } else if (out_device & AUDIO_DEVICE_OUT_ALL_SCO) {
            if (adev->bt_wb_speech_enabled) {
                if (adev->bluetooth_nrec)
                    snd_device = SND_DEVICE_IN_BT_SCO_MIC_WB_NREC;
                else
                    snd_device = SND_DEVICE_IN_BT_SCO_MIC_WB;
            } else {
                if (adev->bluetooth_nrec)
                    snd_device = SND_DEVICE_IN_BT_SCO_MIC_NREC;
                else
                    snd_device = SND_DEVICE_IN_BT_SCO_MIC;
            }
        } else if (out_device & AUDIO_DEVICE_OUT_SPEAKER) {
            if (my_data->fluence_type != FLUENCE_NONE &&
                my_data->fluence_in_voice_call &&
                my_data->fluence_in_spkr_mode) {
                if((my_data->fluence_type & FLUENCE_QUAD_MIC) &&
                   (my_data->source_mic_type & SOURCE_QUAD_MIC)) {
                    snd_device = SND_DEVICE_IN_VOICE_SPEAKER_QMIC;
                } else {
                    if (my_data->fluence_mode == FLUENCE_BROADSIDE)
                       snd_device = SND_DEVICE_IN_VOICE_SPEAKER_DMIC_BROADSIDE;
                    else
                       snd_device = SND_DEVICE_IN_VOICE_SPEAKER_DMIC;
                }
            } else {
                snd_device = SND_DEVICE_IN_VOICE_SPEAKER_MIC;
                if (audio_extn_hfp_is_active(adev))
                    platform_set_echo_reference(adev, true, out_device);
            }
        } else if (out_device & AUDIO_DEVICE_OUT_TELEPHONY_TX)
            snd_device = SND_DEVICE_IN_VOICE_RX;
    } else if (source == AUDIO_SOURCE_CAMCORDER) {
        if (in_device & AUDIO_DEVICE_IN_BUILTIN_MIC ||
            in_device & AUDIO_DEVICE_IN_BACK_MIC) {
            snd_device = SND_DEVICE_IN_CAMCORDER_MIC;
        }
    } else if (source == AUDIO_SOURCE_VOICE_RECOGNITION) {
        if (in_device & AUDIO_DEVICE_IN_BUILTIN_MIC) {
            if (my_data->fluence_in_voice_rec && channel_count == 1) {
                if ((my_data->fluence_type & FLUENCE_QUAD_MIC) &&
                    (my_data->source_mic_type & SOURCE_QUAD_MIC)) {
                    snd_device = SND_DEVICE_IN_HANDSET_QMIC;
                } else if ((my_data->fluence_type & FLUENCE_QUAD_MIC) &&
                           (my_data->source_mic_type & SOURCE_THREE_MIC)) {
                    snd_device = SND_DEVICE_IN_HANDSET_TMIC;
                } else if ((my_data->fluence_type & FLUENCE_DUAL_MIC) &&
                           (my_data->source_mic_type & SOURCE_DUAL_MIC)) {
                    snd_device = SND_DEVICE_IN_VOICE_REC_DMIC_FLUENCE;
                }
                platform_set_echo_reference(adev, true, out_device);
            } else if (((channel_mask == AUDIO_CHANNEL_IN_FRONT_BACK) ||
                       (channel_mask == AUDIO_CHANNEL_IN_STEREO)) &&
                       (my_data->source_mic_type & SOURCE_DUAL_MIC)) {
                snd_device = SND_DEVICE_IN_VOICE_REC_DMIC_STEREO;
            } else if (((int)channel_mask == AUDIO_CHANNEL_INDEX_MASK_3) &&
                       (my_data->source_mic_type & SOURCE_THREE_MIC)) {
                snd_device = SND_DEVICE_IN_THREE_MIC;
            } else if (((int)channel_mask == AUDIO_CHANNEL_INDEX_MASK_4) &&
                       (my_data->source_mic_type & SOURCE_QUAD_MIC)) {
                snd_device = SND_DEVICE_IN_QUAD_MIC;
            }
            if (snd_device == SND_DEVICE_NONE) {
                if (adev->active_input->enable_ns)
                    snd_device = SND_DEVICE_IN_VOICE_REC_MIC_NS;
                 else
                    snd_device = SND_DEVICE_IN_VOICE_REC_MIC;
            }
        }
    } else if (source == AUDIO_SOURCE_UNPROCESSED) {
         if (in_device & AUDIO_DEVICE_IN_BUILTIN_MIC) {
             if (((channel_mask == AUDIO_CHANNEL_IN_FRONT_BACK) ||
                 (channel_mask == AUDIO_CHANNEL_IN_STEREO)) &&
                 (my_data->source_mic_type & SOURCE_DUAL_MIC)) {
                 snd_device = SND_DEVICE_IN_UNPROCESSED_STEREO_MIC;
             } else if (((int)channel_mask == AUDIO_CHANNEL_INDEX_MASK_3) &&
                 (my_data->source_mic_type & SOURCE_THREE_MIC)) {
                 snd_device = SND_DEVICE_IN_UNPROCESSED_THREE_MIC;
             } else if (((int)channel_mask == AUDIO_CHANNEL_INDEX_MASK_4) &&
                 (my_data->source_mic_type & SOURCE_QUAD_MIC)) {
                 snd_device = SND_DEVICE_IN_UNPROCESSED_QUAD_MIC;
             } else {
                 snd_device = SND_DEVICE_IN_UNPROCESSED_MIC;
             }
         } else if (in_device & AUDIO_DEVICE_IN_WIRED_HEADSET) {
             snd_device = SND_DEVICE_IN_UNPROCESSED_HEADSET_MIC;
         }
    } else if (source == AUDIO_SOURCE_VOICE_COMMUNICATION) {
        if (out_device & AUDIO_DEVICE_OUT_SPEAKER)
            in_device = AUDIO_DEVICE_IN_BACK_MIC;
        if (adev->active_input) {
            if (adev->active_input->enable_aec &&
                    adev->active_input->enable_ns) {
                if (in_device & AUDIO_DEVICE_IN_BACK_MIC) {
                    if (my_data->fluence_in_spkr_mode) {
                        if ((my_data->fluence_type & FLUENCE_QUAD_MIC) &&
                            (my_data->source_mic_type & SOURCE_QUAD_MIC)) {
                            snd_device = SND_DEVICE_IN_SPEAKER_QMIC_AEC_NS;
                        } else if ((my_data->fluence_type & FLUENCE_DUAL_MIC) &&
                                   (my_data->source_mic_type & SOURCE_DUAL_MIC)) {
                            if (my_data->fluence_mode == FLUENCE_BROADSIDE)
                                snd_device = SND_DEVICE_IN_SPEAKER_DMIC_AEC_NS_BROADSIDE;
                            else
                                snd_device = SND_DEVICE_IN_SPEAKER_DMIC_AEC_NS;
                        }
                    } else
                        snd_device = SND_DEVICE_IN_SPEAKER_MIC_AEC_NS;
                } else if (in_device & AUDIO_DEVICE_IN_BUILTIN_MIC) {
                    if ((my_data->fluence_type & FLUENCE_DUAL_MIC) &&
                        (my_data->source_mic_type & SOURCE_DUAL_MIC))
                        snd_device = SND_DEVICE_IN_HANDSET_DMIC_AEC_NS;
                    } else
                        snd_device = SND_DEVICE_IN_HANDSET_MIC_AEC_NS;
                } else if (in_device & AUDIO_DEVICE_IN_WIRED_HEADSET) {
                    snd_device = SND_DEVICE_IN_HEADSET_MIC_FLUENCE;
                }
                platform_set_echo_reference(adev, true, out_device);
            } else if (adev->active_input->enable_aec) {
                if (in_device & AUDIO_DEVICE_IN_BACK_MIC) {
                    if (my_data->fluence_in_spkr_mode) {
                        if ((my_data->fluence_type & FLUENCE_QUAD_MIC) &&
                            (my_data->source_mic_type & SOURCE_QUAD_MIC)) {
                            snd_device = SND_DEVICE_IN_SPEAKER_QMIC_AEC;
                        } else if ((my_data->fluence_type & FLUENCE_DUAL_MIC) &&
                                   (my_data->source_mic_type & SOURCE_DUAL_MIC)) {
                            if (my_data->fluence_mode == FLUENCE_BROADSIDE)
                                snd_device = SND_DEVICE_IN_SPEAKER_DMIC_AEC_BROADSIDE;
                            else
                                snd_device = SND_DEVICE_IN_SPEAKER_DMIC_AEC;
                        }
                    } else
                        snd_device = SND_DEVICE_IN_SPEAKER_MIC_AEC;
                } else if (in_device & AUDIO_DEVICE_IN_BUILTIN_MIC) {
                    if ((my_data->fluence_type & FLUENCE_DUAL_MIC) &&
                        (my_data->source_mic_type & SOURCE_DUAL_MIC))
                        snd_device = SND_DEVICE_IN_HANDSET_DMIC_AEC;
                    } else
                        snd_device = SND_DEVICE_IN_HANDSET_MIC_AEC;
                } else if (in_device & AUDIO_DEVICE_IN_WIRED_HEADSET) {
                    snd_device = SND_DEVICE_IN_HEADSET_MIC_FLUENCE;
                }
                platform_set_echo_reference(adev, true, out_device);
            } else if (adev->active_input->enable_ns) {
                if (in_device & AUDIO_DEVICE_IN_BACK_MIC) {
                    if (my_data->fluence_in_spkr_mode) {
                        if ((my_data->fluence_type & FLUENCE_QUAD_MIC) &&
                            (my_data->source_mic_type & SOURCE_QUAD_MIC)) {
                            snd_device = SND_DEVICE_IN_SPEAKER_QMIC_NS;
                        } else if ((my_data->fluence_type & FLUENCE_DUAL_MIC) &&
                                   (my_data->source_mic_type & SOURCE_DUAL_MIC)) {
                            if (my_data->fluence_mode == FLUENCE_BROADSIDE)
                                snd_device = SND_DEVICE_IN_SPEAKER_DMIC_NS_BROADSIDE;
                            else
                                snd_device = SND_DEVICE_IN_SPEAKER_DMIC_NS;
                        }
                    } else
                        snd_device = SND_DEVICE_IN_SPEAKER_MIC_NS;
                } else if (in_device & AUDIO_DEVICE_IN_BUILTIN_MIC) {
                    if ((my_data->fluence_type & FLUENCE_DUAL_MIC) &&
                        (my_data->source_mic_type & SOURCE_DUAL_MIC)) {
                        snd_device = SND_DEVICE_IN_HANDSET_DMIC_NS;
                    } else
                        snd_device = SND_DEVICE_IN_HANDSET_MIC_NS;
                } else if (in_device & AUDIO_DEVICE_IN_WIRED_HEADSET) {
                    snd_device = SND_DEVICE_IN_HEADSET_MIC_FLUENCE;
                }
                platform_set_echo_reference(adev, false, out_device);
            } else
                platform_set_echo_reference(adev, false, out_device);
        }
    } else if (source == AUDIO_SOURCE_MIC) {
        if (in_device & AUDIO_DEVICE_IN_BUILTIN_MIC &&
                channel_count == 1 ) {
            if(my_data->fluence_in_audio_rec) {
                if((my_data->fluence_type & FLUENCE_QUAD_MIC) &&
                    (my_data->source_mic_type & SOURCE_QUAD_MIC)) {
                    snd_device = SND_DEVICE_IN_HANDSET_QMIC;
                    platform_set_echo_reference(adev, true, out_device);
                } else if ((my_data->fluence_type & FLUENCE_QUAD_MIC) &&
                    (my_data->source_mic_type & SOURCE_THREE_MIC)) {
                    snd_device = SND_DEVICE_IN_HANDSET_TMIC;
                } else if ((my_data->fluence_type & FLUENCE_DUAL_MIC) &&
                            (my_data->source_mic_type & SOURCE_DUAL_MIC)) {
                    snd_device = SND_DEVICE_IN_HANDSET_DMIC;
                    platform_set_echo_reference(adev, true, out_device);
                }
            }
        }
    } else if (source == AUDIO_SOURCE_FM_TUNER) {
        snd_device = SND_DEVICE_IN_CAPTURE_FM;
    } else if (source == AUDIO_SOURCE_DEFAULT) {
        goto exit;
    }

    if (adev->active_input && (audio_extn_ssr_get_stream() == adev->active_input))
        snd_device = SND_DEVICE_IN_THREE_MIC;

    if (snd_device != SND_DEVICE_NONE) {
        goto exit;
    }

    if (in_device != AUDIO_DEVICE_NONE &&
            !(in_device & AUDIO_DEVICE_IN_VOICE_CALL) &&
            !(in_device & AUDIO_DEVICE_IN_COMMUNICATION)) {
        if (in_device & AUDIO_DEVICE_IN_BUILTIN_MIC) {
            if (adev->active_input && (audio_extn_ssr_get_stream() == adev->active_input))
                snd_device = SND_DEVICE_IN_QUAD_MIC;
            else if ((my_data->fluence_type & (FLUENCE_DUAL_MIC | FLUENCE_QUAD_MIC)) &&
                    (channel_count == 2) && (my_data->source_mic_type & SOURCE_DUAL_MIC))
                snd_device = SND_DEVICE_IN_HANDSET_STEREO_DMIC;
            else
                snd_device = SND_DEVICE_IN_HANDSET_MIC;
        } else if (in_device & AUDIO_DEVICE_IN_BACK_MIC) {
            snd_device = SND_DEVICE_IN_SPEAKER_MIC;
        } else if (in_device & AUDIO_DEVICE_IN_WIRED_HEADSET) {
            snd_device = SND_DEVICE_IN_HEADSET_MIC;
        } else if (in_device & AUDIO_DEVICE_IN_BLUETOOTH_SCO_HEADSET) {
            if (adev->bt_wb_speech_enabled) {
                if (adev->bluetooth_nrec)
                    snd_device = SND_DEVICE_IN_BT_SCO_MIC_WB_NREC;
                else
                    snd_device = SND_DEVICE_IN_BT_SCO_MIC_WB;
            } else {
                if (adev->bluetooth_nrec)
                    snd_device = SND_DEVICE_IN_BT_SCO_MIC_NREC;
                else
                    snd_device = SND_DEVICE_IN_BT_SCO_MIC;
            }
        } else if (in_device & AUDIO_DEVICE_IN_AUX_DIGITAL) {
            snd_device = SND_DEVICE_IN_HDMI_MIC;
        } else if (in_device & AUDIO_DEVICE_IN_ANLG_DOCK_HEADSET ||
                   in_device & AUDIO_DEVICE_IN_DGTL_DOCK_HEADSET) {
            snd_device = SND_DEVICE_IN_USB_HEADSET_MIC;
        } else if (in_device & AUDIO_DEVICE_IN_FM_TUNER) {
            snd_device = SND_DEVICE_IN_CAPTURE_FM;
        } else {
            ALOGE("%s: Unknown input device(s) %#x", __func__, in_device);
            ALOGW("%s: Using default handset-mic", __func__);
            snd_device = SND_DEVICE_IN_HANDSET_MIC;
        }
    } else {
        if (out_device & AUDIO_DEVICE_OUT_EARPIECE) {
            snd_device = SND_DEVICE_IN_HANDSET_MIC;
        } else if (out_device & AUDIO_DEVICE_OUT_WIRED_HEADSET) {
            snd_device = SND_DEVICE_IN_HEADSET_MIC;
        } else if (out_device & AUDIO_DEVICE_OUT_SPEAKER) {
            if ((my_data->source_mic_type & SOURCE_DUAL_MIC) &&
                (channel_count == 2)) {
                snd_device = SND_DEVICE_IN_SPEAKER_STEREO_DMIC;
            } else if ((my_data->source_mic_type & SOURCE_MONO_MIC) &&
                       (channel_count == 1)) {
                 snd_device = SND_DEVICE_IN_SPEAKER_MIC;
            } else {
                ALOGE("%s: something wrong: source type (%d) channel_count (%d) .."
                      " no combination found .. setting to mono", __func__,
                      my_data->source_mic_type, channel_count);
                snd_device = SND_DEVICE_IN_SPEAKER_MIC;
            }
        } else if (out_device & AUDIO_DEVICE_OUT_WIRED_HEADPHONE) {
            snd_device = SND_DEVICE_IN_HANDSET_MIC;
        } else if (out_device & AUDIO_DEVICE_OUT_BLUETOOTH_SCO_HEADSET) {
            if (adev->bt_wb_speech_enabled) {
                if (adev->bluetooth_nrec)
                    snd_device = SND_DEVICE_IN_BT_SCO_MIC_WB_NREC;
                else
                    snd_device = SND_DEVICE_IN_BT_SCO_MIC_WB;
            } else {
                if (adev->bluetooth_nrec)
                    snd_device = SND_DEVICE_IN_BT_SCO_MIC_NREC;
                else
                    snd_device = SND_DEVICE_IN_BT_SCO_MIC;
            }
        } else if (out_device & AUDIO_DEVICE_OUT_AUX_DIGITAL) {
            snd_device = SND_DEVICE_IN_HDMI_MIC;
        } else if (out_device & AUDIO_DEVICE_OUT_ANLG_DOCK_HEADSET ||
                   out_device & AUDIO_DEVICE_OUT_DGTL_DOCK_HEADSET) {
            snd_device = SND_DEVICE_IN_USB_HEADSET_MIC;
        } else {
            ALOGE("%s: Unknown output device(s) %#x", __func__, out_device);
            ALOGW("%s: Using default handset-mic", __func__);
            snd_device = SND_DEVICE_IN_HANDSET_MIC;
        }
    }
exit:
    ALOGV("%s: exit: in_snd_device(%s)", __func__, device_table[snd_device]);
    return snd_device;
}

int platform_set_hdmi_channels(void *platform,  int channel_count)
{
    struct platform_data *my_data = (struct platform_data *)platform;
    struct audio_device *adev = my_data->adev;
    struct mixer_ctl *ctl;
    const char *channel_cnt_str = NULL;
    const char *mixer_ctl_name = "HDMI_RX Channels";
    switch (channel_count) {
    case 8:
        channel_cnt_str = "Eight"; break;
    case 7:
        channel_cnt_str = "Seven"; break;
    case 6:
        channel_cnt_str = "Six"; break;
    case 5:
        channel_cnt_str = "Five"; break;
    case 4:
        channel_cnt_str = "Four"; break;
    case 3:
        channel_cnt_str = "Three"; break;
    default:
        channel_cnt_str = "Two"; break;
    }
    ctl = mixer_get_ctl_by_name(adev->mixer, mixer_ctl_name);
    if (!ctl) {
        ALOGE("%s: Could not get ctl for mixer cmd - %s",
              __func__, mixer_ctl_name);
        return -EINVAL;
    }
    ALOGV("HDMI channel count: %s", channel_cnt_str);
    mixer_ctl_set_enum_by_string(ctl, channel_cnt_str);
    return 0;
}

int platform_edid_get_max_channels(void *platform)
{
    int channel_count;
    int max_channels = 2;
    int i = 0, ret = 0;
    struct platform_data *my_data = (struct platform_data *)platform;
    struct audio_device *adev = my_data->adev;
    edid_audio_info *info = NULL;
    ret = platform_get_edid_info(platform);
    info = (edid_audio_info *)my_data->edid_info;

    if(ret == 0 && info != NULL) {
        for (i = 0; i < info->audio_blocks && i < MAX_EDID_BLOCKS; i++) {
            ALOGV("%s:format %d channel %d", __func__,
                   info->audio_blocks_array[i].format_id,
                   info->audio_blocks_array[i].channels);
            if (info->audio_blocks_array[i].format_id == LPCM) {
                channel_count = info->audio_blocks_array[i].channels;
                if (channel_count > max_channels) {
                   max_channels = channel_count;
                }
            }
        }
    }
    return max_channels;
}

static int platform_set_slowtalk(struct platform_data *my_data, bool state)
{
    int ret = 0;
    struct audio_device *adev = my_data->adev;
    struct mixer_ctl *ctl;
    const char *mixer_ctl_name = "Slowtalk Enable";
    uint32_t set_values[ ] = {0,
                              ALL_SESSION_VSID};

    set_values[0] = state;
    ctl = mixer_get_ctl_by_name(adev->mixer, mixer_ctl_name);
    if (!ctl) {
        ALOGE("%s: Could not get ctl for mixer cmd - %s",
              __func__, mixer_ctl_name);
        ret = -EINVAL;
    } else {
        ALOGV("Setting slowtalk state: %d", state);
        ret = mixer_ctl_set_array(ctl, set_values, ARRAY_SIZE(set_values));
        my_data->slowtalk = state;
    }

    if (my_data->csd != NULL) {
        ret = my_data->csd->slow_talk(ALL_SESSION_VSID, state);
        if (ret < 0) {
            ALOGE("%s: csd_client_disable_device, failed, error %d",
                  __func__, ret);
        }
    }
    return ret;
}

static int set_hd_voice(struct platform_data *my_data, bool state)
{
    struct audio_device *adev = my_data->adev;
    struct mixer_ctl *ctl;
    char *mixer_ctl_name = "HD Voice Enable";
    int ret = 0;
    uint32_t set_values[ ] = {0,
                              ALL_SESSION_VSID};

    set_values[0] = state;
    ctl = mixer_get_ctl_by_name(adev->mixer, mixer_ctl_name);
    if (!ctl) {
        ALOGE("%s: Could not get ctl for mixer cmd - %s",
              __func__, mixer_ctl_name);
        return -EINVAL;
    } else {
        ALOGV("Setting HD Voice state: %d", state);
        ret = mixer_ctl_set_array(ctl, set_values, ARRAY_SIZE(set_values));
        my_data->hd_voice = state;
    }

    return ret;
}

static int update_external_device_status(struct platform_data *my_data,
                                 char* event_name, bool status)
{
    int ret = 0;
    struct audio_usecase *usecase;
    struct listnode *node;

    ALOGD("Recieved  external event switch %s", event_name);

    if (!strcmp(event_name, EVENT_EXTERNAL_SPK_1))
        my_data->external_spk_1 = status;
    else if (!strcmp(event_name, EVENT_EXTERNAL_SPK_2))
        my_data->external_spk_2 = status;
    else if (!strcmp(event_name, EVENT_EXTERNAL_MIC))
        my_data->external_mic = status;
    else {
        ALOGE("The audio event type is not found");
        return -EINVAL;
    }

    list_for_each(node, &my_data->adev->usecase_list) {
        usecase = node_to_item(node, struct audio_usecase, list);
        select_devices(my_data->adev, usecase->id);
    }

    return ret;
}

static int parse_audiocal_cfg(struct str_parms *parms, acdb_audio_cal_cfg_t *cal)
{
    int err;
    unsigned int val;
    char value[64];
    int ret = 0;

    if(parms == NULL || cal == NULL)
        return ret;

    err = str_parms_get_str(parms, "cal_persist", value, sizeof(value));
    if (err >= 0) {
        str_parms_del(parms, "cal_persist");
        cal->persist = (uint32_t) strtoul(value, NULL, 0);
        ret = ret | 0x1;
    }
    err = str_parms_get_str(parms, "cal_apptype", value, sizeof(value));
    if (err >= 0) {
        str_parms_del(parms, "cal_apptype");
        cal->app_type = (uint32_t) strtoul(value, NULL, 0);
        ret = ret | 0x2;
    }
    err = str_parms_get_str(parms, "cal_caltype", value, sizeof(value));
    if (err >= 0) {
        str_parms_del(parms, "cal_caltype");
        cal->cal_type = (uint32_t) strtoul(value, NULL, 0);
        ret = ret | 0x4;
    }
    err = str_parms_get_str(parms, "cal_samplerate", value, sizeof(value));
    if (err >= 0) {
        str_parms_del(parms, "cal_samplerate");
        cal->sampling_rate = (uint32_t) strtoul(value, NULL, 0);
        ret = ret | 0x8;
    }
    err = str_parms_get_str(parms, "cal_devid", value, sizeof(value));
    if (err >= 0) {
        str_parms_del(parms, "cal_devid");
        cal->dev_id = (uint32_t) strtoul(value, NULL, 0);
        ret = ret | 0x10;
    }
    err = str_parms_get_str(parms, "cal_snddevid", value, sizeof(value));
    if (err >= 0) {
        str_parms_del(parms, "cal_snddevid");
        cal->snd_dev_id = (uint32_t) strtoul(value, NULL, 0);
        ret = ret | 0x20;
    }
    err = str_parms_get_str(parms, "cal_topoid", value, sizeof(value));
    if (err >= 0) {
        str_parms_del(parms, "cal_topoid");
        cal->topo_id = (uint32_t) strtoul(value, NULL, 0);
        ret = ret | 0x40;
    }
    err = str_parms_get_str(parms, "cal_moduleid", value, sizeof(value));
    if (err >= 0) {
        str_parms_del(parms, "cal_moduleid");
        cal->module_id = (uint32_t) strtoul(value, NULL, 0);
        ret = ret | 0x80;
    }
    err = str_parms_get_str(parms, "cal_paramid", value, sizeof(value));
    if (err >= 0) {
        str_parms_del(parms, "cal_paramid");
        cal->param_id = (uint32_t) strtoul(value, NULL, 0);
        ret = ret | 0x100;
    }
    return ret;
}

static void set_audiocal(void *platform, struct str_parms *parms, char *value, int len) {
    struct platform_data *my_data = (struct platform_data *)platform;
    struct stream_out out;
    acdb_audio_cal_cfg_t cal={0};
    uint8_t *dptr = NULL;
    int32_t dlen;
    int err, ret;
    if(value == NULL || platform == NULL || parms == NULL) {
        ALOGE("[%s] received null pointer, failed",__func__);
        goto done_key_audcal;
    }

    /* parse audio calibration keys */
    ret = parse_audiocal_cfg(parms, &cal);

    /* handle audio calibration data now */
    err = str_parms_get_str(parms, AUDIO_PARAMETER_KEY_AUD_CALDATA, value, len);
    if (err >= 0) {
        str_parms_del(parms, AUDIO_PARAMETER_KEY_AUD_CALDATA);
        dlen = strlen(value);
        if(dlen <= 0) {
            ALOGE("[%s] null data received",__func__);
            goto done_key_audcal;
        }
        dptr = (uint8_t*) calloc(dlen, sizeof(uint8_t));
        if(dptr == NULL) {
            ALOGE("[%s] memory allocation failed for %d",__func__, dlen);
            goto done_key_audcal;
        }
        dlen = b64decode(value, strlen(value), dptr);
        if(dlen<=0) {
            ALOGE("[%s] data decoding failed %d", __func__, dlen);
            goto done_key_audcal;
        }

        if(cal.dev_id) {
          if(audio_is_input_device(cal.dev_id)) {
              cal.snd_dev_id = platform_get_input_snd_device(platform, cal.dev_id);
          } else {
              out.devices = cal.dev_id;
              cal.snd_dev_id = platform_get_output_snd_device(platform, &out);
          }
        }
        cal.acdb_dev_id = platform_get_snd_device_acdb_id(cal.snd_dev_id);
        ALOGD("Setting audio calibration for snd_device(%d) acdb_id(%d)",
                cal.snd_dev_id, cal.acdb_dev_id);
        if(cal.acdb_dev_id == -EINVAL) {
            ALOGE("[%s] Invalid acdb_device id %d for snd device id %d",
                       __func__, cal.acdb_dev_id, cal.snd_dev_id);
            goto done_key_audcal;
        }
        if(my_data->acdb_set_audio_cal) {
            ret = my_data->acdb_set_audio_cal((void *)&cal, (void*)dptr, dlen);
        }
    }
done_key_audcal:
    if(dptr != NULL)
        free(dptr);
}

static void true_32_bit_set_params(struct str_parms *parms,
                                 char *value, int len)
{
    int ret = 0;

    ret = str_parms_get_str(parms, AUDIO_PARAMETER_KEY_TRUE_32_BIT,
                            value,len);
    if (ret >= 0) {
        if (value && !strncmp(value, "true", sizeof("src")))
            supports_true_32_bit = true;
        else
            supports_true_32_bit = false;
        str_parms_del(parms, AUDIO_PARAMETER_KEY_TRUE_32_BIT);
    }

}


bool platform_supports_true_32bit()
{
   return supports_true_32_bit;
}

int platform_set_parameters(void *platform, struct str_parms *parms)
{
    struct platform_data *my_data = (struct platform_data *)platform;
    char *str;
    char *value=NULL;
    int val, len;
    int ret = 0, err;
    char *kv_pairs = str_parms_to_str(parms);

    if(kv_pairs == NULL) {
        ret = -ENOMEM;
        ALOGE("[%s] key-value pair is NULL",__func__);
        goto done;
    }

    ALOGV_IF(kv_pairs != NULL, "%s: enter: %s", __func__, kv_pairs);

    len = strlen(kv_pairs);
    value = (char*)calloc(len, sizeof(char));
    if(value == NULL) {
        ret = -ENOMEM;
        ALOGE("[%s] failed to allocate memory",__func__);
        goto done;
    }
    err = str_parms_get_str(parms, AUDIO_PARAMETER_KEY_SLOWTALK, value, len);
    if (err >= 0) {
        bool state = false;
        if (!strncmp("true", value, sizeof("true"))) {
            state = true;
        }

        str_parms_del(parms, AUDIO_PARAMETER_KEY_SLOWTALK);
        ret = platform_set_slowtalk(my_data, state);
        if (ret)
            ALOGE("%s: Failed to set slow talk err: %d", __func__, ret);
    }

    err = str_parms_get_str(parms, AUDIO_PARAMETER_KEY_HD_VOICE, value, len);
    if (err >= 0) {
        bool state = false;
        if (!strncmp("true", value, sizeof("true"))) {
            state = true;
        }

        str_parms_del(parms, AUDIO_PARAMETER_KEY_HD_VOICE);
        if (my_data->hd_voice != state) {
            ret = set_hd_voice(my_data, state);
            if (ret)
                ALOGE("%s: Failed to set HD voice err: %d", __func__, ret);
        } else {
            ALOGV("%s: HD Voice already set to %d", __func__, state);
        }
    }

    err = str_parms_get_str(parms, AUDIO_PARAMETER_KEY_VOLUME_BOOST,
                            value, len);
    if (err >= 0) {
        str_parms_del(parms, AUDIO_PARAMETER_KEY_VOLUME_BOOST);

        if (my_data->acdb_reload_vocvoltable == NULL) {
            ALOGE("%s: acdb_reload_vocvoltable is NULL", __func__);
        } else if (!strcmp(value, "on")) {
            if (!my_data->acdb_reload_vocvoltable(VOICE_FEATURE_SET_VOLUME_BOOST)) {
                my_data->voice_feature_set = 1;
            }
        } else {
            if (!my_data->acdb_reload_vocvoltable(VOICE_FEATURE_SET_DEFAULT)) {
                my_data->voice_feature_set = 0;
            }
        }
    }

    err = str_parms_get_str(parms, AUDIO_PARAMETER_KEY_MONO_SPEAKER, value, sizeof(value));
    if (err >= 0) {
        if (!strncmp("left", value, sizeof("left")))
            my_data->mono_speaker = SPKR_1;
        else if (!strncmp("right", value, sizeof("right")))
            my_data->mono_speaker = SPKR_2;

        str_parms_del(parms, AUDIO_PARAMETER_KEY_MONO_SPEAKER);
    }

    err = str_parms_get_str(parms, AUDIO_PARAMETER_KEY_EXT_AUDIO_DEVICE,
                            value, len);
    if (err >= 0) {
        char *event_name, *status_str;
        bool status = false;
        str_parms_del(parms, AUDIO_PARAMETER_KEY_EXT_AUDIO_DEVICE);
        event_name = strtok_r(value, ",", &status_str);
        if (!event_name) {
            ret = -EINVAL;
            ALOGE("%s: event_name is NULL", __func__);
            goto done;
        }
        ALOGV("%s: recieved update of external audio device %s %s",
                         __func__,
                         event_name, status_str);
        if (!strncmp(status_str, "ON", sizeof("ON")))
            status = true;
        else if (!strncmp(status_str, "OFF", sizeof("OFF")))
            status = false;
        update_external_device_status(my_data, event_name, status);
    }

    err = str_parms_get_str(parms, PLATFORM_MAX_MIC_COUNT,
                            value, sizeof(value));
    if (err >= 0) {
        str_parms_del(parms, PLATFORM_MAX_MIC_COUNT);
        my_data->max_mic_count = atoi(value);
        ALOGV("%s: max_mic_count %d", __func__, my_data->max_mic_count);
    }

    /* handle audio calibration parameters */
    set_audiocal(platform, parms, value, len);
    native_audio_set_params(platform, parms, value, len);
    true_32_bit_set_params(parms, value, len);
done:
    ALOGV("%s: exit with code(%d)", __func__, ret);
    if(kv_pairs != NULL)
        free(kv_pairs);
    if(value != NULL)
        free(value);
    return ret;
}

int platform_set_incall_recording_session_id(void *platform,
                                             uint32_t session_id, int rec_mode)
{
    int ret = 0;
    struct platform_data *my_data = (struct platform_data *)platform;
    struct audio_device *adev = my_data->adev;
    struct mixer_ctl *ctl;
    const char *mixer_ctl_name = "Voc VSID";
    int num_ctl_values;
    int i;

    ctl = mixer_get_ctl_by_name(adev->mixer, mixer_ctl_name);
    if (!ctl) {
        ALOGE("%s: Could not get ctl for mixer cmd - %s",
              __func__, mixer_ctl_name);
        ret = -EINVAL;
    } else {
        num_ctl_values = mixer_ctl_get_num_values(ctl);
        for (i = 0; i < num_ctl_values; i++) {
            if (mixer_ctl_set_value(ctl, i, session_id)) {
                ALOGV("Error: invalid session_id: %x", session_id);
                ret = -EINVAL;
                break;
            }
        }
    }

    if (my_data->csd != NULL) {
        ret = my_data->csd->start_record(ALL_SESSION_VSID, rec_mode);
        if (ret < 0) {
            ALOGE("%s: csd_client_start_record failed, error %d",
                  __func__, ret);
        }
    }

    return ret;
}

int platform_stop_incall_recording_usecase(void *platform)
{
    int ret = 0;
    struct platform_data *my_data = (struct platform_data *)platform;

    if (my_data->csd != NULL) {
        ret = my_data->csd->stop_record(ALL_SESSION_VSID);
        if (ret < 0) {
            ALOGE("%s: csd_client_stop_record failed, error %d",
                  __func__, ret);
        }
    }

    return ret;
}

int platform_start_incall_music_usecase(void *platform)
{
    int ret = 0;
    struct platform_data *my_data = (struct platform_data *)platform;

    if (my_data->csd != NULL) {
        ret = my_data->csd->start_playback(ALL_SESSION_VSID);
        if (ret < 0) {
            ALOGE("%s: csd_client_start_playback failed, error %d",
                  __func__, ret);
        }
    }

    return ret;
}

int platform_stop_incall_music_usecase(void *platform)
{
    int ret = 0;
    struct platform_data *my_data = (struct platform_data *)platform;

    if (my_data->csd != NULL) {
        ret = my_data->csd->stop_playback(ALL_SESSION_VSID);
        if (ret < 0) {
            ALOGE("%s: csd_client_stop_playback failed, error %d",
                  __func__, ret);
        }
    }

    return ret;
}

int platform_update_lch(void *platform, struct voice_session *session,
                        enum voice_lch_mode lch_mode)
{
    int ret = 0;
    struct platform_data *my_data = (struct platform_data *)platform;

    if ((my_data->csd != NULL) && (my_data->csd->set_lch != NULL))
        ret = my_data->csd->set_lch(session->vsid, lch_mode);
    else
        ret = pcm_ioctl(session->pcm_tx, SNDRV_VOICE_IOCTL_LCH, &lch_mode);

    return ret;
}

static void get_audiocal(void *platform, void *keys, void *pReply) {
    struct platform_data *my_data = (struct platform_data *)platform;
    struct stream_out out;
    struct str_parms *query = (struct str_parms *)keys;
    struct str_parms *reply=(struct str_parms *)pReply;
    acdb_audio_cal_cfg_t cal={0};
    uint8_t *dptr = NULL;
    char value[512] = {0};
    char *rparms=NULL;
    int ret=0, err;
    uint32_t param_len;

    if(query==NULL || platform==NULL || reply==NULL) {
        ALOGE("[%s] received null pointer",__func__);
        ret=-EINVAL;
        goto done;
    }
    /* parse audiocal configuration keys */
    ret = parse_audiocal_cfg(query, &cal);
    if(ret == 0) {
        /* No calibration keys found */
        goto done;
    }
    err = str_parms_get_str(query, AUDIO_PARAMETER_KEY_AUD_CALDATA, value, sizeof(value));
    if (err >= 0) {
        str_parms_del(query, AUDIO_PARAMETER_KEY_AUD_CALDATA);
    } else {
        goto done;
    }

    if(cal.dev_id & AUDIO_DEVICE_BIT_IN) {
        cal.snd_dev_id = platform_get_input_snd_device(platform, cal.dev_id);
    } else if(cal.dev_id) {
        out.devices = cal.dev_id;
        cal.snd_dev_id = platform_get_output_snd_device(platform, &out);
    }
    cal.acdb_dev_id =  platform_get_snd_device_acdb_id(cal.snd_dev_id);
    if (cal.acdb_dev_id < 0) {
        ALOGE("%s: Failed. Could not find acdb id for snd device(%d)",
              __func__, cal.snd_dev_id);
        ret = -EINVAL;
        goto done_key_audcal;
    }
    ALOGD("[%s] Getting audio calibration for snd_device(%d) acdb_id(%d)",
           __func__, cal.snd_dev_id, cal.acdb_dev_id);

    param_len = MAX_SET_CAL_BYTE_SIZE;
    dptr = (uint8_t*)calloc(param_len, sizeof(uint8_t));
    if(dptr == NULL) {
        ALOGE("[%s] Memory allocation failed for length %d",__func__,param_len);
        ret = -ENOMEM;
        goto done_key_audcal;
    }
    if (my_data->acdb_get_audio_cal != NULL) {
        ret = my_data->acdb_get_audio_cal((void*)&cal, (void*)dptr, &param_len);
        if (ret == 0) {
            int dlen;
            if(param_len == 0 || param_len == MAX_SET_CAL_BYTE_SIZE) {
                ret = -EINVAL;
                goto done_key_audcal;
            }
            /* Allocate memory for encoding */
            rparms = (char*)calloc((param_len*2), sizeof(char));
            if(rparms == NULL) {
                ALOGE("[%s] Memory allocation failed for size %d",
                            __func__, param_len*2);
                ret = -ENOMEM;
                goto done_key_audcal;
            }
            if(cal.persist==0 && cal.module_id && cal.param_id) {
                err = b64encode(dptr+12, param_len-12, rparms);
            } else {
                err = b64encode(dptr, param_len, rparms);
            }
            if(err < 0) {
                ALOGE("[%s] failed to convert data to string", __func__);
                ret = -EINVAL;
                goto done_key_audcal;
            }
            str_parms_add_int(reply, AUDIO_PARAMETER_KEY_AUD_CALRESULT, ret);
            str_parms_add_str(reply, AUDIO_PARAMETER_KEY_AUD_CALDATA, rparms);
        }
    }
done_key_audcal:
    if(ret != 0) {
        str_parms_add_int(reply, AUDIO_PARAMETER_KEY_AUD_CALRESULT, ret);
        str_parms_add_str(reply, AUDIO_PARAMETER_KEY_AUD_CALDATA, "");
    }
done:
    if(dptr != NULL)
        free(dptr);
    if(rparms != NULL)
        free(rparms);
}

void platform_get_parameters(void *platform,
                            struct str_parms *query,
                            struct str_parms *reply)
{
    struct platform_data *my_data = (struct platform_data *)platform;
    char *str = NULL;
    char value[512] = {0};
    int ret;
    char *kv_pairs = NULL;

    ret = str_parms_get_str(query, AUDIO_PARAMETER_KEY_SLOWTALK,
                            value, sizeof(value));
    if (ret >= 0) {
        str_parms_add_str(reply, AUDIO_PARAMETER_KEY_SLOWTALK,
                          my_data->slowtalk?"true":"false");
    }

    ret = str_parms_get_str(query, AUDIO_PARAMETER_KEY_HD_VOICE,
                            value, sizeof(value));
    if (ret >= 0) {
        str_parms_add_str(reply, AUDIO_PARAMETER_KEY_HD_VOICE,
                          my_data->hd_voice?"true":"false");
    }

    ret = str_parms_get_str(query, AUDIO_PARAMETER_KEY_VOLUME_BOOST,
                            value, sizeof(value));
    if (ret >= 0) {
        if (my_data->voice_feature_set == VOICE_FEATURE_SET_VOLUME_BOOST) {
            strlcpy(value, "on", sizeof(value));
        } else {
            strlcpy(value, "off", sizeof(value));
        }

        str_parms_add_str(reply, AUDIO_PARAMETER_KEY_VOLUME_BOOST, value);
    }

    /* Handle audio calibration keys */
    get_audiocal(platform, query, reply);
    native_audio_get_params(query, reply, value, sizeof(value));

    ret = str_parms_get_str(query, AUDIO_PARAMETER_IS_HW_DECODER_SESSION_ALLOWED,
                                    value, sizeof(value));
    if (ret >= 0) {
        int isallowed = 1; /*true*/

        if (property_get("vendor.voice.playback.conc.disabled", propValue, NULL)) {
            prop_playback_enabled = atoi(propValue) ||
                !strncmp("true", propValue, 4);
        }

        if ((prop_playback_enabled && (voice_is_in_call(my_data->adev))) ||
             (CARD_STATUS_OFFLINE == my_data->adev->card_status)) {
            char *decoder_mime_type = value;

            //check if unsupported mime type or not
            if(decoder_mime_type) {
                int i = 0;
                for (i = 0; i < sizeof(dsp_only_decoders_mime)/sizeof(dsp_only_decoders_mime[0]); i++) {
                    if (!strncmp(decoder_mime_type, dsp_only_decoders_mime[i],
                    strlen(dsp_only_decoders_mime[i]))) {
                       ALOGD("Rejecting request for DSP only session from HAL during voice call/SSR state");
                       isallowed = 0;
                       break;
                    }
                }
            }
        }
        str_parms_add_int(reply, AUDIO_PARAMETER_IS_HW_DECODER_SESSION_ALLOWED, isallowed);
    }

done:
    kv_pairs = str_parms_to_str(reply);
    ALOGV_IF(kv_pairs != NULL, "%s: exit: returns - %s", __func__, kv_pairs);
    free(kv_pairs);
}

/* Delay in Us, only to be used for PCM formats */
int64_t platform_render_latency(audio_usecase_t usecase)
{
    switch (usecase) {
        case USECASE_AUDIO_PLAYBACK_DEEP_BUFFER:
            return DEEP_BUFFER_PLATFORM_DELAY;
        case USECASE_AUDIO_PLAYBACK_LOW_LATENCY:
            return LOW_LATENCY_PLATFORM_DELAY;
        case USECASE_AUDIO_PLAYBACK_OFFLOAD:
        case USECASE_AUDIO_PLAYBACK_OFFLOAD2:
             return PCM_OFFLOAD_PLATFORM_DELAY;
        default:
            return 0;
    }
}

int platform_update_usecase_from_source(int source, int usecase)
{
    ALOGV("%s: input source :%d", __func__, source);
    switch(source) {
        case AUDIO_SOURCE_VOICE_UPLINK:
            return USECASE_INCALL_REC_UPLINK;
        case AUDIO_SOURCE_VOICE_DOWNLINK:
            return USECASE_INCALL_REC_DOWNLINK;
        case AUDIO_SOURCE_VOICE_CALL:
            return USECASE_INCALL_REC_UPLINK_AND_DOWNLINK;
        case AUDIO_SOURCE_FM_TUNER:
            return USECASE_AUDIO_RECORD_FM_VIRTUAL;
        default:
            return usecase;
    }
}

bool platform_listen_device_needs_event(snd_device_t snd_device)
{
    bool needs_event = false;

    if ((snd_device >= SND_DEVICE_IN_BEGIN) &&
        (snd_device < SND_DEVICE_IN_END) &&
        (snd_device != SND_DEVICE_IN_CAPTURE_FM) &&
        (snd_device != SND_DEVICE_IN_CAPTURE_VI_FEEDBACK) &&
        (snd_device != SND_DEVICE_IN_CAPTURE_VI_FEEDBACK_MONO_1) &&
        (snd_device != SND_DEVICE_IN_CAPTURE_VI_FEEDBACK_MONO_2))
        needs_event = true;

    return needs_event;
}

bool platform_listen_usecase_needs_event(audio_usecase_t uc_id __unused)
{
    return false;
}

bool platform_sound_trigger_device_needs_event(snd_device_t snd_device)
{
    bool needs_event = false;

    if ((snd_device >= SND_DEVICE_IN_BEGIN) &&
        (snd_device < SND_DEVICE_IN_END) &&
        (snd_device != SND_DEVICE_IN_CAPTURE_FM) &&
        (snd_device != SND_DEVICE_IN_CAPTURE_VI_FEEDBACK) &&
        (snd_device != SND_DEVICE_IN_CAPTURE_VI_FEEDBACK_MONO_1) &&
        (snd_device != SND_DEVICE_IN_CAPTURE_VI_FEEDBACK_MONO_2))
        needs_event = true;

    return needs_event;
}

bool platform_sound_trigger_usecase_needs_event(audio_usecase_t uc_id __unused)
{
    return false;
}

/* Read  offload buffer size from a property.
 * If value is not power of 2  round it to
 * power of 2.
 */
uint32_t platform_get_compress_offload_buffer_size(audio_offload_info_t* info)
{
    char value[PROPERTY_VALUE_MAX] = {0};
    uint32_t fragment_size = COMPRESS_OFFLOAD_FRAGMENT_SIZE;
    if((property_get("vendor.audio.offload.buffer.size.kb", value, "")) &&
            atoi(value)) {
        fragment_size =  atoi(value) * 1024;
    }

    /* Use incoming offload buffer size if default buffer size is less */
    if ((info != NULL) && (fragment_size < info->offload_buffer_size)) {
        ALOGI("%s:: Overwriting offload buffer size default:%d new:%d", __func__,
              fragment_size,
              info->offload_buffer_size);
        fragment_size = info->offload_buffer_size;
    }

    // For FLAC use max size since it is loss less, and has sampling rates
    // upto 192kHZ
    if (info != NULL && !info->has_video &&
        info->format == AUDIO_FORMAT_FLAC) {
       fragment_size = MAX_COMPRESS_OFFLOAD_FRAGMENT_SIZE;
       ALOGV("FLAC fragment size %d", fragment_size);
    }

    if (info != NULL && info->has_video && info->is_streaming) {
        fragment_size = COMPRESS_OFFLOAD_FRAGMENT_SIZE_FOR_AV_STREAMING;
        ALOGV("%s: offload fragment size reduced for AV streaming to %d",
               __func__, fragment_size);
    }

    fragment_size = ALIGN( fragment_size, 1024);

    if(fragment_size < MIN_COMPRESS_OFFLOAD_FRAGMENT_SIZE)
        fragment_size = MIN_COMPRESS_OFFLOAD_FRAGMENT_SIZE;
    else if(fragment_size > MAX_COMPRESS_OFFLOAD_FRAGMENT_SIZE)
        fragment_size = MAX_COMPRESS_OFFLOAD_FRAGMENT_SIZE;
    ALOGV("%s: fragment_size %d", __func__, fragment_size);
    return fragment_size;
}

uint32_t platform_get_pcm_offload_buffer_size(audio_offload_info_t* info)
{
    uint32_t fragment_size = 0;
    uint32_t bytes_per_sample;
    uint32_t pcm_offload_time = PCM_OFFLOAD_BUFFER_DURATION;

    bytes_per_sample = audio_bytes_per_sample(info->format);

    //duration is set to 40 ms worth of stereo data at 48Khz
    //with 16 bit per sample, modify this when the channel
    //configuration is different
    fragment_size = (pcm_offload_time
                     * info->sample_rate
                     * bytes_per_sample
                     * popcount(info->channel_mask))/1000;
    if(fragment_size < MIN_PCM_OFFLOAD_FRAGMENT_SIZE)
        fragment_size = MIN_PCM_OFFLOAD_FRAGMENT_SIZE;
    else if(fragment_size > MAX_PCM_OFFLOAD_FRAGMENT_SIZE)
        fragment_size = MAX_PCM_OFFLOAD_FRAGMENT_SIZE;

    // To have same PCM samples for all channels, the buffer size requires to
    // be multiple of (number of channels * bytes per sample)
    // For writes to succeed, the buffer must be written at address which is multiple of 32
    fragment_size = ALIGN(fragment_size, ((bytes_per_sample) * popcount(info->channel_mask) * 32));

    ALOGI("PCM offload Fragment size to %d bytes", fragment_size);
    return fragment_size;
}

/*
 * configures afe with bit width and Sample Rate
 */
static int platform_set_codec_backend_cfg(struct audio_device* adev,
                         snd_device_t snd_device, unsigned int bit_width,
                         unsigned int sample_rate, audio_format_t format)
{
    int ret = 0;
    int backend_idx = DEFAULT_CODEC_BACKEND;
    struct platform_data *my_data = (struct platform_data *)adev->platform;

    backend_idx = platform_get_backend_index(snd_device);
    ALOGV("%s bit width: %d, sample rate: %d backend_idx - %d",
            __func__, bit_width, sample_rate, backend_idx);

    if (bit_width !=
        my_data->current_backend_cfg[backend_idx].bit_width) {

        struct  mixer_ctl *ctl;
        ctl = mixer_get_ctl_by_name(adev->mixer,
                    my_data->current_backend_cfg[backend_idx].bitwidth_mixer_ctl);
        if (!ctl) {
            ALOGE("%s: Could not get ctl for mixer command - %s",
                    __func__, my_data->current_backend_cfg[backend_idx].bitwidth_mixer_ctl);
            return -EINVAL;
        }

        if (bit_width == 24) {
            if (format == AUDIO_FORMAT_PCM_24_BIT_PACKED)
                 mixer_ctl_set_enum_by_string(ctl, "S24_3LE");
            else
                 mixer_ctl_set_enum_by_string(ctl, "S24_LE");
        } else {
            mixer_ctl_set_enum_by_string(ctl, "S16_LE");
            if (backend_idx != HEADPHONE_44_1_BACKEND)
                sample_rate = CODEC_BACKEND_DEFAULT_SAMPLE_RATE;
        }
        my_data->current_backend_cfg[backend_idx].bit_width = bit_width;
        ALOGD("%s:becf: afe: %s mixer set to %d bit for %x format", __func__,
              my_data->current_backend_cfg[backend_idx].bitwidth_mixer_ctl, bit_width, format);
    }

    /*
     * Backend sample rate configuration follows:
     * 16 bit playback - 48khz for streams at any valid sample rate
     * 24 bit playback - 48khz for stream sample rate less than 48khz
     * 24 bit playback - 96khz for sample rate range of 48khz to 96khz
     * 24 bit playback - 192khz for sample rate range of 96khz to 192 khz
     * native support is present: 16/24 bit 44.1Khz clip uses backend at 44.1Khz
     * native support is not present: 44.1Khz clips uses backend at 48Khz
     * Upper limit is inclusive in the sample rate range.
     */
    if (sample_rate !=
       my_data->current_backend_cfg[backend_idx].sample_rate) {
            char *rate_str = NULL;
            struct  mixer_ctl *ctl;

            switch (sample_rate) {
            case 8000:
            case 11025:
            case 16000:
            case 22050:
            case 32000:
            case 48000:
                rate_str = "KHZ_48";
                break;
            case 44100:
                rate_str = "KHZ_44P1";
                break;
            case 64000:
            case 88200:
            case 96000:
                rate_str = "KHZ_96";
                break;
            case 176400:
            case 192000:
                rate_str = "KHZ_192";
                break;
            default:
                rate_str = "KHZ_48";
                break;
            }

            ctl = mixer_get_ctl_by_name(adev->mixer,
                my_data->current_backend_cfg[backend_idx].samplerate_mixer_ctl);
            if(!ctl) {
                ALOGE("%s: Could not get ctl for mixer command - %s",
                    __func__, my_data->current_backend_cfg[backend_idx].samplerate_mixer_ctl);
                return -EINVAL;
            }

            ALOGD("%s: %s set to %s", __func__,
                my_data->current_backend_cfg[backend_idx].samplerate_mixer_ctl, rate_str);
            mixer_ctl_set_enum_by_string(ctl, rate_str);
            my_data->current_backend_cfg[backend_idx].sample_rate = sample_rate;
    }

    return ret;
}

bool platform_check_codec_backend_cfg(struct audio_device* adev,
                                   struct audio_usecase* usecase,
                                   snd_device_t snd_device,
                                   unsigned int* new_bit_width,
                                   unsigned int* new_sample_rate)
{
    bool backend_change = false;
    struct listnode *node;
    struct stream_out *out = NULL;
    unsigned int bit_width;
    unsigned int sample_rate;
    char value[PROPERTY_VALUE_MAX] = {0};
    int backend_idx = DEFAULT_CODEC_BACKEND;
    int usecase_backend_idx = DEFAULT_CODEC_BACKEND;
    struct platform_data *my_data = (struct platform_data *)adev->platform;

    backend_idx = platform_get_backend_index(snd_device);

    bit_width = *new_bit_width;
    sample_rate = *new_sample_rate;

    ALOGI("%s Codec selected backend: %d current bit width: %d and sample rate: %d",
               __func__, backend_idx, bit_width, sample_rate);

    // For voice calls use default configuration i.e. 16b/48K, only applicable to
    // default backend
    // force routing is not required here, caller will do it anyway
    if ((voice_is_in_call(adev) || adev->mode == AUDIO_MODE_IN_COMMUNICATION) &&
        backend_idx == DEFAULT_CODEC_BACKEND) {
        ALOGW("%s:Use default bw and sr for voice/voip calls ",__func__);
        bit_width = CODEC_BACKEND_DEFAULT_BIT_WIDTH;
        sample_rate =  CODEC_BACKEND_DEFAULT_SAMPLE_RATE;
    } else {
        /*
         * The backend should be configured at highest bit width and/or
         * sample rate amongst all playback usecases.
         * If the selected sample rate and/or bit width differ with
         * current backend sample rate and/or bit width, then, we set the
         * backend re-configuration flag.
         *
         * Exception: 16 bit playbacks is allowed through 16 bit/48 khz backend only
         */
        list_for_each(node, &adev->usecase_list) {
            struct audio_usecase *curr_usecase;
            curr_usecase = node_to_item(node, struct audio_usecase, list);
            if (curr_usecase->type == PCM_PLAYBACK &&
                usecase != curr_usecase) {
                struct stream_out *out =
                           (struct stream_out*) curr_usecase->stream.out;
                usecase_backend_idx = platform_get_backend_index(curr_usecase->out_snd_device);

                if (out != NULL &&
                    usecase_backend_idx == backend_idx) {
                    ALOGV("%s: usecase Offload playback running bw %d sr %d device %s be_idx %d",
                            __func__, out->bit_width, out->sample_rate,
                            platform_get_snd_device_name(curr_usecase->out_snd_device), usecase_backend_idx);
                        if (bit_width < out->bit_width)
                            bit_width = out->bit_width;
                        if (sample_rate < out->sample_rate)
                            sample_rate = out->sample_rate;
                        if (out->sample_rate < OUTPUT_SAMPLING_RATE_44100)
                            sample_rate = CODEC_BACKEND_DEFAULT_SAMPLE_RATE;
                }
            }
        }
    }
    if (backend_idx != HEADPHONE_44_1_BACKEND) {
        // 16 bit playbacks are allowed through 16 bit/48 khz backend only for
        // all non-native streams
        if (16 == bit_width) {
            sample_rate = CODEC_BACKEND_DEFAULT_SAMPLE_RATE;
            ALOGD("%s: resetting sample_rate back to default, "
                   "backend_idx: %d", __func__, backend_idx);
        }

        //check if mulitchannel clip needs to be down sampled  to 48k
        property_get("vendor.audio.playback.mch.downsample",value,"");
        if (!strncmp("true", value, sizeof("true"))) {
            out = usecase->stream.out;
            if ((popcount(out->channel_mask) > 2) &&
                          (out->sample_rate > CODEC_BACKEND_DEFAULT_SAMPLE_RATE) &&
                          !(out->flags & AUDIO_OUTPUT_FLAG_COMPRESS_PASSTHROUGH)) {
                /* update out sample rate to reflect current backend sample rate  */
                sample_rate = CODEC_BACKEND_DEFAULT_SAMPLE_RATE;
                ALOGD("%s: MCH session defaulting sample rate to %d",
                     __func__, sample_rate);
            }
        }
        // 24 bit playback on speakers is allowed through 48 khz backend only
        // bit width re-configured based on platform info
        if ((24 == bit_width) &&
            (usecase->stream.out->devices & AUDIO_DEVICE_OUT_SPEAKER)) {
            bit_width = (uint32_t)platform_get_snd_device_bit_width(SND_DEVICE_OUT_SPEAKER);
            sample_rate = CODEC_BACKEND_DEFAULT_SAMPLE_RATE;
        }
        // 24 bit native clips must be played at 48Khz for non native backend
        if ((24 == bit_width) && (OUTPUT_SAMPLING_RATE_44100 == sample_rate)) {
            sample_rate = CODEC_BACKEND_DEFAULT_SAMPLE_RATE;
        }
    }
    ALOGI("%s Codec selected backend: %d updated bit width: %d and sample rate: %d",
               __func__, backend_idx, bit_width, sample_rate);
    // Force routing if the expected bitwdith or samplerate
    // is not same as current backend comfiguration
    if ((bit_width != my_data->current_backend_cfg[backend_idx].bit_width) ||
        (sample_rate != my_data->current_backend_cfg[backend_idx].sample_rate)) {
        *new_bit_width = bit_width;
        *new_sample_rate = sample_rate;
        backend_change = true;
        ALOGI("%s Codec backend needs to be updated. new bit width: %d new sample rate: %d",
               __func__, *new_bit_width, *new_sample_rate);
    }

    return backend_change;
}

bool platform_check_and_set_codec_backend_cfg(struct audio_device* adev,
    struct audio_usecase *usecase, snd_device_t snd_device)
{
    unsigned int new_bit_width;
    unsigned int new_sample_rate;
    int backend_idx = DEFAULT_CODEC_BACKEND;
    struct platform_data *my_data = (struct platform_data *)adev->platform;
    audio_format_t format;

    ALOGV("%s: usecase = %d", __func__, usecase->id );

    backend_idx = platform_get_backend_index(snd_device);

    new_bit_width = usecase->stream.out->bit_width;
    new_sample_rate = usecase->stream.out->sample_rate;
    format = usecase->stream.out->format;

    ALOGI("%s: Usecase bitwidth %d, samplerate %d, backend_idx %d",
        __func__, new_bit_width, new_sample_rate, backend_idx);
    if (platform_check_codec_backend_cfg(adev, usecase, snd_device,
                                      &new_bit_width, &new_sample_rate)) {
        platform_set_codec_backend_cfg(adev, snd_device,
                                       new_bit_width, new_sample_rate, format);
        return true;
    }

    return false;
}

int platform_set_snd_device_backend(snd_device_t device, const char *backend)
{
    int ret = 0;

    if ((device < SND_DEVICE_MIN) || (device >= SND_DEVICE_MAX)) {
        ALOGE("%s: Invalid snd_device = %d",
            __func__, device);
        ret = -EINVAL;
        goto done;
    }

    if (backend_table[device]) {
        free(backend_table[device]);
    }
    backend_table[device] = strdup(backend);
done:
    return ret;
}

int platform_set_usecase_pcm_id(audio_usecase_t usecase, int32_t type, int32_t pcm_id)
{
    int ret = 0;
    if ((usecase <= USECASE_INVALID) || (usecase >= AUDIO_USECASE_MAX)) {
        ALOGE("%s: invalid usecase case idx %d", __func__, usecase);
        ret = -EINVAL;
        goto done;
    }

    if ((type != 0) && (type != 1)) {
        ALOGE("%s: invalid usecase type", __func__);
        ret = -EINVAL;
    }
    pcm_device_table[usecase][type] = pcm_id;
done:
    return ret;
}

void platform_get_device_to_be_id_map(int **device_to_be_id, int *length)
{
     *device_to_be_id = msm_device_to_be_id;
     *length = msm_be_id_array_len;
}

int platform_set_stream_channel_map(void *platform, audio_channel_mask_t channel_mask, int snd_id)
{
    int ret = 0;
    int channels = audio_channel_count_from_out_mask(channel_mask);

    char channel_map[8];
    memset(channel_map, 0, sizeof(channel_map));
    /* Following are all most common standard WAV channel layouts
       overridden by channel mask if its allowed and different */
    switch (channels) {
        case 1:
            /* AUDIO_CHANNEL_OUT_MONO */
            channel_map[0] = PCM_CHANNEL_FC;
            break;
        case 2:
            /* AUDIO_CHANNEL_OUT_STEREO */
            channel_map[0] = PCM_CHANNEL_FL;
            channel_map[1] = PCM_CHANNEL_FR;
            break;
        case 3:
            /* AUDIO_CHANNEL_OUT_2POINT1 */
            channel_map[0] = PCM_CHANNEL_FL;
            channel_map[1] = PCM_CHANNEL_FR;
            channel_map[2] = PCM_CHANNEL_FC;
            break;
        case 4:
            /* AUDIO_CHANNEL_OUT_QUAD_SIDE */
            channel_map[0] = PCM_CHANNEL_FL;
            channel_map[1] = PCM_CHANNEL_FR;
            channel_map[2] = PCM_CHANNEL_LS;
            channel_map[3] = PCM_CHANNEL_RS;
            if (channel_mask == AUDIO_CHANNEL_OUT_QUAD_BACK)
            {
                channel_map[2] = PCM_CHANNEL_LB;
                channel_map[3] = PCM_CHANNEL_RB;
            }
            if (channel_mask == AUDIO_CHANNEL_OUT_SURROUND)
            {
                channel_map[2] = PCM_CHANNEL_FC;
                channel_map[3] = PCM_CHANNEL_CS;
            }
            break;
        case 5:
            /* AUDIO_CHANNEL_OUT_PENTA */
            channel_map[0] = PCM_CHANNEL_FL;
            channel_map[1] = PCM_CHANNEL_FR;
            channel_map[2] = PCM_CHANNEL_FC;
            channel_map[3] = PCM_CHANNEL_LB;
            channel_map[4] = PCM_CHANNEL_RB;
            break;
        case 6:
            /* AUDIO_CHANNEL_OUT_5POINT1 */
            channel_map[0] = PCM_CHANNEL_FL;
            channel_map[1] = PCM_CHANNEL_FR;
            channel_map[2] = PCM_CHANNEL_FC;
            channel_map[3] = PCM_CHANNEL_LFE;
            channel_map[4] = PCM_CHANNEL_LB;
            channel_map[5] = PCM_CHANNEL_RB;
            if (channel_mask == AUDIO_CHANNEL_OUT_5POINT1_SIDE)
            {
                channel_map[4] = PCM_CHANNEL_LS;
                channel_map[5] = PCM_CHANNEL_RS;
            }
            break;
        case 7:
            /* AUDIO_CHANNEL_OUT_6POINT1 */
            channel_map[0] = PCM_CHANNEL_FL;
            channel_map[1] = PCM_CHANNEL_FR;
            channel_map[2] = PCM_CHANNEL_FC;
            channel_map[3] = PCM_CHANNEL_LFE;
            channel_map[4] = PCM_CHANNEL_LB;
            channel_map[5] = PCM_CHANNEL_RB;
            channel_map[6] = PCM_CHANNEL_CS;
            break;
        case 8:
            /* AUDIO_CHANNEL_OUT_7POINT1 */
            channel_map[0] = PCM_CHANNEL_FL;
            channel_map[1] = PCM_CHANNEL_FR;
            channel_map[2] = PCM_CHANNEL_FC;
            channel_map[3] = PCM_CHANNEL_LFE;
            channel_map[4] = PCM_CHANNEL_LB;
            channel_map[5] = PCM_CHANNEL_RB;
            channel_map[6] = PCM_CHANNEL_LS;
            channel_map[7] = PCM_CHANNEL_RS;
            break;
        default:
            ALOGE("unsupported channels %d for setting channel map", channels);
            return -1;
    }
    ret = platform_set_channel_map(platform, channels, channel_map, snd_id);
    return ret;
}

int platform_get_edid_info(void *platform)
{
    struct platform_data *my_data = (struct platform_data *)platform;
    struct audio_device *adev = my_data->adev;
    char block[MAX_SAD_BLOCKS * SAD_BLOCK_SIZE];
    char *sad = block;
    int num_audio_blocks;
    int channel_count = 2;
    int i, ret, count;

    struct mixer_ctl *ctl;
    char edid_data[MAX_SAD_BLOCKS * SAD_BLOCK_SIZE + 1] = {0};
    edid_audio_info *info;

    if (my_data->edid_valid) {
        /* use cached edid */
        return 0;
    }

    if (my_data->edid_info == NULL) {
        my_data->edid_info =
            (struct edid_audio_info *)calloc(1, sizeof(struct edid_audio_info));
    }

    info = my_data->edid_info;

    ctl = mixer_get_ctl_by_name(adev->mixer, AUDIO_DATA_BLOCK_MIXER_CTL);
    if (!ctl) {
        ALOGE("%s: Could not get ctl for mixer cmd - %s",
              __func__, AUDIO_DATA_BLOCK_MIXER_CTL);
        goto fail;
    }

    mixer_ctl_update(ctl);

    count = mixer_ctl_get_num_values(ctl);

    /* Read SAD blocks, clamping the maximum size for safety */
    if (count > (int)sizeof(block))
        count = (int)sizeof(block);

    ret = mixer_ctl_get_array(ctl, block, count);
    if (ret != 0) {
        ALOGE("%s: mixer_ctl_get_array() failed to get EDID info", __func__);
        goto fail;
    }
    edid_data[0] = count;
    memcpy(&edid_data[1], block, count);

    if (!edid_get_sink_caps(info, edid_data)) {
        ALOGE("%s: Failed to get HDMI sink capabilities", __func__);
        goto fail;
    }
    my_data->edid_valid = true;
    return 0;
fail:
    if (my_data->edid_info) {
        free(my_data->edid_info);
        my_data->edid_info = NULL;
        my_data->edid_valid = false;
    }
    ALOGE("%s: return -EINVAL", __func__);
    return -EINVAL;
}


int platform_set_channel_allocation(void *platform, int channel_alloc)
{
    struct mixer_ctl *ctl;
    const char *mixer_ctl_name = "HDMI RX CA";
    int ret;
    struct platform_data *my_data = (struct platform_data *)platform;
    struct audio_device *adev = my_data->adev;

    ctl = mixer_get_ctl_by_name(adev->mixer, mixer_ctl_name);
    if (!ctl) {
        ALOGE("%s: Could not get ctl for mixer cmd - %s",
              __func__, mixer_ctl_name);
        ret = EINVAL;
    }
    ALOGD(":%s channel allocation = 0x%x", __func__, channel_alloc);
    ret = mixer_ctl_set_value(ctl, 0, channel_alloc);

    if (ret < 0) {
        ALOGE("%s: Could not set ctl, error:%d ", __func__, ret);
    }

    return ret;
}

int platform_set_channel_map(void *platform, int ch_count, char *ch_map, int snd_id)
{
    struct mixer_ctl *ctl;
    char mixer_ctl_name[44] = {0}; // max length of name is 44 as defined
    int ret;
    unsigned int i;
    int set_values[FCC_8] = {0};
    struct platform_data *my_data = (struct platform_data *)platform;
    struct audio_device *adev = my_data->adev;
    ALOGV("%s channel_count:%d",__func__, ch_count);
    if (NULL == ch_map || (ch_count < 1) || (ch_count > FCC_8)) {
        ALOGE("%s: Invalid channel mapping or channel count value", __func__);
        return -EINVAL;
    }

    /*
     * If snd_id is greater than 0, stream channel mapping
     * If snd_id is below 0, typically -1, device channel mapping
     */
    if (snd_id >= 0) {
        snprintf(mixer_ctl_name, sizeof(mixer_ctl_name), "Playback Channel Map%d", snd_id);
    } else {
        strlcpy(mixer_ctl_name, "Playback Device Channel Map", sizeof(mixer_ctl_name));
    }

    ALOGD("%s mixer_ctl_name:%s", __func__, mixer_ctl_name);

    ctl = mixer_get_ctl_by_name(adev->mixer, mixer_ctl_name);
    if (!ctl) {
        ALOGE("%s: Could not get ctl for mixer cmd - %s",
              __func__, mixer_ctl_name);
        return -EINVAL;
    }
    for (i = 0; i < (unsigned int)ch_count; i++) {
        set_values[i] = ch_map[i];
    }

    ALOGD("%s: set mapping(%d %d %d %d %d %d %d %d) for channel:%d", __func__,
        set_values[0], set_values[1], set_values[2], set_values[3], set_values[4],
        set_values[5], set_values[6], set_values[7], ch_count);

    ret = mixer_ctl_set_array(ctl, set_values, ARRAY_SIZE(set_values));
    if (ret < 0) {
        ALOGE("%s: Could not set ctl, error:%d ch_count:%d",
              __func__, ret, ch_count);
    }
    return ret;
}

unsigned char platform_map_to_edid_format(int audio_format)
{
    unsigned char format;
    switch (audio_format & AUDIO_FORMAT_MAIN_MASK) {
    case AUDIO_FORMAT_AC3:
        ALOGV("%s: AC3", __func__);
        format = AC3;
        break;
    case AUDIO_FORMAT_AAC:
        ALOGV("%s:AAC", __func__);
        format = AAC;
        break;
    case AUDIO_FORMAT_AAC_ADTS:
        ALOGV("%s:AAC_ADTS", __func__);
        format = AAC;
        break;
    case AUDIO_FORMAT_E_AC3:
        ALOGV("%s:E_AC3", __func__);
        format = DOLBY_DIGITAL_PLUS;
        break;
    case AUDIO_FORMAT_PCM_16_BIT:
    case AUDIO_FORMAT_PCM_24_BIT_PACKED:
    case AUDIO_FORMAT_PCM_8_24_BIT:
    default:
        ALOGV("%s:PCM", __func__);
        format =  LPCM;
        break;
    }
    return format;
}

uint32_t platform_get_compress_passthrough_buffer_size(
                                          audio_offload_info_t* info)
{
    uint32_t fragment_size = MIN_COMPRESS_PASSTHROUGH_FRAGMENT_SIZE;
    if (!info->has_video)
        fragment_size = MIN_COMPRESS_PASSTHROUGH_FRAGMENT_SIZE;

    return fragment_size;
}

void platform_reset_edid_info(void *platform) {

    ALOGV("%s:", __func__);
    struct platform_data *my_data = (struct platform_data *)platform;
    if (my_data->edid_info) {
        ALOGV("%s :free edid", __func__);
        free(my_data->edid_info);
        my_data->edid_info = NULL;
    }
}

bool platform_is_edid_supported_format(void *platform, int format)
{
    struct platform_data *my_data = (struct platform_data *)platform;
    struct audio_device *adev = my_data->adev;
    edid_audio_info *info = NULL;
    int num_audio_blocks;
    int i, ret, count;
    unsigned char format_id = platform_map_to_edid_format(format);

    ret = platform_get_edid_info(platform);
    info = (edid_audio_info *)my_data->edid_info;
    if (ret == 0 && info != NULL) {
        for (i = 0; i < info->audio_blocks && i < MAX_EDID_BLOCKS; i++) {
             /*
              * To check
              *  is there any special for CONFIG_HDMI_PASSTHROUGH_CONVERT
              *  & DOLBY_DIGITAL_PLUS
              */
            if (info->audio_blocks_array[i].format_id == format_id) {
                ALOGV("%s:platform_is_edid_supported_format true %x",
                      __func__, format);
                return true;
            }
        }
    }
    ALOGV("%s:platform_is_edid_supported_format false %x",
           __func__, format);
    return false;
}

int platform_set_edid_channels_configuration(void *platform, int channels) {

    struct platform_data *my_data = (struct platform_data *)platform;
    struct audio_device *adev = my_data->adev;
    edid_audio_info *info = NULL;
    int num_audio_blocks;
    int channel_count = 2;
    int i, ret, count;
    char default_channelMap[MAX_CHANNELS_SUPPORTED] = {0};

    ret = platform_get_edid_info(platform);
    info = (edid_audio_info *)my_data->edid_info;
    if(ret == 0 && info != NULL) {
        if (channels > 2) {

            ALOGV("%s:able to get HDMI sink capabilities multi channel playback",
                   __func__);
            for (i = 0; i < info->audio_blocks && i < MAX_EDID_BLOCKS; i++) {
                if (info->audio_blocks_array[i].format_id == LPCM &&
                      info->audio_blocks_array[i].channels > channel_count &&
                      info->audio_blocks_array[i].channels <= MAX_HDMI_CHANNEL_CNT) {
                    channel_count = info->audio_blocks_array[i].channels;
                }
            }
            ALOGVV("%s:channel_count:%d", __func__, channel_count);
            /*
             * Channel map is set for supported hdmi max channel count even
             * though the input channel count set on adm is less than or equal to
             * max supported channel count
             */
            platform_set_channel_map(platform, channel_count, info->channel_map, -1);
            platform_set_channel_allocation(platform, info->channel_allocation);
        } else {
            default_channelMap[0] = PCM_CHANNEL_FL;
            default_channelMap[1] = PCM_CHANNEL_FR;
            platform_set_channel_map(platform,2,default_channelMap,-1);
            platform_set_channel_allocation(platform,0);
        }
    }

    return 0;
}

void platform_cache_edid(void * platform)
{
    platform_get_edid_info(platform);
}

void platform_invalidate_edid(void * platform)
{
    struct platform_data *my_data = (struct platform_data *)platform;
    my_data->edid_valid = false;
    if (my_data->edid_info) {
        memset(my_data->edid_info, 0, sizeof(struct edid_audio_info));
    }
}

int platform_set_mixer_control(struct stream_out *out, const char * mixer_ctl_name,
                      const char *mixer_val)
{
    struct audio_device *adev = out->dev;
    struct mixer_ctl *ctl = NULL;
    ALOGD("setting mixer ctl %s with value %s", mixer_ctl_name, mixer_val);
    ctl = mixer_get_ctl_by_name(adev->mixer, mixer_ctl_name);
    if (!ctl) {
        ALOGE("%s: could not get ctl for mixer cmd - %s",
              __func__, mixer_ctl_name);
        return -EINVAL;
    }

    return mixer_ctl_set_enum_by_string(ctl, mixer_val);
}

int platform_set_hdmi_config(struct stream_out *out)
{
    struct listnode *node;
    struct audio_usecase *usecase;
    struct audio_device *adev = out->dev;
    const char *hdmi_format_ctrl = "HDMI RX Format";
    const char *hdmi_rate_ctrl = "HDMI_RX SampleRate";
    int sample_rate = out->sample_rate;
    /*TODO: Add rules and check if this needs to be done.*/
    if((is_offload_usecase(out->usecase)) &&
        (out->compr_config.codec->compr_passthr == PASSTHROUGH ||
        out->compr_config.codec->compr_passthr == PASSTHROUGH_CONVERT)) {
        /* TODO: can we add mixer control for channels here avoid setting */
        if ((out->format == AUDIO_FORMAT_E_AC3 ||
            out->format == AUDIO_FORMAT_E_AC3_JOC) &&
            (out->compr_config.codec->compr_passthr == PASSTHROUGH))
            sample_rate = out->sample_rate * 4;
        ALOGD("%s:HDMI compress format and samplerate %d, sample_rate %d",
               __func__, out->sample_rate, sample_rate);
        platform_set_mixer_control(out, hdmi_format_ctrl, "Compr");
        switch (sample_rate) {
            case 32000:
                platform_set_mixer_control(out, hdmi_rate_ctrl, "KHZ_32");
                break;
            case 44100:
                platform_set_mixer_control(out, hdmi_rate_ctrl, "KHZ_44_1");
                break;
            case 96000:
                platform_set_mixer_control(out, hdmi_rate_ctrl, "KHZ_96");
                break;
            case 176400:
                platform_set_mixer_control(out, hdmi_rate_ctrl, "KHZ_176_4");
                break;
            case 192000:
                platform_set_mixer_control(out, hdmi_rate_ctrl, "KHZ_192");
                break;
            case 128000:
                if (out->format != AUDIO_FORMAT_E_AC3) {
                    platform_set_mixer_control(out, hdmi_rate_ctrl, "KHZ_128");
                    break;
                } else
                   ALOGW("Unsupported sample rate for E_AC3 32K");
            default:
            case 48000:
                platform_set_mixer_control(out, hdmi_rate_ctrl, "KHZ_48");
                break;
        }
    } else {
        ALOGD("%s: HDMI pcm and samplerate %d", __func__,
               out->sample_rate);
        platform_set_mixer_control(out, hdmi_format_ctrl, "LPCM");
        platform_set_mixer_control(out, hdmi_rate_ctrl, "KHZ_48");
    }

    /*
     * Deroute all the playback streams routed to HDMI so that
     * the back end is deactivated. Note that backend will not
     * be deactivated if any one stream is connected to it.
     */
    list_for_each(node, &adev->usecase_list) {
        usecase = node_to_item(node, struct audio_usecase, list);
        ALOGV("%s:disable: usecase type %d, devices 0x%x", __func__,
               usecase->type, usecase->devices);
        if (usecase->type == PCM_PLAYBACK &&
                usecase->devices & AUDIO_DEVICE_OUT_AUX_DIGITAL) {
            disable_audio_route(adev, usecase);
        }
    }

    /*
     * Enable all the streams disabled above. Now the HDMI backend
     * will be activated with new channel configuration
     */
    list_for_each(node, &adev->usecase_list) {
        usecase = node_to_item(node, struct audio_usecase, list);
        ALOGV("%s:enable: usecase type %d, devices 0x%x", __func__,
               usecase->type, usecase->devices);
        if (usecase->type == PCM_PLAYBACK &&
                usecase->devices & AUDIO_DEVICE_OUT_AUX_DIGITAL) {
            enable_audio_route(adev, usecase);
        }
    }

    return 0;
}

int platform_set_device_params(struct stream_out *out, int param, int value)
{
    struct audio_device *adev = out->dev;
    struct mixer_ctl *ctl;
    char *mixer_ctl_name = "Device PP Params";
    int ret = 0;
    uint32_t set_values[] = {0,0};

    set_values[0] = param;
    set_values[1] = value;

    ctl = mixer_get_ctl_by_name(adev->mixer, mixer_ctl_name);
    if (!ctl) {
        ALOGE("%s: Could not get ctl for mixer cmd - %s",
              __func__, mixer_ctl_name);
        ret = -EINVAL;
        goto end;
    }

    ALOGV("%s: Setting device pp params param: %d, value %d mixer ctrl:%s",
          __func__,param, value, mixer_ctl_name);
    mixer_ctl_set_array(ctl, set_values, ARRAY_SIZE(set_values));

end:
    return ret;
}

/*
 * This is a lookup table to map names of speaker device with respective left and right TZ names.
 * Also the tz names for a particular left or right speaker can be overriden by adding
 * corresponding entry in audio_platform_info.xml file.
 */
struct speaker_device_to_tz_names speaker_device_tz_names = {
    {SND_DEVICE_OUT_SPEAKER, "", ""},
};

/*
 * This is a lookup table to map android audio input device to audio h/w interface (backend).
 * The table can be extended for other input devices by adding appropriate entries.
 * Also the audio interface for a particular input device can be overriden by adding
 * corresponding entry in audio_platform_info.xml file.
 */
struct audio_device_to_audio_interface audio_device_to_interface_table[] = {
    {AUDIO_DEVICE_IN_BUILTIN_MIC, ENUM_TO_STRING(AUDIO_DEVICE_IN_BUILTIN_MIC), "SLIMBUS_0"},
    {AUDIO_DEVICE_IN_BACK_MIC, ENUM_TO_STRING(AUDIO_DEVICE_IN_BACK_MIC), "SLIMBUS_0"},
};

int audio_device_to_interface_table_len  =
    sizeof(audio_device_to_interface_table) / sizeof(audio_device_to_interface_table[0]);

int platform_set_audio_device_interface(const char *device_name, const char *intf_name,
                                        const char *codec_type __unused)
{
    int ret = 0;
    int i;

    if (device_name == NULL || intf_name == NULL) {
        ALOGE("%s: Invalid input", __func__);

        ret = -EINVAL;
        goto done;
    }

    ALOGD("%s: Enter, device name:%s, intf name:%s", __func__, device_name, intf_name);

    size_t device_name_len = strlen(device_name);
    for (i = 0; i < audio_device_to_interface_table_len; i++) {
        char* name = audio_device_to_interface_table[i].device_name;
        size_t name_len = strlen(name);
        if ((name_len == device_name_len) &&
            (strncmp(device_name, name, name_len) == 0)) {
            ALOGD("%s: Matched device name:%s, overwrite intf name with %s",
                  __func__, device_name, intf_name);

            strlcpy(audio_device_to_interface_table[i].interface_name, intf_name,
                    sizeof(audio_device_to_interface_table[i].interface_name));
            goto done;
        }
    }
    ALOGE("%s: Could not find matching device name %s",
            __func__, device_name);

    ret = -EINVAL;

done:
    return ret;
}

int platform_set_spkr_device_tz_names(snd_device_t index,
                                      const char *spkr_1_tz_name, const char *spkr_2_tz_name)
{
    int ret = 0;
    int i;

    if (spkr_1_tz_name == NULL && spkr_2_tz_name == NULL) {
        ALOGE("%s: Invalid input", __func__);
        ret = -EINVAL;
        goto done;
    }
    if (index != speaker_device_tz_names.snd_device) {
        ALOGE("%s: not matching speaker device\n");
        ret = -EINVAL;
        goto done;
    }
    ALOGD("%s: Enter, spkr_1_tz_name :%s, spkr_2_tz_name:%s",
           __func__, spkr_1_tz_name, spkr_2_tz_name);

    if (spkr_1_tz_name != NULL)
        strlcpy(speaker_device_tz_names.spkr_1_tz_name, spkr_1_tz_name,
                sizeof(speaker_device_tz_names.spkr_1_tz_name));

    if (spkr_2_tz_name != NULL)
        strlcpy(speaker_device_tz_names.spkr_2_tz_name, spkr_2_tz_name,
                sizeof(speaker_device_tz_names.spkr_2_tz_name));
done:
    return ret;
}

int platform_get_wsa_mode (void *adev)
{
    return 0;
}

int platform_get_max_mic_count(void *platform) {
    struct platform_data *my_data = (struct platform_data *)platform;
    return my_data->max_mic_count;
}

int platform_set_snd_device_name(snd_device_t device, const char *name)
{
    int ret = 0;

    if ((device < SND_DEVICE_MIN) || (device >= SND_DEVICE_MAX)) {
        ALOGE("%s:: Invalid snd_device = %d", __func__, device);
        ret = -EINVAL;
        goto done;
    }

    device_table[device] = strdup(name);
done:
    return ret;
}
