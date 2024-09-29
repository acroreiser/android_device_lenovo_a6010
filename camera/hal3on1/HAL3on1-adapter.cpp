/*
 * Copyright (C) 2024 acroreiser
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

#include <hardware/hardware.h>
#include <hardware/camera3.h>
#include <hardware/camera.h>
#include <camera/Camera.h>
#include <camera/CameraParameters.h>
#include <cutils/ashmem.h>
#include <cutils/properties.h>
#include <linux/errno.h>

#define MAX_SIZES_CNT 40
#define NSEC_PER_33MSEC 33000000LL

using namespace android;

static Mutex gHAL3AdapterLock;

static camera_module_t *gLegacyModule = 0;

typedef struct {
    camera3_device_t base;
    camera_device_t *hal1_device;
    const camera3_callback_ops_t *callback_ops;
    uint8_t focus_state;

    int buffer_size;
    int jpeg_size;
    uint8_t* buffer;
    uint8_t* jpeg;

    int stream_width;
    int stream_height;
} adapter_camera3_device_t;

#define HAL1_CALL(hal1_device, func, ...) ({ \
    hal1_device->ops->func(hal1_device, ##__VA_ARGS__); \
})

typedef struct {
    bool use_memfd;
    bool use_limited_level;
    bool use_sysfs_torch;

} adapter_config_t;

struct CameraMemory {
    void* data;
    camera_memory_t mem;
    int fd;
};

typedef struct {
    camera_module_callbacks_t hal1_callbacks;
    const camera_module_callbacks_t *hal3_callbacks;
} adapter_camera_module_callbacks_t;

static adapter_camera_module_callbacks_t adapter_callbacks;

adapter_camera3_device_t *hal3on1_dev;

adapter_config_t properties = {
    .use_memfd = false,
    .use_limited_level = false,
    .use_sysfs_torch = false,
};

// Put your sysfs path here or use HAL1 torch mode
#define SYSFS_FLASH_PATH_BRIGHTNESS "/sys/class/leds/torch-light0/brightness"

static CameraMetadata static_metadata[2];
static CameraParameters default_parameters[2];
static bool static_parameters_initialized[2] = { false, false };
static int gCameraId = -1;

static int device_api_version = CAMERA_DEVICE_API_VERSION_3_3;

static camera_device_t *torch_hal1_device;
static bool torch_in_use = false;
static CameraParameters torch_params;

static int get_legacy_module()
{
    int ret = 0;

    if (gLegacyModule)
        return NO_ERROR;

    ret = hw_get_module_by_class("camera", "legacy",
                                 (const hw_module_t**)&gLegacyModule);
    if (ret)
        ALOGE("failed to open legacy HAL1 camera module");

    return ret;
}

camera_device_t* get_hal1_device(const struct camera3_device *device)
{
    adapter_camera3_device_t *adapter_dev = (adapter_camera3_device_t *)device;

    return adapter_dev->hal1_device;
}

void hal1_data_callback(int32_t msg_type,
                        const camera_memory_t *data, unsigned int index,
                        camera_frame_metadata_t *metadata, void *user)
{
    adapter_camera3_device_t* adapter = hal3on1_dev;

    if (!data || !adapter || !adapter->buffer) {
        ALOGE("Error: Invalid preview callback data or adapter\n");
        return;
    }
    switch(msg_type) {
    case CAMERA_MSG_PREVIEW_FRAME:
        memcpy(adapter->buffer, data->data, data->size);
        adapter->buffer_size = data->size;

        break;

    case CAMERA_MSG_COMPRESSED_IMAGE:
        memcpy(adapter->jpeg, data->data, data->size);
        adapter->jpeg_size = data->size;

        while(adapter->jpeg_size != 0) {
            usleep(100);
        }

        break;
    }
}

void process_camera_frame(void* buffer, size_t size, nsecs_t timestamp)
{
    ALOGE("Processing frame of size: %zu at timestamp: %lld\n", size, (long long)timestamp);
}

void hal1_data_timestamp_callback(nsecs_t timestamp, int32_t msg_type,
                                  const camera_memory_t* data, unsigned index, void* user)
{
    if (msg_type == CAMERA_MSG_VIDEO_FRAME) {
        ALOGE("Received video frame with timestamp: %lld ns, buffer index: %u\n",
              (long long)timestamp, index);

        if (data && data->data)
            process_camera_frame(data->data, data->size, timestamp);
        else
            ALOGE("Error: Data buffer is null!\n");

    } else
        ALOGE("Unexpected message type: %d\n", msg_type);
}

void release_memory(camera_memory_t* memory)
{
    if (memory) {
        CameraMemory* cameraMem = static_cast<CameraMemory*>(memory->handle);
        if (cameraMem) {
            if (cameraMem->data) {
                munmap(cameraMem->data, cameraMem->mem.size);
            }
            free(cameraMem);
        }
    }
}

camera_memory_t* get_memory(int fd, size_t buf_size, uint_t num_bufs, void* user)
{
    CameraMemory* cameraMem =
        static_cast<CameraMemory*>(malloc(sizeof(CameraMemory)));

    if (!cameraMem) {
        ALOGE("Error: Failed to allocate CameraMemory structure.");
        return nullptr;
    }

    size_t total_size = buf_size * num_bufs;
    if (fd < 0) {
        if(properties.use_memfd)
            fd = memfd_create("CameraHeap", 0);
        else
            fd = ashmem_create_region("CameraHeap", total_size);

        if (fd < 0) {
            ALOGE("Error: Failed to create memfd.");
            free(cameraMem);
            return nullptr;
        }
        ftruncate(fd, total_size);
    }

    cameraMem->fd = fd;
    cameraMem->data = mmap(nullptr, total_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    if (cameraMem->data == MAP_FAILED) {
        ALOGW("Warning: Failed to mmap memfd region: %s, trying to alloc and swap new fd (%d)",
              strerror(errno), fd);

        int newfd = memfd_create("CameraHeap", 0);

        cameraMem->data = mmap(nullptr, total_size, PROT_READ | PROT_WRITE, MAP_SHARED,
                               dup2(dup(newfd), fd), 0);

        if (cameraMem->data == MAP_FAILED)
            ALOGE("Error: Failed to mmap new memfd: %s", strerror(errno));
    }

    cameraMem->mem.data = cameraMem->data;
    cameraMem->mem.size = total_size;
    cameraMem->mem.handle = cameraMem;
    cameraMem->mem.release = release_memory;

    return &cameraMem->mem;
}

void hal1_notify_callback(int32_t msg_type, int32_t ext1, int32_t ext2, void* user)
{
    switch (msg_type) {
    case CAMERA_MSG_ERROR:
        switch (ext1) {
        case CAMERA_ERROR_UNKNOWN:
            ALOGE("Camera Error: Unknown error occurred.\n");
            break;
        case CAMERA_ERROR_SERVER_DIED:
            ALOGE("Camera Error: Media server died.\n");
            break;
        default:
            ALOGE("Camera Error: Error code %d\n", ext1);
            break;
        }
        break;

    case CAMERA_MSG_FOCUS:
        if (ext1 == true)
            hal3on1_dev->focus_state = ANDROID_CONTROL_AF_STATE_FOCUSED_LOCKED;
        else
            hal3on1_dev->focus_state = ANDROID_CONTROL_AF_STATE_NOT_FOCUSED_LOCKED;
        break;

    case CAMERA_MSG_SHUTTER:
        ALOGE("!!!!!!!!!!!!! Camera Shutter: Shutter fired.\n");
        break;

    case CAMERA_MSG_FOCUS_MOVE:
        if (ext1 == true)
            hal3on1_dev->focus_state = ANDROID_CONTROL_AF_STATE_ACTIVE_SCAN;
        else
            hal3on1_dev->focus_state = ANDROID_CONTROL_AF_STATE_PASSIVE_SCAN;
        break;

    case CAMERA_MSG_ZOOM:
        ALOGE("Camera Zoom: Zoom level changed to %d.\n", ext1);
        break;
    default:
        ALOGE("Camera Notify: Unknown message type %d with ext1: %d, ext2: %d\n", msg_type, ext1, ext2);
        break;
    }
}

/*
 * Camera3
 */

static int camera3_close(hw_device_t *device)
{
    adapter_camera3_device_t *adapter_dev = (adapter_camera3_device_t *)device;
    if (adapter_dev->hal1_device) {
        adapter_dev->hal1_device->common.close((hw_device_t *)adapter_dev->hal1_device);
    }

    free(adapter_dev);
    hal3on1_dev = NULL;
    gCameraId = -1;

    return NO_ERROR;
}

/*
 * Camera3 device ops
 */

static int camera3_initialize(const struct camera3_device *dev,
                              const camera3_callback_ops_t *callback_ops)
{
    adapter_camera3_device_t *adapter_dev = (adapter_camera3_device_t *)dev;
    adapter_dev->callback_ops = callback_ops;

    return NO_ERROR;
}

static CameraParameters current_params;

static int camera3_configure_streams(const struct camera3_device *dev, camera3_stream_configuration_t* stream_config)
{
    Mutex::Autolock lock(gHAL3AdapterLock);
    adapter_camera3_device_t *adapter = (adapter_camera3_device_t *)dev;
    camera_device_t* hal1_device = adapter->hal1_device;

    if (!hal1_device) {
        return -ENODEV;
    }

    char *settings = HAL1_CALL(hal1_device, get_parameters);
    CameraParameters preview_params;
    preview_params.unflatten(String8(settings));

    ALOGI("----------------------------");
    ALOGI("| Configuring streams:");

    for (uint32_t i = 0; i < stream_config->num_streams; i++) {
        camera3_stream_t* stream = stream_config->streams[i];
        ALOGI("| p=%p  fmt=0x%.2x  type=%u  usage=0x%.8x  size=%4ux%-4u  buf_no=%u",
              stream,
              stream->format,
              stream->stream_type,
              stream->usage,
              stream->width,
              stream->height,
              stream->max_buffers);

        stream->max_buffers = 1;

        if (stream->stream_type == CAMERA3_STREAM_OUTPUT) {

            switch(stream->format) {

            case HAL_PIXEL_FORMAT_BLOB:
                adapter->jpeg = (uint8_t*)malloc((stream->width * stream->height)*2);

                preview_params.set("picture-format", "jpeg");
                preview_params.setPictureSize(stream->width, stream->height);
                break;

            case HAL_PIXEL_FORMAT_YCbCr_420_888:
                // Override format for legacy gralloc
                stream->format = HAL_PIXEL_FORMAT_YCrCb_420_SP;
                break;

            case HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED:
                // Override format for legacy gralloc
                stream->format = HAL_PIXEL_FORMAT_YCrCb_420_SP;

                if(stream->width > adapter->stream_width &&
                    stream->height > adapter->stream_height)
                {
                    adapter->stream_width = stream->width;
                    adapter->stream_height = stream->height;
                    preview_params.setPreviewSize(stream->width, stream->height);
                }

                if(stream->width < adapter->stream_width &&
                    stream->height < adapter->stream_height)
                {
                    stream->width = adapter->stream_width;
                    stream->height = adapter->stream_height;
                }


                if (stream->usage == 0x00010000)
                    stream->format = 0x102;
                break;
            }
        }
    }

    adapter->buffer_size = adapter->stream_width * adapter->stream_height * 3 / 2;  // For YUV420
    adapter->buffer = (uint8_t*)malloc(adapter->buffer_size);

    ALOGE("----------------------------");

    HAL1_CALL(hal1_device, store_meta_data_in_buffers, 0);

    preview_params.set("preview-fps-range", "12000,30000");
    preview_params.set("preview-frame-rate", "27");
    preview_params.set("preview-format", "yuv420sp");
    preview_params.set("preview-flip", "off");
    preview_params.set("video-frame-format", "yuv420sp");
    preview_params.set("video-hfr", "off");
    preview_params.set("video-hsr", "off");
    preview_params.set("video-flip", "off");
    preview_params.set("selectable-zone-af", "auto");
    preview_params.set("video-size", "1920x1080");
    preview_params.set("auto-exposure", "center-weighted");
    preview_params.set("exposure-mode", "auto");
    preview_params.set("whitebalance", "auto");
    preview_params.set("scene-mode", "auto");
    preview_params.set("scene-detect", "off");
    preview_params.set("focus-mode", "auto");
    preview_params.set("auto-exposure-lock", "false");
    preview_params.set("auto-whitebalance-lock", "false");
    preview_params.set("flash-mode", "off");
    preview_params.set("antibanding", "auto");
    preview_params.set("zsl", "on");
    preview_params.set("long-shot", "off");
    preview_params.set("face-detection", "off");
    preview_params.set("denoise", "denoise-off");
    preview_params.set("effect", "none");
    preview_params.set("zoom", "0");
    preview_params.set("lensshade", "enable");
    preview_params.set("exposure-compensation", "0");
    preview_params.set("jpeg-quality", "95");
    preview_params.set("jpeg-thumbnail-height", "288");
    preview_params.set("jpeg-thumbnail-width", "512");
    preview_params.set("jpeg-thumbnail-quality", "85");
    preview_params.set("iso", "auto");

    HAL1_CALL(hal1_device, set_parameters, preview_params.flatten());
    current_params = preview_params;

    HAL1_CALL(hal1_device, set_callbacks, hal1_notify_callback, hal1_data_callback, hal1_data_timestamp_callback, get_memory, hal1_device);
    HAL1_CALL(hal1_device, enable_msg_type, CAMERA_MSG_PREVIEW_FRAME);
    HAL1_CALL(hal1_device, enable_msg_type, CAMERA_MSG_FOCUS);
    HAL1_CALL(hal1_device, enable_msg_type, CAMERA_MSG_FOCUS_MOVE);
    HAL1_CALL(hal1_device, enable_msg_type, CAMERA_MSG_SHUTTER);

    /*
     * Some HAL1 implementations require ANativeWindow to display (and call back)
     * preview frames.
     * TODO: implement fake preview window for such implementations.
     *
     * HAL1_CALL(hal1_device, set_preview_window, (struct preview_stream_ops *)????);
     *
     * On QCamera2 HAL1: add persist.camera.no-display=1 property to build.prop.
     */

    HAL1_CALL(hal1_device, start_preview);

    return NO_ERROR;
}

static const camera_metadata_t* camera3_construct_default_request_settings(const struct camera3_device *device, int template_type)
{
    CameraMetadata settings;

    static const uint8_t requestType = ANDROID_REQUEST_TYPE_CAPTURE;
    settings.update(ANDROID_REQUEST_TYPE, &requestType, 1);
    int32_t defaultRequestID = 0;
    settings.update(ANDROID_REQUEST_ID, &defaultRequestID, 1);
    static uint8_t jpeg_quality = 95;

    uint8_t controlIntent = 0;
    uint8_t focusMode = ANDROID_CONTROL_AF_MODE_AUTO;
    uint8_t vsMode = ANDROID_CONTROL_VIDEO_STABILIZATION_MODE_OFF;
    uint8_t optStabMode = ANDROID_LENS_OPTICAL_STABILIZATION_MODE_OFF;

    switch (template_type) {
    case CAMERA3_TEMPLATE_PREVIEW:
        controlIntent = ANDROID_CONTROL_CAPTURE_INTENT_PREVIEW;
        focusMode = ANDROID_CONTROL_AF_MODE_CONTINUOUS_PICTURE;
        break;

    case CAMERA3_TEMPLATE_MANUAL:
        jpeg_quality = 100;
        controlIntent = ANDROID_CONTROL_CAPTURE_INTENT_STILL_CAPTURE;
        focusMode = ANDROID_CONTROL_AF_MODE_AUTO;
        break;

    case CAMERA3_TEMPLATE_STILL_CAPTURE:
        controlIntent = ANDROID_CONTROL_CAPTURE_INTENT_STILL_CAPTURE;
        focusMode = ANDROID_CONTROL_AF_MODE_CONTINUOUS_PICTURE;
        break;

    case CAMERA3_TEMPLATE_VIDEO_RECORD:
        controlIntent = ANDROID_CONTROL_CAPTURE_INTENT_VIDEO_RECORD;
        focusMode = ANDROID_CONTROL_AF_MODE_CONTINUOUS_PICTURE;
        break;

    default:
        return NULL;
    }

    settings.update(ANDROID_CONTROL_CAPTURE_INTENT, &controlIntent, 1);
    settings.update(ANDROID_CONTROL_VIDEO_STABILIZATION_MODE, &vsMode, 1);
    settings.update(ANDROID_CONTROL_AF_MODE, &focusMode, 1);

    settings.update(ANDROID_LENS_OPTICAL_STABILIZATION_MODE, &optStabMode, 1);

    static const int32_t exposure_compensation = 0;
    settings.update(ANDROID_CONTROL_AE_EXPOSURE_COMPENSATION,
            &exposure_compensation, 1);

    static const uint8_t aeLock = ANDROID_CONTROL_AE_LOCK_OFF;
    settings.update(ANDROID_CONTROL_AE_LOCK, &aeLock, 1);

    static const uint8_t awbLock = ANDROID_CONTROL_AWB_LOCK_OFF;
    settings.update(ANDROID_CONTROL_AWB_LOCK, &awbLock, 1);

    static const uint8_t awbMode = ANDROID_CONTROL_AWB_MODE_AUTO;
    settings.update(ANDROID_CONTROL_AWB_MODE, &awbMode, 1);

    static const uint8_t controlMode = ANDROID_CONTROL_MODE_AUTO;
    settings.update(ANDROID_CONTROL_MODE, &controlMode, 1);

    static const uint8_t effectMode = ANDROID_CONTROL_EFFECT_MODE_OFF;
    settings.update(ANDROID_CONTROL_EFFECT_MODE, &effectMode, 1);

    static const uint8_t sceneMode = 0;
    settings.update(ANDROID_CONTROL_SCENE_MODE, &sceneMode, 1);

    static const uint8_t aeMode = ANDROID_CONTROL_AE_MODE_ON_AUTO_FLASH;
    settings.update(ANDROID_CONTROL_AE_MODE, &aeMode, 1);

    static const uint8_t abMode = ANDROID_CONTROL_AE_ANTIBANDING_MODE_AUTO;
    settings.update(ANDROID_CONTROL_AE_ANTIBANDING_MODE, &abMode, 1);

    /*flash*/
    static const uint8_t flashMode = ANDROID_FLASH_MODE_OFF;
    settings.update(ANDROID_FLASH_MODE, &flashMode, 1);

//    /* Exposure time(Update the Min Exposure Time)*/
//    int64_t default_exposure_time = 10000000;
//   settings.update(ANDROID_SENSOR_EXPOSURE_TIME, &default_exposure_time, 1);

    /* frame duration */
    static const int64_t default_frame_duration = NSEC_PER_33MSEC;
    settings.update(ANDROID_SENSOR_FRAME_DURATION, &default_frame_duration, 1);

    /* sensitivity */
    static const int32_t default_sensitivity = 0;
    settings.update(ANDROID_SENSOR_SENSITIVITY, &default_sensitivity, 1);

    int32_t sensor_width = static_metadata[gCameraId].find(ANDROID_SENSOR_INFO_ACTIVE_ARRAY_SIZE).data.i32[2];
    int32_t sensor_height = static_metadata[gCameraId].find(ANDROID_SENSOR_INFO_ACTIVE_ARRAY_SIZE).data.i32[3];

    static const int32_t requestId = 0;
    settings.update(ANDROID_REQUEST_ID, &requestId, 1);

    static const float lensFocusDistance = 0.0f;
    settings.update(ANDROID_LENS_FOCUS_DISTANCE, &lensFocusDistance, 1);

    static const uint8_t jpegGpsProcessingMethod[32] = "None";
    settings.update(ANDROID_JPEG_GPS_PROCESSING_METHOD, jpegGpsProcessingMethod, NELEM(jpegGpsProcessingMethod));

    static const int64_t jpegGpsTimestamp = 0;
    settings.update(ANDROID_JPEG_GPS_TIMESTAMP, &jpegGpsTimestamp, 1);

    static const int32_t jpegOrientation = 0;
    settings.update(ANDROID_JPEG_ORIENTATION, &jpegOrientation, 1);

    static const int32_t jpeg_thumbnail_size[] = {
        0, 0
    };
    settings.update(ANDROID_JPEG_THUMBNAIL_SIZE, jpeg_thumbnail_size, NELEM(jpeg_thumbnail_size));

    static const uint8_t jpeg_thumbnail_quality = 85;
    settings.update(ANDROID_JPEG_THUMBNAIL_QUALITY, &jpeg_thumbnail_quality, 1);

    settings.update(ANDROID_JPEG_QUALITY, &jpeg_quality, 1);

    static const double jpegGpsCoordinates[] = {
        0, 0
    };
    settings.update(ANDROID_JPEG_GPS_COORDINATES, jpegGpsCoordinates, NELEM(jpegGpsCoordinates));

    int32_t scaler_crop_region[4];
    scaler_crop_region[0] = 0;
    scaler_crop_region[1] = 0;
    scaler_crop_region[2] = sensor_width;
    scaler_crop_region[3] = sensor_height;
    settings.update(ANDROID_SCALER_CROP_REGION, scaler_crop_region, 4);

    return settings.release();
}

/* Helpers for CameraParameters and stream processing */

static int32_t clamp(int32_t value, int32_t min, int32_t max)
{
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

void get_crop_point(int src_width, int src_height, int crop_width, int crop_height, int* crop_x, int* crop_y)
{
    if (crop_width > src_width ||
        crop_height > src_height) {
        *crop_x = 0;
        *crop_y = 0;
        return;
    }

    *crop_x = (src_width - crop_width) / 2;
    *crop_y = (src_height - crop_height) / 2;
}

struct yuv420sp_buffer {
    uint8_t* data;
    int width;
    int height;
    int size;
};

yuv420sp_buffer* yuv420sp_crop(const uint8_t* src_yuv420sp, int src_width, int src_height,
    int crop_x, int crop_y, int crop_width, int crop_height)
{
    int src_y_size = src_width * src_height;
    const uint8_t* src_y = src_yuv420sp;
    const uint8_t* src_uv = src_yuv420sp + src_y_size;

    yuv420sp_buffer* dst_buffer = (yuv420sp_buffer*)malloc(sizeof(yuv420sp_buffer));
    dst_buffer->width = crop_width;
    dst_buffer->height = crop_height;

    int dst_y_size = crop_width * crop_height;
    int dst_uv_size = (crop_width / 2) * (crop_height / 2) * 2;
    dst_buffer->data = (uint8_t*)malloc(dst_y_size + dst_uv_size);
    dst_buffer->size = dst_y_size + dst_uv_size;

    uint8_t* dst_y = dst_buffer->data;
    uint8_t* dst_uv = dst_buffer->data + dst_y_size;

    for (int i = 0; i < crop_height; ++i) {
        const uint8_t* src_row = src_y + (crop_y + i) * src_width + crop_x;
        uint8_t* dst_row = dst_y + i * crop_width;
        memcpy(dst_row, src_row, crop_width);
    }

    for (int i = 0; i < crop_height / 2; ++i) {
        const uint8_t* src_row = src_uv + (crop_y / 2 + i) * src_width + crop_x;
        uint8_t* dst_row = dst_uv + i * crop_width;
        memcpy(dst_row, src_row, crop_width);
    }

    return dst_buffer;
}

void nv21_to_nv12(unsigned char* nv21, unsigned char* nv12, int width, int height) {
    int frameSize = width * height;
    int i;

    memcpy(nv12, nv21, frameSize);

    for (i = 0; i < frameSize / 2; i += 2) {
        nv12[frameSize + i] = nv21[frameSize + i + 1]; // U
        nv12[frameSize + i + 1] = nv21[frameSize + i]; // V
    }
}

// Ugly zoom mapping implementation. But better than nothing.
int hal3_to_hal1_zoom(int crop_left, int crop_top, int crop_right, int crop_bottom,
    int sensor_width, int sensor_height, int max_zoom_value) {
    float max_ratio = 2.0;
    int crop_width = crop_right - crop_left;
    int crop_height = crop_bottom - crop_top;

    if(crop_left == 0)
        return 0;

    float zoom_factor = ((float)sensor_width + (float)sensor_height) / ((float)crop_width + (float)crop_height);
    zoom_factor = zoom_factor / 2;

    int zoom_value = (int)(zoom_factor / (max_ratio / max_zoom_value));

    return zoom_value;
}

static int camera3_process_capture_request(const camera3_device_t* device, camera3_capture_request_t* request)
{
    adapter_camera3_device_t *adapter = (adapter_camera3_device_t *)device;
    camera_device_t* hal1_device = adapter->hal1_device;
    uint8_t captureIntent;
    status_t e;

    if (!request || request->num_output_buffers == 0 || !request->output_buffers) {
        ALOGE("Invalid capture request");
        return -EINVAL;
    }

    int32_t sensor_width = static_metadata[gCameraId].find(ANDROID_SENSOR_INFO_ACTIVE_ARRAY_SIZE).data.i32[2];
    int32_t sensor_height = static_metadata[gCameraId].find(ANDROID_SENSOR_INFO_ACTIVE_ARRAY_SIZE).data.i32[3];

    CameraMetadata cm;
    cm = request->settings;

    uint8_t aeMode = ANDROID_CONTROL_AE_MODE_ON;
    bool use_scene = false;

    if (cm.exists(ANDROID_CONTROL_MODE)) {
        switch(cm.find(ANDROID_CONTROL_MODE).data.u8[0]) {
        case ANDROID_CONTROL_MODE_USE_SCENE_MODE:
            use_scene = true;
            current_params.set("scene-detect", "off");
            break;

        case ANDROID_CONTROL_MODE_AUTO:
            use_scene = false;
            current_params.set("scene-detect", "on");
            current_params.set("scene-mode", "auto");
            break;

        case ANDROID_CONTROL_MODE_OFF:
            use_scene = false;
            current_params.set("scene-detect", "off");
            current_params.set("scene-mode", "auto");
            break;
        }
    }

    if (cm.exists(ANDROID_CONTROL_SCENE_MODE) && use_scene) {
        uint8_t sceneMode = cm.find(ANDROID_CONTROL_SCENE_MODE).data.u8[0];
        char sceneMode_str[32];

        switch(sceneMode) {
        case ANDROID_CONTROL_SCENE_MODE_LANDSCAPE:
            strcpy(sceneMode_str, "landscape");
            break;
        case ANDROID_CONTROL_SCENE_MODE_SNOW:
            strcpy(sceneMode_str, "snow");
            break;
        case ANDROID_CONTROL_SCENE_MODE_SUNSET:
            strcpy(sceneMode_str, "sunset");
            break;
        case ANDROID_CONTROL_SCENE_MODE_STEADYPHOTO:
            strcpy(sceneMode_str, "steadyphoto");
            break;
        case ANDROID_CONTROL_SCENE_MODE_PARTY:
            strcpy(sceneMode_str, "party");
            break;
        case ANDROID_CONTROL_SCENE_MODE_CANDLELIGHT:
            strcpy(sceneMode_str, "candelight");
            break;
        case ANDROID_CONTROL_SCENE_MODE_PORTRAIT:
            strcpy(sceneMode_str, "portrait");
            break;
        case ANDROID_CONTROL_SCENE_MODE_NIGHT_PORTRAIT:
            strcpy(sceneMode_str, "night-portrait");
            break;
        case ANDROID_CONTROL_SCENE_MODE_THEATRE:
            strcpy(sceneMode_str, "theatre");
            break;
        case ANDROID_CONTROL_SCENE_MODE_NIGHT:
            strcpy(sceneMode_str, "night");
            break;
        case ANDROID_CONTROL_SCENE_MODE_ACTION:
            strcpy(sceneMode_str, "action");
            break;
        case ANDROID_CONTROL_SCENE_MODE_BEACH:
            strcpy(sceneMode_str, "beach");
            break;
        case ANDROID_CONTROL_SCENE_MODE_FIREWORKS:
            strcpy(sceneMode_str, "fireworks");
            break;
        case ANDROID_CONTROL_SCENE_MODE_SPORTS:
            strcpy(sceneMode_str, "sports");
            break;
        case ANDROID_CONTROL_SCENE_MODE_FACE_PRIORITY:
            strcpy(sceneMode_str, "face-priority");
            break;
        case ANDROID_CONTROL_SCENE_MODE_BARCODE:
            strcpy(sceneMode_str, "barcode");
            break;
        }

        current_params.set("scene-mode", sceneMode_str);
        current_params.set("scene-detect", "off");
        use_scene = true;
    }

    if (cm.exists(ANDROID_CONTROL_AE_MODE) && !use_scene) {
        aeMode = cm.find(ANDROID_CONTROL_AE_MODE).data.u8[0];

        current_params.set("redeye-reduction", "disable");
        current_params.set("iso", "auto");

        switch(aeMode) {
        case ANDROID_CONTROL_AE_MODE_OFF:
        case ANDROID_CONTROL_AE_MODE_ON:
            current_params.set("flash-mode", "off");
            break;
        case ANDROID_CONTROL_AE_MODE_ON_AUTO_FLASH:
            current_params.set("flash-mode", "auto");
            break;
        case ANDROID_CONTROL_AE_MODE_ON_ALWAYS_FLASH:
            current_params.set("flash-mode", "on");
            break;
        case ANDROID_CONTROL_AE_MODE_ON_AUTO_FLASH_REDEYE:
            current_params.set("redeye-reduction", "enable");
            current_params.set("flash-mode", "auto");
            break;
        }
    }

    if (cm.exists(ANDROID_FLASH_MODE) && !use_scene) {
        uint8_t flashMode = cm.find(ANDROID_FLASH_MODE).data.u8[0];
        switch(flashMode) {
        case ANDROID_FLASH_MODE_OFF:
            if(aeMode == ANDROID_CONTROL_AE_MODE_OFF ||
                    aeMode == ANDROID_CONTROL_AE_MODE_ON)
                current_params.set("flash-mode", "off");
            break;
        case ANDROID_FLASH_MODE_SINGLE:
            if(aeMode == ANDROID_CONTROL_AE_MODE_OFF)
                current_params.set("flash-mode", "on");
            break;
        case ANDROID_FLASH_MODE_TORCH:
            current_params.set("flash-mode", "torch");
            break;
        }
    }

    if (cm.exists(ANDROID_CONTROL_AE_EXPOSURE_COMPENSATION) && !use_scene) {
        int32_t exposureCompensation = cm.find(ANDROID_CONTROL_AE_EXPOSURE_COMPENSATION).data.i32[0];
        char exposureCompensation_str[4];
        sprintf(exposureCompensation_str, "%d", exposureCompensation);
        current_params.set("exposure-compensation", exposureCompensation_str);
    }

    if (cm.exists(ANDROID_CONTROL_AWB_MODE) && !use_scene) {
        uint8_t awbMode = cm.find(ANDROID_CONTROL_AWB_MODE).data.u8[0];
        char awbMode_str[32];

        switch(awbMode) {
        case ANDROID_CONTROL_AWB_MODE_AUTO:
            strcpy(awbMode_str, "auto");
            break;
        case ANDROID_CONTROL_AWB_MODE_INCANDESCENT:
            strcpy(awbMode_str, "incandescent");
            break;
        case ANDROID_CONTROL_AWB_MODE_FLUORESCENT:
            strcpy(awbMode_str, "fluorescent");
            break;
        case ANDROID_CONTROL_AWB_MODE_WARM_FLUORESCENT:
            strcpy(awbMode_str, "warm-fluorescent");
            break;
        case ANDROID_CONTROL_AWB_MODE_DAYLIGHT:
            strcpy(awbMode_str, "daylight");
            break;
        case ANDROID_CONTROL_AWB_MODE_CLOUDY_DAYLIGHT:
            strcpy(awbMode_str, "cloudy-daylight");
            break;
        case ANDROID_CONTROL_AWB_MODE_TWILIGHT:
            strcpy(awbMode_str, "twilight");
            break;
        case ANDROID_CONTROL_AWB_MODE_SHADE:
            strcpy(awbMode_str, "shade");
            break;
        case ANDROID_CONTROL_AWB_MODE_OFF:
            strcpy(awbMode_str, "manual");
            break;
        }
        current_params.set("whitebalance", awbMode_str);
    }

    bool manual_focus = false;

    if (cm.exists(ANDROID_CONTROL_AF_MODE) && !use_scene) {
        uint8_t afMode = cm.find(ANDROID_CONTROL_AF_MODE).data.u8[0];
        char afMode_str[32];

        switch(afMode) {
        case ANDROID_CONTROL_AF_MODE_AUTO:
            strcpy(afMode_str, "auto");
            break;
        case ANDROID_CONTROL_AF_MODE_MACRO:
            strcpy(afMode_str, "macro");
            break;
        case ANDROID_CONTROL_AF_MODE_CONTINUOUS_VIDEO:
            strcpy(afMode_str, "continuous-video");
            break;
        case ANDROID_CONTROL_AF_MODE_CONTINUOUS_PICTURE:
            strcpy(afMode_str, "continuous-picture");
            break;
        case ANDROID_CONTROL_AF_MODE_EDOF:
            strcpy(afMode_str, "edof");
            break;
        case ANDROID_CONTROL_AF_MODE_OFF:
            strcpy(afMode_str, "manual");
            manual_focus = true;
            break;
        }
        current_params.set("focus-mode", afMode_str);
    }

    if (cm.exists(ANDROID_CONTROL_EFFECT_MODE)) {
        uint8_t effectMode = cm.find(ANDROID_CONTROL_EFFECT_MODE).data.u8[0];
        char effectMode_str[32];

        switch(effectMode) {
        case ANDROID_CONTROL_EFFECT_MODE_OFF:
            strcpy(effectMode_str, "none");
            break;
        case ANDROID_CONTROL_EFFECT_MODE_MONO:
            strcpy(effectMode_str, "mono");
            break;
        case ANDROID_CONTROL_EFFECT_MODE_NEGATIVE:
            strcpy(effectMode_str, "negative");
            break;
        case ANDROID_CONTROL_EFFECT_MODE_SOLARIZE:
            strcpy(effectMode_str, "solarize");
            break;
        case ANDROID_CONTROL_EFFECT_MODE_SEPIA:
            strcpy(effectMode_str, "sepia");
            break;
        case ANDROID_CONTROL_EFFECT_MODE_POSTERIZE:
            strcpy(effectMode_str, "posterize");
            break;
        case ANDROID_CONTROL_EFFECT_MODE_WHITEBOARD:
            strcpy(effectMode_str, "whiteboard");
            break;
        case ANDROID_CONTROL_EFFECT_MODE_BLACKBOARD:
            strcpy(effectMode_str, "blackboard");
            break;
        case ANDROID_CONTROL_EFFECT_MODE_AQUA:
            strcpy(effectMode_str, "aqua");
            break;
        }
        current_params.set("effect", effectMode_str);
    }

    if (cm.exists(ANDROID_CONTROL_AE_ANTIBANDING_MODE)) {
        uint8_t antibandingMode = cm.find(ANDROID_CONTROL_AE_ANTIBANDING_MODE).data.u8[0];
        char antibandingMode_str[8];

        switch(antibandingMode) {
        case ANDROID_CONTROL_AE_ANTIBANDING_MODE_OFF:
            strcpy(antibandingMode_str, "off");
            break;
        case ANDROID_CONTROL_AE_ANTIBANDING_MODE_AUTO:
            strcpy(antibandingMode_str, "auto");
            break;
        case ANDROID_CONTROL_AE_ANTIBANDING_MODE_50HZ:
            strcpy(antibandingMode_str, "50hz");
            break;
        case ANDROID_CONTROL_AE_ANTIBANDING_MODE_60HZ:
            strcpy(antibandingMode_str, "60hz");
            break;
        }
        current_params.set("antibanding", antibandingMode_str);
    }

    if (aeMode == ANDROID_CONTROL_AE_MODE_OFF &&
        cm.exists(ANDROID_SENSOR_SENSITIVITY) /* &&
        cm.exists(ANDROID_SENSOR_EXPOSURE_TIME) */) {
/*
        int64_t exposure_time = cm.find(ANDROID_SENSOR_EXPOSURE_TIME).data.i64[0];
        int64_t exposure_time_min = static_metadata[gCameraId].find(ANDROID_SENSOR_INFO_EXPOSURE_TIME_RANGE).data.i64[0];
        int64_t exposure_time_max = static_metadata[gCameraId].find(ANDROID_SENSOR_INFO_EXPOSURE_TIME_RANGE).data.i64[1];

        if(exposure_time > exposure_time_max)
            exposure_time = exposure_time_max;
        if (exposure_time < exposure_time_min)
            exposure_time = exposure_time_min;

        char exposure_time_str[20];
        double result = (double)exposure_time / 1000000.0;
        snprintf(exposure_time_str, sizeof(exposure_time_str), "%.6f", result);
*/
        int32_t iso = cm.find(ANDROID_SENSOR_SENSITIVITY).data.i32[0];
        int32_t min_iso = static_metadata[gCameraId].find(ANDROID_SENSOR_INFO_SENSITIVITY_RANGE).data.i32[0];
        int32_t max_iso = static_metadata[gCameraId].find(ANDROID_SENSOR_INFO_SENSITIVITY_RANGE).data.i32[1];
        char iso_str[8];

        if(iso > max_iso)
            iso = max_iso;
        if (iso < min_iso)
            iso = min_iso;

        if (iso > 0)
            strcpy(iso_str, "ISO100");
        if (iso > 100)
            strcpy(iso_str, "ISO200");
        if (iso > 200)
            strcpy(iso_str, "ISO400");
        if (iso > 400)
            strcpy(iso_str, "ISO800");
        if (iso > 800)
            strcpy(iso_str, "ISO1600");
        if (iso > 1600)
            strcpy(iso_str, "ISO3200");

        current_params.set("zsl", "off");
        HAL1_CALL(hal1_device, set_parameters, current_params.flatten());

        current_params.set("iso", iso_str);
//        current_params.set("exposure-time", exposure_time_str);
        HAL1_CALL(hal1_device, set_parameters, current_params.flatten());

        current_params.set("zsl", "on");
    }

    if (cm.exists(ANDROID_SCALER_CROP_REGION)) {
        int32_t* crop_region = cm.find(ANDROID_SCALER_CROP_REGION).data.i32;
        int zoom_value_hal3 = hal3_to_hal1_zoom(crop_region[0], crop_region[1], crop_region[2], crop_region[3],
            sensor_width, sensor_height, atoi(current_params.get("max-zoom")));

        current_params.set("zoom", zoom_value_hal3);
    }

    bool trigger_af = false;

    if (cm.exists(ANDROID_CONTROL_AF_REGIONS)) {
        int32_t* af_regions = cm.find(ANDROID_CONTROL_AF_REGIONS).data.i32;

        if (!af_regions[0] && !af_regions[1] && !af_regions[2] && !af_regions[3] && !af_regions[4])
        {
            current_params.set("focus-areas", "(0, 0, 0, 0, 0)");
            trigger_af = true;
        } else {
            char focusAreas_str[31];
            int32_t left, top, right, bottom, weight = 0;
            left = af_regions[0];
            top = af_regions[1];
            right = af_regions[2];
            bottom = af_regions[3];
            weight = af_regions[4];

            left = clamp((left * 2000 / sensor_width) - 1000, -999, 999);
            top = clamp((top * 2000 / sensor_height) - 1000, -999, 999);
            right = clamp((right * 2000 / sensor_width) - 1000, -999, 999);
            bottom = clamp((bottom * 2000 / sensor_height) - 1000, -999, 999);
            weight = clamp(weight, 1, 1000);

            sprintf(focusAreas_str, "(%d, %d, %d, %d, %d)", left, top, right, bottom, weight);
            current_params.set("focus-areas", focusAreas_str);
            trigger_af = true;
        }
    }

    if (cm.exists(ANDROID_CONTROL_AE_REGIONS)) {
        int32_t* ae_regions = cm.find(ANDROID_CONTROL_AE_REGIONS).data.i32;

        if (!ae_regions[0] && !ae_regions[1] && !ae_regions[2] && !ae_regions[3] && !ae_regions[4])
            current_params.set("metering-areas", "(0, 0, 0, 0, 0)");
        else {
            char ae_areas_str[31];
            int32_t left, top, right, bottom, weight = 0;
            left = ae_regions[0];
            top = ae_regions[1];
            right = ae_regions[2];
            bottom = ae_regions[3];
            weight = ae_regions[4];

            left = clamp((left * 2000 / sensor_width) - 1000, -999, 999);
            top = clamp((top * 2000 / sensor_height) - 1000, -999, 999);
            right = clamp((right * 2000 / sensor_width) - 1000, -999, 999);
            bottom = clamp((bottom * 2000 / sensor_height) - 1000, -999, 999);
            weight = clamp(weight, 1, 1000);

            sprintf(ae_areas_str, "(%d, %d, %d, %d, %d)", left, top, right, bottom, weight);
            current_params.set("metering-areas", ae_areas_str);
        }
    }

    if (cm.exists(ANDROID_CONTROL_AE_LOCK)) {
        uint8_t ae_lock = cm.find(ANDROID_CONTROL_AE_LOCK).data.u8[0];
        if (ae_lock == ANDROID_CONTROL_AE_LOCK_ON)
            current_params.set("auto-exposure-lock", "true");
        else
            current_params.set("auto-exposure-lock", "false");
    }

    if (cm.exists(ANDROID_CONTROL_AWB_LOCK)) {
        uint8_t awb_lock = cm.find(ANDROID_CONTROL_AWB_LOCK).data.u8[0];
        if (awb_lock == ANDROID_CONTROL_AWB_LOCK_ON)
            current_params.set("auto-whitebalance-lock", "true");
        else
            current_params.set("auto-whitebalance-lock", "false");
    }

    if (cm.exists(ANDROID_NOISE_REDUCTION_MODE)) {
        uint8_t noise_reduction = cm.find(ANDROID_NOISE_REDUCTION_MODE).data.u8[0];

        if(noise_reduction == ANDROID_NOISE_REDUCTION_MODE_FAST)
            current_params.set("denoise", "denoise-on");
        else
            current_params.set("denoise", "denoise-off");
    }

    if (cm.exists(ANDROID_JPEG_THUMBNAIL_SIZE)) {
        int32_t* jpeg_thumbnail_size = cm.find(ANDROID_JPEG_THUMBNAIL_SIZE).data.i32;
        char jpeg_thumbnail_size_str[2][5];
        sprintf(jpeg_thumbnail_size_str[0], "%u", jpeg_thumbnail_size[0]);
        sprintf(jpeg_thumbnail_size_str[1], "%u", jpeg_thumbnail_size[1]);
        current_params.set("jpeg-thumbnail-width", jpeg_thumbnail_size_str[0]);
        current_params.set("jpeg-thumbnail-height", jpeg_thumbnail_size_str[1]);
    }

    if (cm.exists(ANDROID_JPEG_THUMBNAIL_QUALITY)) {
        int32_t jpeg_thumbnail_quality = cm.find(ANDROID_JPEG_THUMBNAIL_QUALITY).data.u8[0];
        char jpeg_thumbnail_quality_str[4];
        sprintf(jpeg_thumbnail_quality_str, "%u", jpeg_thumbnail_quality);
        current_params.set("jpeg-thumbnail-quality", jpeg_thumbnail_quality_str);
    }

    if (cm.exists(ANDROID_JPEG_QUALITY)) {
        int32_t jpeg_quality = cm.find(ANDROID_JPEG_QUALITY).data.u8[0];
        char jpeg_quality_str[4];
        sprintf(jpeg_quality_str, "%u", jpeg_quality);
        current_params.set("jpeg-quality", jpeg_quality_str);
    }

    if (cm.exists(ANDROID_LENS_FOCUS_DISTANCE) &&
        manual_focus == true)
    {
        float focus_distance = cm.find(ANDROID_LENS_FOCUS_DISTANCE).data.f[0];
        char focus_distance_str[5];
        current_params.set("focus-mode", "manual");
        sprintf(focus_distance_str, "%.1f", focus_distance);
        current_params.set("manual-focus-pos-type", "3");
        current_params.set("manual-focus-position", focus_distance);

        trigger_af = true;
    }

    if (trigger_af)
        HAL1_CALL(hal1_device, cancel_auto_focus);

    HAL1_CALL(hal1_device, set_parameters, current_params.flatten());

    if (trigger_af)
        HAL1_CALL(hal1_device, auto_focus);

    cm.update(ANDROID_CONTROL_AF_STATE, &hal3on1_dev->focus_state, 1);

    if (cm.exists(ANDROID_CONTROL_CAPTURE_INTENT)) {
        captureIntent = cm.find(ANDROID_CONTROL_CAPTURE_INTENT).data.u8[0];
        if(captureIntent == CAMERA3_TEMPLATE_STILL_CAPTURE)
        {
            HAL1_CALL(hal1_device, enable_msg_type, CAMERA_MSG_COMPRESSED_IMAGE);
            HAL1_CALL(hal1_device, take_picture);
        }
    }

    /* Ignore input buffer */
    if(request->input_buffer)
        request->input_buffer->release_fence = -1;

    Vector<camera3_stream_buffer> buffers;
    for (uint32_t i = 0; i < request->num_output_buffers; ++i) {
        const camera3_stream_buffer_t& output_buffer = request->output_buffers[i];
        const Rect rect((int)output_buffer.stream->width, (int)output_buffer.stream->height);

        sp<Fence> acquireFence = new Fence(output_buffer.acquire_fence);
        e = acquireFence->wait(1000);
        if(e == TIMED_OUT) {
            ALOGE("buffer %p  frame %-4u  Wait on acquire fence timed out",
                  output_buffer.buffer, request->frame_number);
        }

        uint8_t *buf = NULL;

        GraphicBufferMapper::get().lock(*output_buffer.buffer, GRALLOC_USAGE_SW_WRITE_OFTEN, rect, (void **)&buf);

        buffers.setCapacity(request->num_output_buffers);

        if(output_buffer.stream->format == HAL_PIXEL_FORMAT_BLOB)
        {
            while(adapter->jpeg_size == 0) {
                usleep(100);
            }

            memcpy(buf, adapter->jpeg, adapter->jpeg_size);

            camera3_jpeg_blob *jpegBlob = reinterpret_cast<camera3_jpeg_blob*>(buf + sizeof(camera3_jpeg_blob));
            jpegBlob->jpeg_blob_id  = CAMERA3_JPEG_BLOB_ID;
            jpegBlob->jpeg_size     = (uint32_t)adapter->jpeg_size;
            adapter->jpeg_size = 0;

            HAL1_CALL(hal1_device, disable_msg_type, CAMERA_MSG_COMPRESSED_IMAGE);
        } else {
            // Crop yuv buffer for streams smaller than preview
            int crop_x, crop_y;
            yuv420sp_buffer *crop_buf = NULL;
            unsigned char* nv12_buf;

            if(output_buffer.stream->format == 0x102)
            {
                nv12_buf = (unsigned char*)malloc(output_buffer.stream->width * output_buffer.stream->height * 3 / 2);
                nv21_to_nv12(adapter->buffer, nv12_buf, output_buffer.stream->width, output_buffer.stream->height);
                memcpy(buf, nv12_buf, adapter->buffer_size);
                free(nv12_buf);
            } else if(output_buffer.stream->width < adapter->stream_width ||
                output_buffer.stream->height < adapter->stream_height)
            {
                get_crop_point(adapter->stream_width, adapter->stream_height,
                    output_buffer.stream->width, output_buffer.stream->height, &crop_x, &crop_y);

                crop_buf = yuv420sp_crop(adapter->buffer, adapter->stream_width, adapter->stream_height,
                    crop_x, crop_y, output_buffer.stream->width, output_buffer.stream->height);

                memcpy(buf, crop_buf->data, crop_buf->size);

                free(crop_buf->data);
                free(crop_buf);
            } else
                memcpy(buf, adapter->buffer, adapter->buffer_size);
        }
    }

    /*  Unlocking all buffers in separate loop allows to copy data from
        already processed buffer to not yet processed one */
    for(size_t i = 0; i < request->num_output_buffers; ++i) {
        const camera3_stream_buffer_t& output_buffer = request->output_buffers[i];

        GraphicBufferMapper::get().unlock(*output_buffer.buffer);
        buffers.push_back(output_buffer);
        buffers.editTop().acquire_fence = -1;
        buffers.editTop().release_fence = -1;
        buffers.editTop().status = CAMERA3_BUFFER_STATUS_OK;
    }

    auto timestamp = systemTime();
    int64_t sensorTimestamp = timestamp;
    int64_t syncFrameNumber = request->frame_number;

    camera3_notify_msg_t msg;
    msg.type = CAMERA3_MSG_SHUTTER;
    msg.message.shutter.frame_number = syncFrameNumber;
    msg.message.shutter.timestamp = timestamp;
    adapter->callback_ops->notify(adapter->callback_ops, &msg);

    cm.update(ANDROID_SENSOR_TIMESTAMP, &sensorTimestamp, 1);
    cm.update(ANDROID_SYNC_FRAME_NUMBER, &syncFrameNumber, 1);

    auto result = cm.getAndLock();
    camera3_capture_result captureResult;
    captureResult.frame_number = syncFrameNumber;
    captureResult.result = result;
    captureResult.num_output_buffers = buffers.size();
    captureResult.output_buffers = buffers.array();
    captureResult.input_buffer = NULL;
    captureResult.partial_result = 1;

    // TODO: Ratelimit capturing by max Fps supported by camera
    usleep(1000000/30);

    adapter->callback_ops->process_capture_result(adapter->callback_ops, &captureResult);
    cm.unlock(result);

    return NO_ERROR;
}

static void camera3_dump(const struct camera3_device *dev, int fd) { }

static int camera3_flush(const struct camera3_device *dev)
{
    HAL1_CALL(hal3on1_dev->hal1_device, cancel_picture);
    HAL1_CALL(hal3on1_dev->hal1_device, stop_preview);

    return NO_ERROR;
}

static camera3_device_ops_t camera3_ops = {
    .initialize = camera3_initialize,
    .configure_streams = camera3_configure_streams,
    .register_stream_buffers = NULL,
    .construct_default_request_settings = camera3_construct_default_request_settings,
    .process_capture_request = camera3_process_capture_request,
    .get_metadata_vendor_tag_ops = NULL,
    .dump = camera3_dump,
    .flush = camera3_flush,
};

static int camera_device_open(const hw_module_t *module, const char *id, hw_device_t **device)
{
    int camera_id = atoi(id);

    if (get_legacy_module())
        return -EINVAL;

    // We need to turn off sysfs torch before open HAL1 device.
    if(properties.use_sysfs_torch)
    {
        int fd_brightness(-1);
        char buffer[16];

        fd_brightness = open(SYSFS_FLASH_PATH_BRIGHTNESS, O_RDWR);
        if (fd_brightness < 0)
            ALOGW("%s: failed to open '%s'\n", __FUNCTION__, SYSFS_FLASH_PATH_BRIGHTNESS);
        else {
            int bytes = snprintf(buffer, sizeof(buffer), "0");
            int ret = write(fd_brightness, buffer, (size_t)bytes);
            if (ret <= 0)
                ALOGW("%s: failed to write to '%s'\n", __FUNCTION__, SYSFS_FLASH_PATH_BRIGHTNESS);
            close(fd_brightness);
        }
    } else if (torch_in_use) {
        torch_params.set("flash-mode", "off");

        HAL1_CALL(torch_hal1_device, set_parameters, torch_params.flatten());

        torch_hal1_device->common.close((hw_device_t*)torch_hal1_device);
        torch_in_use = false;
    }

    camera_device_t *hal1_device;
    int ret = gLegacyModule->common.methods->open(&gLegacyModule->common, id, (hw_device_t **)&hal1_device);
    if (ret != 0) {
        ALOGE("Failed to open HAL1 device");
        return ret;
    }

    adapter_camera3_device_t *adapter = (adapter_camera3_device_t *)malloc(sizeof(adapter_camera3_device_t));
    if (!adapter) {
        ALOGE("Failed to allocate memory for adapter_camera3_device_t");
        hal1_device->common.close((hw_device_t *)hal1_device);
        return -ENOMEM;
    }

    memset(adapter, 0, sizeof(*adapter));
    adapter->base.common.tag = HARDWARE_DEVICE_TAG;
    adapter->base.common.version = device_api_version;
    adapter->base.common.module = (hw_module_t *)module;
    adapter->base.common.close = camera3_close;
    adapter->base.ops = &camera3_ops;
    adapter->hal1_device = hal1_device;
    gCameraId = camera_id;
    *device = &adapter->base.common;

    hal3on1_dev = adapter;
    return NO_ERROR;
}

/*
 * HAL3on1 Adapter module
 */
static int get_number_of_cameras(void)
{
    if (get_legacy_module())
        return -EINVAL;

    return gLegacyModule->get_number_of_cameras();
}

static void camera_convert_parameters(int camera_id, const char *settings, android::CameraMetadata *metadata)
{
    CameraParameters params;
    params.unflatten(String8(settings));
    char *token = NULL;

    static const camera_metadata_rational controlAeCompensationStep = {1, 3};
    metadata->update(ANDROID_CONTROL_AE_COMPENSATION_STEP, &controlAeCompensationStep, 1);

    int32_t controlAeCompensationRange[2] = { 0, 0 };
    const char* min_aec_str = params.get("min-exposure-compensation");
    if (min_aec_str)
        controlAeCompensationRange[0] = atoi(min_aec_str);
    else
        controlAeCompensationRange[0] = 0;

    const char* max_aec_str = params.get("max-exposure-compensation");
    if (max_aec_str)
        controlAeCompensationRange[1] = atoi(max_aec_str);
    else
        controlAeCompensationRange[1] = 0;

    metadata->update(ANDROID_CONTROL_AE_COMPENSATION_RANGE, controlAeCompensationRange,
                     NELEM(controlAeCompensationRange));

    int32_t scalar_formats[] = {
        ANDROID_SCALER_AVAILABLE_FORMATS_YCrCb_420_SP,
        ANDROID_SCALER_AVAILABLE_FORMATS_BLOB,
        ANDROID_SCALER_AVAILABLE_FORMATS_IMPLEMENTATION_DEFINED,
        ANDROID_SCALER_AVAILABLE_FORMATS_YCbCr_420_888,
    };
    size_t scalar_formats_count = sizeof(scalar_formats) / sizeof(int32_t);
    metadata->update(ANDROID_SCALER_AVAILABLE_FORMATS,
                     scalar_formats,
                     scalar_formats_count);

    const char* antibanding_mode_values = params.get("antibanding-values");
    char ab_modes[128];
    if(antibanding_mode_values)
        strcpy(ab_modes, antibanding_mode_values);
    else
        strcpy(ab_modes, "auto");
    token = strtok(ab_modes, ",");

    int antibanding_counter = 0;
    uint8_t available_antibanding_modes[4];

    while (token != NULL) {
        if(!strcmp(token, "off"))
            available_antibanding_modes[antibanding_counter] = ANDROID_CONTROL_AE_ANTIBANDING_MODE_OFF;
        if(!strcmp(token, "auto"))
            available_antibanding_modes[antibanding_counter] = ANDROID_CONTROL_AE_ANTIBANDING_MODE_AUTO;
        if(!strcmp(token, "50hz"))
            available_antibanding_modes[antibanding_counter] = ANDROID_CONTROL_AE_ANTIBANDING_MODE_50HZ;
        if(!strcmp(token, "60hz"))
            available_antibanding_modes[antibanding_counter] = ANDROID_CONTROL_AE_ANTIBANDING_MODE_60HZ;

        if(available_antibanding_modes[antibanding_counter] ||
                (!strcmp(token, "none") || !strcmp(token, "off")))
            antibanding_counter++;

        token = strtok(NULL, ",");
    }

    metadata->update(ANDROID_CONTROL_AE_AVAILABLE_ANTIBANDING_MODES, available_antibanding_modes, NELEM(available_antibanding_modes));

    static const float minFocusDistance = 10.0;
    metadata->update(ANDROID_LENS_INFO_MINIMUM_FOCUS_DISTANCE, &minFocusDistance, 1);

    const char* focus_mode_values = params.get("focus-mode-values");
    char fm_modes[128];
    strcpy(fm_modes, focus_mode_values);
    token = strtok(fm_modes, ",");

    int fm_counter = 0;
    uint8_t avail_af_modes[6];

    while (token != NULL) {
        if(!strcmp(token, "auto"))
            avail_af_modes[fm_counter] = ANDROID_CONTROL_AF_MODE_AUTO;
        if(!strcmp(token, "macro"))
            avail_af_modes[fm_counter] = ANDROID_CONTROL_AF_MODE_MACRO;
        if(!strcmp(token, "continuous-video"))
            avail_af_modes[fm_counter] = ANDROID_CONTROL_AF_MODE_CONTINUOUS_VIDEO;
        if(!strcmp(token, "continuous-picture"))
            avail_af_modes[fm_counter] = ANDROID_CONTROL_AF_MODE_CONTINUOUS_PICTURE;
        if(!strcmp(token, "edof"))
            avail_af_modes[fm_counter] = ANDROID_CONTROL_AF_MODE_EDOF;
        if(!strcmp(token, "manual")) {
            avail_af_modes[fm_counter] = ANDROID_CONTROL_AF_MODE_OFF;

            static const float minFocusDistance = 10.0;
            metadata->update(ANDROID_LENS_INFO_MINIMUM_FOCUS_DISTANCE, &minFocusDistance, 1);
        }

        if(avail_af_modes[fm_counter])
            fm_counter++;

        token = strtok(NULL, ",");
    }

    avail_af_modes[fm_counter] = ANDROID_CONTROL_AF_MODE_OFF;
    fm_counter++;

    metadata->update(ANDROID_CONTROL_AF_AVAILABLE_MODES, avail_af_modes, fm_counter);

    const char* manual_focus_modes = params.get("manual-focus-modes");
    uint8_t focus_distance_calibration;

    if (manual_focus_modes)
        focus_distance_calibration = ANDROID_LENS_INFO_FOCUS_DISTANCE_CALIBRATION_CALIBRATED;

    metadata->update(ANDROID_LENS_INFO_FOCUS_DISTANCE_CALIBRATION, &focus_distance_calibration,
                     1);

    float focus_range[2] = { 10.0f, 0.1f };
    metadata->update(ANDROID_LENS_FOCUS_RANGE, focus_range,
                     2);

    const char* wb_values = params.get("whitebalance-values");
    char wb_modes[128];
    strcpy(wb_modes, wb_values);
    token = strtok(wb_modes, ",");

    int wb_counter = 0;
    uint8_t available_awb_modes[9];

    while (token != NULL) {
        if(!strcmp(token, "auto"))
            available_awb_modes[wb_counter] = ANDROID_CONTROL_AWB_MODE_AUTO;
        if(!strcmp(token, "incandescent"))
            available_awb_modes[wb_counter] = ANDROID_CONTROL_AWB_MODE_INCANDESCENT;
        if(!strcmp(token, "fluorescent"))
            available_awb_modes[wb_counter] = ANDROID_CONTROL_AWB_MODE_FLUORESCENT;
        if(!strcmp(token, "warm-fluorescent"))
            available_awb_modes[wb_counter] = ANDROID_CONTROL_AWB_MODE_WARM_FLUORESCENT;
        if(!strcmp(token, "daylight"))
            available_awb_modes[wb_counter] = ANDROID_CONTROL_AWB_MODE_DAYLIGHT;
        if(!strcmp(token, "cloudy-daylight"))
            available_awb_modes[wb_counter] = ANDROID_CONTROL_AWB_MODE_CLOUDY_DAYLIGHT;
        if(!strcmp(token, "twilight"))
            available_awb_modes[wb_counter] = ANDROID_CONTROL_AWB_MODE_TWILIGHT;
        if(!strcmp(token, "shade"))
            available_awb_modes[wb_counter] = ANDROID_CONTROL_AWB_MODE_SHADE;
        if(!strcmp(token, "manual"))
            available_awb_modes[wb_counter] = ANDROID_CONTROL_AWB_MODE_OFF;

        if(available_awb_modes[wb_counter])
            wb_counter++;

        token = strtok(NULL, ",");
    }

    metadata->update(ANDROID_CONTROL_AWB_AVAILABLE_MODES, available_awb_modes,
                     wb_counter);

    const char* scene_values = params.get("scene-mode-values");
    char scene_modes[256];
    if (scene_values)
        strcpy(scene_modes, scene_values);
    else
        strcpy(scene_modes, "auto");

    token = strtok(scene_modes, ",");

    int scene_counter = 0;
    uint8_t available_scene_modes[16];

    while (token != NULL) {
        if(!strcmp(token, "landscape"))
            available_scene_modes[scene_counter] = ANDROID_CONTROL_SCENE_MODE_LANDSCAPE;
        if(!strcmp(token, "snow"))
            available_scene_modes[scene_counter] = ANDROID_CONTROL_SCENE_MODE_SNOW;
        if(!strcmp(token, "sunset"))
            available_scene_modes[scene_counter] = ANDROID_CONTROL_SCENE_MODE_SUNSET;
        if(!strcmp(token, "steadyphoto"))
            available_scene_modes[scene_counter] = ANDROID_CONTROL_SCENE_MODE_STEADYPHOTO;
        if(!strcmp(token, "party"))
            available_scene_modes[scene_counter] = ANDROID_CONTROL_SCENE_MODE_PARTY;
        if(!strcmp(token, "candelight"))
            available_scene_modes[scene_counter] = ANDROID_CONTROL_SCENE_MODE_CANDLELIGHT;
        if(!strcmp(token, "night-portrait"))
            available_scene_modes[scene_counter] = ANDROID_CONTROL_SCENE_MODE_NIGHT_PORTRAIT;
        if(!strcmp(token, "portrait"))
            available_scene_modes[scene_counter] = ANDROID_CONTROL_SCENE_MODE_PORTRAIT;
        if(!strcmp(token, "theatre"))
            available_scene_modes[scene_counter] = ANDROID_CONTROL_SCENE_MODE_THEATRE;
        if(!strcmp(token, "night"))
            available_scene_modes[scene_counter] = ANDROID_CONTROL_SCENE_MODE_NIGHT;
        if(!strcmp(token, "action"))
            available_scene_modes[scene_counter] = ANDROID_CONTROL_SCENE_MODE_ACTION;
        if(!strcmp(token, "beach"))
            available_scene_modes[scene_counter] = ANDROID_CONTROL_SCENE_MODE_BEACH;
        if(!strcmp(token, "fireworks"))
            available_scene_modes[scene_counter] = ANDROID_CONTROL_SCENE_MODE_FIREWORKS;
        if(!strcmp(token, "sports"))
            available_scene_modes[scene_counter] = ANDROID_CONTROL_SCENE_MODE_SPORTS;
        if(!strcmp(token, "face-priority"))
            available_scene_modes[scene_counter] = ANDROID_CONTROL_SCENE_MODE_FACE_PRIORITY;
        if(!strcmp(token, "barcode"))
            available_scene_modes[scene_counter] = ANDROID_CONTROL_SCENE_MODE_BARCODE;

        if(available_scene_modes[scene_counter])
            scene_counter++;

        token = strtok(NULL, ",");
    }

    metadata->update(ANDROID_CONTROL_AVAILABLE_SCENE_MODES, available_scene_modes,
                     scene_counter);

    uint8_t controlModes[] = {
        ANDROID_CONTROL_MODE_OFF,
        ANDROID_CONTROL_MODE_AUTO,
        ANDROID_CONTROL_MODE_USE_SCENE_MODE,
    };
    metadata->update(ANDROID_CONTROL_MODE, controlModes,
                     3);

    const char* effect_values = params.get("effect-values");
    char effect_modes[256];
    if(effect_values)
        strcpy(effect_modes, effect_values);
    else
        strcpy(effect_modes, "none");

    token = strtok(effect_modes, ",");

    int effect_counter = 0;
    uint8_t available_effect_modes[9];

    while (token != NULL) {
        if(!strcmp(token, "none"))
            available_effect_modes[effect_counter] = ANDROID_CONTROL_EFFECT_MODE_OFF;
        if(!strcmp(token, "mono"))
            available_effect_modes[effect_counter] = ANDROID_CONTROL_EFFECT_MODE_MONO;
        if(!strcmp(token, "negative"))
            available_effect_modes[effect_counter] = ANDROID_CONTROL_EFFECT_MODE_NEGATIVE;
        if(!strcmp(token, "solarize"))
            available_effect_modes[effect_counter] = ANDROID_CONTROL_EFFECT_MODE_SOLARIZE;
        if(!strcmp(token, "sepia"))
            available_effect_modes[effect_counter] = ANDROID_CONTROL_EFFECT_MODE_SEPIA;
        if(!strcmp(token, "posterize"))
            available_effect_modes[effect_counter] = ANDROID_CONTROL_EFFECT_MODE_POSTERIZE;
        if(!strcmp(token, "whiteboard"))
            available_effect_modes[effect_counter] = ANDROID_CONTROL_EFFECT_MODE_WHITEBOARD;
        if(!strcmp(token, "blackboard"))
            available_effect_modes[effect_counter] = ANDROID_CONTROL_EFFECT_MODE_BLACKBOARD;
        if(!strcmp(token, "aqua"))
            available_effect_modes[effect_counter] = ANDROID_CONTROL_EFFECT_MODE_AQUA;

        if(available_effect_modes[effect_counter] ||
                (!strcmp(token, "none") || !strcmp(token, "off")))
            effect_counter++;

        token = strtok(NULL, ",");
    }

    metadata->update(ANDROID_CONTROL_AVAILABLE_EFFECTS, available_effect_modes,
                     effect_counter);

    const char* noise_reduction_supported = params.get("denoise");
    Vector<uint8_t> avail_noise_reduction_modes;
    avail_noise_reduction_modes.add(ANDROID_NOISE_REDUCTION_MODE_OFF);
    if(noise_reduction_supported) {
        avail_noise_reduction_modes.add(ANDROID_NOISE_REDUCTION_MODE_FAST);
    }
    metadata->update(ANDROID_NOISE_REDUCTION_AVAILABLE_NOISE_REDUCTION_MODES,
                     avail_noise_reduction_modes.array(),
                     avail_noise_reduction_modes.size());

    uint8_t availableFaceDetectModes[] = {
        ANDROID_STATISTICS_FACE_DETECT_MODE_OFF,
    };
    metadata->update(ANDROID_STATISTICS_INFO_AVAILABLE_FACE_DETECT_MODES,
                     availableFaceDetectModes,
                     sizeof(availableFaceDetectModes)/sizeof(availableFaceDetectModes[0]));

    android::Vector<android::Size> previewSizes;
    params.getSupportedPreviewSizes(previewSizes);
    android::Vector<android::Size> pictureSizes;
    params.getSupportedPictureSizes(pictureSizes);
    android::Vector<android::Size> videoSizes;
    params.getSupportedVideoSizes(videoSizes);

    int sensor_width;
    int sensor_height;

    int32_t available_resolutions[MAX_SIZES_CNT * 2];
    size_t max_jpeg_size_sz = 0;

    int n = 0;
    for (size_t i = 0; i < pictureSizes.size(); i++) {
        available_resolutions[n] = pictureSizes[i].width;
        available_resolutions[n += 1] = pictureSizes[i].height;

        if (pictureSizes[i].width * pictureSizes[i].height > max_jpeg_size_sz) {
            sensor_width = pictureSizes[i].width;
            sensor_height = pictureSizes[i].height;
            max_jpeg_size_sz = pictureSizes[i].width * pictureSizes[i].height;
        }

        n++;
    }
    metadata->update(ANDROID_SCALER_AVAILABLE_PROCESSED_SIZES, available_resolutions,
                     sizeof(available_resolutions) / sizeof(available_resolutions[0]));

    max_jpeg_size_sz = max_jpeg_size_sz * 3/2 + sizeof(camera3_jpeg_blob_t);

    int32_t max_jpeg_size = (int32_t)max_jpeg_size_sz;
    metadata->update(ANDROID_JPEG_MAX_SIZE,
                     &max_jpeg_size, 1);

    // Try to get full (raw) sensor size if available
    const char* sensor_size_str = params.get("raw-size");
    if(sensor_size_str)
        sscanf(sensor_size_str, "%dx%d", &sensor_width, &sensor_height);

    int32_t sensor_size[2] = {sensor_width, sensor_height};
    metadata->update(ANDROID_SENSOR_INFO_PIXEL_ARRAY_SIZE, sensor_size, 2);

    int32_t scaler_crop_region[4];
    scaler_crop_region[0] = 0;
    scaler_crop_region[1] = 0;
    scaler_crop_region[2] = sensor_width;
    scaler_crop_region[3] = sensor_height;
    metadata->update(ANDROID_SCALER_CROP_REGION, scaler_crop_region, 4);

    /* fake, but valid aspect ratio */
    const float sensorInfoPhysicalSize[] = {
        5.0f,
        5.0f * (float)sensor_height / (float)sensor_width
    };
    metadata->update(ANDROID_SENSOR_INFO_PHYSICAL_SIZE, sensorInfoPhysicalSize, NELEM(sensorInfoPhysicalSize));

    int32_t active_array_size[] = {0, 0, sensor_width, sensor_height};
    metadata->update(ANDROID_SENSOR_INFO_ACTIVE_ARRAY_SIZE,
                     active_array_size, sizeof(active_array_size)/sizeof(active_array_size[0]));

    metadata->update(ANDROID_SCALER_AVAILABLE_RAW_SIZES, available_resolutions,
                     sizeof(available_resolutions) / sizeof(available_resolutions[0]));

    const int32_t available_thumbnail_sizes[] = {0, 0,
                                                 176, 144,
                                                 320, 240,
                                                 432, 288,
                                                 480, 288,
                                                 512, 288,
                                                 512, 384
                                                };

    metadata->update(ANDROID_JPEG_AVAILABLE_THUMBNAIL_SIZES,
                     available_thumbnail_sizes,
                     sizeof(available_thumbnail_sizes)/sizeof(int32_t));

    const char* fps_ranges_str = params.get("preview-fps-range");

    int32_t available_fps_ranges[2];
    size_t j = 0;
    if (fps_ranges_str) {
        int min_fps = 0;
        int max_fps = 0;
        sscanf(fps_ranges_str, "%d,%d", &min_fps, &max_fps);

        for (size_t i = 0; i < previewSizes.size(); i++) {
            available_fps_ranges[j] = min_fps / 1000;
            available_fps_ranges[j+1] = max_fps / 1000;
            j+=2;
        }
    }

    metadata->update(ANDROID_CONTROL_AE_AVAILABLE_TARGET_FPS_RANGES,
                     available_fps_ranges, sizeof(available_fps_ranges));

    size_t max_stream_configs_size = (pictureSizes.size() + previewSizes.size() + videoSizes.size()) * scalar_formats_count * 4;
    int32_t available_stream_configs[max_stream_configs_size];
    size_t idx = 0;
    for (size_t j = 0; j < scalar_formats_count; j++) {
        switch (scalar_formats[j]) {
        case HAL_PIXEL_FORMAT_BLOB:
            for (size_t i = 0; i < pictureSizes.size(); i++) {
                available_stream_configs[idx] = scalar_formats[j];
                available_stream_configs[idx+1] = pictureSizes[i].width;
                available_stream_configs[idx+2] = pictureSizes[i].height;
                available_stream_configs[idx+3] = ANDROID_SCALER_AVAILABLE_STREAM_CONFIGURATIONS_OUTPUT;
                idx+=4;
            }
            break;
        case HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED:
        case HAL_PIXEL_FORMAT_YCrCb_420_SP:
            case HAL_PIXEL_FORMAT_YCbCr_420_888:
            for (size_t i = 0; i < previewSizes.size(); i++) {
                available_stream_configs[idx] = scalar_formats[j];
                available_stream_configs[idx+1] = previewSizes[i].width;
                available_stream_configs[idx+2] = previewSizes[i].height;
                available_stream_configs[idx+3] = ANDROID_SCALER_AVAILABLE_STREAM_CONFIGURATIONS_OUTPUT;
                idx+=4;
            }
            break;
        }
    }
    metadata->update(ANDROID_SCALER_AVAILABLE_STREAM_CONFIGURATIONS,
                     available_stream_configs, idx);

    /* android.scaler.availableMinFrameDurations */
    int64_t available_min_durations[max_stream_configs_size];
    idx = 0;
    for (size_t j = 0; j < scalar_formats_count; j++) {
        switch (scalar_formats[j]) {
        default:
            for (size_t i = 0; i < previewSizes.size(); i++) {
                available_min_durations[idx] = scalar_formats[j];
                available_min_durations[idx+1] = previewSizes[i].width;
                available_min_durations[idx+2] = previewSizes[i].height;
                available_min_durations[idx+3] = 33333333;
                idx+=4;
            }
            break;
        }
    }
    metadata->update(ANDROID_SCALER_AVAILABLE_MIN_FRAME_DURATIONS,
                     &available_min_durations[0], idx);

    /*available stall durations depend on the hw + sw and will be different for different devices */
    /*have to add for raw after implementation*/
    int32_t stall_formats[] = {HAL_PIXEL_FORMAT_BLOB};
    size_t stall_formats_count = sizeof(stall_formats)/sizeof(int32_t);

    size_t count = pictureSizes.size();
    size_t available_stall_size = count * 4;
    int64_t available_stall_durations[available_stall_size];
    idx = 0;
    for (uint32_t j = 0; j < stall_formats_count; j++) {
        if (stall_formats[j] == HAL_PIXEL_FORMAT_BLOB) {
            for (uint32_t i = 0; i < count; i++) {
                available_stall_durations[idx]   = stall_formats[j];
                available_stall_durations[idx+1] = pictureSizes[i].width;
                available_stall_durations[idx+2] = pictureSizes[i].height;
                available_stall_durations[idx+3] = 33333333;
                idx+=4;
            }
        }
    }
    metadata->update(ANDROID_SCALER_AVAILABLE_STALL_DURATIONS,
                     available_stall_durations,
                     idx);

    int32_t sensitivity_range[2];

    const char* min_iso_str = params.get("min-iso");
    if (min_iso_str)
        sensitivity_range[0] = atoi(min_iso_str);
    else
        sensitivity_range[0] = 0;

    const char* max_iso_str = params.get("max-iso");
    if (max_iso_str)
        sensitivity_range[1] = atoi(max_iso_str);
    else
        sensitivity_range[1] = 0;

    metadata->update(ANDROID_SENSOR_INFO_SENSITIVITY_RANGE, sensitivity_range,
                     sizeof(sensitivity_range) / sizeof(int32_t));

    int64_t exposure_time_range[2];
//    const char* min_exposure_time_str = params.get("min-exposure-time");
//    if (min_exposure_time_str)
//        exposure_time_range[0] = (int64_t)(atoi(min_exposure_time_str) * 1000000);
//    else
        exposure_time_range[0] = 0;

//    const char* max_exposure_time_str = params.get("max-exposure-time");
//    if (max_exposure_time_str)
//        exposure_time_range[1] = (int64_t)(atoi(max_exposure_time_str) * 1000000);
//    else
        exposure_time_range[1] = 0;

    metadata->update(ANDROID_SENSOR_INFO_EXPOSURE_TIME_RANGE, exposure_time_range,
                     sizeof(exposure_time_range) / sizeof(int64_t));

    const char* flashMode = params.get("flash-mode-values");

    uint8_t flashAvailable;
    if(flashMode)
        flashAvailable = ANDROID_FLASH_INFO_AVAILABLE_TRUE;
    else
        flashAvailable = ANDROID_FLASH_INFO_AVAILABLE_FALSE;

    metadata->update(ANDROID_FLASH_INFO_AVAILABLE,
                     &flashAvailable, 1);

    uint8_t avail_leds = 0;
    metadata->update(ANDROID_LED_AVAILABLE_LEDS,
                     &avail_leds, 0);

    Vector<uint8_t> avail_ae_modes;
    avail_ae_modes.add(ANDROID_CONTROL_AE_MODE_ON);
    avail_ae_modes.add(ANDROID_CONTROL_AE_MODE_OFF);
    if(flashMode) {
        avail_ae_modes.add(ANDROID_CONTROL_AE_MODE_ON_AUTO_FLASH);
        avail_ae_modes.add(ANDROID_CONTROL_AE_MODE_ON_ALWAYS_FLASH);
        const char* redeye = params.get("redeye-reduction");
        if(redeye)
            avail_ae_modes.add(ANDROID_CONTROL_AE_MODE_ON_AUTO_FLASH_REDEYE);
    }
    metadata->update(ANDROID_CONTROL_AE_AVAILABLE_MODES,
                     avail_ae_modes.array(),
                     avail_ae_modes.size());

    const char* focal_length_str = params.get("focal-length");
    float focal_length = 3.0;
    if (focal_length_str)
        sscanf(focal_length_str, "%f", &focal_length);

    metadata->update(ANDROID_LENS_INFO_AVAILABLE_FOCAL_LENGTHS,
                     &focal_length,
                     1);

    int numerator;
    int stub;
    sscanf(params.get("exposure-compensation-step"), "%d.%d", &stub, &numerator) ;
    camera_metadata_rational exposureCompensationStep = {
        numerator,
        1000000
    };
    metadata->update(ANDROID_CONTROL_AE_COMPENSATION_STEP,
                     &exposureCompensationStep, 1);

    const char* ae_lock_str = params.get("auto-exposure-lock-supported");
    uint8_t ae_lock_av = 0;
    if(ae_lock_str && !strcmp(ae_lock_str, "true"))
        ae_lock_av = 1;

    metadata->update(ANDROID_CONTROL_AE_LOCK_AVAILABLE, &ae_lock_av, 1);

    const char* awb_lock_str = params.get("auto-whitebalance-lock-supported");
    uint8_t awb_lock_av = 0;
    if(awb_lock_str && !strcmp(awb_lock_str, "true"))
        awb_lock_av = 1;

    metadata->update(ANDROID_CONTROL_AWB_LOCK_AVAILABLE, &awb_lock_av, 1);

    uint8_t availableVstabModes[] = {ANDROID_CONTROL_VIDEO_STABILIZATION_MODE_OFF};
    metadata->update(ANDROID_CONTROL_AVAILABLE_VIDEO_STABILIZATION_MODES,
                     availableVstabModes, sizeof(availableVstabModes));

    // TODO: implement zoom properly
    const char* zoom_support_str = params.get("zoom-supported");
    float zoom_max = 1.0;
    if (zoom_support_str)
        zoom_max = 2.0f;
    metadata->update(ANDROID_SCALER_AVAILABLE_MAX_DIGITAL_ZOOM, &zoom_max, 1);

    uint8_t croppingType = ANDROID_SCALER_CROPPING_TYPE_CENTER_ONLY;
    metadata->update(ANDROID_SCALER_CROPPING_TYPE, &croppingType, 1);

    int32_t max3aRegions[3] = {/*AE*/1,/*AWB*/ 0,/*AF*/ 1};
    metadata->update(ANDROID_CONTROL_MAX_REGIONS,
                     max3aRegions, 3);

    uint8_t timestampSource = ANDROID_SENSOR_INFO_TIMESTAMP_SOURCE_UNKNOWN;
    metadata->update(ANDROID_SENSOR_INFO_TIMESTAMP_SOURCE,
                     &timestampSource, 1);

    int32_t max_input_streams = 0;
    metadata->update(ANDROID_REQUEST_MAX_NUM_INPUT_STREAMS,
                     &max_input_streams,
                     1);

    int32_t io_format_map[] = {};
    metadata->update(ANDROID_SCALER_AVAILABLE_INPUT_OUTPUT_FORMATS_MAP,
                     io_format_map, 0);

    int32_t request_keys_basic[] = {
        ANDROID_CONTROL_AE_ANTIBANDING_MODE,
        ANDROID_CONTROL_AE_EXPOSURE_COMPENSATION,
        ANDROID_CONTROL_AE_LOCK,
        ANDROID_CONTROL_AE_MODE,
        ANDROID_CONTROL_AE_REGIONS,
        ANDROID_CONTROL_AE_TARGET_FPS_RANGE,
        ANDROID_CONTROL_AF_MODE,
        ANDROID_CONTROL_AF_REGIONS,
        ANDROID_CONTROL_AF_TRIGGER,
        ANDROID_CONTROL_AWB_LOCK,
        ANDROID_CONTROL_AWB_MODE,
        ANDROID_CONTROL_CAPTURE_INTENT,
        ANDROID_CONTROL_EFFECT_MODE,
        ANDROID_CONTROL_MODE,
        ANDROID_CONTROL_SCENE_MODE,
        ANDROID_CONTROL_VIDEO_STABILIZATION_MODE,
        ANDROID_FLASH_MODE,
        ANDROID_JPEG_ORIENTATION,
        ANDROID_JPEG_QUALITY,
        ANDROID_JPEG_THUMBNAIL_QUALITY,
        ANDROID_JPEG_THUMBNAIL_SIZE,
        //ANDROID_LENS_APERTURE,
        //ANDROID_LENS_FILTER_DENSITY,
        ANDROID_LENS_FOCAL_LENGTH,
        ANDROID_LENS_FOCUS_DISTANCE,
        //ANDROID_LENS_OPTICAL_STABILIZATION_MODE,
        ANDROID_NOISE_REDUCTION_MODE,
        ANDROID_REQUEST_ID,
        ANDROID_REQUEST_TYPE,
        ANDROID_SCALER_CROP_REGION,
//        ANDROID_SENSOR_EXPOSURE_TIME,
        ANDROID_SENSOR_FRAME_DURATION,
        ANDROID_SENSOR_SENSITIVITY,
        //ANDROID_SHADING_MODE,
//       ANDROID_SHADING_STRENGTH,
    };

    size_t request_keys_cnt =
        sizeof(request_keys_basic)/sizeof(request_keys_basic[0]);
    Vector<int32_t> available_request_keys;
    available_request_keys.appendArray(request_keys_basic, request_keys_cnt);
    metadata->update(ANDROID_REQUEST_AVAILABLE_REQUEST_KEYS,
                     available_request_keys.array(), available_request_keys.size());

    int32_t result_keys_basic[] = {
        ANDROID_CONTROL_AE_MODE,
        ANDROID_CONTROL_AE_REGIONS,
        ANDROID_CONTROL_AE_STATE,
        ANDROID_CONTROL_AF_MODE,
        ANDROID_CONTROL_AF_REGIONS,
        ANDROID_CONTROL_AF_STATE,
        ANDROID_CONTROL_AWB_MODE,
        ANDROID_CONTROL_AWB_STATE,
        ANDROID_CONTROL_MODE,
        ANDROID_FLASH_MODE,
        ANDROID_FLASH_STATE,
        ANDROID_JPEG_QUALITY,
        ANDROID_JPEG_THUMBNAIL_QUALITY,
        ANDROID_JPEG_THUMBNAIL_SIZE,
        //ANDROID_LENS_APERTURE,
        //ANDROID_LENS_FILTER_DENSITY,
        ANDROID_LENS_FOCAL_LENGTH,
        ANDROID_LENS_FOCUS_DISTANCE,
        ANDROID_LENS_FOCUS_RANGE,
        //ANDROID_LENS_STATE,
        //ANDROID_LENS_OPTICAL_STABILIZATION_MODE,
        ANDROID_NOISE_REDUCTION_MODE,
        ANDROID_REQUEST_ID,
        ANDROID_SCALER_CROP_REGION,
        //ANDROID_SHADING_MODE,
//        ANDROID_SENSOR_EXPOSURE_TIME,
        ANDROID_SENSOR_FRAME_DURATION,
        ANDROID_SENSOR_SENSITIVITY,
        ANDROID_SENSOR_TIMESTAMP,
    };
    size_t result_keys_cnt =
        sizeof(result_keys_basic)/sizeof(result_keys_basic[0]);

    Vector<int32_t> available_result_keys;
    available_result_keys.appendArray(result_keys_basic, result_keys_cnt);
    metadata->update(ANDROID_REQUEST_AVAILABLE_RESULT_KEYS,
                     available_result_keys.array(), available_result_keys.size());

    int32_t available_characteristics_keys[] = {
        ANDROID_CONTROL_AE_AVAILABLE_ANTIBANDING_MODES,
        ANDROID_CONTROL_AE_AVAILABLE_MODES,
        ANDROID_CONTROL_AE_AVAILABLE_TARGET_FPS_RANGES,
        ANDROID_CONTROL_AE_COMPENSATION_RANGE,
        ANDROID_CONTROL_AE_COMPENSATION_STEP,
        ANDROID_CONTROL_AF_AVAILABLE_MODES,
        ANDROID_CONTROL_AVAILABLE_EFFECTS,
        ANDROID_SCALER_CROPPING_TYPE,
        ANDROID_SENSOR_INFO_TIMESTAMP_SOURCE,
        ANDROID_CONTROL_AVAILABLE_SCENE_MODES,
        ANDROID_CONTROL_AVAILABLE_VIDEO_STABILIZATION_MODES,
        ANDROID_CONTROL_AWB_AVAILABLE_MODES,
        ANDROID_CONTROL_MAX_REGIONS,
        ANDROID_FLASH_INFO_AVAILABLE,
        ANDROID_JPEG_AVAILABLE_THUMBNAIL_SIZES,
        ANDROID_JPEG_MAX_SIZE,
//      ANDROID_LENS_INFO_AVAILABLE_APERTURES,
        //   ANDROID_LENS_INFO_AVAILABLE_OPTICAL_STABILIZATION,
        ANDROID_LENS_INFO_MINIMUM_FOCUS_DISTANCE,
        //    ANDROID_LENS_INFO_SHADING_MAP_SIZE,
        ANDROID_LENS_INFO_FOCUS_DISTANCE_CALIBRATION,
        ANDROID_LENS_FACING,
//       ANDROID_REQUEST_MAX_NUM_OUTPUT_STREAMS, 
        ANDROID_REQUEST_MAX_NUM_INPUT_STREAMS,
        //     ANDROID_REQUEST_PIPELINE_MAX_DEPTH,
        ANDROID_REQUEST_AVAILABLE_CAPABILITIES,
        ANDROID_REQUEST_AVAILABLE_REQUEST_KEYS, ANDROID_REQUEST_AVAILABLE_RESULT_KEYS,
        ANDROID_REQUEST_AVAILABLE_CHARACTERISTICS_KEYS, //ANDROID_REQUEST_PARTIAL_RESULT_COUNT,
        ANDROID_SCALER_AVAILABLE_MAX_DIGITAL_ZOOM,
        ANDROID_SCALER_AVAILABLE_INPUT_OUTPUT_FORMATS_MAP,
        ANDROID_SCALER_AVAILABLE_STREAM_CONFIGURATIONS,
        /*ANDROID_SCALER_AVAILABLE_STALL_DURATIONS,*/
        ANDROID_SCALER_AVAILABLE_MIN_FRAME_DURATIONS,
        ANDROID_SENSOR_INFO_ACTIVE_ARRAY_SIZE,
        ANDROID_SENSOR_INFO_SENSITIVITY_RANGE,
//        ANDROID_SENSOR_INFO_EXPOSURE_TIME_RANGE,
        ANDROID_SENSOR_INFO_MAX_FRAME_DURATION,
        ANDROID_SENSOR_INFO_PHYSICAL_SIZE,
        ANDROID_SENSOR_INFO_PIXEL_ARRAY_SIZE,
        ANDROID_SENSOR_ORIENTATION,
        ANDROID_NOISE_REDUCTION_AVAILABLE_NOISE_REDUCTION_MODES,
        ANDROID_INFO_SUPPORTED_HARDWARE_LEVEL
    };
    metadata->update(ANDROID_REQUEST_AVAILABLE_CHARACTERISTICS_KEYS,
                     available_characteristics_keys,
                     sizeof(available_characteristics_keys)/sizeof(int32_t));
}

static int get_camera_info(int camera_id, struct camera_info *info)
{
    if (get_legacy_module())
        return -EINVAL;

    int ret = gLegacyModule->get_camera_info(camera_id, info);
    if (ret != 0)
        return ret;

    camera_device_t *hal1_device;
    ret = gLegacyModule->common.methods->open(&gLegacyModule->common, !camera_id ? "0" : "1",
            (hw_device_t **)&hal1_device);
    if (ret != 0) {
        ALOGE("Failed to open HAL1 device");
        return ret;
    }

    info->device_version = device_api_version;

    android::CameraMetadata staticInfo;

    char *params = HAL1_CALL(hal1_device, get_parameters);

    hal1_device->common.close((hw_device_t*)hal1_device);

    camera_convert_parameters(camera_id, params, &staticInfo);

    Vector<uint8_t> available_capabilities;
    available_capabilities.add(ANDROID_REQUEST_AVAILABLE_CAPABILITIES_BACKWARD_COMPATIBLE);

    staticInfo.update(ANDROID_REQUEST_AVAILABLE_CAPABILITIES,
                      available_capabilities.array(),
                      available_capabilities.size());

    uint8_t supportedHardwareLevel = ANDROID_INFO_SUPPORTED_HARDWARE_LEVEL_LEGACY;
    if(properties.use_limited_level)
        supportedHardwareLevel = ANDROID_INFO_SUPPORTED_HARDWARE_LEVEL_LIMITED;

    staticInfo.update(ANDROID_INFO_SUPPORTED_HARDWARE_LEVEL, &supportedHardwareLevel, 1);

    int32_t sensor_orientation = info->orientation;
    staticInfo.update(ANDROID_SENSOR_ORIENTATION, &sensor_orientation, 1);

    uint8_t facing = (info->facing == CAMERA_FACING_BACK) ? ANDROID_LENS_FACING_BACK : ANDROID_LENS_FACING_FRONT;
    staticInfo.update(ANDROID_LENS_FACING, &facing, 1);

    info->static_camera_characteristics = staticInfo.release();

    if(!static_parameters_initialized[camera_id])
    {
        static_metadata[camera_id] = info->static_camera_characteristics;
        default_parameters[camera_id].unflatten(String8(params));
        static_parameters_initialized[camera_id] = true;
    }

    return NO_ERROR;
}

static int set_callbacks(const camera_module_callbacks_t *callbacks)
{
    adapter_callbacks.hal3_callbacks = callbacks;

    return NO_ERROR;
}

static int sysfs_torch_mode(const char* camera_id, bool enabled)
{
    int fd_brightness(-1);
    char buffer[16];
    int retVal;

    fd_brightness = open(SYSFS_FLASH_PATH_BRIGHTNESS, O_RDWR);
    if (fd_brightness < 0) {
        ALOGE("%s: failed to open '%s'\n", __FUNCTION__, SYSFS_FLASH_PATH_BRIGHTNESS);
        return -EBADF;
    }

    if (enabled) {
        int bytes = snprintf(buffer, sizeof(buffer), "1");
        retVal = write(fd_brightness, buffer, (size_t)bytes);
        if (retVal <= 0) {
            ALOGE("%s: failed to write to '%s'\n", __FUNCTION__, SYSFS_FLASH_PATH_BRIGHTNESS);
            return -EBADFD;
        }
    } else {
        int bytes = snprintf(buffer, sizeof(buffer), "0");
        retVal = write(fd_brightness, buffer, (size_t)bytes);
        if (retVal <= 0) {
            ALOGE("%s: failed to write to '%s'\n", __FUNCTION__, SYSFS_FLASH_PATH_BRIGHTNESS);
            return -EBADFD;
        }
    }
    close(fd_brightness);

    return NO_ERROR;
}

static int hal1_torch_mode(const char* camera_id, bool enabled)
{
    if (get_legacy_module())
        return -EINVAL;

    if(!torch_in_use && enabled)
    {
        int ret = gLegacyModule->common.methods->open(&gLegacyModule->common, camera_id, (hw_device_t **)&torch_hal1_device);
        if (ret != 0) {
            ALOGE("Failed to open HAL1 device");
            return ret;
        }

        char *settings = HAL1_CALL(torch_hal1_device, get_parameters);
        torch_params.unflatten(String8(settings));
        torch_params.set("flash-mode", "torch");

        HAL1_CALL(torch_hal1_device, set_parameters, torch_params.flatten());
        torch_in_use = true;
    } else if (torch_in_use && !enabled) {
        torch_params.set("flash-mode", "off");

        HAL1_CALL(torch_hal1_device, set_parameters, torch_params.flatten());

        torch_hal1_device->common.close((hw_device_t*)torch_hal1_device);
        torch_in_use = false;
    }

    return NO_ERROR;
}

static int set_torch_mode(const char* camera_id, bool enabled)
{
    Mutex::Autolock lock(gHAL3AdapterLock);

    if(properties.use_sysfs_torch)
        return sysfs_torch_mode(camera_id, enabled);

    return hal1_torch_mode(camera_id, enabled);
}

static int init()
{
    char value[PROPERTY_VALUE_MAX];
    property_get("persist.camera.hal3on1.use_memfd", value, "0");
    if (atoi(value) == 1) {
        ALOGI("HAL3on1: using memfd_create as shared memory");
        properties.use_memfd = true;
    }

    property_get("persist.camera.hal3on1.use_limited_level", value, "0");
    if (atoi(value) == 1) {
        ALOGI("HAL3on1: using hardware support level LIMITED");
        properties.use_limited_level = true;
    }

    property_get("persist.camera.hal3on1.use_sysfs_torch", value, "0");
    if (atoi(value) == 1) {
        ALOGI("HAL3on1: using sysfs torch control instead of HAL1");
        properties.use_sysfs_torch = true;
    }

    return NO_ERROR;
}

static struct hw_module_methods_t camera_module_methods = {
    .open = camera_device_open,
};

static camera_module_t camera3_module = {
    .common = {
        .tag = HARDWARE_MODULE_TAG,
        .module_api_version = CAMERA_MODULE_API_VERSION_2_4,
        .hal_api_version = HARDWARE_HAL_API_VERSION,
        .id = CAMERA_HARDWARE_MODULE_ID,
        .name = "Camera HAL3on1 Adapter",
        .author = "acroreiser",
        .methods = &camera_module_methods,
        .dso = NULL,
        .reserved = {0},
    },
    .get_number_of_cameras = get_number_of_cameras,
    .get_camera_info = get_camera_info,
    .set_callbacks = set_callbacks,
    .get_vendor_tag_ops = NULL,
    .open_legacy = NULL,
    .set_torch_mode = set_torch_mode,
    .init = init,
};

camera_module_t HAL_MODULE_INFO_SYM = camera3_module;
