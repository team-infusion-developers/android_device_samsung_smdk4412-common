/*
 ** Copyright 2007, The Android Open Source Project
 **
 ** Licensed under the Apache License, Version 2.0 (the "License");
 ** you may not use this file except in compliance with the License.
 ** You may obtain a copy of the License at
 **
 **     http://www.apache.org/licenses/LICENSE-2.0
 **
 ** Unless required by applicable law or agreed to in writing, software
 ** distributed under the License is distributed on an "AS IS" BASIS,
 ** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 ** See the License for the specific language governing permissions and
 ** limitations under the License.
 */

#include <log/log.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>

#include "../egl_impl.h"

#include "egl_display.h"

using namespace android;

// ----------------------------------------------------------------------------

void getNativePixelFormat(EGLDisplay dpy, egl_connection_t* cnx, EGLConfig config, EGLint& format) {
    // Set the native window's buffers format to match what this config requests.
    // Whether to use sRGB gamma is not part of the EGLconfig, but is part
    // of our native format. So if sRGB gamma is requested, we have to
    // modify the EGLconfig's format before setting the native window's
    // format.

    EGLint componentType = EGL_COLOR_COMPONENT_TYPE_FIXED_EXT;
    cnx->egl.eglGetConfigAttrib(dpy, config, EGL_COLOR_COMPONENT_TYPE_EXT, &componentType);

    if (!cnx->egl.eglGetConfigAttrib(dpy, config, EGL_NATIVE_VISUAL_ID,
            &format)) {
        ALOGE("eglGetConfigAttrib(EGL_NATIVE_VISUAL_ID) failed: %#x",
                eglGetError());
        format = 0;
    }
}
