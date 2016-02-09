/*******************************************************************************
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
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name of the Xilinx shall not be used
 * in advertising or otherwise to promote the sale, use or other dealings in
 * this Software without prior written authorization from Xilinx.
 *
*******************************************************************************/
/******************************************************************************/
/**
 *
 * @file xvprocss_log.c
 *
 * Contains Event Logging routines for xvprocss.
 *
 * @note	None.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.0   dmc  01/11/16 Initial release.
 * </pre>
 *
*******************************************************************************/

/******************************* Include Files ********************************/

#include "xvprocss.h"

/**************************** Function Prototypes *****************************/

/**************************** Function Definitions ****************************/

/*****************************************************************************/
/**
* This function will reset the driver's logging mechanism.
*
* @param	InstancePtr is a pointer to the XVprocSs core instance.
*
* @return	None.
*
* @note		HeadIndex == TailIndex means that the log is empty.
*
******************************************************************************/
void XVprocSs_LogReset(XVprocSs *InstancePtr)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);

	InstancePtr->Log.HeadIndex = 0;
	InstancePtr->Log.TailIndex = 0;
}

/*****************************************************************************/
/**
* This function will insert an event in the driver's logging mechanism.
*
* @param	InstancePtr is a pointer to the XVprocSs core instance.
* @param	Evt is the event type to log.
* @param	Data is the associated data for the event.
*
* @return	None.
*
* @note		The DataBuffer is circular.  If full, new events will overwrite
*           oldest events.
*
******************************************************************************/
void XVprocSs_LogWrite(XVprocSs *InstancePtr, XVprocSs_LogEvent Evt, u8 Data)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Evt < (XVPROCSS_EVT_LAST_ENUM));
	Xil_AssertVoid(Data < 0xFF);

	/* Write data and event into log buffer */
	InstancePtr->Log.DataBuffer[InstancePtr->Log.HeadIndex] =
			((u16)Data << 8) | Evt;

	/* Update head pointer if reached to end of the buffer */
	if (InstancePtr->Log.HeadIndex == (XVPROCSS_EVT_BUFFSIZE-1)) {
		/* Clear pointer */
		InstancePtr->Log.HeadIndex = 0;
	}
	else {
		/* Increment pointer */
		InstancePtr->Log.HeadIndex++;
	}

	/* Check tail pointer. When the two pointer are equal, then the buffer
	 * is full. In this case then increment the tail pointer as well to
	 * remove the oldest entry from the buffer. */
	if (InstancePtr->Log.TailIndex == InstancePtr->Log.HeadIndex) {
		if (InstancePtr->Log.TailIndex == (XVPROCSS_EVT_BUFFSIZE-1)) {
			InstancePtr->Log.TailIndex = 0;
		}
		else {
			InstancePtr->Log.TailIndex++;
		}
	}
}

/*****************************************************************************/
/**
* This function will read the last event from the log.
*
* @param	InstancePtr is a pointer to the XVprocSs core instance.
*
* @return	The log data.
*
* @note		The read is "destructive" in the sense an event may only be
*           read once, the read pointer then advancing to the next entry.
*           After reading all the events (see routine XVprocSs_LogDisplay),
*           the log will be empty.
*
******************************************************************************/
u16 XVprocSs_LogRead(XVprocSs *InstancePtr)
{
	u16 Log;

	/* Verify argument. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Check if there is any data in the log */
	if (InstancePtr->Log.TailIndex == InstancePtr->Log.HeadIndex) {
		Log = XVPROCSS_EVT_NONE;
	}
	else {
		Log = InstancePtr->Log.DataBuffer[InstancePtr->Log.TailIndex];

		/* Increment tail pointer */
		if (InstancePtr->Log.TailIndex == (XVPROCSS_EVT_BUFFSIZE-1)) {
			InstancePtr->Log.TailIndex = 0;
		}
		else {
			InstancePtr->Log.TailIndex++;
		}
	}

	return Log;
}

/*****************************************************************************/
/**
* This function will read and print the entire event log.
*
* @param	InstancePtr is a pointer to the XVprocSs core instance.
*
* @return	None.
*
* @note		After reading out all events, the log will be empty.
*
******************************************************************************/
void XVprocSs_LogDisplay(XVprocSs *InstancePtr)
{
	u16 Log;
	u8 Evt;
	u8 Data;
	const char topo_name[XVPROCSS_TOPOLOGY_NUM_SUPPORTED][32]={
		"Scaler-only",
		"Full Fledged",
		"Deinterlacer-only",
		"Csc-only",
		"VerticalChromaResample-only",
		"HorizontalChromaResample-only"};

	/* Verify argument. */
	Xil_AssertVoid(InstancePtr != NULL);

	xil_printf("\r\n\n\nVPROCSS log\r\n");
	xil_printf("-----------\r\n");

	do {
		/* Read log data */
		Log = XVprocSs_LogRead(InstancePtr);

		/* Event */
		Evt = Log & 0xff;

		/* Data */
		Data = (Log >> 8) & 0xFF;

		switch (Evt) {
		case (XVPROCSS_EVT_NONE):
			xil_printf("log end\r\n-----------\r\n");
			break;
		case (XVPROCSS_EVT_INIT):
			switch (Data) {
			case (XVPROCSS_EDAT_BEGIN):
				xil_printf("Info: Video Proc Subsystem start initialization\r\n");
				break;
			case (XVPROCSS_EDAT_END):
				xil_printf("Info: Video Proc Subsystem initialized and ready\r\n");
				break;
			}
			break;
		case (XVPROCSS_EVT_RESET_VPSS):
			xil_printf("Info: Video Proc Subsystem reset\r\n");
			break;
		case (XVPROCSS_EVT_START_VPSS):
			xil_printf("Info: Video Proc Subsystem ready to accept AXIS video\r\n");
			break;
		case (XVPROCSS_EVT_STOP_VPSS):
			xil_printf("Info: Video Proc Subsystem stopped\r\n");
			break;
		case (XVPROCSS_EVT_CHK_TOPO):
			if (Data == XVPROCSS_EDAT_INITFAIL) {
				xil_printf("Error: Subsystem Topology Not Supported\r\n");
			}
			else if ((Data>=0)&&(Data<XVPROCSS_TOPOLOGY_NUM_SUPPORTED)) {
				xil_printf("Info: Subsystem Topology is %s\r\n",topo_name[Data]);
			}
			break;
		case (XVPROCSS_EVT_INIT_RESAXIS):
			switch (Data) {
			case (XVPROCSS_EDAT_CFABSENT):
				xil_printf("Error: Reset_AxiS subcore Config structure not found\r\n");
				break;
			case (XVPROCSS_EDAT_BADADDR):
				xil_printf("Error: Reset_AxiS subcore base address invalid\r\n");
				break;
			case (XVPROCSS_EDAT_INITFAIL):
				xil_printf("Error: Reset_AxiS initialization failed\r\n");
				break;
			case (XVPROCSS_EDAT_INITOK):
				xil_printf("Info: Reset_AxiS subcore initialized\r\n");
				break;
			}
			break;
		case (XVPROCSS_EVT_INIT_RESAXIM):
			switch (Data) {
			case (XVPROCSS_EDAT_CFABSENT):
				xil_printf("Error: Reset_AxiMM subcore Config structure not found\r\n");
				break;
			case (XVPROCSS_EDAT_BADADDR):
				xil_printf("Error: Reset_AxiMM subcore base address invalid\r\n");
				break;
			case (XVPROCSS_EDAT_INITFAIL):
				xil_printf("Error: Reset_AxiMM initialization failed\r\n");
				break;
			case (XVPROCSS_EDAT_INITOK):
				xil_printf("Info: Reset_AxiMM subcore initialized\r\n");
				break;
			}
			break;
		case (XVPROCSS_EVT_INIT_ROUTER):
			if (Data == XVPROCSS_EDAT_CFABSENT) {
				xil_printf("Error: Router subcore Config structure not found\r\n");
			}
			else if (Data == XVPROCSS_EDAT_BADADDR) {
				xil_printf("Error: Router subcore base address invalid\r\n");
			}
			else if (Data == XVPROCSS_EDAT_INITFAIL) {
				xil_printf("Error: Router initialization failed\r\n");
			}
			else if (Data == XVPROCSS_EDAT_INITOK) {
				xil_printf("Info: Router subcore initialized\r\n");
			}
			break;
		case (XVPROCSS_EVT_CFG_CSC):
			switch (Data) {
			case (XVPROCSS_EDAT_CFABSENT):
				xil_printf("Error: Csc subcore Config structure not found\r\n");
				break;
			case (XVPROCSS_EDAT_BADADDR):
				xil_printf("Error: Csc subcore base address invalid\r\n");
				break;
			case (XVPROCSS_EDAT_INITFAIL):
				xil_printf("Error: Csc initialization failed\r\n");
				break;
			case (XVPROCSS_EDAT_INITOK):
				xil_printf("Info: Csc subcore initialized\r\n");
				break;
			case (XVPROCSS_EDAT_NO422):
				xil_printf("Csc error: 422 color format not allowed\r\n");
				break;
			case (XVPROCSS_EDAT_VMDIFF):
				xil_printf("Csc error: Input & Output Video Mode different\r\n");
				break;
			case (XVPROCSS_EDAT_HDIFF):
				xil_printf("Csc error: Input & Output H Active different\r\n");
				break;
			case (XVPROCSS_EDAT_VDIFF):
				xil_printf("Csc error: Input & Output V Active different\r\n");
				break;
			case (XVPROCSS_EDAT_VALID):
				xil_printf("Info: Csc-only configuration is valid\r\n");
				break;
			case (XVPROCSS_EDAT_IPABSENT):
				xil_printf("Csc error: IP subcore not found\r\n");
				break;
			case (XVPROCSS_EDAT_SETUPOK):
				xil_printf("Info: Csc set up and ready to accept AXIS video\r\n");
				break;
			case (XVPROCSS_EDAT_IGNORE):
				xil_printf("Info: Csc setup ignored\r\n");
				break;
			case (XVPROCSS_EDAT_CSC_BADWIN):
				xil_printf("Csc error: Demo window is invalid\r\n");
				break;
			case (XVPROCSS_EDAT_CSC_SETWIN):
				xil_printf("Info: Csc demo window set\r\n");
				break;
			}
			break;
		case (XVPROCSS_EVT_CFG_DEINT):
			switch (Data) {
			case (XVPROCSS_EDAT_CFABSENT):
				xil_printf("Error: Deinterlacer subcore Config structure not found\r\n");
				break;
			case (XVPROCSS_EDAT_BADADDR):
				xil_printf("Error: Deinterlacer subcore base address invalid\r\n");
				break;
			case (XVPROCSS_EDAT_INITFAIL):
				xil_printf("Error: Deinterlacer initialization failed\r\n");
				break;
			case (XVPROCSS_EDAT_INITOK):
				xil_printf("Info: Deinterlacer subcore initialized\r\n");
				break;
			case (XVPROCSS_EDAT_INTPRG):
				xil_printf("Deinterlacer error: Input must be interlaced and Output must be progressive\r\n");
				break;
			case (XVPROCSS_EDAT_CDIFF):
				xil_printf("Deinterlacer error: Input & Output Color Format different\r\n");
				break;
			case (XVPROCSS_EDAT_HDIFF):
				xil_printf("Deinterlacer error: Input & Output H Active different\r\n");
				break;
			case (XVPROCSS_EDAT_VALID):
				xil_printf("Info: Deinterlacer-only configuration is valid\r\n");
				break;
			case (XVPROCSS_EDAT_IPABSENT):
				xil_printf("Deinterlacer error: IP subcore not found\r\n");
				break;
			case (XVPROCSS_EDAT_SETUPOK):
				xil_printf("Info: Deinterlacer set up and ready to accept AXIS video\r\n");
				break;
			case (XVPROCSS_EDAT_IGNORE):
				xil_printf("Info: Deinterlacer setup ignored\r\n");
				break;
			}
			break;
		case (XVPROCSS_EVT_CFG_HSCALER):
			switch (Data) {
			case (XVPROCSS_EDAT_CFABSENT):
				xil_printf("Error: HScaler subcore Config structure not found\r\n");
				break;
			case (XVPROCSS_EDAT_BADADDR):
				xil_printf("Error: HScaler subcore base address invalid\r\n");
				break;
			case (XVPROCSS_EDAT_INITFAIL):
				xil_printf("Error: HScaler initialization failed\r\n");
				break;
			case (XVPROCSS_EDAT_INITOK):
				xil_printf("Info: HScaler subcore initialized\r\n");
				break;
			case (XVPROCSS_EDAT_IODIFF):
				xil_printf("Scaler error: Input & Output Color Format different\r\n");
				break;
			case (XVPROCSS_EDAT_NO422):
				xil_printf("Scaler error: 422 color format not allowed\r\n");
				break;
			case (XVPROCSS_EDAT_VALID):
				xil_printf("Info: Scaler-only configuration is valid\r\n");
				break;
			case (XVPROCSS_EDAT_IPABSENT):
				xil_printf("HScaler error: IP subcore not found\r\n");
				break;
			case (XVPROCSS_EDAT_SETUPOK):
				xil_printf("Info: Scalers set up and ready to accept AXIS video\r\n");
				break;
			case (XVPROCSS_EDAT_IGNORE):
				xil_printf("Info: HScaler setup ignored\r\n");
				break;
			case (XVPROCSS_EDAT_LDCOEF):
				xil_printf("Info: HScaler coefficients loaded\r\n");
				break;
			}
			break;
		case (XVPROCSS_EVT_CFG_VSCALER):
			switch (Data) {
			case (XVPROCSS_EDAT_CFABSENT):
				xil_printf("Error: VScaler subcore Config structure not found\r\n");
				break;
			case (XVPROCSS_EDAT_BADADDR):
				xil_printf("Error: VScaler subcore base address invalid\r\n");
				break;
			case (XVPROCSS_EDAT_INITFAIL):
				xil_printf("Error: VScaler initialization failed\r\n");
				break;
			case (XVPROCSS_EDAT_INITOK):
				xil_printf("Info: VScaler subcore initialized\r\n");
				break;
			case (XVPROCSS_EDAT_IPABSENT):
				xil_printf("VScaler error: IP subcore not found\r\n");
				break;
			case (XVPROCSS_EDAT_IGNORE):
				xil_printf("Info: VScaler setup ignored\r\n");
				break;
			case (XVPROCSS_EDAT_LDCOEF):
				xil_printf("Info: VScaler coefficients loaded\r\n");
				break;
			}
			break;
		case (XVPROCSS_EVT_CFG_HCR):
			switch (Data) {
			case (XVPROCSS_EDAT_CFABSENT):
				xil_printf("Error: HCResampler subcore Config structure not found\r\n");
				break;
			case (XVPROCSS_EDAT_BADADDR):
				xil_printf("Error: HCResampler subcore base address invalid\r\n");
				break;
			case (XVPROCSS_EDAT_INITFAIL):
				xil_printf("Error: HCResampler initialization failed\r\n");
				break;
			case (XVPROCSS_EDAT_INITOK):
				xil_printf("Info: HCResampler subcore initialized\r\n");
				break;
			case (XVPROCSS_EDAT_CFIN):
				xil_printf("HCResampler error: Video Input must be YUV422 or YUV444\r\n");
				break;
			case (XVPROCSS_EDAT_CFOUT):
				xil_printf("HCResampler error: Video Output must be YUV422 or YUV444\r\n");
				break;
			case (XVPROCSS_EDAT_VMDIFF):
				xil_printf("HCResampler error: Input & Output Video Mode different\r\n");
				break;
			case (XVPROCSS_EDAT_HDIFF):
				xil_printf("HCResampler error: Input & Output H Active different\r\n");
				break;
			case (XVPROCSS_EDAT_VDIFF):
				xil_printf("HCResampler error: Input & Output V Active different\r\n");
				break;
			case (XVPROCSS_EDAT_VALID):
				xil_printf("Info: HCResampler-only configuration is valid\r\n");
				break;
			case (XVPROCSS_EDAT_IPABSENT):
				xil_printf("HCResampler error: IP subcore not found\r\n");
				break;
			case (XVPROCSS_EDAT_SETUPOK):
				xil_printf("Info: HCResampler set up and ready to accept AXIS video\r\n");
				break;
			case (XVPROCSS_EDAT_IGNORE):
				xil_printf("Info: HCResampler setup ignored\r\n");
				break;
			case (XVPROCSS_EDAT_LDCOEF):
				xil_printf("Info: HCResampler coefficients loaded\r\n");
				break;
			}
			break;
		case (XVPROCSS_EVT_CFG_VCRI):
			switch (Data) {
			case (XVPROCSS_EDAT_CFABSENT):
				xil_printf("Error: VCResamplerIn subcore Config structure not found\r\n");
				break;
			case (XVPROCSS_EDAT_BADADDR):
				xil_printf("Error: VCResamplerIn subcore base address invalid\r\n");
				break;
			case (XVPROCSS_EDAT_INITFAIL):
				xil_printf("Error: VCResamplerIn initialization failed\r\n");
				break;
			case (XVPROCSS_EDAT_INITOK):
				xil_printf("Info: VCResamplerIn subcore initialized\r\n");
				break;
			case (XVPROCSS_EDAT_CFIN):
				xil_printf("VCResamplerIn error: Video Input must be YUV420 or YUV422\r\n");
				break;
			case (XVPROCSS_EDAT_CFOUT):
				xil_printf("VCResamplerIn error: Video Output must be YUV420 or YUV422\r\n");
				break;
			case (XVPROCSS_EDAT_VMDIFF):
				xil_printf("VCResamplerIn error: Input & Output Video Mode different\r\n");
				break;
			case (XVPROCSS_EDAT_HDIFF):
				xil_printf("VCResamplerIn error: Input & Output H Active different\r\n");
				break;
			case (XVPROCSS_EDAT_VDIFF):
				xil_printf("VCResamplerIn error: Input & Output V Active different\r\n");
				break;
			case (XVPROCSS_EDAT_VALID):
				xil_printf("Info: VCResampler-only configuration is valid\r\n");
				break;
			case (XVPROCSS_EDAT_IPABSENT):
				xil_printf("VCResamplerIn error: IP subcore not found\r\n");
				break;
			case (XVPROCSS_EDAT_SETUPOK):
				xil_printf("Info: VCResampler set up and ready to accept AXIS video\r\n");
				break;
			case (XVPROCSS_EDAT_IGNORE):
				xil_printf("Info: VCResamplerIn setup ignored\r\n");
				break;
			case (XVPROCSS_EDAT_LDCOEF):
				xil_printf("Info: VCResamplerIn coefficients loaded\r\n");
				break;
			}
			break;
		case (XVPROCSS_EVT_CFG_VCRO):
			switch (Data) {
			case (XVPROCSS_EDAT_CFABSENT):
				xil_printf("Error: VCResamplerOut subcore Config structure not found\r\n");
				break;
			case (XVPROCSS_EDAT_BADADDR):
				xil_printf("Error: VCResamplerOut subcore base address invalid\r\n");
				break;
			case (XVPROCSS_EDAT_INITFAIL):
				xil_printf("Error: VCResamplerOut initialization failed\r\n");
				break;
			case (XVPROCSS_EDAT_INITOK):
				xil_printf("Info: VCResamplerOut subcore initialized\r\n");
				break;
			case (XVPROCSS_EDAT_IPABSENT):
				xil_printf("VCResamplerOut error: IP subcore not found\r\n");
				break;
			case (XVPROCSS_EDAT_LDCOEF):
				xil_printf("Info: VCResamplerOut coefficients loaded\r\n");
				break;
			}
			break;
		case (XVPROCSS_EVT_INIT_LBOX):
			switch (Data) {
			case (XVPROCSS_EDAT_CFABSENT):
				xil_printf("Error: LetterBox subcore Config structure not found\r\n");
				break;
			case (XVPROCSS_EDAT_BADADDR):
				xil_printf("Error: LetterBox subcore base address invalid\r\n");
				break;
			case (XVPROCSS_EDAT_INITFAIL):
				xil_printf("Error: LetterBox initialization failed\r\n");
				break;
			case (XVPROCSS_EDAT_INITOK):
				xil_printf("Info: LetterBox subcore initialized\r\n");
				break;
			}
			break;
		case (XVPROCSS_EVT_INIT_VDMA):
			switch (Data) {
			case (XVPROCSS_EDAT_CFABSENT):
				xil_printf("Error: Video DMA subcore Config structure not found\r\n");
				break;
			case (XVPROCSS_EDAT_BADADDR):
				xil_printf("Error: Video DMA subcore base address invalid\r\n");
				break;
			case (XVPROCSS_EDAT_INITFAIL):
				xil_printf("Error: Video DMA initialization failed\r\n");
				break;
			case (XVPROCSS_EDAT_INITOK):
				xil_printf("Info: Video DMA subcore initialized\r\n");
				break;
			}
			break;
		case (XVPROCSS_EVT_CFG_MAX):
			switch (Data) {
			case (XVPROCSS_EDAT_MAX_TABLEOK):
				xil_printf("Info: Full mode - Video Routing Table setup OK\r\n");
				break;
			case (XVPROCSS_EDAT_MAX_DFLOWOK):
				xil_printf("Info: Full mode - Video Data Flow setup OK\r\n");
				break;
			case (XVPROCSS_EDAT_MAX_ROUTEOK):
				xil_printf("Info: Full mode - Video Router setup OK\r\n");
				break;
			case (XVPROCSS_EDAT_MAX_SCALE11):
				xil_printf("Info: Full mode - Set scale_1:1 mode\r\n");
				break;
			case (XVPROCSS_EDAT_MAX_SCALEUP):
				xil_printf("Info: Full mode - Set scale_up mode\r\n");
				break;
			case (XVPROCSS_EDAT_MAX_SCALEDN):
				xil_printf("Info: Full mode - Set scale_down mode\r\n");
				break;
			case (XVPROCSS_EDAT_CFIN):
				xil_printf("Error: Full mode - Input color format invalid as configured\r\n");
				break;
			case (XVPROCSS_EDAT_CFOUT):
				xil_printf("Error: Full mode - Output color format invalid as configured\r\n");
				break;
			case (XVPROCSS_EDAT_MAX_WRWINBAD):
				xil_printf("Error: Full mode - VDMA Write Channel Window Invalid\r\n");
				break;
			case (XVPROCSS_EDAT_MAX_RDWINBAD):
				xil_printf("Error: Full mode - VDMA Read Channel Window Invalid\r\n");
				break;
			case (XVPROCSS_EDAT_MAX_SCALEBAD):
				xil_printf("Error: Full mode - Scaling Mode not supported\r\n");
				break;
			}
			break;
		case (XVPROCSS_EVT_CFG_VPSS):
			switch (Data) {
			case (XVPROCSS_EDAT_INITFAIL):
				xil_printf("Error: Video Proc Subsystem configuration failed\r\n");
				break;
			case (XVPROCSS_EDAT_VALID):
				xil_printf("Info: Video Proc Subsystem configuration is valid\r\n");
				break;
			case (XVPROCSS_EDAT_VPSS_FRDIFF):
				xil_printf("Error: Not Full mode, and Input & Output Frame Rate different\r\n");
				break;
			case (XVPROCSS_EDAT_VPSS_IVRANGE):
				xil_printf("Error: Input Stream Resolution out of range 0...MAX\r\n");
				break;
			case (XVPROCSS_EDAT_VPSS_OVRANGE):
				xil_printf("Error: Output Stream Resolution out of range 0...MAX\r\n");
				break;
			case (XVPROCSS_EDAT_VPSS_WIDBAD):
				xil_printf("Error: Input/Output Width not aligned with Samples/Clk \r\n");
				break;
			case (XVPROCSS_EDAT_VPSS_RESBAD):
				xil_printf("Error: 1 Sample/Clk max resolution is 4K2K@30Hz\r\n");
				break;
			case (XVPROCSS_EDAT_VPSS_WIDODD):
				xil_printf("Error: YUV422 stream width must be even\r\n");
				break;
			case (XVPROCSS_EDAT_VPSS_SIZODD):
				xil_printf("Error: YUV420 input width and height must be even\r\n");
				break;
			case (XVPROCSS_EDAT_VPSS_NOHCR):
				xil_printf("Error: HCResampler is required but not found\r\n");
				break;
			case (XVPROCSS_EDAT_VPSS_NOVCRI):
				xil_printf("Error: VCResampler is required at input but not found\r\n");
				break;
			case (XVPROCSS_EDAT_VPSS_NOVCRO):
				xil_printf("Error: VCResampler is required at output but not found\r\n");
				break;
			case (XVPROCSS_EDAT_NO420):
				xil_printf("Error: Interlaced YUV420 stream not supported\r\n");
				break;
			case (XVPROCSS_EDAT_VPSS_NODEIN):
				xil_printf("Error: Input is interlaced but no Deinterlacer found\r\n");
				break;
			}
			break;
		case (XVPROCSS_EVT_CHK_BASEADDR):
			xil_printf("Error: Video Frame Buffer base address not set\r\n" \
			           "       Use XVprocSs_SetFrameBufBaseaddr() before VPSS init\r\n");
			break;
		case (XVPROCSS_EVT_UPDATE_ZPWIN):
			switch (Data) {
			case (XVPROCSS_EDAT_SUCCESS):
				xil_printf("Info: Zoom/Pip window updated\r\n");
				break;
			case (XVPROCSS_EDAT_FAILURE):
				xil_printf("Error: Zoom/Pip window not supported\r\n");
				break;
			}
			break;
		case (XVPROCSS_EVT_SET_PIPWIN):
			switch (Data) {
			case (XVPROCSS_EDAT_SUCCESS):
				xil_printf("Info: Pip window set\r\n");
				break;
			case (XVPROCSS_EDAT_FAILURE):
				xil_printf("Error: Pip window not supported\r\n");
				break;
			}
			break;
		case (XVPROCSS_EVT_SET_ZOOMWIN):
			switch (Data) {
			case (XVPROCSS_EDAT_SUCCESS):
				xil_printf("Info: Zoom window set\r\n");
				break;
			case (XVPROCSS_EDAT_FAILURE):
				xil_printf("Error: Zoom window not supported\r\n");
				break;
			}
			break;
		case (XVPROCSS_EVT_GET_ZPWIN):
			if (Data == XVPROCSS_EDAT_FAILURE) {
				xil_printf("Error: Zoom/Pip window not supported\r\n");
			}
			break;
		case (XVPROCSS_EVT_SET_ZOOMMODE):
			switch (Data) {
			case (XVPROCSS_EDAT_OFF):
				xil_printf("Info: Turn off Zoom window\r\n");
				break;
			case (XVPROCSS_EDAT_ON):
				xil_printf("Info: Turn on Zoom window\r\n");
				break;
			case (XVPROCSS_EDAT_FAILURE):
				xil_printf("Error: Zoom window not supported\r\n");
				break;
			}
			break;
		case (XVPROCSS_EVT_SET_PIPMODE):
			switch (Data) {
			case (XVPROCSS_EDAT_OFF):
				xil_printf("Info: Turn off PIP window\r\n");
				break;
			case (XVPROCSS_EDAT_ON):
				xil_printf("Info: Turn on PIP window\r\n");
				break;
			case (XVPROCSS_EDAT_BGND_SET):
				xil_printf("Info: Set the background color\r\n");
				break;
			case (XVPROCSS_EDAT_FAILURE):
				xil_printf("Error: PIP window not supported\r\n");
				break;
			case (XVPROCSS_EDAT_LBOX_ABSENT):
				xil_printf("Error: No PIP window, cannot set background color\r\n");
				break;
			}
			break;
		default:
			xil_printf("Unknown event\r\n");
			break;
		}
	} while (Log != XVPROCSS_EVT_NONE);
}
