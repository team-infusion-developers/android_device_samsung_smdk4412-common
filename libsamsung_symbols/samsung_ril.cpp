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
#include <cutils/log.h>
#include <dlfcn.h>
#include <errno.h>

#define REAL_LIBBINDER_NAME				"/system/lib/libbinder.so"

/* status_t Parcel::writeString16 */
extern "C" int _ZN7android6Parcel13writeString16EPKDsj();
extern "C" int _ZN7android6Parcel13writeString16EPKtj() {
    return _ZN7android6Parcel13writeString16EPKDsj();
}

/* libril constructor / destructor */

void libEvtLoading(void) __attribute__((constructor));
void libEvtUnloading(void) __attribute__((destructor));

/* wrapped library handle */

void *realBinderLibHandle;

/* wrapped callbacks */
int (*fRealWriteAlignedInt)(int val);

void libEvtLoading(void)
{
	realBinderLibHandle = dlopen(REAL_LIBBINDER_NAME, RTLD_LOCAL);
	if (!realBinderLibHandle) {
		RLOGE("Failed to load the binder lib '" REAL_LIBBINDER_NAME  "': %s\n", dlerror());
		return;
	}

	fRealWriteAlignedInt = (int (*)(int))dlsym(realBinderLibHandle, "_ZN7android6Parcel12writeAlignedIiEEiT_");
	if (!fRealWriteAlignedInt) {
		RLOGE("Failed to find the Parcel::writeAligned function");
		goto out_fail;
	}

	return;

out_fail:
	dlclose(realBinderLibHandle);
}

void libEvtUnloading(void)
{
	if (realBinderLibHandle)
		 dlclose(realBinderLibHandle);
}

// some misterious magic happens here
// and crash of rild doesn't occur if this function is wrapped
extern "C" int _ZN7android6Parcel12writeAlignedIiEEiT_(int val)
{
    ALOGE("shimmed!");

    return fRealWriteAlignedInt(val);
}
