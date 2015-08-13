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

/**
 * @file waveform.c
 *
 * @brief Waveform generation for vibration demo
 */

#include "includes.h"

/*!
 * \addtogroup VIBRATION
 *
 * @{
 */

/*!
 * \addtogroup WAVEFORM
 *
 * @{
 */

void initWaveform(TypeWaveformStruct * wPtr){
	wPtr->Shape_enum=SQUAREWAVE;
	wPtr->Period_int32=0; // Waveform period in 16kHz samples
	wPtr->Offset_int32= 0; // Waveform offset in 16kHz samples
	wPtr->Amp_ND_f=0.0f;   // Waveform amplitude, non-dimensional (to be multiplied by fullscale and units)
	wPtr->count_int32=0;  // Instantaneous count value, will increment from zero to (Period_int32-1)
	wPtr->value_ND_f=0.0f;    // Waveform instantaneous output value.
	wPtr->reset_Bool=false; // If set TRUE, will reset count_int32 to zero on next updateWaveform()
}


float updateWaveform(TypeWaveformStruct * wPtr){
    
    const float  PI_f = 3.1415926 ;
    
    
    int countAfterOffset_int32 = 0;
    int countMinusOffset_int32 = 0;
    int QtrPeriod_int32 = 0;
    int HalfPeriod_int32 = 0;
    int ThreeQtrPeriod_int32 = 0;
    int countPlusMinusHalfPeriod_int32 = 0;
    int countPlusMinusQtrPeriod_int32 = 0;
    
    float invPeriod_f = 0.0f;
    float waveformValueNorm_ND_f = 0.0f;
    
    // Reset counter if requested:
    if (wPtr->reset_Bool==true)
    {
        wPtr->count_int32=0;
    }
    
    // Update countAfterOffset (creates positive count after subtracting offset):
    countMinusOffset_int32 = wPtr->count_int32 - wPtr->Offset_int32;
    if (countMinusOffset_int32 < 0)
    {
        countAfterOffset_int32 = countMinusOffset_int32 + wPtr->Period_int32;
    }
    else
    {
        countAfterOffset_int32 = countMinusOffset_int32;
    }
    
    // Update counter for the next cycle:
    if ( wPtr->count_int32 >= (wPtr->Period_int32-1))
    {
        wPtr->count_int32=0;
    }
    else
    {
        wPtr->count_int32++;
    }
    
    // For convenience, calculate fractions of a period:
    // Note: could use shifts rather than divides, but safer not to use 
    // shifts with signed ints. Compilers can apply shifts as an 
    // optimization if OK to do so. 
    HalfPeriod_int32 = wPtr->Period_int32 / (int)(2);
    QtrPeriod_int32  = HalfPeriod_int32 / (int)(2);
    ThreeQtrPeriod_int32 = HalfPeriod_int32 + QtrPeriod_int32;
    
    // ...and to reduce the number of divides, do single divide in advance:
    invPeriod_f = 1.0f / (float)(wPtr->Period_int32);
    
    switch(wPtr->Shape_enum)
    {
        case SAWTOOTHWAVE:
            if (countAfterOffset_int32 < HalfPeriod_int32)
            {
                countPlusMinusHalfPeriod_int32 = countAfterOffset_int32;
            }
            else
            {
                countPlusMinusHalfPeriod_int32 = countAfterOffset_int32 - wPtr->Period_int32;
            }
            // Multiply by 2 to scale to full
            // period and multiply by invPeriod to create signal with
            // plus/minus amplitude of 1:
            waveformValueNorm_ND_f = (float)((int)(2)*countPlusMinusHalfPeriod_int32) * invPeriod_f;
            break;
        case SQUAREWAVE:
            if (countAfterOffset_int32 < HalfPeriod_int32)
            {
                waveformValueNorm_ND_f = 1.0f;
            }
            else
            {
                waveformValueNorm_ND_f = -1.0f;
            }
            break;
        case TRIANGLEWAVE:
            if (countAfterOffset_int32 < QtrPeriod_int32)
            {
                countPlusMinusQtrPeriod_int32 = countAfterOffset_int32;
            }
            else
            {
                if (countAfterOffset_int32 < ThreeQtrPeriod_int32)
                {
                    countPlusMinusQtrPeriod_int32 = HalfPeriod_int32 - countAfterOffset_int32;
                }
                else
                {
                    countPlusMinusQtrPeriod_int32 = countAfterOffset_int32 - wPtr->Period_int32;
                }
            }
            // Multiply countPlusMinusHalfPeriod_int32 by 4 to scale to full
            // period and multiply by invPeriod to create signal with
            // plus/minus amplitude of 1:
            waveformValueNorm_ND_f = (float)((int)(4)*countPlusMinusQtrPeriod_int32) * invPeriod_f;
            break;
        default: //SINEWAVE or default case if Shape_enum not understood
            waveformValueNorm_ND_f = sin((double)((((float)(countAfterOffset_int32)) * invPeriod_f) * 2.0f * PI_f));
            break;
    }
    
    // Scale to requested amplitude
    wPtr->value_ND_f = waveformValueNorm_ND_f * wPtr->Amp_ND_f;
    return wPtr->value_ND_f;
        
}

/*!
 * @}
 */
/*!
 * @}
 */
