/************************************************
* FFT code from the book Numerical Recipes in C *
* Visit www.nr.com for the licence.             *
************************************************/

#include "includes.h"

/**
 * @file fft.c
 *
 * @brief Place holder for software fft
 * 
 * Software FFT is not supported in this release of the motor control reference design.
 */

/*!
 * \addtogroup VIBRATION
 *
 * @{
 */


/*!
 * \addtogroup FFT FFT Processing
 *
 * @brief Software FFT algorithm
 *
 * Software FFT is not supported in this release of the motor control reference design.
 *
 * See pages 507-508 of the book "Numerical Recipes in C" for an algorithm to start from
 *
 * @{
 */
// Changed to float data type, always 4096 data, always forward transform
#define PI	3.14159265359	/* pi to machine precision, defined in math.h */
#define TWOPI	(2.0*PI)

#define nn 4096
#define isign -1
/*
 FFT/IFFT routine. (see pages 507-508 of Numerical Recipes in C)
*/
void four1(float data[]) {

}

/*!
 * @}
 */
/*!
 * @}
 */
