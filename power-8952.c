/*
 * Copyright (c) 2015, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * *    * Redistributions of source code must retain the above copyright
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

#define LOG_NIDEBUG 0

#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <stdlib.h>

#define LOG_TAG "QTI PowerHAL"
#include <log/log.h>
#include <hardware/hardware.h>
#include <hardware/power.h>

#include "utils.h"
#include "metadata-defs.h"
#include "hint-data.h"
#include "performance.h"
#include "power-common.h"

static int saved_interactive_mode = -1;
static int display_hint_sent;
static int video_encode_hint_sent;

static int current_power_profile = PROFILE_BALANCED;

static int profile_high_performance[] = {
    CPUS_ONLINE_MIN_BIG, 0x4,
    CPUS_ONLINE_MIN_LITTLE, 0x4,
    CPU0_MIN_FREQ_TURBO_MAX,
    CPU4_MIN_FREQ_TURBO_MAX,
};

static int profile_power_save[] = {
    CPUS_ONLINE_MAX_LIMIT_BIG, 0x0,
    CPU0_MAX_FREQ_NONTURBO_MAX,
    CPU4_MAX_FREQ_NONTURBO_MAX,
};

static int profile_bias_power[] = {
    CPU0_MAX_FREQ_NONTURBO_MAX,
    CPU4_MAX_FREQ_NONTURBO_MAX,
};

static int profile_bias_performance[] = {
    CPU0_MIN_FREQ_NONTURBO_MAX + 1,
    CPU4_MIN_FREQ_NONTURBO_MAX + 1,
};

int get_number_of_profiles()
{
    return 5;
}

static void set_power_profile(int profile)
{
    if (profile == current_power_profile)
        return;

    ALOGV("%s: Profile=%d", __func__, profile);

    if (current_power_profile != PROFILE_BALANCED) {
        undo_hint_action(DEFAULT_PROFILE_HINT_ID);
        ALOGV("%s: Hint undone", __func__);
    }

    if (profile == PROFILE_POWER_SAVE) {
        perform_hint_action(DEFAULT_PROFILE_HINT_ID, profile_power_save,
                ARRAY_SIZE(profile_power_save));
        ALOGD("%s: Set powersave mode", __func__);

    } else if (profile == PROFILE_HIGH_PERFORMANCE) {
        perform_hint_action(DEFAULT_PROFILE_HINT_ID, profile_high_performance,
                ARRAY_SIZE(profile_high_performance));
        ALOGD("%s: Set performance mode", __func__);

    } else if (profile == PROFILE_BIAS_POWER) {
        perform_hint_action(DEFAULT_PROFILE_HINT_ID, profile_bias_power,
                ARRAY_SIZE(profile_bias_power));
        ALOGD("%s: Set bias power mode", __func__);

    } else if (profile == PROFILE_BIAS_PERFORMANCE) {
        perform_hint_action(DEFAULT_PROFILE_HINT_ID, profile_bias_performance,
                ARRAY_SIZE(profile_bias_performance));
        ALOGD("%s: Set bias perf mode", __func__);
    }

    current_power_profile = profile;
}

static void process_video_encode_hint(void *metadata)
{
    char governor[80];
    struct video_encode_metadata_t video_encode_metadata;

    ALOGI("Got process_video_encode_hint");

    if (get_scaling_governor_check_cores(governor,
                sizeof(governor),CPU0) == -1) {
        if (get_scaling_governor_check_cores(governor,
                    sizeof(governor),CPU1) == -1) {
            if (get_scaling_governor_check_cores(governor,
                        sizeof(governor),CPU2) == -1) {
                if (get_scaling_governor_check_cores(governor,
                            sizeof(governor),CPU3) == -1) {
                    ALOGE("Can't obtain scaling governor.");
                    return;
                }
            }
        }
    }

    if (!metadata) {
        return;
    }

    /* Initialize encode metadata struct fields. */
    memset(&video_encode_metadata, 0, sizeof(struct video_encode_metadata_t));
    video_encode_metadata.state = -1;
    video_encode_metadata.hint_id = DEFAULT_VIDEO_ENCODE_HINT_ID;

    if (parse_video_encode_metadata((char *)metadata,
                &video_encode_metadata) == -1) {
        ALOGE("Error occurred while parsing metadata.");
        return;
    }

    if (video_encode_metadata.state == 1) {
        if (is_interactive_governor(governor)) {
            /* Sched_load and migration_notif*/
            int resource_values[] = {
                INT_OP_CLUSTER0_USE_SCHED_LOAD, 0x1,
                INT_OP_CLUSTER1_USE_SCHED_LOAD, 0x1,
                INT_OP_CLUSTER0_USE_MIGRATION_NOTIF, 0x1,
                INT_OP_CLUSTER1_USE_MIGRATION_NOTIF, 0x1,
                INT_OP_CLUSTER0_TIMER_RATE, BIG_LITTLE_TR_MS_40,
                INT_OP_CLUSTER1_TIMER_RATE, BIG_LITTLE_TR_MS_40
            };
            if (!video_encode_hint_sent) {
                perform_hint_action(video_encode_metadata.hint_id,
                        resource_values, ARRAY_SIZE(resource_values));
                video_encode_hint_sent = 1;
            }
        }
    } else if (video_encode_metadata.state == 0) {
        if (is_interactive_governor(governor)) {
            undo_hint_action(video_encode_metadata.hint_id);
            video_encode_hint_sent = 0;
        }
    }
}

int power_hint_override(power_hint_t hint, void *data)
{
    if (hint == POWER_HINT_SET_PROFILE) {
        set_power_profile(*(int32_t *)data);
        return HINT_HANDLED;
    }

    // Skip other hints in high/low power modes
    if (current_power_profile == PROFILE_POWER_SAVE ||
            current_power_profile == PROFILE_HIGH_PERFORMANCE) {
        return HINT_HANDLED;
    }

    switch (hint) {
        case POWER_HINT_VIDEO_ENCODE:
            process_video_encode_hint(data);
            return HINT_HANDLED;
        default:
            break;
    }
    return HINT_NONE;
}

int set_interactive_override(int on)
{
    char governor[80];
    char tmp_str[NODE_MAX];
    struct video_encode_metadata_t video_encode_metadata;
    int rc;

    ALOGI("Got set_interactive hint");

    if (get_scaling_governor_check_cores(governor, sizeof(governor),CPU0) == -1) {
        if (get_scaling_governor_check_cores(governor, sizeof(governor),CPU1) == -1) {
            if (get_scaling_governor_check_cores(governor, sizeof(governor),CPU2) == -1) {
                if (get_scaling_governor_check_cores(governor, sizeof(governor),CPU3) == -1) {
                    ALOGE("Can't obtain scaling governor.");
                    return HINT_NONE;
                }
            }
        }
    }

    if (!on) {
        /* Display off. */
        if (is_interactive_governor(governor)) {
            int resource_values[] = {
                INT_OP_CLUSTER0_TIMER_RATE, BIG_LITTLE_TR_MS_50,
                INT_OP_CLUSTER1_TIMER_RATE, BIG_LITTLE_TR_MS_50,
                INT_OP_NOTIFY_ON_MIGRATE, 0x00
            };

            if (!display_hint_sent) {
                perform_hint_action(
                        DISPLAY_STATE_HINT_ID,
                        resource_values,
                        ARRAY_SIZE(resource_values));
                display_hint_sent = 1;
            }
        } /* Perf time rate set for CORE0,CORE4 8952 target*/
    } else {
        /* Display on. */
        if (is_interactive_governor(governor)) {
            undo_hint_action(DISPLAY_STATE_HINT_ID);
            display_hint_sent = 0;
        }
    }
    saved_interactive_mode = !!on;
    return HINT_HANDLED;
}
