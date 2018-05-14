#ifndef __ASM_ARM_CPUTYPE_H
#define __ASM_ARM_CPUTYPE_H

#include <linux/stringify.h>
#include <linux/kernel.h>

#define CPUID_ID	0
#define CPUID_CACHETYPE	1
#define CPUID_TCM	2
#define CPUID_TLBTYPE	3
#define CPUID_MPIDR	5

#define CPUID_EXT_PFR0	"c1, 0"
#define CPUID_EXT_PFR1	"c1, 1"
#define CPUID_EXT_DFR0	"c1, 2"
#define CPUID_EXT_AFR0	"c1, 3"
#define CPUID_EXT_MMFR0	"c1, 4"
#define CPUID_EXT_MMFR1	"c1, 5"
#define CPUID_EXT_MMFR2	"c1, 6"
#define CPUID_EXT_MMFR3	"c1, 7"
#define CPUID_EXT_ISAR0	"c2, 0"
#define CPUID_EXT_ISAR1	"c2, 1"
#define CPUID_EXT_ISAR2	"c2, 2"
#define CPUID_EXT_ISAR3	"c2, 3"
#define CPUID_EXT_ISAR4	"c2, 4"
#define CPUID_EXT_ISAR5	"c2, 5"

/* ARM implemented processors */
#define ARM_CPU_PART_ARM1136		0x4100b360
#define ARM_CPU_PART_ARM1156		0x4100b560
#define ARM_CPU_PART_ARM1176		0x4100b760
#define ARM_CPU_PART_ARM11MPCORE	0x4100b020
#define ARM_CPU_PART_CORTEX_A8		0x4100c080
#define ARM_CPU_PART_CORTEX_A9		0x4100c090
#define ARM_CPU_PART_CORTEX_A5		0x4100c050
#define ARM_CPU_PART_CORTEX_A7		0x4100c070
#define ARM_CPU_PART_CORTEX_A12		0x4100c0d0
#define ARM_CPU_PART_CORTEX_A17		0x4100c0e0
#define ARM_CPU_PART_CORTEX_A15		0x4100c0f0
#define ARM_CPU_PART_CORTEX_A53		0x4100d030
#define ARM_CPU_PART_CORTEX_A57		0x4100d070
#define ARM_CPU_PART_CORTEX_A72		0x4100d080
#define ARM_CPU_PART_CORTEX_A73		0x4100d090
#define ARM_CPU_PART_CORTEX_A75		0x4100d0a0
#define ARM_CPU_PART_MASK		0xff00fff0

/* Broadcom implemented processors */
#define ARM_CPU_PART_BRAHMA_B15		0x420000f0
#define ARM_CPU_PART_BRAHMA_B53		0x42001000

extern unsigned int processor_id;

#ifdef CONFIG_CPU_CP15
#define read_cpuid(reg)							\
	({								\
		unsigned int __val;					\
		asm("mrc	p15, 0, %0, c0, c0, " __stringify(reg)	\
		    : "=r" (__val)					\
		    :							\
		    : "cc");						\
		__val;							\
	})
#define read_cpuid_ext(ext_reg)						\
	({								\
		unsigned int __val;					\
		asm("mrc	p15, 0, %0, c0, " ext_reg		\
		    : "=r" (__val)					\
		    :							\
		    : "cc");						\
		__val;							\
	})
#else
#define read_cpuid(reg) (processor_id)
#define read_cpuid_ext(reg) 0
#endif

/*
 * The CPU ID never changes at run time, so we might as well tell the
 * compiler that it's constant.  Use this function to read the CPU ID
 * rather than directly reading processor_id or read_cpuid() directly.
 */
static inline unsigned int __attribute_const__ read_cpuid_id(void)
{
	return read_cpuid(CPUID_ID);
}

static inline unsigned int __attribute_const__ read_cpuid_cachetype(void)
{
	return read_cpuid(CPUID_CACHETYPE);
}

static inline unsigned int __attribute_const__ read_cpuid_tcmstatus(void)
{
	return read_cpuid(CPUID_TCM);
}

static inline unsigned int __attribute_const__ read_cpuid_mpidr(void)
{
	return read_cpuid(CPUID_MPIDR);
}

/*
 * The CPU part number is meaningless without referring to the CPU
 * implementer: implementers are free to define their own part numbers
 * which are permitted to clash with other implementer part numbers.
 */
static inline unsigned int __attribute_const__ read_cpuid_part(void)
{
	return read_cpuid_id() & ARM_CPU_PART_MASK;
}

/*
 * Intel's XScale3 core supports some v6 features (supersections, L2)
 * but advertises itself as v5 as it does not support the v6 ISA.  For
 * this reason, we need a way to explicitly test for this type of CPU.
 */
#ifndef CONFIG_CPU_XSC3
#define cpu_is_xsc3()	0
#else
static inline int cpu_is_xsc3(void)
{
	unsigned int id;
	id = read_cpuid_id() & 0xffffe000;
	/* It covers both Intel ID and Marvell ID */
	if ((id == 0x69056000) || (id == 0x56056000))
		return 1;

	return 0;
}
#endif

#if !defined(CONFIG_CPU_XSCALE) && !defined(CONFIG_CPU_XSC3)
#define	cpu_is_xscale()	0
#else
#define	cpu_is_xscale()	1
#endif

#endif
