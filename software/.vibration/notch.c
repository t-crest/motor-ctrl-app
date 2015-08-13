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
 * @file notch.c
 *
 * @brief Notch filter for vibration demo
 */

#include "includes.h"

/*!
 * \addtogroup VIBRATION
 *
 * @{
 */

/*!
 * \addtogroup FILTER
 *
 * @{
 */

void calcNotchCoeffsNoStruct(float * a1_ND_fPtr, float * a2_ND_fPtr,
                     float * b0_ND_fPtr, float * b1_ND_fPtr, float * b2_ND_fPtr,
                     float * invKn_ND_fPtr, float * invKd_ND_fPtr, float * inva0_ND_fPtr,
					 float Fn_Hz_f, float Fd_Hz_f, 
					 float zetan_ND_f, float zetad_ND_f, 
					 float T_s_f){


float invKn_ND_f = 0.0;
float invKd_ND_f = 0.0;

float inva0_ND_f = 0.0;
float PI = 3.1415926 ;

// Calculate 'bilinear with pre-warping' constants used in conversion from
// continous time to discrete time

// tan(0.5*Omegan_rad_s*T_s) = tan(pi*Fn_Hz*T_s)
// Calculate reciprocals to avoid divides in following calculations.
// tanf exists in Nios math.h for 'float'. However, tanf does not seem to exist in lcc math.h for MATLAB
// invKn_ND_f = 1/tanf(M_PI*Fn_Hz_f*T_s_f); 
// invKd_ND_f = 1/tanf(M_PI*Fd_Hz_f*T_s_f); 
// For compatibility  with MATLAB/lcc, convert to double, use tan, and convert back to float:
invKn_ND_f = 1.0f/((float)(tan((double)(PI*Fn_Hz_f*T_s_f)))); // tanf is for 'float', tan for 'double'
invKd_ND_f = 1.0f/((float)(tan((double)(PI*Fd_Hz_f*T_s_f)))); // tanf is for 'float', tan for 'double'

// Calculate filter coefficients. a0 is just used to normalise others
inva0_ND_f = 1.0f/(1.0f + 2.0f*zetad_ND_f*invKd_ND_f + invKd_ND_f*invKd_ND_f);
// inva0_ND_f = 1.0f + 2.0f*zetad_ND_f*invKd_ND_f + invKd_ND_f*invKd_ND_f;
// inva0_ND_f = (double)(1.0)/(double)(1.0f + 2.0f*zetad_ND_f*invKd_ND_f + invKd_ND_f*invKd_ND_f);

* a1_ND_fPtr = inva0_ND_f*2*(1 - invKd_ND_f*invKd_ND_f); //a1
* a2_ND_fPtr = inva0_ND_f*(1 - 2*zetad_ND_f*invKd_ND_f + invKd_ND_f*invKd_ND_f); //a2
* b0_ND_fPtr = inva0_ND_f*(1 + 2*zetan_ND_f*invKn_ND_f + invKn_ND_f*invKn_ND_f); //b0
* b1_ND_fPtr = inva0_ND_f*2*(1 - invKn_ND_f*invKn_ND_f); //b1
* b2_ND_fPtr = inva0_ND_f*(1 - 2*zetan_ND_f*invKn_ND_f + invKn_ND_f*invKn_ND_f); //b2

// Send these back for debugging:
* invKn_ND_fPtr = invKn_ND_f;
* invKd_ND_fPtr = invKd_ND_f;
* inva0_ND_fPtr = inva0_ND_f;
}


// This function not currently used
float updateNotchNoPtr(float a1_float, float a2_float, 
                  float b0_float, float b1_float, float b2_float, 
				  float u_k_float, float u_k_1_float, float u_k_2_float, 
				  float y_k_1_float, float y_k_2_float){

    // Notch filter is implemented as a 2nd-order IIR filter
    // Implement y[k] = b0*u[k] + b1*u[k-1] + b2*u[k-2] - a1*y[k-1] - a2*y[k-2]
				
    return (b0_float*u_k_float + b1_float*u_k_1_float + b2_float*u_k_2_float - a1_float*y_k_1_float - a2_float*y_k_2_float);
}

void updateNotchNoStruct(float * y_k_floatPtr, 
                    float a1_float, float a2_float, 
					float b0_float, float b1_float, float b2_float, 
					float u_k_float, float u_k_1_float, float u_k_2_float, 
					float y_k_1_float, float y_k_2_float){

    // Notch filter is implemented as a 2nd-order IIR filter
    // Implement y[k] = b0*u[k] + b1*u[k-1] + b2*u[k-2] - a1*y[k-1] - a2*y[k-2]
				
    * y_k_floatPtr = b0_float*u_k_float + b1_float*u_k_1_float + b2_float*u_k_2_float - a1_float*y_k_1_float - a2_float*y_k_2_float;
}

float updateNotch(Type2ndOrderIIRFilterStruct * fPtr, float u_k_f){
    
    // Notch filter is implemented as a 2nd-order IIR filter
    // Implement y[k] = b0*u[k] + b1*u[k-1] + b2*u[k-2] - a1*y[k-1] - a2*y[k-2]
	
    float y_k_f = 0.0f; //Local variable for temporary storage of output
    float u_k_post_gain_f = u_k_f * fPtr->dc_gain_f; //Add pre-gain stage to input

    //If the filter configuration has changed then reset the filter, otherwise execute the filter
    if (fPtr->config_changed == 1) {
    	initNotch(fPtr, u_k_f);
    	fPtr->config_changed = 0;
    	return u_k_f;
    } else {

		y_k_f = fPtr->b0_f * u_k_post_gain_f +
				fPtr->b1_f * fPtr->u_k_1_f +
				fPtr->b2_f * fPtr->u_k_2_f -
				fPtr->a1_f * fPtr->y_k_1_f -
				fPtr->a2_f * fPtr->y_k_2_f;

		// Update states in struct with latest inputs and outputs
		fPtr->u_k_2_f = fPtr->u_k_1_f;
		fPtr->u_k_1_f = u_k_post_gain_f;
		fPtr->y_k_2_f = fPtr->y_k_1_f;
		fPtr->y_k_1_f = y_k_f;
		return y_k_f;
    }
	
}

void initNotch(Type2ndOrderIIRFilterStruct * fPtr, float u_k_f) {

	//Initiate notch filter whenever filter settings change or filter is disabled
	fPtr->config_valid = 0;
	fPtr->u_k_1_f = u_k_f * fPtr->dc_gain_f; //Add pre-gain stage to input;
	fPtr->u_k_2_f = u_k_f * fPtr->dc_gain_f; //Add pre-gain stage to input;
	fPtr->y_k_1_f = u_k_f * fPtr->dc_gain_f; //Add pre-gain stage to input;
	fPtr->y_k_2_f = u_k_f * fPtr->dc_gain_f; //Add pre-gain stage to input;
	fPtr->config_valid = 1;

}

void calcNotchCoeffs(Type2ndOrderIIRFilterStruct * fPtr) {
	float invKn_ND_f = 0.0;
	float invKd_ND_f = 0.0;

	float inva0_ND_f = 0.0;
	float PI = 3.1415926 ;

	//unset config_valid before updating the filter parameters
	fPtr->config_valid = 0;
	// Calculate 'bilinear with pre-warping' constants used in conversion from
	// continous time to discrete time

	// tan(0.5*Omegan_rad_s*T_s) = tan(pi*Fn_Hz*T_s)
	// Calculate reciprocals to avoid divides in following calculations.
	// tanf exists in Nios math.h for 'float'. However, tanf does not seem to exist in lcc math.h for MATLAB
	// invKn_ND_f = 1/tanf(M_PI*Fn_Hz_f*T_s_f);
	// invKd_ND_f = 1/tanf(M_PI*Fd_Hz_f*T_s_f);
	// For compatibility  with MATLAB/lcc, convert to double, use tan, and convert back to float:
	invKn_ND_f = 1.0f/((float)(tan((double)(PI*fPtr->Fn_Hz_f*fPtr->T_s_f)))); // tanf is for 'float', tan for 'double'
	invKd_ND_f = 1.0f/((float)(tan((double)(PI*fPtr->Fd_Hz_f*fPtr->T_s_f)))); // tanf is for 'float', tan for 'double'

	// Calculate filter coefficients. a0 is just used to normalise others
	inva0_ND_f = 1.0f/(1.0f + 2.0f*fPtr->zetad_ND_f*invKd_ND_f + invKd_ND_f*invKd_ND_f);
	// inva0_ND_f = 1.0f + 2.0f*zetad_ND_f*invKd_ND_f + invKd_ND_f*invKd_ND_f;
	// inva0_ND_f = (double)(1.0)/(double)(1.0f + 2.0f*zetad_ND_f*invKd_ND_f + invKd_ND_f*invKd_ND_f);

	fPtr->a1_f = inva0_ND_f*2*(1 - invKd_ND_f*invKd_ND_f); //a1
	fPtr->a2_f = inva0_ND_f*(1 - 2*fPtr->zetad_ND_f*invKd_ND_f + invKd_ND_f*invKd_ND_f); //a2
	fPtr->b0_f = inva0_ND_f*(1 + 2*fPtr->zetan_ND_f*invKn_ND_f + invKn_ND_f*invKn_ND_f); //b0
	fPtr->b1_f = inva0_ND_f*2*(1 - invKn_ND_f*invKn_ND_f); //b1
	fPtr->b2_f = inva0_ND_f*(1 - 2*fPtr->zetan_ND_f*invKn_ND_f + invKn_ND_f*invKn_ND_f); //b2

	// Send these back for debugging:
	//* invKn_ND_fPtr = invKn_ND_f;
	//* invKd_ND_fPtr = invKd_ND_f;
	//* inva0_ND_fPtr = inva0_ND_f;

	//set config_valid after updating the filter parameters
	fPtr->config_valid = 1;

}


void update_filter(Type2ndOrderIIRFilterStruct * ActivefPtr, Type2ndOrderIIRFilterStruct * NewfPtr) {

	//Use the config_valid flag to make sure filter coefficients are valid before copying (i.e. not interrupted partially calculating)
	//Copy over Active filter state variables

	if (NewfPtr->config_valid == 1) {
		ActivefPtr->Fn_Hz_f = NewfPtr->Fn_Hz_f;
		ActivefPtr->Fd_Hz_f = NewfPtr->Fd_Hz_f;
		ActivefPtr->zetan_ND_f = NewfPtr->zetan_ND_f;
		ActivefPtr->zetad_ND_f = NewfPtr->zetad_ND_f;
		ActivefPtr->T_s_f = NewfPtr->T_s_f;
		//Check if filter has been updated, and set the config_changed bit so that filter is reset during the next update
		if ((ActivefPtr->a1_f != NewfPtr->a1_f) ||
			(ActivefPtr->a2_f != NewfPtr->a2_f) ||
			(ActivefPtr->b0_f != NewfPtr->b0_f) ||
			(ActivefPtr->b1_f != NewfPtr->b1_f) ||
			(ActivefPtr->b2_f != NewfPtr->b2_f) ||
			(ActivefPtr->dc_gain_f != NewfPtr->dc_gain_f)
		) {
			ActivefPtr->config_changed = 1;
		} else {
			ActivefPtr->config_changed = 0;
		}

		ActivefPtr->dc_gain_f = NewfPtr->dc_gain_f;
		ActivefPtr->a1_f = NewfPtr->a1_f;
		ActivefPtr->a2_f = NewfPtr->a2_f;
		ActivefPtr->b0_f = NewfPtr->b0_f;
		ActivefPtr->b1_f = NewfPtr->b1_f;
		ActivefPtr->b2_f = NewfPtr->b2_f;

	};


}

/*!
 * @}
 */

/*!
 * @}
 */
