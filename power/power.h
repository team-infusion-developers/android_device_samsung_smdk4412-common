/*
 * Copyright (C) 2016 The CyanogenMod Project
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
#define GOVERNOR_PATH "/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor"

#define GOV_PERFORMANCE      "performance"
#define GOV_ONDEMAND         "ondemand"
#define GOV_PEGASUSQ         "pegasusq"
#define GOV_DYNAMIC          "dynamic"
#define GOV_POWERSAVE        "conservative"

#define MS_TO_NS (1000000L)

// Custom Lineage hints
const static power_hint_t POWER_HINT_CPU_BOOST   = (power_hint_t)0x00000110;
const static power_hint_t POWER_HINT_SET_PROFILE = (power_hint_t)0x00000111;

enum {
    PROFILE_POWER_SAVE = 0,
    PROFILE_BALANCED,
    PROFILE_PERFORMANCE,
    PROFILE_MAX
};

typedef struct governor_settings {
    // freq values for core up/down
    int hotplug_freq_1_1;
    int hotplug_freq_2_0;
    // rq sizes for up/down
    int hotplug_rq_1_1;
    int hotplug_rq_2_0;
    // max/min freqs (-1 for default)
    int max_freq;
    int min_freq;
    // load at which to start scaling up
    int up_threshold;
    // load to jump freq immediately to freq_for_responsiveness
    int up_threshold_at_min_freq;
    // freq when the load of up_threshold_at_min_freq is reached
    int freq_for_responsiveness;
    // percentage of max freq for scaling up or down (1200 * (freq_step/100))
    int freq_step;
    // higher down_differential == slower downscaling
    int down_differential;
    // min/max num of cpus to have online
    int min_cpu_lock;
    int max_cpu_lock;
    // wait sampling_rate * cpu_up_rate us before trying to upscale
    int cpu_up_rate;
    // wait sampling_rate * cpu_down_rate us before trying to downscale
    int cpu_down_rate;
    int io_is_busy;
    int sampling_rate; // in microseconds
    int sampling_down_factor;
    // Active mode
    int power_optimal_freq;
    int sampling_down_factor_relax_khz;
    int standby_delay_factor;
    int standby_threshold_freq;
    int max_non_oc_freq;
    int oc_freq_boost_ms;
    // Standby mode
    int standby_sampling_rate;
    int standby_sampling_up_factor;
    // Suspend mode
    int suspend_sampling_rate;
    int suspend_sampling_up_factor;
    int suspend_max_freq;
    // boosting
    int boost_freq;
    int boost_mincpus;
    long interaction_boost_time;
    long launch_boost_time;
} power_profile;

static power_profile profiles[PROFILE_MAX] = {
    [PROFILE_POWER_SAVE] = {
        .hotplug_freq_1_1 = 400000,
        .hotplug_freq_2_0 = 300000,
        .hotplug_rq_1_1 = 300,
        .hotplug_rq_2_0 = 250,
        .min_freq = 100000,
        .max_freq = 800000,
        .up_threshold = 80,
        .up_threshold_at_min_freq = 50,
        .freq_for_responsiveness = 200000,
        .freq_step = 18,
        .down_differential = 5,
        .min_cpu_lock = 2,
        .max_cpu_lock = 0,
        .cpu_up_rate = 1,
        .cpu_down_rate = 1,
        .sampling_rate = 20000,
        .io_is_busy = 15,
        .power_optimal_freq = 700000,
        .sampling_down_factor = 2,
        .sampling_down_factor_relax_khz = 500000,
        .standby_delay_factor = 1,
        .standby_threshold_freq = 200000,
        .max_non_oc_freq = 800000,
        .oc_freq_boost_ms = 2000,
        .standby_sampling_rate = 20000,
        .standby_sampling_up_factor = 4,
        .suspend_sampling_rate = 60000,
        .suspend_sampling_up_factor = 6,
        .suspend_max_freq = 800000,

        .boost_freq = 400000,
        .boost_mincpus = 0,
        .interaction_boost_time = 120 * (MS_TO_NS),
        .launch_boost_time = 2000 * (MS_TO_NS),
    },
    [PROFILE_BALANCED] = {
        .hotplug_freq_1_1 = 400000,
        .hotplug_freq_2_0 = 300000,
        .hotplug_rq_1_1 = 300,
        .hotplug_rq_2_0 = 250,
        .min_freq = -1,
        .max_freq = -1,
        .up_threshold = 80,
        .up_threshold_at_min_freq = 50,
        .freq_for_responsiveness = 200000,
        .freq_step = 18,
        .down_differential = 5,
        .min_cpu_lock = 2,
        .max_cpu_lock = 0,
        .cpu_up_rate = 1,
        .cpu_down_rate = 1,
        .sampling_rate = 20000,
        .io_is_busy = 15,
        .power_optimal_freq = 700000,
        .sampling_down_factor = 2,
        .sampling_down_factor_relax_khz = 500000,
        .standby_delay_factor = 1,
        .standby_threshold_freq = 200000,
        .max_non_oc_freq = 800000,
        .oc_freq_boost_ms = 2000,
        .standby_sampling_rate = 20000,
        .standby_sampling_up_factor = 4,
        .suspend_sampling_rate = 60000,
        .suspend_sampling_up_factor = 6,
        .suspend_max_freq = 800000,

        .boost_freq = 800000,
        .boost_mincpus = 0,
        .interaction_boost_time = 120 * (MS_TO_NS),
        .launch_boost_time = 2000 * (MS_TO_NS),
    },
    [PROFILE_PERFORMANCE] = {
        .hotplug_freq_1_1 = 400000,
        .hotplug_freq_2_0 = 300000,
        .hotplug_rq_1_1 = 300,
        .hotplug_rq_2_0 = 250,
        .min_freq = -1,
        .max_freq = -1,
        .freq_step = 37,
        .up_threshold = 90,
        .up_threshold_at_min_freq = 40,
        .freq_for_responsiveness = 800000,
        .down_differential = 5,
        .min_cpu_lock = 2,
        .max_cpu_lock = 0,
        .cpu_up_rate = 1,
        .cpu_down_rate = 1,
        .sampling_rate = 40000,
        .io_is_busy = 1,
        .power_optimal_freq = 0,
        .sampling_down_factor = 3,
        .sampling_down_factor_relax_khz = 0,
        .standby_delay_factor = 3,
        .standby_threshold_freq = 200000,
        .max_non_oc_freq = 0,
        .oc_freq_boost_ms = 0,
        .standby_sampling_rate = 40000,
        .standby_sampling_up_factor = 3,
        .suspend_sampling_rate = 60000,
        .suspend_sampling_up_factor = 5,
        .suspend_max_freq = 800000,

        .boost_freq = 800000,
        .boost_mincpus = 2,
        .interaction_boost_time = 180 * (MS_TO_NS),
        .launch_boost_time = 2000 * (MS_TO_NS),
    },
};

// for non-interactive profiles we don't need to worry about
// boosting as it (should) only occur while the screen is on
static power_profile profiles_low_power[PROFILE_MAX] = {
    [PROFILE_POWER_SAVE] = {
        .hotplug_freq_1_1 = 400000,
        .hotplug_freq_2_0 = 300000,
        .hotplug_rq_1_1 = 300,
        .hotplug_rq_2_0 = 250,
        .max_freq = 400000,
        .min_freq = -1,
        .up_threshold = 95,
        .up_threshold_at_min_freq = 40,
        .freq_for_responsiveness = 100000,
        .down_differential = 5,
        .min_cpu_lock = 0,
        .max_cpu_lock = 2,
        .cpu_up_rate = 1,
        .cpu_down_rate = 1,
        .sampling_rate = 20000,
        .io_is_busy = 0,
        .boost_freq = 0,
        .boost_mincpus = 0,
        .interaction_boost_time = 0,
        .launch_boost_time = 0,
    },
    [PROFILE_BALANCED] = {
        .hotplug_freq_1_1 = 400000,
        .hotplug_freq_2_0 = 300000,
        .hotplug_rq_1_1 = 300,
        .hotplug_rq_2_0 = 250,
        .min_freq = 100000,
        .max_freq = -1,
        .up_threshold = 95,
        .up_threshold_at_min_freq = 50,
        .freq_for_responsiveness = 200000,
        .down_differential = 5,
        .min_cpu_lock = 0,
        .max_cpu_lock = 0,
        .cpu_up_rate = 1,
        .cpu_down_rate = 1,
        .sampling_rate = 20000,
        .io_is_busy = 1,
        .boost_freq = 1000000,
        .boost_mincpus = 0,
        .interaction_boost_time = 0,
        .launch_boost_time = 0,
    },
    [PROFILE_PERFORMANCE] = {
        .hotplug_freq_1_1 = 400000,
        .hotplug_freq_2_0 = 300000,
        .hotplug_rq_1_1 = 300,
        .hotplug_rq_2_0 = 250,
        .min_freq = 1400000,
        .max_freq = -1,
        .up_threshold = 95,
        .up_threshold_at_min_freq = 40,
        .freq_for_responsiveness = 800000,
        .down_differential = 5,
        .min_cpu_lock = 2,
        .max_cpu_lock = 0,
        .cpu_up_rate = 1,
        .cpu_down_rate = 1,
        .sampling_rate = 20000,
        .io_is_busy = 1,
        .boost_freq = 0,
        .boost_mincpus = 2,
        .interaction_boost_time = 0,
        .launch_boost_time = 0,
    },
};


