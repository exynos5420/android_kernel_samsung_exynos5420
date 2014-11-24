/*
 * linux/arch/arm/mach-exynos/include/mach/debug-bL.h
 *
 * Copyright (c) 2013 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com/
 *
 * EXYNOS - Debug architecture support
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#ifndef __ASM_ARCH_DEBUG_BL_H
#define __ASM_ARCH_DEBUG_BL_H __FILE__

void print_bL_state(char buf[], const unsigned int max_len);
void print_bL_current_core(char buf[], const unsigned int max_len);

#endif /* __ASM_ARCH_DEBUG_BL_H */
