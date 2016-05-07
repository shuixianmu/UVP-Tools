/******************************************************************************
 * evtchn.h
 * 
 * Communication via Xen event channels.
 * Also definitions for the device that demuxes notifications to userspace.
 * 
 * Copyright (c) 2004-2005, K A Fraser
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation; or, when distributed
 * separately from the Linux kernel or incorporated into other
 * software packages, subject to the following license:
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this source file (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy, modify,
 * merge, publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#ifndef __ASM_EVTCHN_H__
#define __ASM_EVTCHN_H__

#include <linux/interrupt.h>
#include <asm/ptrace.h>
#include <linux/smp.h>
/*
 * pv drivers header files
 */
#include "hypervisor.h"
#include "synch_bitops.h"
#include "event_channel.h"

/*
 * LOW-LEVEL DEFINITIONS
 */

/*
 * Dynamically bind an event source to an IRQ-like callback handler.
 * On some platforms this may not be implemented via the Linux IRQ subsystem.
 * The IRQ argument passed to the callback handler is the same as returned
 * from the bind call. It may not correspond to a Linux IRQ number.
 * Returns IRQ or negative errno.
 */
int bind_caller_port_to_irqhandler(
	unsigned int caller_port,
	irq_handler_t handler,
	unsigned long irqflags,
	const char *devname,
	void *dev_id);
int bind_listening_port_to_irqhandler(
	unsigned int remote_domain,
	irq_handler_t handler,
	unsigned long irqflags,
	const char *devname,
	void *dev_id);
int bind_interdomain_evtchn_to_irqhandler(
	unsigned int remote_domain,
	unsigned int remote_port,
	irq_handler_t handler,
	unsigned long irqflags,
	const char *devname,
	void *dev_id);
int bind_virq_to_irqhandler(
	unsigned int virq,
	unsigned int cpu,
	irq_handler_t handler,
	unsigned long irqflags,
	const char *devname,
	void *dev_id);
int bind_ipi_to_irqhandler(
	unsigned int ipi,
	unsigned int cpu,
	irq_handler_t handler,
	unsigned long irqflags,
	const char *devname,
	void *dev_id);

/*
 * Common unbind function for all event sources. Takes IRQ to unbind from.
 * Automatically closes the underlying event channel (except for bindings
 * made with bind_caller_port_to_irqhandler()).
 */
void unbind_from_irqhandler(unsigned int irq, void *dev_id);

void irq_resume(void);

/* Entry point for notifications into Linux subsystems. */
asmlinkage void evtchn_do_upcall(struct pt_regs *regs);

/* Entry point for notifications into the userland character device. */
void evtchn_device_upcall(int port);

/* Mark a PIRQ as unavailable for dynamic allocation. */
void evtchn_register_pirq(int irq);
/* Map a Xen-supplied PIRQ to a dynamically allocated one. */
int evtchn_map_pirq(int irq, int xen_pirq);
/* Look up a Xen-supplied PIRQ for a dynamically allocated one. */
int evtchn_get_xen_pirq(int irq);

void mask_evtchn(int port);
void disable_all_local_evtchn(void);
void unmask_evtchn(int port);

#ifdef CONFIG_SMP
void rebind_evtchn_to_cpu(int port, unsigned int cpu);
#else
#define rebind_evtchn_to_cpu(port, cpu)	((void)0)
#endif

static inline int test_and_set_evtchn_mask(int port)
{
	shared_info_t *s = HYPERVISOR_shared_info;
	return synch_test_and_set_bit(port, s->evtchn_mask);
}

static inline void clear_evtchn(int port)
{
	shared_info_t *s = HYPERVISOR_shared_info;
	synch_clear_bit(port, s->evtchn_pending);
}

static inline void notify_remote_via_evtchn(int port)
{
	struct evtchn_send send = { .port = port };
	VOID(HYPERVISOR_event_channel_op(EVTCHNOP_send, &send));
}

/*
 * Use these to access the event channel underlying the IRQ handle returned
 * by bind_*_to_irqhandler().
 */
void notify_remote_via_irq(int irq);
int irq_to_evtchn_port(int irq);

#endif /* __ASM_EVTCHN_H__ */
