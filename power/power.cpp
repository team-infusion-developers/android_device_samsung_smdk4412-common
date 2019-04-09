#pragma clang diagnostic ignored "-Wwritable-strings"
/*
 * Copyright (C) 2012 The Android Open Source Project
 * Copyright (C) 2014 The CyanogenMod Project
 * Copyright (C) 2014-2015 Andreas Schneider <asn@cryptomilk.org>
 * Copyright (C) 2014-2015 Christopher N. Hesse <raymanfx@gmail.com>
 * Copyright (C) 2017 Dominggoes Isakh <drjisakh@gmail.com>
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
#define LOG_TAG "PegasusPowerHAL"

#include <hardware/hardware.h>
#include <hardware/power.h>

#include <stdbool.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>

#include <pthread.h>
#include <cutils/properties.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

// #define LOG_NDEBUG 0
#define DEBUG 1
#include <log/log.h>

#include "power.h"

#define PEGASUSQ_PATH "/sys/devices/system/cpu/cpufreq/pegasusq/"
#define DYNAMIC_PATH "/sys/devices/system/cpu/cpufreq/dynamic/"
#define MINMAX_CPU_PATH "/sys/power/"

#define US_TO_NS (1000L)

static int debug_level = 0;

static int current_power_profile = -1;
static bool is_low_power = false;
static bool is_vsync_active = false;

static int current_cpufreq_limit = -1;

static pthread_mutex_t boost_mutex;
static pthread_t end_boost_pthread;

static bool is_thread_running = false;

/*static*/ void update_cpufreq_max_limit() {
    current_cpufreq_limit = property_get_int32("sys.power.cpufreq_max_limit", -1);
}

/*static*/ void end_boost();

/*static*/ void* end_boost_thread(void *data) {
    long time = reinterpret_cast<long>(data);
    is_thread_running = true;

    ALOGD_IF(debug_level > 2, "%s: time=%ld", __func__, time);

    usleep(time);
    end_boost();
    is_thread_running = false;
    return NULL;
}

/*static*/ void do_end_boost(long time) {
    pthread_attr_t thread_attr;
    ALOGD_IF(debug_level > 2, "%s: time=%ld", __func__, time);

    if (is_thread_running)
        return;

    //pthread_mutex_lock(&boost_mutex);
    /*if (is_thread_running) {
        pthread_kill(end_boost_pthread, 0);
        is_thread_running = false;
    }*/

    pthread_attr_init(&thread_attr);
    pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED);
    int rc = pthread_create(&end_boost_pthread, &thread_attr,
                    end_boost_thread, reinterpret_cast<void*>(time));
    //pthread_mutex_unlock(&boost_mutex);

    if (rc < 0)
        ALOGE_IF(debug_level > 2, "%s: Unable to create thread", __func__);
}

static int sysfs_write_str(char *path, char *s) {
    char buf[80];
    int len;
    int ret = 0;
    int fd;

    fd = open(path, O_WRONLY);
    if (fd < 0) {
        strerror_r(errno, buf, sizeof(buf));
        ALOGD_IF(debug_level > 0, "Error opening %s: %s\n", path, buf);
        return -1 ;
    }

    len = write(fd, s, strlen(s));
    if (len < 0) {
        strerror_r(errno, buf, sizeof(buf));
        ALOGD_IF(debug_level > 0, "Error writing to %s: %s\n", path, buf);
        ret = -1;
    }

    close(fd);

    return ret;
}

static int sysfs_write_int(char *path, int value) {
    char buf[80];
    snprintf(buf, 80, "%d", value);
    return sysfs_write_str(path, buf);
}
/*
static int sysfs_write_long(char *path, long value) {
    char buf[80];
    snprintf(buf, 80, "%ld", value);
    return sysfs_write_str(path, buf);
}*/

#define WRITE_PEGASUSQ_PARAM(profile, param) do { \
    ALOGD_IF(debug_level > 0, "%s: WRITE_PEGASUSQ_PARAM(profile=%d, param=%s): new val => %d", __func__, profile, #param, profiles[profile].param); \
    sysfs_write_int(PEGASUSQ_PATH #param, profiles[profile].param); \
} while (0)
#define WRITE_DYNAMIC_PARAM(profile, param) do { \
    ALOGV("%s: WRITE_DYNAMIC_PARAM(profile=%d, param=%s): new val => %d", __func__, profile, #param, profiles[profile].param); \
    sysfs_write_int(DYNAMIC_PATH #param, profiles[profile].param); \
} while (0)
#define WRITE_LOW_POWER_PARAM(profile, param) do { \
    ALOGD_IF(debug_level > 0, "%s: WRITE_LOW_POWER_PARAM(profile=%d, param=%s): new val => %d", \
            __func__, profile, #param, profiles_low_power[profile].param); \
    sysfs_write_int(PEGASUSQ_PATH #param, profiles_low_power[profile].param); \
} while (0)
#define WRITE_PEGASUSQ_VALUE(param, value) do { \
    ALOGD_IF(debug_level > 0, "%s: WRITE_PEGASUSQ_VALUE(param=%s, value=%d)", __func__, #param, value); \
    sysfs_write_int(PEGASUSQ_PATH #param, value); \
} while (0)
#define WRITE_DYNAMIC_VALUE(param, value) do { \
    ALOGV("%s: WRITE_DYNAMIC_VALUE(param=%s, value=%d)", __func__, #param, value); \
    sysfs_write_int(DYNAMIC_PATH #param, value); \
} while (0)
#define WRITE_MINMAX_CPU(param, value) do { \
    ALOGD_IF(debug_level > 0, "%s: WRITE_MINMAX_CPU(param=%s, value=%d)", __func__, #param, value); \
    sysfs_write_int(MINMAX_CPU_PATH #param, value); \
} while(0)

static bool check_governor_dynamic() {
    struct stat s;
    int err = stat(DYNAMIC_PATH, &s);
    if (err != 0) return false;
    return S_ISDIR(s.st_mode);
}

static bool is_profile_valid(int profile) {
    return profile >= 0 && profile < PROFILE_MAX;
}

static void set_power_profile(int profile) {
    ALOGE("%s: %d", __func__, profile);

    if (!is_profile_valid(profile)) {
        ALOGD_IF(debug_level > 0, "%s: unknown profile: %d", __func__, profile);
        return;
    }

    if (profile == current_power_profile) return;

    //WRITE_MINMAX_CPU(cpufreq_max_limit, profiles[profile].max_freq);
    //WRITE_MINMAX_CPU(cpufreq_min_limit, profiles[profile].min_freq);

    if (check_governor_dynamic()) {
        //WRITE_DYNAMIC_PARAM(profile, hotplug_freq_1_1);
        //WRITE_DYNAMIC_PARAM(profile, hotplug_freq_2_0);
        //WRITE_DYNAMIC_PARAM(profile, hotplug_rq_1_1);
        //WRITE_DYNAMIC_PARAM(profile, hotplug_rq_2_0);
        WRITE_DYNAMIC_PARAM(profile, up_threshold);
        WRITE_DYNAMIC_PARAM(profile, down_differential);
        update_cpufreq_max_limit();
        //WRITE_DYNAMIC_PARAM(profile, min_cpu_lock);
        //WRITE_DYNAMIC_PARAM(profile, max_cpu_lock);
        WRITE_DYNAMIC_PARAM(profile, cpu_up_rate);
        WRITE_DYNAMIC_PARAM(profile, cpu_down_rate);
        WRITE_DYNAMIC_PARAM(profile, sampling_rate);
        WRITE_DYNAMIC_PARAM(profile, io_is_busy);

        WRITE_DYNAMIC_PARAM(profile, power_optimal_freq);
        WRITE_DYNAMIC_PARAM(profile, sampling_down_factor);
        WRITE_DYNAMIC_PARAM(profile, sampling_down_factor_relax_khz);
        WRITE_DYNAMIC_PARAM(profile, standby_delay_factor);
        WRITE_DYNAMIC_PARAM(profile, standby_threshold_freq);
        WRITE_DYNAMIC_PARAM(profile, max_non_oc_freq);
        WRITE_DYNAMIC_PARAM(profile, oc_freq_boost_ms);
        WRITE_DYNAMIC_PARAM(profile, standby_sampling_rate);
        WRITE_DYNAMIC_PARAM(profile, standby_sampling_up_factor);
        WRITE_DYNAMIC_PARAM(profile, suspend_sampling_rate);
        WRITE_DYNAMIC_PARAM(profile, suspend_sampling_up_factor);
        WRITE_DYNAMIC_PARAM(profile, suspend_max_freq);
    }

    current_power_profile = profile;

    ALOGD_IF(debug_level > 2, "%s: %d", __func__, profile);
}

// nanoseconds to microseconds
#define ns2us(ns) ns / 1000

static void boost(long boost_time, bool boost_minfreq) {
    (void)boost_minfreq;

    if (boost_minfreq)
        WRITE_MINMAX_CPU(cpufreq_min_limit, profiles[current_power_profile].boost_freq);

    WRITE_MINMAX_CPU(cpufreq_max_limit, profiles[PROFILE_PERFORMANCE].max_freq);
    do_end_boost(ns2us(boost_time));
    //do_end_boost(5000000); // 5 sec
}

/*static*/ void end_boost() {
    update_cpufreq_max_limit();
    ALOGD_IF(debug_level > 2, "%s: current_cpufreq_limit=%d", __func__, current_cpufreq_limit);
    WRITE_MINMAX_CPU(cpufreq_min_limit, 200000);
    WRITE_MINMAX_CPU(cpufreq_max_limit, current_cpufreq_limit);
}

static void set_power(bool low_power) {
    if (!is_profile_valid(current_power_profile)) {
        ALOGD_IF(debug_level > 2, "%s: current_power_profile not set yet", __func__);
        return;
    }

    if (is_low_power == low_power) return;

    if (low_power) {
        end_boost();
        set_power_profile(PROFILE_POWER_SAVE);
        is_low_power = true;
    } else {
        set_power_profile(PROFILE_BALANCED);
        is_low_power = false;
    }
}

static __attribute__((constructor)) void ctr();
static __attribute__((destructor)) void dtr();

static void ctr()
{
    pthread_mutex_init(&boost_mutex, NULL);
}

static void dtr()
{
    pthread_mutex_destroy(&boost_mutex);
}

extern "C" {

/*
 * (*init)() performs power management setup actions at runtime
 * startup, such as to set default cpufreq parameters.  This is
 * called only by the Power HAL instance loaded by
 * PowerManagerService.
 */
void power_init(void) {
    set_power_profile(PROFILE_BALANCED);
    ALOGD_IF(debug_level > 2, "%s", __func__);
}

/*
 * The setInteractive function performs power management actions upon the
 * system entering interactive state (that is, the system is awake and ready
 * for interaction, often with UI devices such as display and touchscreen
 * enabled) or non-interactive state (the system appears asleep, display
 * usually turned off).  The non-interactive state is usually entered after a
 * period of inactivity, in order to conserve battery power during such
 * inactive periods.
 *
 * Typical actions are to turn on or off devices and adjust cpufreq parameters.
 * This function may also call the appropriate interfaces to allow the kernel
 * to suspend the system to low-power sleep state when entering non-interactive
 * state, and to disallow low-power suspend when the system is in interactive
 * state.  When low-power suspend state is allowed, the kernel may suspend the
 * system whenever no wakelocks are held.
 *
 * on is non-zero when the system is transitioning to an interactive / awake
 * state, and zero when transitioning to a non-interactive / asleep state.
 *
 * This function is called to enter non-interactive state after turning off the
 * screen (if present), and called to enter interactive state prior to turning
 * on the screen.
 */
void power_set_interactive(int on) {
    if (!is_profile_valid(current_power_profile)) {
        ALOGD_IF(debug_level > 0, "%s: no power profile selected", __func__);
        return;
    }

    ALOGD_IF(debug_level > 2, "%s: setting interactive => %d", __func__, on);
    set_power(!on);
}

/*
 * The powerHint function is called to pass hints on power requirements, which
 * may result in adjustment of power/performance parameters of the cpufreq
 * governor and other controls.
 *
 * The possible hints are:
 *
 * POWER_HINT_VSYNC
 *
 *     Foreground app has started or stopped requesting a VSYNC pulse
 *     from SurfaceFlinger.  If the app has started requesting VSYNC
 *     then CPU and GPU load is expected soon, and it may be appropriate
 *     to raise speeds of CPU, memory bus, etc.  The data parameter is
 *     non-zero to indicate VSYNC pulse is now requested, or zero for
 *     VSYNC pulse no longer requested.
 *
 * POWER_HINT_INTERACTION
 *
 *     User is interacting with the device, for example, touchscreen
 *     events are incoming.  CPU and GPU load may be expected soon,
 *     and it may be appropriate to raise speeds of CPU, memory bus,
 *     etc.  The data parameter is unused.
 *
 * POWER_HINT_LOW_POWER
 *
 *     Low power mode is activated or deactivated. Low power mode
 *     is intended to save battery at the cost of performance. The data
 *     parameter is non-zero when low power mode is activated, and zero
 *     when deactivated.
 *
 * POWER_HINT_CPU_BOOST
 *
 *     An operation is happening where it would be ideal for the CPU to
 *     be boosted for a specific duration. The data parameter is an
 *     integer value of the boost duration in nanoseconds.
 */
void power_hint(power_hint_t hint, void *data) {
    int32_t val;

    if (hint == POWER_HINT_SET_PROFILE) {
        ALOGD_IF(debug_level > 2, "%s: set profile %d", __func__, *(int32_t *)data);
        if (is_vsync_active) {
            is_vsync_active = false;
            end_boost();
        }
        set_power_profile(*(int32_t *)data);

    }

    if (current_power_profile == PROFILE_POWER_SAVE) return;

    switch (hint) {
        case POWER_HINT_INTERACTION:
            if (data) {
                ALOGD_IF(debug_level > 2, "%s: interaction", __func__);
                val = *(int32_t *)data;
                if (val > 0) {
                    boost(val * US_TO_NS, false);
                } else {
                    boost(profiles[current_power_profile].interaction_boost_time, false);
                }
            }
            break;
        case POWER_HINT_LAUNCH:
            ALOGD_IF(debug_level > 2, "%s: launch", __func__);
            boost(profiles[current_power_profile].launch_boost_time, true);
            break;
        case POWER_HINT_CPU_BOOST:
            ALOGD_IF(debug_level > 2, "%s: cpu_boost", __func__);
            boost((*(int32_t *)data) * US_TO_NS, false);
            break;
        default:
            break;
    }
}

int get_number_of_profiles()
{
    return PROFILE_MAX;
}

} // extern "C"
