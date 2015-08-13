/******************************************************************************
*                                                                             *
* License Agreement                                                           *
*                                                                             *
* Copyright (c) 2013 Altera Corporation, San Jose, California, USA.           *
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

#ifndef WAVEFORM_H_
#define WAVEFORM_H_

/**
 * @file waveform.h
 *
 * @brief Header file for waveform generation for vibration demo
 */

#include "includes.h"

/*!
 * \addtogroup VIBRATION Vibration suppression
 *
 * @brief Functions to implement the vibration suppression demo
 *
 * @{
 */

/*!
 * \addtogroup WAVEFORM Waveform Generator
 *
 * @brief Waveform generator.
 *
 * Functions to generate a periodic test waveform that can be applied to the motor control system.
 *
 * @{
 */


/**
 * Declare the waveform shape - Sine, Triangle, Square or Sawtooth
 */
typedef enum {
	 SINEWAVE=0, 		//!< Sine wave
	 TRIANGLEWAVE=1,	//!< Triangle wave
	 SQUAREWAVE=2, 		//!< Square wave
	 SAWTOOTHWAVE=3		//!< Sawtooth wave
} TypeWaveshape;

/**
 * Waveform definition and state structure
 */
typedef struct
{
	//################################################################################################
	// Waveform parameters and instantaneous values
	//################################################################################################

    TypeWaveshape Shape_enum;
    int Period_int32; // Waveform period in 16kHz samples
    int Offset_int32; // Waveform offset in 16kHz samples
    float Amp_ND_f;   // Waveform amplitude, non-dimensional (to be multiplied by fullscale and units)
    int count_int32;  // Instantaneous count value, will increment from zero to (Period_int32-1)
    float value_ND_f;    // Waveform instantaneous output value.
    bool reset_Bool; // If set TRUE, will reset count_int32 to zero on next updateWaveform()
            
} TypeWaveformStruct;

/**
 * Initialise/reset the waveform state
 * @param wPtr Waveform structure
 */
void initWaveform(TypeWaveformStruct * wPtr);

/**
 * Generates the next value in the waveform
 * @param fPtr Waveform structure
 * @return Next waveform value
 */
float updateWaveform(TypeWaveformStruct * fPtr);

/*!
 * @}
 */

/*!
 * @}
 */

#endif /* WAVEFORM_H_ */
