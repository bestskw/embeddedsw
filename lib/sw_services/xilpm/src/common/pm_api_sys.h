/******************************************************************************
*
* Copyright (C) 2015 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

#ifndef _PM_API_SYS_H_
#define _PM_API_SYS_H_

#include <xil_types.h>
#include <xstatus.h>
#include "pm_defs.h"
#include "pm_common.h"

enum XPmBootStatus XPm_GetBootStatus();

/* System-level API function declarations */
XStatus XPm_RequestSuspend(const enum XPmNodeId node,
			   const enum XPmRequestAck ack,
			   const u32 latency,
			   const u8 state);

XStatus XPm_SelfSuspend(const enum XPmNodeId node,
			const u32 latency,
			const u8 state,
			const u64 address);

XStatus XPm_ForcePowerDown(const enum XPmNodeId node,
			   const enum XPmRequestAck ack);

XStatus XPm_AbortSuspend(const enum XPmAbortReason reason);

XStatus XPm_RequestWakeUp(const enum XPmNodeId node,
			  const bool setAddress,
			  const u64 address,
			  const enum XPmRequestAck ack);

XStatus XPm_SetWakeUpSource(const enum XPmNodeId target,
			    const enum XPmNodeId wkup_node,
			    const u8 enable);

XStatus XPm_SystemShutdown(const u8 restart);

/* Callback API function */
/**
 * pm_init_suspend - Init suspend callback arguments (save for custom handling)
 * @received	Has init suspend callback been received/handled
 * @reason	Reason of initializing suspend
 * @latency	Maximum allowed latency
 * @timeout	Period of time the client has to response
 */
struct pm_init_suspend {
	volatile bool received;
	enum XPmSuspendReason reason;
	u32 latency;
	u32 state;
	u32 timeout;
};

/**
 * pm_acknowledge - Acknowledge callback arguments (save for custom handling)
 * @received	Has acknowledge argument been received
 * @node	Node argument about which the acknowledge is
 * @status	Acknowledged status
 * @opp		Operating point of node in question
 */
struct pm_acknowledge {
	volatile bool received;
	enum XPmNodeId node;
	XStatus status;
	u32 opp;
};

/* Forward declaration to enable self reference in struct definition */
typedef struct XPm_Notifier XPm_Notifier;

/**
 * XPm_Notifier - Notifier structure registered with a callback by app
 * @callback	Custom callback handler to be called when the notification is
 *		received. The custom handler would execute from interrupt
 *		context, it shall return quickly and must not block! (enables
 *		event-driven notifications)
 * @node	Node argument (the node to receive notifications about)
 * @event	Event argument (the event type to receive notifications about)
 * @flag	Flags
 * @oppoint	Operating point of node in question. Contains the value updated
 *		when the last event notification is received. User shall not
 *		modify this value while the notifier is registered.
 * @received	How many times the notification has been received - to be used
 *		by application (enables polling). User shall not modify this
 *		value while the notifier is registered.
 * @next	Pointer to next notifier in linked list. Must not be modified
 *		while the notifier is registered. User shall not ever modify
 *		this value.
 */
typedef struct XPm_Notifier {
	void (*const callback)(XPm_Notifier* const notifier);
	enum XPmNodeId node;
	enum XPmNotifyEvent event;
	u32 flags;
	volatile u32 oppoint;
	volatile u32 received;
	XPm_Notifier* next;
} XPm_Notifier;

/* Notifier Flags */
#define XILPM_NOTIFIER_FLAG_WAKE	BIT(0) /* wake up PU for notification */

/**
 * XPm_NodeStatus - struct containing node status information
 * @status	 Node power state
 * @requirements Current requirements asserted on the node (slaves only)
 * @usage	 Usage information (which master is currently using the slave)
 */
typedef struct XPm_NodeStatus {
	u32 status;
	u32 requirements;
	u32 usage;
} XPm_NodeStatus;

/*********************************************************************
 * Global data declarations
 ********************************************************************/
extern struct pm_init_suspend pm_susp;
extern struct pm_acknowledge pm_ack;

void XPm_InitSuspendCb(const enum XPmSuspendReason reason,
		       const u32 latency,
		       const u32 state,
		       const u32 timeout);

void XPm_AcknowledgeCb(const enum XPmNodeId node,
		       const XStatus status,
		       const u32 oppoint);

void XPm_NotifyCb(const enum XPmNodeId node,
		  const u32 event,
		  const u32 oppoint);

/* API functions for managing PM Slaves */
XStatus XPm_RequestNode(const enum XPmNodeId node,
			const u32 capabilities,
			const u32 qos,
			const enum XPmRequestAck ack);
XStatus XPm_ReleaseNode(const enum XPmNodeId node,
			const u32 latency);
XStatus XPm_SetRequirement(const enum XPmNodeId node,
			   const u32 capabilities,
			   const u32 qos,
			   const enum XPmRequestAck ack);
XStatus XPm_SetMaxLatency(const enum XPmNodeId node,
			  const u32 latency);

/* Miscellaneous API functions */
XStatus XPm_GetApiVersion(u32 *version);

XStatus XPm_GetNodeStatus(const enum XPmNodeId node,
			  XPm_NodeStatus *const nodestatus);

XStatus XPm_RegisterNotifier(XPm_Notifier* const notifier);
XStatus XPm_UnregisterNotifier(XPm_Notifier* const notifier);

XStatus XPm_GetOpCharacteristic(const enum XPmNodeId node,
				const enum XPmOpCharType type,
				u32* const result);

/* Direct-Control API functions */
XStatus XPm_ResetAssert(const enum XPmReset reset,
			const enum XPmResetAction assert);

XStatus XPm_ResetGetStatus(const enum XPmReset reset, u32 *status);

XStatus XPm_MmioWrite(const u32 address, const u32 mask, const u32 value);

XStatus XPm_MmioRead(const u32 address, u32 *const value);

#endif /* _PM_API_SYS_H_ */
