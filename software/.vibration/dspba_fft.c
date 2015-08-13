/******************************************************************************
*                                                                             *
* License Agreement                                                           *
*                                                                             *
* Copyright (c) 2014 Altera Corporation, San Jose, California, USA.           *
* All rights reserved.                                                        *
*                                                                             *
* Permission is hereby granted, free of charge, to any person obtaining a     *
* copy of this software and associated documentation files (the "Software"),  *
* to deal in the Software without restriction, including without limitation   *
* the rights to use, copy, modify, merge, publish, distribute, sublicense,    *
* and/or sell copies of the Software, and to permit persons to whom the       *
* Software is furnished to do so, subject to the following conditions:        *
*                                                                             *
* The above copyright notice and this permission notice shall be included in  *
* all copies or substantial portions of the Software.                         *
*                                                                             *
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR  *
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,    *
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE *
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER      *
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING     *
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER         *
* DEALINGS IN THE SOFTWARE.                                                   *
*                                                                             *
* This agreement shall be governed in all respects by the laws of the State   *
* of California and by the laws of the United States of America.              *
* Altera does not recommend, suggest or require that this reference design    *
* file be used in conjunction or combination with any other product.          *
******************************************************************************/

/**
 * @file dspba_fft.c
 *
 * @brief FFT function in vibration reference design
 */

#include "includes.h"

/*!
 * \addtogroup VIBRATION
 *
 * @{
 */

/*!
 * \addtogroup FFT FFT Processing
 *
 * @{
 */

fft_t fft_0, fft_1;

#ifdef __DOC_SOC_UCOSII__
/**
 * Arrays for DMA buffers. Need to be big enough to allow an extra cache line for avoidance of
 * the DMA cache prefetch issue
 *
 * Linker script preserves a memory region for the fft buffers this memory
 */
extern char _fft_buffers;
char * hps_hard_fft_buffer_0 = (char *)(&_fft_buffers);
char * hps_hard_fft_buffer_1 = (char *)(&_fft_buffers + 0x10000);
#endif

/**
 * Arrays to hold filtered real and imaginary and magnitude squared values
 */
float fft_0_filt[FFT_LENGTH*2] __attribute__ ((aligned (32)));
float fft_1_filt[FFT_LENGTH*2] __attribute__ ((aligned (32)));
float fft_0_magsq[FFT_LENGTH] __attribute__ ((aligned (32)));
float fft_1_magsq[FFT_LENGTH] __attribute__ ((aligned (32)));
float fft_0_input[FFT_LENGTH*2] __attribute__ ((aligned (32)));
float fft_1_input[FFT_LENGTH*2] __attribute__ ((aligned (32)));
float fft_0_fft[FFT_LENGTH*2] __attribute__ ((aligned (32)));
float fft_1_fft[FFT_LENGTH*2] __attribute__ ((aligned (32)));
float fft_0_magsq_pk[FFT_LENGTH/64] __attribute__ ((aligned (32)));
float fft_1_magsq_pk[FFT_LENGTH/64] __attribute__ ((aligned (32)));
int fft_0_magsq_freq[FFT_LENGTH/64] __attribute__ ((aligned (32)));
int fft_1_magsq_freq[FFT_LENGTH/64] __attribute__ ((aligned (32)));

#define FFT_PING 0

void fft_filter_i(void * __restrict__ n, float c, float * __restrict__ f);
void fft_filter_f(void * __restrict__ n, float c, float * __restrict__ f);

/**
 * Initialise FFT structure(s)
 */
void fft_init(void) {
	int i;
#ifdef __DOC_SOC_VXWORKS__
	void * pBuf;
#endif
	
	//Set FFT size
	IOWR_32DIRECT(FFT_0_BASE, SIZE_CONFIG_CFG, FFT_SIZE_CFG);
	IOWR_32DIRECT(FFT_1_BASE, SIZE_CONFIG_CFG, FFT_SIZE_CFG);

	// Initialise FFT data
	fft_0.fft_enable = 0;
	fft_0.fft_axis = 0;
	fft_0.fft_instance = 0;
	fft_0.fft_type = FFT_HARD;
	fft_0.ping_state = FFT_RESET;
	fft_0.ping_input_buffer = FFTINPUT_BUFFER_0_PING_BASE;
	fft_0.ping_output_buffer = FFTOUTPUT_BUFFER_0_PING_BASE;
	fft_0.ping_samples = -1;
	fft_0.hps_state = FFT_RESET;
	fft_0.hps_samples = 0;
	fft_0.hps_samples64 = 0;
#ifdef __DOC_SOC_UCOSII__
	fft_0.hps_hard_fft_buffer = (void *)hps_hard_fft_buffer_0 + 32;
#else
#ifdef __DOC_SOC_VXWORKS__
	// Ensure DMA buffers are cache line aligned and big enough to allow an extra cache line for avoidance of
	// the DMA cache prefetch issue
	pBuf = cacheDmaMalloc(0x10000);
	fft_0.hps_hard_fft_buffer = (void *)(((unsigned int)pBuf & 0xffffffe0) + 64);
#else
#error "Usupoorted OS"
#endif
#endif
	fft_0.hps_filt_buffer = fft_0_filt;
	fft_0.hps_magsq_buffer = fft_0_magsq;
	fft_0.hps_filt_coeff = 0.9f;
	fft_0.hps_magsq_pk_buffer = fft_0_magsq_pk;
	fft_0.hps_magsq_freq_buffer = fft_0_magsq_freq;

	fft_0.fft_trigger = 0;

	fft_0.hps_input_buffer = fft_0_input;
	fft_0.hps_soft_fft_buffer = fft_0_fft;

	fft_0.ping_program = (ALT_DMA_PROGRAM_t *)FFT_0_PING_PROGRAM;
	fft_0.hps_program = (ALT_DMA_PROGRAM_t *)FFT_0_HPS_PROGRAM;

	fft_0.dspba_fft_state = 0;
	fft_0.fft_hps_busy_edge = 0;
	fft_0.fft_ping_sys_busy_edge = 0;
	fft_0.dspba_fft_base = FFT_0_BASE;
	fft_0.dspba_fft_buffer_base = FFTOUTPUT_BUFFER_0_BASE;

	if (fft_0.fft_type == FFT_SOFT) {
		fft_0.fft_filter_fn = fft_filter_f;
	} else {
		fft_0.fft_filter_fn = fft_filter_i;
	}

	for (i = 0; i < FFT_LENGTH*2; i++) {
		((unsigned int *)fft_0.hps_hard_fft_buffer)[i] = 0;
		((unsigned int *)fft_0.hps_input_buffer)[i] = 0;
		((unsigned int *)fft_0.hps_soft_fft_buffer)[i] = 0;
		((unsigned int *)fft_0.hps_filt_buffer)[i] = 0;
	}

	fft_dump(&fft_0);

	fft_1.fft_enable = 0;
	fft_1.fft_axis = 0;
	fft_1.fft_instance = 1;
	fft_1.fft_type = FFT_HARD;
	fft_1.ping_state = FFT_RESET;
	fft_1.ping_input_buffer = FFTINPUT_BUFFER_1_PING_BASE;
	fft_1.ping_output_buffer = FFTOUTPUT_BUFFER_1_PING_BASE;
	fft_1.ping_samples = -1;
	fft_1.hps_state = FFT_RESET;
	fft_1.hps_samples = 0;
	fft_1.hps_samples64 = 0;
#ifdef __DOC_SOC_UCOSII__
	fft_1.hps_hard_fft_buffer = (void *)hps_hard_fft_buffer_1 + 32;
#else
#ifdef __DOC_SOC_VXWORKS__
	// Ensure DMA buffers are cache line aligned and big enough to allow an extra cache line for avoidance of
	// the DMA cache prefetch issue
	pBuf = cacheDmaMalloc(0x10000);
	fft_1.hps_hard_fft_buffer = (void *)(((unsigned int)pBuf & 0xffffffe0) + 64);
#else
#error "Usupoorted OS"
#endif
#endif
	fft_1.hps_filt_buffer = fft_1_filt;
	fft_1.hps_magsq_buffer = fft_1_magsq;
	fft_1.hps_filt_coeff = 0.9f;
	fft_1.hps_magsq_pk_buffer = fft_0_magsq_pk;
	fft_1.hps_magsq_freq_buffer = fft_0_magsq_freq;

	fft_1.fft_trigger = 0;

	fft_1.hps_input_buffer = fft_1_input;
	fft_1.hps_soft_fft_buffer = fft_1_fft;

	fft_1.ping_program = (ALT_DMA_PROGRAM_t *)FFT_1_PING_PROGRAM;
	fft_1.hps_program = (ALT_DMA_PROGRAM_t *)FFT_1_HPS_PROGRAM;

	fft_1.dspba_fft_state = 0;
	fft_1.fft_hps_busy_edge = 0;
	fft_1.fft_ping_sys_busy_edge = 0;
	fft_1.dspba_fft_base = FFT_1_BASE;
	fft_1.dspba_fft_buffer_base = FFTOUTPUT_BUFFER_1_BASE;

	if (fft_1.fft_type == FFT_SOFT) {
		fft_1.fft_filter_fn = fft_filter_f;
	} else {
		fft_1.fft_filter_fn = fft_filter_i;
	}

	for (i = 0; i < FFT_LENGTH*2; i++) {
		((unsigned int *)fft_1.hps_hard_fft_buffer)[i] = 0;
		((unsigned int *)fft_1.hps_input_buffer)[i] = 0;
		((unsigned int *)fft_1.hps_soft_fft_buffer)[i] = 0;
		((unsigned int *)fft_1.hps_filt_buffer)[i] = 0;
	}

	fft_dump(&fft_1);
}

/**
 * Dump structure members of an FFT instance
 *
 * @param fft	Pointer to an FFT structure
 */
void fft_dump(fft_t * fft) {
	debug_printf(DBG_DEBUG, "Axis:                  %d\n", fft->fft_axis);
	debug_printf(DBG_DEBUG, "Instance               %d\n", fft->fft_instance);

	debug_printf(DBG_DEBUG, "ping_state             %d\n", fft->ping_state);
	debug_printf(DBG_DEBUG, "ping_input_buffer      %x\n", fft->ping_input_buffer);
	debug_printf(DBG_DEBUG, "ping_output_buffer     %x\n", fft->ping_output_buffer);
	debug_printf(DBG_DEBUG, "ping_samples           %d\n", fft->ping_samples);

	debug_printf(DBG_DEBUG, "hps_state              %d\n", fft->hps_state);
	debug_printf(DBG_DEBUG, "hps_hard_fft_buffer             %x\n", fft->hps_hard_fft_buffer);

	debug_printf(DBG_DEBUG, "fft_trigger            %d\n", fft->fft_trigger);

	debug_printf(DBG_DEBUG, "dspba_fft_state       %d\n", fft->dspba_fft_state);
	debug_printf(DBG_DEBUG, "fft_hps_busy_edge         %d\n", fft->fft_hps_busy_edge);
	debug_printf(DBG_DEBUG, "fft_ping_sys_busy_edge         %d\n", fft->fft_ping_sys_busy_edge);
	debug_printf(DBG_DEBUG, "dspba_fft_base        %x\n", fft->dspba_fft_base);
	debug_printf(DBG_DEBUG, "dspba_fft_buffer_base %x\n", fft->dspba_fft_buffer_base);
}

/**
 * Get the status of a hardware FFT block. Save the previous state so we can detect edges
 * on BUSY state.
 *
 * @param fft	Pointer to an FFT structure
 */
void fft_update_status(fft_t * fft) {
	int old_state = fft->dspba_fft_state & DSPBA_FFT_BUSY_BIT;
	fft->dspba_fft_state = IORD_32DIRECT(fft->dspba_fft_buffer_base, 0x8000);
	if ((fft->fft_enable) && (old_state == 0) && ((fft->dspba_fft_state & DSPBA_FFT_BUSY_BIT) == DSPBA_FFT_BUSY_BIT)) {
		fft->fft_hps_busy_edge = 1;
		fft->fft_ping_sys_busy_edge = 1;
	}
}

/**
 * Reset the READY status bit and local copy to acknowledge read of data
 *
 * @param fft	Pointer to an FFT structure
 */
void fft_reset_ready_status(fft_t * fft) {
	fft->dspba_fft_state &= ~DSPBA_FFT_READY_BIT;
	IOWR_32DIRECT(fft->dspba_fft_buffer_base, 0x8000, 0);
}

/**
 * Set the status bit for FFT
 *
 * Each channel that becomes ready increments the status. System console will wait until
 * both channels have signalled ready by incrementing the status to 2
 *
 * @param fft	Pointer to an FFT structure
 */
void fft_set_buffer_status(fft_t * fft) {
	unsigned int status_word = 0;
	int status = 0;
	status_word = DOC_DBG_FFT_PING_STATUS;
	if (status_word) {
		status = debug_read_command(fft->fft_axis, status_word);
		//Max out at 2 in case of race conditions (both FFTs ready)
		status = (status < 2)? status + 1 : 2;
		debug_write_status(fft->fft_axis, status_word, status);
	}
}

/**
 * Get the status bit for FFT by axis, instance and buffer
 *
 * @param fft	Pointer to an FFT structure
 */
int fft_get_buffer_status(fft_t * fft) {
	unsigned int status_word = 0;
	int status;

	status_word = DOC_DBG_FFT_PING_STATUS;
	if (status_word) {
		status =  debug_read_command(0, status_word);
		return status;
	} else {
		return 0;
	}
}

/**
 * Run the HPS FFT control state machine.
 *
 * Called from the ISR to update the HPS FFT state and trigger DMA. MAy take longer than one ISR
 * to complete, e.g. when invalidating caches.
 *
 * FFT data is always sent to the HPS on the assumption it has completed processing of the
 * previous FFT results.
 *
 * @param fft	Pointer to an FFT structure
 */
void fft_update_hps_state(fft_t * fft) {
#ifdef __DOC_SOC_UCOSII__
    CPU_INT08U  os_err;
	int flags;
#else
#ifdef __DOC_SOC_VXWORKS__
	unsigned int events;
	STATUS evStatus;
#else
#error "Usupoorted OS"
#endif
#endif

	ALT_STATUS_CODE status;

	// State machine for HPS
	if (fft->fft_enable == 0) {
		fft->fft_hps_busy_edge = 0;
		fft->hps_state = FFT_RESET;
	}
	switch(fft->hps_state) {
		case FFT_RESET:
			if (fft->fft_enable == 1) {
				fft->hps_state = FFT_WAIT_SINK;
			}
			break;

		case FFT_WAIT_SINK:
			// Waiting for FFT to accumulate 64 samples and begin calculating
			if (fft->fft_hps_busy_edge) {
				fft->fft_hps_busy_edge = 0;
				// Sample count is not updated in the state machine as some states can take longer than one ISR.
				// Instead, it is updated in the ISR just before waking drive_task()
				fft->hps_samples = 0;
				fft->hps_samples64 = (fft->hps_samples64 + 1)%64;
				if (fft->fft_type == FFT_HARD) {
					fft->hps_state = FFT_WAIT_FFT;
				} else {
					fft->hps_state = FFT_SOFT_FFT;
				}
			}
			break;

		case FFT_SOFT_FFT:
			// Never visits this state for hard fft
			// Signal processor that it can do the soft fft
#ifdef __DOC_SOC_UCOSII__
			OSFlagPost(fft_flags, FFT0_HPS_READY<<(fft->fft_instance&1), OS_FLAG_SET, &os_err);
#else
#ifdef __DOC_SOC_VXWORKS__
			if (fft->fft_instance == 0) {
				eventSend(hpsFftTaskId, FFT_TASK_EV_FFT0_HPS_READY);
			} else if (fft->fft_instance == 1) {
				eventSend(hpsFftTaskId, FFT_TASK_EV_FFT1_HPS_READY);
			}
#else
#error "Usupoorted OS"
#endif
#endif
			fft->hps_state = FFT_WAIT_UPLOAD;
			break;

		case FFT_WAIT_FFT:
			// Waiting for FFT result to be calculated so that data can be sent to HPS via DMA
			// Never visits this state for soft fft
			if (fft->dspba_fft_state & DSPBA_FFT_READY_BIT) {
				// Invalidate the caches in preparation for new DMA data
#ifdef __DOC_SOC_UCOSII__
				l2_invalidate_range((unsigned int)fft->hps_hard_fft_buffer, (unsigned int)fft->hps_hard_fft_buffer + DSPBA_FFT_BLOCK_SIZE);
				l1_invalidate_range((unsigned int)fft->hps_hard_fft_buffer, (unsigned int)fft->hps_hard_fft_buffer + DSPBA_FFT_BLOCK_SIZE);
#else
#ifdef __DOC_SOC_VXWORKS__
				CACHE_DMA_INVALIDATE((unsigned char *)fft->hps_hard_fft_buffer, DSPBA_FFT_BLOCK_SIZE);
#else
#error "Usupoorted OS"
#endif
#endif
				status = alt_dma_memory_to_memory(fft->hps_channel, fft->hps_program, fft->hps_hard_fft_buffer, (const void *)fft->dspba_fft_buffer_base,
								DSPBA_FFT_BLOCK_SIZE, false, 0);
				if (status != ALT_E_SUCCESS) {
					debug_printf(DBG_ERROR, "%d HPS DMA error %d\n", fft->fft_instance , (unsigned int)status);
					debug_printf(DBG_ERROR, "%d HPS -> _ERROR\n", fft->fft_instance);
					fft->hps_state = FFT_ERROR;
				} else {
					fft->hps_state = FFT_WAIT_DMA;
				}
			}
			break;

		case FFT_WAIT_DMA:
			// Waiting for HPS DMA channel to complete then reset ready status bit and signal processor
			// Never visits this state for soft fft
			alt_dma_channel_state_get(fft->hps_channel, &fft->hps_chan_state);
			if (fft->hps_chan_state == ALT_DMA_CHANNEL_STATE_STOPPED) {
				fft_reset_ready_status(fft);
#ifdef __DOC_SOC_UCOSII__
				OSFlagPost(fft_flags, FFT0_HPS_READY<<(fft->fft_instance&1), OS_FLAG_SET, &os_err);
#else
#ifdef __DOC_SOC_VXWORKS__
				if (fft->fft_instance == 0) {
					eventSend(hpsFftTaskId, FFT_TASK_EV_FFT0_HPS_READY);
				} else if (fft->fft_instance == 1) {
					eventSend(hpsFftTaskId, FFT_TASK_EV_FFT1_HPS_READY);
				}
#else
#error "Usupoorted OS"
#endif
#endif
				fft->hps_state = FFT_WAIT_UPLOAD;
			}
			break;

		case FFT_WAIT_UPLOAD:
			// Waiting for HPS to acknowledge use of fft buffer, no wait so does not block
#ifdef __DOC_SOC_UCOSII__
			flags = OSFlagAccept(fft_flags, FFT0_HPS_ACK<<(fft->fft_instance&1), OS_FLAG_WAIT_SET_ANY + OS_FLAG_CONSUME, &os_err);
			if (flags & (FFT0_HPS_ACK<<(fft->fft_instance&1))) {
#else
#ifdef __DOC_SOC_VXWORKS__
			evStatus = -1;
			if (fft->fft_instance == 0) {
				evStatus = eventReceive(DRIVE_TASK_EV_FFT0_HPS_ACK, EVENTS_WAIT_ALL | EVENTS_KEEP_UNWANTED, NO_WAIT, &events);
			} else if (fft->fft_instance == 1) {
				evStatus = eventReceive(DRIVE_TASK_EV_FFT1_HPS_ACK, EVENTS_WAIT_ALL | EVENTS_KEEP_UNWANTED, NO_WAIT, &events);
			}
			if (evStatus == OK) {
#else
#error "Usupoorted OS"
#endif
#endif
				fft->hps_state = FFT_WAIT_SINK;
			}
			break;

		default:
			debug_printf(DBG_ERROR, "%d HPS default state %d\n", fft->fft_instance , fft->hps_state);
			fft->hps_state = FFT_ERROR;
			break;

		case FFT_ERROR:
			break;

	}
}

/**
 * Run the system console FFT control state machine.
 *
 * Called from dump_data to update the system console FFT state and trigger DMA.
 *
 * FFT data is sent to system console via a single buffer. When a trigger condition occurs and the
 * buffer is available, the buffer will be filled with the results of the next FFT.
 *
 * @param fft	Pointer to an FFT structure
 */
void fft_update_sys_state(fft_t * fft) {
	ALT_STATUS_CODE status;
	int buffer_status;

	// State machine for ping buffer
	if (fft->fft_enable == 0) {
		fft->fft_ping_sys_busy_edge = 0;
		fft->ping_state = FFT_RESET;
	}
	switch(fft->ping_state) {
		case FFT_RESET:
			if (fft->fft_enable == 1) {
				fft->ping_state = FFT_WAIT;
				debug_printf(DBG_DEBUG_MORE, "%d Moving to FFT_WAIT\n", fft->fft_instance);
			}
			break;

		case FFT_WAIT:
			// Synchronise to FFT by waiting for busy bit
			if (fft->fft_ping_sys_busy_edge) {
				fft->fft_ping_sys_busy_edge = 0;
				// We have written the last sample of a cycle, so reset counter for next cycle
				fft->ping_samples = 0;
				fft->ping_state = FFT_WAIT_TRIGGER;
				debug_printf(DBG_DEBUG_MORE, "%d Moving to FFT_WAIT_TRIGGER\n", fft->fft_instance);
			}
			break;

		case FFT_WAIT_TRIGGER:
			// Waiting for trigger
			fft->ping_samples++;
			if (fft->fft_trigger == 1) {
				fft->fft_trigger = 0;
				fft->ping_state = FFT_WAIT_SINK;
				debug_printf(DBG_DEBUG_MORE, "%d Moving to FFT_WAIT_SINK\n", fft->fft_instance);
			} else if (fft->fft_ping_sys_busy_edge) {
				// FFT has started processing but we have not had a trigger so re-arm and wait here
				fft->fft_ping_sys_busy_edge = 0;
				fft->ping_samples = 0;
			}
			break;

		case FFT_WAIT_SINK:
			// Wait until we have accumulated enough samples
			fft->ping_samples++;
			if (fft->ping_samples >= FFT_LENGTH) {
				fft->ping_samples = -1;
				if (fft->fft_type == FFT_HARD) {
					// FFT should begin processing and set busy bit
					fft->ping_state = FFT_WAIT_FFT;
					debug_printf(DBG_DEBUG_MORE, "%d Moving to FFT_WAIT_FFT\n", fft->fft_instance);
				} else {
					fft->ping_state = FFT_SOFT_FFT;
				}
			}
			break;

		case FFT_SOFT_FFT:
			// Wait for soft or hard fft to complete
			buffer_status = fft_get_buffer_status(fft);
			if (buffer_status == 2) {
				fft->ping_state = FFT_WAIT_UPLOAD;
				debug_printf(DBG_DEBUG_MORE, "%d Moving to FFT_WAIT_UPLOAD\n", fft->fft_instance);
			} else if (buffer_status > 2) {
				debug_printf(DBG_ERROR, "%d Ping buffer_status error (>2) :%d\n", fft->fft_instance, (unsigned int)buffer_status);
				fft->ping_state = FFT_WAIT_UPLOAD;
			}
			break;

		case FFT_WAIT_FFT:
			// Never visits this state for soft fft
			if (fft->dspba_fft_state & DSPBA_FFT_READY_BIT) {
				// FFT processing complete start DMA to system console buffer
				status = alt_dma_memory_to_memory(fft->ping_channel, fft->ping_program, (void *)fft->ping_output_buffer, (const void *)fft->dspba_fft_buffer_base,
									DSPBA_FFT_BLOCK_SIZE, false, 0);
				if (status != ALT_E_SUCCESS) {
					debug_printf(DBG_ERROR, "%d Ping DMA error %d\n", fft->fft_instance, (unsigned int)status);
					debug_printf(DBG_ERROR, "%d Ping -> _WAIT_ERROR\n", fft->fft_instance);
					fft->ping_state = FFT_ERROR;
				} else {
					debug_printf(DBG_DEBUG_MORE, "%d Moving to FFT_WAIT_DMA\n", fft->fft_instance);
					fft->ping_state = FFT_WAIT_DMA;
				}
			}
			break;

		case FFT_WAIT_DMA:
			// Wait for DMA channel to complete and set status bit to allow system console to start accessing buffer
			// Never visits this state for soft fft
			alt_dma_channel_state_get(fft->ping_channel, &fft->ping_chan_state);
			if (fft->ping_chan_state == ALT_DMA_CHANNEL_STATE_STOPPED) {
				// Buffer is available for upload
				// Always transition through FFT_SOFT_FFT state as this carries out post-processing even if we are using the HARD FFT
				debug_printf(DBG_DEBUG_MORE, "%d Moving to FFT_SOFT_FFT\n", fft->fft_instance);
				fft->ping_state = FFT_SOFT_FFT;
			}
			break;

		case FFT_WAIT_UPLOAD:
			// Wait for system console to signal upload complete by clearing status bit
			// Now only look at channel 0 status as system console can be quite slow causing a skew between the two channels
			// in recognising the next busy edge
			if (fft_get_buffer_status(fft) == 0) {
				// Prepare to re-synch to FFT, ignoring any recent busy edges whilst we were busy
				fft->fft_ping_sys_busy_edge = 0;
				debug_printf(DBG_DEBUG_MORE, "%d Moving to FFT_WAIT\n", fft->fft_instance);
				fft->ping_state = FFT_WAIT;
			}
			break;

		default:
			fft->ping_state = FFT_ERROR;
			break;

		case FFT_ERROR:
			break;
	}
}

/**
 * Write a new sample to the FFT hardware block.
 * Save the sample in time buffer if the system console FFT feature is armed or triggered.
 * Save the sample in the input buffer for soft fft processing
 *
 * @param fft		Pointer to an FFT structure
 * @param sample	The new sample
 */
void fft_sample(fft_t * fft, int sample) {
	if (fft->fft_enable) {
		IOWR_32DIRECT(fft->dspba_fft_base, D_INPUT, sample);
		IOWR_32DIRECT(fft->dspba_fft_base, DSPBA_START, 1);

		if ((fft->ping_state == FFT_WAIT) || (fft->ping_state == FFT_WAIT_TRIGGER) || (fft->ping_state == FFT_WAIT_SINK)) {
			IOWR_16DIRECT(fft->ping_input_buffer, fft->ping_samples*2, sample);  //
		}

		// Soft fft expects float inputs. Samples are Q17 format at this stage.
		fft->hps_input_buffer[fft->hps_samples64*64*2 + fft->hps_samples*2    ] = ((float)sample*SHIFTR17FLOAT)/16;	// Input Real
		fft->hps_input_buffer[fft->hps_samples64*64*2 + fft->hps_samples*2 + 1] = 0;								// Input Imag
	}
}

/**
 * Select the next sample from the specified time domain signal
 *
 * @param dp	Pointer to drive params structure for current axis
 * @param sel	Index to selected time domain sample
 * @return		The selected sample
 */
int fft_select(drive_params * dp, int sel) {
	switch (sel) {
		case 0:
			return dp->Iq;
			break;
		case 1 :
			return dp->Id;
			break;
		case 2 :
			return dp->i_command_q;
			break;
		case 3 :
			return dp->Vq;
			break;
		case 4 :
			return dp->Vd;
			break;
		case 5 :
			return dp->filt_test0 * SHIFTL10FLOAT; //Filtered Vq
			break;
		case 6 :
			return dp->filt_test1 * SHIFTL10FLOAT; //Filtered Vd
			break;
		case 7 :
			return dp->speed_encoder;
			break;
		case 8 :
			return dp->speed_command_adjusted;
			break;
		case 9 :
			return dp->pos_int;
			break;
		case 10 :
			return dp->pos_setpoint_adjusted;
			break;
		default :
			return 0;
			break;
	}
	return 0;
}

/**
 * Filter fft data with new input samples in scaled int format
 *
 * Only need to use first half (real frequencies) of FFT buffer
 *
 * @param n	void pointer to new sample buffer. Samples are expected to be in Q17
 * @param c float filter coefficient
 * @param f	float pointer to filter buffer
 */
void fft_filter_i(void * __restrict__ n, float c, float * __restrict__ f) {
	int i;
	int * nn = (int *)n;

	for (i = 0; i < FFT_LENGTH; i++) {
		f[i] = c*nn[i]*SHIFTR17FLOAT + (1 - c)*f[i];
	}
}

/**
 * Filter FFT data with new input samples in float format
 *
 * Only need to use first half (real frequencies) of FFT buffer
 *
 * @param n	void pointer to new sample buffer. Samples are expected to be in float format
 * @param c float filter coefficient
 * @param f	float pointer to filter buffer
 */
void fft_filter_f(void * __restrict__ n, float c,float * __restrict__ f) {
	int i;
	float * nn = (float *)n;

	for (i = 0; i < FFT_LENGTH; i++) {
		f[i] = c*nn[i] + (1.0f - c)*f[i];
	}
}

/**
 * Calculate magnitude squared of FFT data and dump to system console buffer
 *
 * Only need to use first half (real frequencies) of FFT buffer
 *
 * @param f	float pointer to FFT buffer
 * @param m	float pointer to magnitude buffer
 * @param dump float pointer to magnitude dump buffer
 * @param dump_en int 0 = disable dump to magnitude buffer
 */
static void fft_magsq(float * __restrict__ f, float * __restrict__ m, float * __restrict__ dump, int dump_en) {
	int i, j = 0;

	for (i = 0; i < FFT_LENGTH/2; i++) {
		m[i] = f[j]*f[j];
		j++;
		m[i] += f[j]*f[j];
		j++;
	}

	if (dump_en != 0){
		// Dump the result for system console
		for (i = 0; i < FFT_LENGTH/2; i++) {
			dump[i] = m[i]*SHIFTL17FLOAT*SHIFTL17FLOAT;

		}
	}
}

/**
 * Search for peak value in magnitude squared data
 *
 * Only need to use first half (real frequencies) of FFT buffer
 *
 * @param mag	pointer to magnitude buffer
 * @param wins	Window start - number of samples to ignore at start of buffer
 * @param wine	Window end - number of samples to ignore at end of buffer
 * @param s64	int current 64 sample fft
 * @param pk	float pointer to peak value
 * @param freq	int pointer to frequency of peak
 * @param dump_en	int 0 = disable dump
 */
void fft_max_pk(float * mag, int wins, int wine, int s64, float * pk, int * freq, void * dump, int dump_en) {
	int i;

	pk[s64] = 0;
	freq[s64] = 0;
	for (i = wins; i < FFT_LENGTH/2 - wine; i++) {
		if (mag[i] > pk[s64]) {
			pk[s64] = mag[i];
			freq[s64] = i;
		}
	}

	if (dump_en != 0){
		int j = s64; //start at current peak value
		j = (j>=63)?0:j+1; //Increment to move on to oldest peak

		for (i = 0; i < 64; i++) {
			// Dump results for system console
			((float *)dump)[i*2] = pk[j]*SHIFTL17FLOAT*SHIFTL17FLOAT;
			((int *)dump)[i*2 + 1] = freq[j];
			j = (j>=63)?0:j+1;
		}
	}

}

/**
 * Task for HPS processing of FFT data.
 *
 * Implemented as a background task to avoid blocking the hps buffer state machine update if processing
 * takes longer than one IRQ period.
 *
 * @param pdata	Task context
 */
#ifdef __DOC_SOC_UCOSII__
void hps_fft_task(void* pdata) {
    CPU_INT08U  os_err;
#else
#ifdef __DOC_SOC_VXWORKS__
int hps_fft_task(void* pdata) {
	unsigned int events;
#else
#error "Usupported OS"
#endif
#endif
	int i;
	void * dst;
	void * src;
	size_t len;
	void * filter_input;

    drive_params * my_dp = (drive_params *)get_dp();

	// Task never returns
	while (1) {
		// Wait on semaphores for both channels and clear them
#ifdef __DOC_SOC_UCOSII__
		OSFlagPend(fft_flags, FFT0_HPS_READY | FFT1_HPS_READY, OS_FLAG_WAIT_SET_ALL + OS_FLAG_CONSUME, 0, &os_err);
#else
#ifdef __DOC_SOC_VXWORKS__
		eventReceive(FFT_TASK_EV_FFT0_HPS_READY | FFT_TASK_EV_FFT1_HPS_READY, EVENTS_WAIT_ALL | EVENTS_KEEP_UNWANTED, WAIT_FOREVER, &events);
#else
#error "Usupported OS"
#endif
#endif

		// FFT algorithm cannot cope with a wrapping buffer, so we have to unwrap the input buffer,
		// which wraps around after 4096 samples, into a new buffer.
		// Each sample is 2 words (8 bytes) for real and imaginary
		if (fft_0.fft_type == FFT_SOFT) {
			// (hps_samples64 + 1) indicates the offset of the most recent samples that have been written to the
			// input buffer.
			// Input buffer from start to most recent samples is copied to the end of the fft buffer
			// Remainder of input buffer is copied to the start of the fft buffer
			// First copy older samples to start of fft buffer
			src = fft_0.hps_input_buffer + (fft_0.hps_samples64 + 0)*64*2;
			len = FFT_LENGTH*2*4 - (fft_0.hps_samples64 + 0)*64*2*4;
			dst = fft_0.hps_soft_fft_buffer;
			memcpy(dst, src, len);

			// Second copy the most recent samples from start of input buffer
			src = fft_0.hps_input_buffer;
			len = (fft_0.hps_samples64 + 0)*64*2*4;
			dst = fft_0.hps_soft_fft_buffer + FFT_LENGTH*2 - (fft_0.hps_samples64 + 0)*64*2;
			memcpy(dst, src, len);

			four1(fft_0.hps_soft_fft_buffer - 1);

			// If the system console state machine is waiting for the soft FFT result then we copy it to
			// the output buffer and update the status.
			if (fft_0.ping_state == FFT_SOFT_FFT) {
				// convert to scaled int representation as it is copied
				for (i = 0; i < FFT_LENGTH*2; i++) {
					((int *)fft_0.ping_output_buffer)[i] = (int)(fft_0.hps_soft_fft_buffer[i]*SHIFTL17FLOAT);
				}
			}
		}
		if (fft_1.fft_type == FFT_SOFT) {
			// First copy older samples to start of fft buffer
			src = fft_1.hps_input_buffer + (fft_1.hps_samples64 + 0)*64*2;
			len = FFT_LENGTH*2*4 - (fft_1.hps_samples64 + 0)*64*2*4;
			dst = fft_1.hps_soft_fft_buffer;
			memcpy(dst, src, len);

			// Second copy the most recent samples from start of input buffer
			src = fft_1.hps_input_buffer;
			len = (fft_1.hps_samples64 + 0)*64*2*4;
			dst = fft_1.hps_soft_fft_buffer + FFT_LENGTH*2 - (fft_1.hps_samples64 + 0)*64*2;
			memcpy(dst, src, len);

			four1(fft_1.hps_soft_fft_buffer - 1);

			if (fft_1.ping_state == FFT_SOFT_FFT) {
				// convert to scaled int representation as it is copied
				for (i = 0; i < FFT_LENGTH*2; i++) {
					((int *)fft_1.ping_output_buffer)[i] = (int)(fft_1.hps_soft_fft_buffer[i]*SHIFTL17FLOAT);
				}
			}
		}

		// Select input buffer for filter function and then filter FFT data using function pointer to call appropriate filter function for
		// scaled int output from hard FFT or float output from soft fft
		if (fft_0.fft_type == FFT_SOFT) {
			filter_input = (void *)fft_0.hps_soft_fft_buffer;
		} else {
			filter_input = (void *)fft_0.hps_hard_fft_buffer;
		}
		fft_0.fft_filter_fn(filter_input, fft_0.hps_filt_coeff, fft_0.hps_filt_buffer);
		if (fft_1.fft_type == FFT_SOFT) {
			filter_input = (void *)fft_1.hps_soft_fft_buffer;
		} else {
			filter_input = (void *)fft_1.hps_hard_fft_buffer;
		}
		fft_1.fft_filter_fn(filter_input, fft_1.hps_filt_coeff, fft_1.hps_filt_buffer);

		// Calculate the magnitude squared always and dump to buffer for system console if ping_state = FFT_SOFT_FFT

		fft_magsq(fft_0.hps_filt_buffer, fft_0.hps_magsq_buffer, (float *)FFTMAGSQ_BUFFER_0_PING_BASE,(fft_0.ping_state == FFT_SOFT_FFT));
		fft_magsq(fft_1.hps_filt_buffer, fft_1.hps_magsq_buffer, (float *)FFTMAGSQ_BUFFER_1_PING_BASE,(fft_1.ping_state == FFT_SOFT_FFT));

		int low_pk = my_dp[0].fft0_pk_det_lim & 0xFFFF;
		int high_pk = (my_dp[0].fft0_pk_det_lim & 0xFFFF0000) >> 16;

		// Look for the peak and associated frequency always and dump to buffer for system console if ping_state = FFT_SOFT_FFT
		fft_max_pk(fft_0.hps_magsq_buffer, low_pk, high_pk, fft_0.hps_samples64, fft_0.hps_magsq_pk_buffer, fft_0.hps_magsq_freq_buffer, (void *)FFTMAGSQ_PK_BUFFER_0_PING_BASE,(fft_0.ping_state == FFT_SOFT_FFT));
		low_pk = my_dp[0].fft1_pk_det_lim & 0xFFFF;
		high_pk = (my_dp[0].fft1_pk_det_lim & 0xFFFF0000) >> 16;
		fft_max_pk(fft_1.hps_magsq_buffer, low_pk, high_pk, fft_1.hps_samples64, fft_1.hps_magsq_pk_buffer, fft_1.hps_magsq_freq_buffer, (void *)FFTMAGSQ_PK_BUFFER_1_PING_BASE,(fft_0.ping_state == FFT_SOFT_FFT));

		//Always set ping_state after all software processing
		if (fft_0.ping_state == FFT_SOFT_FFT) {
			fft_set_buffer_status(&fft_0);
			debug_printf(DBG_DEBUG, "Soft FFT post-processing finished on FFT 0\n");
		}

		if (fft_1.ping_state == FFT_SOFT_FFT) {
			fft_set_buffer_status(&fft_1);
			debug_printf(DBG_DEBUG, "Soft FFT post-processing finished on FFT 1\n");
		}

#ifdef __DOC_SOC_UCOSII__
		OSFlagPost(fft_flags, FFT0_HPS_ACK | FFT1_HPS_ACK, OS_FLAG_SET, &os_err);
#else
#ifdef __DOC_SOC_VXWORKS__
		eventSend(driveTaskId, DRIVE_TASK_EV_FFT0_HPS_ACK | DRIVE_TASK_EV_FFT1_HPS_ACK);
#else
#error "Usupported OS"
#endif
#endif
	}
}

/*!
 * @}
 */

/*!
 * @}
 */
