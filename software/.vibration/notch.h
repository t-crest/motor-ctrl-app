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

#ifndef NOTCH_H_
#define NOTCH_H_

/**
 * @file notch.h
 *
 * @brief Header file for notch filter for vibration demo
 */

/*!
 * \addtogroup VIBRATION
 *
 * @{
 */

/*!
 * \addtogroup FILTER 2nd Order Filter
 *
 * @brief 2nd Order filter
 *
 * Functions to implement a 2nd order digital filter. The filter can be configured as
 * a notch or other filter type.
 *
 * @{
 */

/**
 * Filter configuration & coefficients structure
 */
typedef struct
{

	//################################################################################################
	// Filter configuration
	//################################################################################################
	 float Fn_Hz_f;
	 float Fd_Hz_f;
	 float zetan_ND_f;
	 float zetad_ND_f;
	 float T_s_f;

	//################################################################################################
	// Filter coefficients, pre-gain
	//################################################################################################

	 float dc_gain_f;

	 //################################################################################################
	// Filter coefficients, denominator
	//################################################################################################
 
    float a1_f;
	float a2_f;

	//################################################################################################
	// Filter coefficients, numerator
	//################################################################################################

	float b0_f;
	float b1_f;
	float b2_f;

	//################################################################################################
	// States: previous values of input
	//################################################################################################
 
	float u_k_1_f;
	float u_k_2_f;
 
	//################################################################################################
	// States: Previous values of output
	//################################################################################################
 
    float y_k_1_f;
	float y_k_2_f;

	//################################################################################################
	// Status bit: When config_valid = 1 structure is valid to use as filter
	//################################################################################################
	int config_valid;
	int config_changed;

} Type2ndOrderIIRFilterStruct;

/**
 * Filter initialisation/reset function
 * @param fPtr Filter Structure
 * @param u_k_f Reset value
 */
void initNotch(Type2ndOrderIIRFilterStruct * fPtr, float u_k_f);


/**
 * Filter update function
 *
 * @param fPtr Filter input data
 * @param u_k_f Filter Structure
 * @return Filter output data
 */
float updateNotch(Type2ndOrderIIRFilterStruct * fPtr, float u_k_f);

/**
 * Calculate notch filter internal coefficients from filter configuration values.
 * @param fPtr
 */
void calcNotchCoeffs(Type2ndOrderIIRFilterStruct * fPtr);

/**
 * This function not currently used
 * @param a1_floatPtr
 * @param a2_floatPtr
 * @param b0_floatPtr
 * @param b1_floatPtr
 * @param b2_floatPtr
 * @param invKn_ND_fPtr
 * @param invKd_ND_fPtr
 * @param inva0_ND_fPtr
 * @param Fn_Hz
 * @param Fd_Hz
 * @param zetan_ND
 * @param zetad_ND
 * @param T_s
 */void calcNotchCoeffsNoStruct(float * a1_floatPtr, float * a2_floatPtr,
                     float * b0_floatPtr, float * b1_floatPtr, float * b2_floatPtr,
                     float * invKn_ND_fPtr, float * invKd_ND_fPtr, float * inva0_ND_fPtr,
					 float Fn_Hz, float Fd_Hz, 
					 float zetan_ND, float zetad_ND, 
					 float T_s);


/**
 * This function not currently used
 * @param a1_float
 * @param a2_float
 * @param b0_float
 * @param b1_float
 * @param b2_float
 * @param u_k_float
 * @param u_k_1_float
 * @param u_k_2_float
 * @param y_k_1_float
 * @param y_k_2_float
 * @return
 */
float updateNotchNoPtr(float a1_float, float a2_float, 
                  float b0_float, float b1_float, float b2_float, 
				  float u_k_float, float u_k_1_float, float u_k_2_float, 
				  float y_k_1_float, float y_k_2_float);

/**
 * This function not currently used
 * @param y_k_floatPtr
 * @param a1_float
 * @param a2_float
 * @param b0_float
 * @param b1_float
 * @param b2_float
 * @param u_k_float
 * @param u_k_1_float
 * @param u_k_2_float
 * @param y_k_1_float
 * @param y_k_2_float
 */
void updateNotchNoStruct(float * y_k_floatPtr, 
                    float a1_float, float a2_float, 
					float b0_float, float b1_float, float b2_float, 
					float u_k_float, float u_k_1_float, float u_k_2_float, 
					float y_k_1_float, float y_k_2_float);


/**
 * Copies internal filter coefficients from a new filter structure to an active filter structure.
 * The filter coefficients in the new filter are first calculated using the calcNotchCoeffs function
 * and then copied across to the active filter. This is to avoid the updateNotch function getting called
 * while the filter coefficients are only partially calculated.
 * @param ActivefPtr Active filter structure
 * @param NewfPtr New filter coefficient structure
 */void update_filter(Type2ndOrderIIRFilterStruct * ActivefPtr, Type2ndOrderIIRFilterStruct * NewfPtr);

/*!
 * @}
 */

/*!
 * @}
 */

#endif /* NOTCH_H_ */
