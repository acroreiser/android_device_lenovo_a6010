/*
 * Copyright (C) 2015 The CyanogenMod Project
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

#ifndef MSIM_VOICE_EXTN_H
#define MSIM_VOICE_EXTN_H

#if defined(HTC_DUAL_SIM) || defined(SAMSUNG_DUAL_SIM)
int msim_voice_extn_start_call(struct audio_device *adev);
int msim_voice_extn_stop_call(struct audio_device *adev);
int msim_voice_extn_get_session_from_use_case(struct audio_device *adev,
                                             const audio_usecase_t usecase_id,
                                             struct voice_session **session);
int msim_voice_extn_set_parameters(struct audio_device *adev,
                                  struct str_parms *parms);
int msim_voice_extn_is_call_state_active(struct audio_device *adev,
                                        bool *is_call_active);
int msim_voice_extn_get_active_session_id(struct audio_device *adev,
                                         uint32_t *session_id);
#else
static int msim_voice_extn_start_call(struct audio_device *adev __unused)
{
    return -ENOSYS;
}

static int msim_voice_extn_stop_call(struct audio_device *adev  __unused)
{
    return -ENOSYS;
}

static int msim_voice_extn_get_session_from_use_case(struct audio_device *adev __unused,
                                                    const audio_usecase_t usecase_id __unused,
                                                    struct voice_session **session __unused)
{
    return -ENOSYS;
}

static int msim_voice_extn_set_parameters(struct audio_device *adev __unused,
                                         struct str_parms *parms __unused)
{
    return -ENOSYS;
}

static int msim_voice_extn_is_call_state_active(struct audio_device *adev __unused,
                                               bool *is_call_active __unused)
{
    return -ENOSYS;
}

static int msim_voice_extn_get_active_session_id(struct audio_device *adev __unused,
                                                uint32_t *session_id __unused)
{
    return -ENOSYS;
}

#endif

#endif /* MSIM_VOICE_EXTN_H */
