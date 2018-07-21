/*
 * Copyright (c) 2012-2017, The Linux Foundation. All rights reserved.
 * Copyright (C) 2017-2018 The LineageOS Project
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

#ifndef __POWER_HELPER_H__
#define __POWER_HELPER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "hardware/power.h"

enum platform_param_id {
    VLOW_COUNT = 0,
    ACCUMULATED_VLOW_TIME,
    VMIN_COUNT,
    ACCUMULATED_VMIN_TIME,
    RPM_PARAM_COUNT,

    XO_ACCUMULATED_DURATION_APSS = RPM_PARAM_COUNT,
    XO_COUNT_APSS,
    XO_ACCUMULATED_DURATION_MPSS,
    XO_COUNT_MPSS,
    XO_ACCUMULATED_DURATION_ADSP,
    XO_COUNT_ADSP,
    XO_ACCUMULATED_DURATION_SLPI,
    XO_COUNT_SLPI,

    //Don't add any lines after that line
    PLATFORM_PARAM_COUNT
};

enum stats_type {
    //Platform Stats
    RPM_MODE_XO = 0,
    RPM_MODE_VMIN,
    RPM_MODE_MAX,
    XO_VOTERS_START = RPM_MODE_MAX,
    VOTER_APSS = XO_VOTERS_START,
    VOTER_MPSS,
    VOTER_ADSP,
    VOTER_SLPI,
    VOTER_PRONTO,
    VOTER_TZ,
    VOTER_LPASS,
    VOTER_SPSS,
    MAX_PLATFORM_STATS,
};

enum subsystem_type {
    //Don't add any lines after this line
    SUBSYSTEM_COUNT
};

#define PLATFORM_SLEEP_MODES_COUNT RPM_MODE_MAX

#define MAX_RPM_PARAMS 2
#define XO_VOTERS 4
#define VMIN_VOTERS 0

struct stat_pair {
    enum stats_type stat;
    const char *label;
    const char **parameters;
    size_t num_parameters;
};


void power_init(void);
void power_hint(power_hint_t hint, void *data);
void power_set_interactive(int on);
void set_feature(feature_t feature, int state);
int extract_platform_stats(uint64_t *list);
int __attribute__ ((weak)) get_number_of_profiles();

#ifdef __cplusplus
}
#endif

#endif //__POWER_HELPER_H__
