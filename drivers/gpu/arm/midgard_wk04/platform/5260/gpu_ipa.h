
#ifndef _GPU_IPA_H_
#define _GPU_IPA_H_

#include <mali_kbase.h>
#include "mali_kbase_platform.h"

int gpu_ipa_dvfs_get_norm_utilisation(struct kbase_device *kbdev);
void gpu_ipa_dvfs_get_utilisation_stats(struct mali_debug_utilisation_stats *stats);
void gpu_ipa_dvfs_calc_norm_utilisation(struct kbase_device *kbdev);
int gpu_ipa_dvfs_max_lock(int level);
int gpu_ipa_dvfs_max_unlock(void);

#endif
