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

#define LOG_TAG "audio_hw_acdb"
//#define LOG_NDEBUG 0
#define LOG_NDDEBUG 0

#include <stdlib.h>
#include <dlfcn.h>
#include <cutils/log.h>
#include <cutils/list.h>
#include "audio_hw.h"
#include "acdb.h"
#include "platform_api.h"


static const char * get_snd_card_name_for_acdb_loader(char *snd_card_name) {

    if(snd_card_name == NULL)
        return NULL;

    // Both tasha & tasha-lite uses tasha ACDB files
    // simulate sound card name for tasha lite, so that
    // ACDB module loads tasha ACDB files for tasha lite
    if(!strncmp(snd_card_name, "msm8976-tashalite-snd-card",
             sizeof("msm8976-tashalite-snd-card"))) {
       ALOGD("using tasha ACDB files for tasha-lite");
       return "msm8976-tasha-snd-card";
   } else {
       return snd_card_name;
   }
}

int acdb_init(int snd_card_num)
{

    int result = -1;
    char *cvd_version = NULL;

    char *snd_card_name = NULL;
    const char *acdb_snd_card_name;
    struct mixer *mixer = NULL;
    struct acdb_platform_data *my_data = NULL;

    if(snd_card_num < 0) {
        ALOGE("invalid sound card number");
        return result;
    }

    mixer = mixer_open(snd_card_num);
    if (!mixer) {
        ALOGE("%s: Unable to open the mixer card: %d", __func__,
               snd_card_num);
        goto cleanup;
    }

    my_data = calloc(1, sizeof(struct acdb_platform_data));
    if (!my_data) {
        ALOGE("failed to allocate acdb platform data");
        goto cleanup;
    }

    list_init(&my_data->acdb_meta_key_list);

    my_data->acdb_handle = dlopen(LIB_ACDB_LOADER, RTLD_NOW);
    if (my_data->acdb_handle == NULL) {
        ALOGE("%s: DLOPEN failed for %s", __func__, LIB_ACDB_LOADER);
        goto cleanup;
    }

    ALOGV("%s: DLOPEN successful for %s", __func__, LIB_ACDB_LOADER);

    my_data->acdb_init_v3 = (acdb_init_v3_t)dlsym(my_data->acdb_handle,
                                                     "acdb_loader_init_v3");
    if (my_data->acdb_init_v3 == NULL)
        ALOGE("%s: dlsym error %s for acdb_loader_init_v3", __func__, dlerror());

    my_data->acdb_init_v2 = (acdb_init_v2_t)dlsym(my_data->acdb_handle,
                                                     "acdb_loader_init_v2");
    if (my_data->acdb_init_v2 == NULL)
        ALOGE("%s: dlsym error %s for acdb_loader_init_v2", __func__, dlerror());

    my_data->acdb_init = (acdb_init_t)dlsym(my_data->acdb_handle,
                                                 "acdb_loader_init_ACDB");
    if (my_data->acdb_init == NULL && my_data->acdb_init_v2 == NULL
                                                 && my_data->acdb_init_v3 == NULL) {
        ALOGE("%s: dlsym error %s for acdb_loader_init_ACDB", __func__, dlerror());
        goto cleanup;
    }

    /* Get CVD version */
    cvd_version = calloc(1, MAX_CVD_VERSION_STRING_SIZE);
    if (!cvd_version) {
        ALOGE("%s: Failed to allocate cvd version", __func__);
        goto cleanup;
    } else {
        struct mixer_ctl *ctl = NULL;
        int count = 0;

        ctl = mixer_get_ctl_by_name(mixer, CVD_VERSION_MIXER_CTL);
        if (!ctl) {
            ALOGE("%s: Could not get ctl for mixer cmd - %s",  __func__, CVD_VERSION_MIXER_CTL);
            goto cleanup;
        }
        mixer_ctl_update(ctl);

        count = mixer_ctl_get_num_values(ctl);
        if (count > MAX_CVD_VERSION_STRING_SIZE)
            count = MAX_CVD_VERSION_STRING_SIZE;

        result = mixer_ctl_get_array(ctl, cvd_version, count);
        if (result != 0) {
            ALOGE("%s: ERROR! mixer_ctl_get_array() failed to get CVD Version", __func__);
            goto cleanup;
        }
    }

    /* Get Sound card name */
    snd_card_name = strdup(mixer_get_name(mixer));
    if (!snd_card_name) {
        ALOGE("failed to allocate memory for snd_card_name");
        result = -1;
        goto cleanup;
    }

    int key = 0;
    struct listnode *node = NULL;
    struct meta_key_list *key_info = NULL;
    acdb_snd_card_name = get_snd_card_name_for_acdb_loader(snd_card_name);
    if (my_data->acdb_init_v3) {
        result = my_data->acdb_init_v3(acdb_snd_card_name, cvd_version,
                                       &my_data->acdb_meta_key_list);
    } else if (my_data->acdb_init_v2) {
        node = list_head(&my_data->acdb_meta_key_list);
        key_info = node_to_item(node, struct meta_key_list, list);
        key = key_info->cal_info.nKey;
        result = my_data->acdb_init_v2(acdb_snd_card_name, cvd_version, key);
    } else {
        result = my_data->acdb_init();
    }

cleanup:
    if (NULL != my_data) {
        if (my_data->acdb_handle)
            dlclose(my_data->acdb_handle);

        struct listnode *node;
        struct meta_key_list *key_info;
        list_for_each(node, &my_data->acdb_meta_key_list) {
            key_info = node_to_item(node, struct meta_key_list, list);
            free(key_info);
        }
        free(my_data);
    }

    if (mixer)
        mixer_close(mixer);

    if (cvd_version)
        free(cvd_version);

    if (snd_card_name)
        free(snd_card_name);

    return result;
}

int acdb_set_metainfo_key(void *platform, char *name, int key) {

    struct meta_key_list *key_info = (struct meta_key_list *)
                                        calloc(1, sizeof(struct meta_key_list));
    struct acdb_platform_data *pdata = (struct acdb_platform_data *)platform;
    if (!key_info) {
        ALOGE("%s: Could not allocate memory for key %d", __func__, key);
        return -ENOMEM;
    }

    key_info->cal_info.nKey = key;
    strlcpy(key_info->name, name, sizeof(key_info->name));
    list_add_tail(&pdata->acdb_meta_key_list, &key_info->list);

    ALOGD("%s: successfully added module %s and key %d to the list", __func__,
               key_info->name, key_info->cal_info.nKey);

    return 0;
}
