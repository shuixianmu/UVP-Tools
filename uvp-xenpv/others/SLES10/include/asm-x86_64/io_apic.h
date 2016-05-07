#ifndef __ASM_IO_APIC_H
#define __ASM_IO_APIC_H

#include <linux/config.h>
#include <asm/types.h>
#include <asm/mpspec.h>

/*
 * Intel IO-APIC support for SMP and UP systems.
 *
 * Copyright (C) 1997, 1998, 1999, 2000 Ingo Molnar
 */

#ifdef CONFIG_X86_IO_APIC

#ifdef CONFIG_PCI_MSI
static inline int use_pci_vector(void)	{return 1;}
static inline void disable_edge_ioapic_vector(unsigned int vector) { }
static inline void mask_and_ack_level_ioapic_vector(unsigned int vector) { }
static inline void end_edge_ioapic_vector (unsigned int vector) { }
#define startup_level_ioapic	startup_level_ioapic_vector
#define shutdown_level_ioapic	mask_IO_APIC_vector
#define enable_level_ioapic	unmask_IO_APIC_vector
#define disable_level_ioapic	mask_IO_APIC_vector
#define mask_and_ack_level_ioapic mask_and_ack_level_ioapic_vector
#define end_level_ioapic	end_level_ioapic_vector
#define set_ioapic_affinity	set_ioapic_affinity_vector

#define startup_edge_ioapic 	startup_edge_ioapic_vector
#define shutdown_edge_ioapic 	disable_edge_ioapic_vector
#define enable_edge_ioapic 	unmask_IO_APIC_vector
#define disable_edge_ioapic 	disable_edge_ioapic_vector
#define ack_edge_ioapic 	ack_edge_ioapic_vector
#define end_edge_ioapic 	end_edge_ioapic_vector
#else
static inline int use_pci_vector(void)	{return 0;}
static inline void disable_edge_ioapic_irq(unsigned int irq) { }
static inline void mask_and_ack_level_ioapic_irq(unsigned int irq) { }
static inline void end_edge_ioapic_irq (unsigned int irq) { }
#define startup_level_ioapic	startup_level_ioapic_irq
#define shutdown_level_ioapic	mask_IO_APIC_irq
#define enable_level_ioapic	unmask_IO_APIC_irq
#define disable_level_ioapic	mask_IO_APIC_irq
#define mask_and_ack_level_ioapic mask_and_ack_level_ioapic_irq
#define end_level_ioapic	end_level_ioapic_irq
#define set_ioapic_affinity	set_ioapic_affinity_irq

#define startup_edge_ioapic 	startup_edge_ioapic_irq
#define shutdown_edge_ioapic 	disable_edge_ioapic_irq
#define enable_edge_ioapic 	unmask_IO_APIC_irq
#define disable_edge_ioapic 	disable_edge_ioapic_irq
#define ack_edge_ioapic 	ack_edge_ioapic_irq
#define end_edge_ioapic 	end_edge_ioapic_irq
#endif

#define APIC_MISMATCH_DEBUG

#ifndef CONFIG_XEN
#define IO_APIC_BASE(idx) \
		((volatile int *)(__fix_to_virt(FIX_IO_APIC_BASE_0 + idx) \
		+ (mp_ioapics[idx].mpc_apicaddr & ~PAGE_MASK)))
#else
#include <xen/interface/xen.h>
#include <xen/interface/physdev.h>
#endif

/*
 * The structure of the IO-APIC:
 */
union IO_APIC_reg_00 {
	u32	raw;
	struct {
		u32	__reserved_2	: 14,
			LTS		:  1,
			delivery_type	:  1,
			__reserved_1	:  8,
			ID		:  8;
	} __attribute__ ((packed)) bits;
};

union IO_APIC_reg_01 {
	u32	raw;
	struct {
		u32	version		:  8,
		__reserved_2	:  7,
		PRQ		:  1,
		entries		:  8,
		__reserved_1	:  8;
	} __attribute__ ((packed)) bits;
};

union IO_APIC_reg_02 {
	u32	raw;
	struct {
		u32	__reserved_2	: 24,
		arbitration	:  4,
		__reserved_1	:  4;
	} __attribute__ ((packed)) bits;
};

union IO_APIC_reg_03 {
	u32	raw;
	struct {
		u32	boot_DT		:  1,
			__reserved_1	: 31;
	} __attribute__ ((packed)) bits;
};

/*
 * # of IO-APICs and # of IRQ routing registers
 */
extern int nr_ioapics;
extern int nr_ioapic_registers[MAX_IO_APICS];

enum ioapic_irq_destination_types {
	dest_Fixed = 0,
	dest_LowestPrio = 1,
	dest_SMI = 2,
	dest__reserved_1 = 3,
	dest_NMI = 4,
	dest_INIT = 5,
	dest__reserved_2 = 6,
	dest_ExtINT = 7
};

struct IO_APIC_route_entry {
        __u32   vector          :  8,
                delivery_mode   :  3,   /* 000: FIXED
                                         * 001: lowest prio
                                         * 111: ExtINT
                                         */
                dest_mode       :  1,   /* 0: physical, 1: logical */
                delivery_status :  1,
                polarity        :  1,
                irr             :  1,
                trigger         :  1,   /* 0: edge, 1: level */
                mask            :  1,   /* 0: enabled, 1: disabled */
                __reserved_2    : 15;

        __u32   __reserved_3    : 24,
                dest          :  8;
} __attribute__ ((packed));

/*
 * MP-BIOS irq configuration table structures:
 */

/* I/O APIC entries */
extern struct mpc_config_ioapic mp_ioapics[MAX_IO_APICS];

/* # of MP IRQ source entries */
extern int mp_irq_entries;

/* MP IRQ source entries */
extern struct mpc_config_intsrc mp_irqs[MAX_IRQ_SOURCES];

/* non-0 if default (table-less) MP configuration */
extern int mpc_default_type;

static inline unsigned int io_apic_read(unsigned int apic, unsigned int reg)
{
#ifndef CONFIG_XEN
	*IO_APIC_BASE(apic) = reg;
	return *(IO_APIC_BASE(apic)+4);
#else
	struct physdev_apic apic_op;
	int ret;

	apic_op.apic_physbase = mp_ioapics[apic].mpc_apicaddr;
	apic_op.reg = reg;
	ret = HYPERVISOR_physdev_op(PHYSDEVOP_apic_read, &apic_op);
	if (ret)
		return ret;
	return apic_op.value;
#endif
}

static inline void io_apic_write(unsigned int apic, unsigned int reg, unsigned int value)
{
#ifndef CONFIG_XEN
	*IO_APIC_BASE(apic) = reg;
	*(IO_APIC_BASE(apic)+4) = value;
#else
	struct physdev_apic apic_op;

	apic_op.apic_physbase = mp_ioapics[apic].mpc_apicaddr;
	apic_op.reg = reg;
	apic_op.value = value;
	WARN_ON(HYPERVISOR_physdev_op(PHYSDEVOP_apic_write, &apic_op));
#endif
}

#ifndef CONFIG_XEN

/*
 * Re-write a value: to be used for read-modify-write
 * cycles where the read already set up the index register.
 */
static inline void io_apic_modify(unsigned int apic, unsigned int value)
{
	*(IO_APIC_BASE(apic)+4) = value;
}

/*
 * Synchronize the IO-APIC and the CPU by doing
 * a dummy read from the IO-APIC
 */
static inline void io_apic_sync(unsigned int apic)
{
	(void) *(IO_APIC_BASE(apic)+4);
}

#else
#define io_apic_modify io_apic_write
#endif

/* 1 if "noapic" boot option passed */
extern int skip_ioapic_setup;

/*
 * If we use the IO-APIC for IRQ routing, disable automatic
 * assignment of PCI IRQ's.
 */
#define io_apic_assign_pci_irqs (mp_irq_entries && !skip_ioapic_setup && io_apic_irqs)

#ifdef CONFIG_ACPI
extern int io_apic_get_version (int ioapic);
extern int io_apic_get_redir_entries (int ioapic);
extern int io_apic_set_pci_routing (int ioapic, int pin, int irq, int, int);
#endif

extern int sis_apic_bug; /* dummy */ 

#else  /* !CONFIG_X86_IO_APIC */
#define io_apic_assign_pci_irqs 0
#endif

extern int assign_irq_vector(int irq);

void enable_NMI_through_LVT0 (void * dummy);

extern spinlock_t i8259A_lock;

#endif
