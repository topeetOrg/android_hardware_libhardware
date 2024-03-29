/*
 * Copyright (C) 2012 The Android Open Source Project
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

#include <cstdlib>
#include <hardware/camera_common.h>
#include <hardware/hardware.h>
#include "Camera.h"

//#define LOG_NDEBUG 0
#define LOG_TAG "DefaultCameraHAL"
#include <cutils/log.h>

#define ATRACE_TAG (ATRACE_TAG_CAMERA | ATRACE_TAG_HAL)
#include <cutils/trace.h>

#include "CameraHAL.h"

/*
 * This file serves as the entry point to the HAL.  It contains the module
 * structure and functions used by the framework to load and interface to this
 * HAL, as well as the handles to the individual camera devices.
 */

namespace default_camera_hal {

// Default Camera HAL has 2 cameras, front and rear.

//dg change 2  to  1
static CameraHAL gCameraHAL(1);

CameraHAL::CameraHAL(int num_cameras)
  : mNumberOfCameras(num_cameras),
    mCallbacks(NULL)
{
    int i;

    // Allocate camera array and instantiate camera devices
    mCameras = new Camera*[mNumberOfCameras];
    for (i = 0; i < mNumberOfCameras; i++) {
        mCameras[i] = new Camera(i);
    }
}

CameraHAL::~CameraHAL()
{
    int i;

    for (i = 0; i < mNumberOfCameras; i++) {
        delete mCameras[i];
    }
    delete [] mCameras;
}

int CameraHAL::getNumberOfCameras()
{
    ALOGV("%s: %d", __func__, mNumberOfCameras);
    return mNumberOfCameras;
}

int CameraHAL::getCameraInfo(int id, struct camera_info* info)
{
    ALOGV("%s: camera id %d: info=%p", __func__, id, info);
    if (id < 0 || id >= mNumberOfCameras) {
        ALOGE("%s: Invalid camera id %d", __func__, id);
        return -ENODEV;
    }
    // TODO: return device-specific static metadata
    return mCameras[id]->getInfo(info);
}

int CameraHAL::setCallbacks(const camera_module_callbacks_t *callbacks)
{
    ALOGV("%s : callbacks=%p", __func__, callbacks);
    mCallbacks = callbacks;
    return 0;
}

int CameraHAL::open(const hw_module_t* mod, const char* name, hw_device_t** dev)
{
    int id;
    char *nameEnd;

    ALOGV("%s: module=%p, name=%s, device=%p", __func__, mod, name, dev);
    if (*name == '\0') {
        ALOGE("%s: Invalid camera id name is NULL", __func__);
        return -EINVAL;
    }
    id = strtol(name, &nameEnd, 10);
    if (*nameEnd != '\0') {
        ALOGE("%s: Invalid camera id name %s", __func__, name);
        return -EINVAL;
    } else if (id < 0 || id >= mNumberOfCameras) {
        ALOGE("%s: Invalid camera id %d", __func__, id);
        return -ENODEV;
    }
    return mCameras[id]->open(mod, dev);
}

extern "C" {

static int get_number_of_cameras()
{
    return gCameraHAL.getNumberOfCameras();
}

static int get_camera_info(int id, struct camera_info* info)
{
    return gCameraHAL.getCameraInfo(id, info);
}

static int set_callbacks(const camera_module_callbacks_t *callbacks)
{
    return gCameraHAL.setCallbacks(callbacks);
}

static int open_dev(const hw_module_t* mod, const char* name, hw_device_t** dev)
{
    return gCameraHAL.open(mod, name, dev);
}

static hw_module_methods_t gCameraModuleMethods = {
    open : open_dev
};

camera_module_t HAL_MODULE_INFO_SYM __attribute__ ((visibility("default"))) = {
    common : {
        tag                : HARDWARE_MODULE_TAG,
        module_api_version : CAMERA_MODULE_API_VERSION_2_0,
        hal_api_version    : HARDWARE_HAL_API_VERSION,
        id                 : CAMERA_HARDWARE_MODULE_ID,
        name               : "Default Camera HAL",
        author             : "The Android Open Source Project",
        methods            : &gCameraModuleMethods,
        dso                : NULL,
        reserved           : {0},
    },
    get_number_of_cameras : get_number_of_cameras,
    get_camera_info       : get_camera_info,
    set_callbacks         : set_callbacks
};
} // extern "C"

} // namespace default_camera_hal
