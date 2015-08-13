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
 * @file dspba_fft.h
 *
 * @brief Header file for FFT function in vibration reference design
 */

#ifndef DSPBA_FFT_H_
#define DSPBA_FFT_H_

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


/**
 * Define states for buffering FFT data for upload to system console
 */
typedef enum fft_state_e {FFT_RESET, FFT_WAIT, FFT_WAIT_TRIGGER, FFT_WAIT_SINK, FFT_SOFT_FFT, FFT_WAIT_FFT, FFT_WAIT_DMA, FFT_WAIT_UPLOAD, FFT_ERROR} fft_state_t;
typedef enum fft_type_e {FFT_HARD, FFT_SOFT} fft_type_t;

/**
 * struct to hold FFT channel state and other info
 */
typedef struct {
	unsigned int				fft_enable;				// Enable this axis
	unsigned int				fft_axis;				// Motor axis that this FFT instance relates to
	unsigned int				fft_instance;			// Which FFT instance for an axis
	fft_type_t					fft_type;				// hard or sof

	fft_state_t					ping_state;				// Current state of ping buffer for system console
	unsigned int				ping_input_buffer;		// Pointer to ping time domain buffer for system console
	unsigned int				ping_output_buffer;		// Pointer to ping buffer for system console
	int							ping_samples;			// Number of samples accumulated

	fft_state_t					hps_state;				// Current state of HPS FFT
	int							hps_samples;			// Count of hps fft samples 0 - 64
	int							hps_samples64;			// Count of hps fft 64 sample blocks

	unsigned int				fft_trigger;			// Trigger input to FFT

	float *						hps_input_buffer;		// Pointer to input samples for soft fft
	float *						hps_soft_fft_buffer;	// Pointer to unwrapped input samples for soft fft, may also be used as
														// output buffer by some fft algorithms

	void *						hps_hard_fft_buffer;	// Pointer to hard fft output buffer for HPS processing
	float						hps_filt_coeff;			// Filter coefficient
	void (*						fft_filter_fn)(void * __restrict__ n, float c, float * __restrict__ f);
	float *						hps_filt_buffer;		// Pointer to buffer for HPS filtered fft
	float *						hps_magsq_buffer;		// Pointer to magnitude squared samples
	float *						hps_magsq_pk_buffer;	// Pointer to peak values from most recent FFT
	int	*						hps_magsq_freq_buffer;	// Pointer to frequency of peak value from most recent FFT

	ALT_DMA_CHANNEL_t			ping_channel;			// DMA channel allocated to ping buffer for system console
	ALT_DMA_CHANNEL_STATE_t		ping_chan_state;		// DMA channel state for ping buffer for system console
	ALT_DMA_PROGRAM_t *			ping_program;			// DMA controller program for ping buffer for system console

	ALT_DMA_CHANNEL_t			hps_channel;			// DMA channel allocated to hps buffer
	ALT_DMA_CHANNEL_STATE_t		hps_chan_state;			// DMA channel state for hps buffer
	ALT_DMA_PROGRAM_t *			hps_program;			// DMA controller program for hps buffer

	unsigned int				dspba_fft_state;		// State of DSPBA FFT Hardware
	unsigned int				fft_hps_busy_edge;		// Signal to HPS state machine
	unsigned int				fft_ping_sys_busy_edge;		// Signal to system console state machine
	unsigned int				dspba_fft_base;			// Base address of DSPBA hardware FFT for this instance
	unsigned int				dspba_fft_buffer_base;	// Base address of DSPBA hardware FFT output buffer for this instance
} fft_t;

extern fft_t fft_0, fft_1;

	void fft_update_status(fft_t * fft);
	void fft_update_hps_state(fft_t * fft);
	void fft_update_sys_state(fft_t * fft);
	void fft_sample(fft_t * fft, int sample);
	int fft_select(drive_params * dp, int sel);
	void fft_dump(fft_t * fft);
	void fft_init(void);

/*
 * Arrays to hold filtered real and imaginary and magnitude squared values
 */
extern float fft_0_filt[FFT_LENGTH*2] __attribute__ ((aligned (32)));
extern float fft_1_filt[FFT_LENGTH*2] __attribute__ ((aligned (32)));
extern float fft_0_magsq[FFT_LENGTH] __attribute__ ((aligned (32)));
extern float fft_1_magsq[FFT_LENGTH] __attribute__ ((aligned (32)));
extern float fft_0_input[FFT_LENGTH*2] __attribute__ ((aligned (32)));
extern float fft_1_input[FFT_LENGTH*2] __attribute__ ((aligned (32)));
extern float fft_0_fft[FFT_LENGTH*2] __attribute__ ((aligned (32)));
extern float fft_1_fft[FFT_LENGTH*2] __attribute__ ((aligned (32)));
extern float fft_0_magsq_pk[FFT_LENGTH/64] __attribute__ ((aligned (32)));
extern float fft_1_magsq_pk[FFT_LENGTH/64] __attribute__ ((aligned (32)));
extern int fft_0_magsq_freq[FFT_LENGTH/64] __attribute__ ((aligned (32)));
extern int fft_1_magsq_freq[FFT_LENGTH/64] __attribute__ ((aligned (32)));

extern void four1(float data[]);

#ifdef __DOC_SOC_UCOSII__
/**
 * FFT flags
 */
#define FFT_HPS_STATE_ERROR			0x1
#define FFT_SYS_PING_STATE_ERROR	0x2
#define FFT0_HPS_READY				0x8
#define FFT1_HPS_READY				0x10
#define FFT0_HPS_ACK				0x20
#define FFT1_HPS_ACK				0x40
#else
#ifdef __DOC_SOC_VXWORKS__
/**
 * hps_fft_task events
 */
#define FFT_TASK_EV_FFT0_HPS_READY		VXEV01
#define FFT_TASK_EV_FFT1_HPS_READY		VXEV02

/**
 * drive task events
 */
#define DRIVE_TASK_EV_WAKE				VXEV01
#define DRIVE_TASK_EV_FFT0_HPS_ACK		VXEV02
#define DRIVE_TASK_EV_FFT1_HPS_ACK		VXEV03
#else
#error "Unsupported OS"
#endif
#endif

/*!
 * @}
 */

/*!
 * @}
 */

#endif /* DSPBA_FFT_H_ */
