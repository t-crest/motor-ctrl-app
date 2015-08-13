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

#include "includes.h"

/**
 * @file endat.c
 *
 * @brief EnDat encoder interface
 */

/*!
 * \addtogroup COMPONENTS
 *
 * @{
 */

/*!
 * \addtogroup ENDAT EnDat Encoder interface
 *
 * @brief Interface to EnDat encoders
 *
 * @{
 */

//
// Application specific initialisation of EnDat interface and encoder
//
// Adapter from Appendix A9 "Procedure After Power-On" of "EnDat Interface
// VERSION 2.2 Bidirectional Synchronous-Serial Interface for Position Encoders"
// D297403-03-B-02
//
void EnDat_Online(drive_params * dp) {
	unsigned int cfg;

	unsigned int base_addr = dp->DOC_ENDAT_BASE_ADDR;

	// Configure the interface with 8.333MHz clock and delay compensation
	cfg = ENDAT_CFG1_ENDAT | ENDAT_CFG1_FSYS_50 | ENDAT_CFG1_COMP
			| ENDAT_CFG1_AUTOREST_OFF | ENDAT_CFG1_RESWIN_OFF | ENDAT_CFG1_LENGTH(dp->encoder_length)
			| ENDAT_CFG1_FTCLK(0);
	EnDat_Write(base_addr, ENDAT_KONF1_ADR, cfg);
	OSTimeDlyHMSM(0, 0, 0, 15);

	// Revert to position info
	EnDat_Enc_Tx_Pos(base_addr);

	// Enable hardware strobe
	cfg = EnDat_Read(base_addr, ENDAT_KONF1_ADR) | ENDAT_CFG1_HWSTR_ON;
	EnDat_Write(base_addr, ENDAT_KONF1_ADR, cfg);

	cfg = ENDAT_CFG2_HWSDEL_OFF
			| ENDAT_CFG2_FILTER_OFF | ENDAT_CFG2_TST_1
			| ENDAT_CFG2_WDOG_OFF
			| ENDAT_CFG2_TIMER_OFF;
	EnDat_Write(base_addr, ENDAT_KONF2_ADR, cfg);
}

//
// Application specific initialisation of EnDat interface and encoder
//
// Adapter from Appendix A9 "Procedure After Power-On" of "EnDat Interface
// VERSION 2.2 Bidirectional Synchronous-Serial Interface for Position Encoders"
// D297403-03-B-02
//
int EnDat_Init(drive_params * dp) {
	unsigned int cfg;

	unsigned int base_addr = dp->DOC_ENDAT_BASE_ADDR;

	// Initialise config register 1 for mode comands at 200KHz
	cfg = ENDAT_CFG1_ENDAT | ENDAT_CFG1_FSYS_50
			| ENDAT_CFG1_AUTOREST_OFF | ENDAT_CFG1_RESWIN_OFF
			| ENDAT_CFG1_FTCLK(0xE);
	EnDat_Write(base_addr, ENDAT_KONF1_ADR, cfg);
	OSTimeDlyHMSM(0, 0, 0, 15);

	// Reset the encoder
	EnDat_Enc_Reset(base_addr);

	// Read and clear errors and warnings
	EnDat_Read_Warnings(dp);
	EnDat_Read_Errors(dp);
	EnDat_Clear_Warnings(dp);
	EnDat_Clear_Errors(dp);

	// Get sensor data
	EnDat_Read_Sensor(dp);

	EnDat_Online(dp);

	return 0;
}

//
// Read encoder warnings and save in application specific way
//
void EnDat_Read_Warnings(drive_params * dp) {
	unsigned int base_addr = dp->DOC_ENDAT_BASE_ADDR;

	dp->sensor_warning_bits = EnDat_Enc_Rd_Sensor_Warning_Data(base_addr);
}

//
// Read encoder errors and save in application specific way
//
void EnDat_Read_Errors(drive_params * dp) {
	unsigned int base_addr = dp->DOC_ENDAT_BASE_ADDR;

	dp->sensor_error_bits = EnDat_Enc_Rd_Sensor_Error_Data(base_addr);
}

//
// Clear encoder warnings in application specific way
//
void EnDat_Clear_Warnings(drive_params * dp) {
	unsigned int base_addr = dp->DOC_ENDAT_BASE_ADDR;

	if ((dp->sensor_warning_bits & 7) != 0) {
		EnDat_Enc_Clear_Sensor_Warnings(base_addr);
	}
}

//
// Clear encoder errors in application specific way
//
unsigned int EnDat_Clear_Errors(drive_params * dp) {
	unsigned int base_addr = dp->DOC_ENDAT_BASE_ADDR;

	if (dp->sensor_error_bits != 0) {
		EnDat_Enc_Clear_Sensor_Warnings(base_addr);
		dp->sensor_error_bits = EnDat_Enc_Rd_Sensor_Error_Data(base_addr);
		if (dp->sensor_error_bits != 0) {
			return 1;
		}
	}
	return 0;
}

//
// Read sensor data
//
void EnDat_Read_Sensor(drive_params * dp) {
	int mphase_tmp;
	int multiturn_temp = 1;
	unsigned int base_addr = dp->DOC_ENDAT_BASE_ADDR;
	dp->encoder_multiturn_bits = 0;

	EnDat_Enc_Parameter_Read(base_addr, 0xA1, 0x0D, &(dp->encoder_length));
	dp->encoder_length &= 0xff;
	EnDat_Enc_Parameter_Read(base_addr, 0xA3, 0x1, &(dp->encoder_multiturn));
	dp->encoder_multiturn &= 0xffff;
	EnDat_Enc_Parameter_Read(base_addr, 0xA1, 0x8, &(dp->encoder_version));
	dp->encoder_version &= 0xff;
	EnDat_Enc_Parameter_Read(base_addr, 0xAD, 0x01, &mphase_tmp);
	dp->mphase = (short)(mphase_tmp & 0xffff);

	if (dp->encoder_multiturn > 0) {
		while (multiturn_temp < dp->encoder_multiturn) {
			dp->encoder_multiturn_bits++;
			multiturn_temp = (1<<dp->encoder_multiturn_bits) + 1;
		}
	}

	dp->encoder_mask = (1<<(dp->encoder_length - dp->encoder_multiturn_bits)) - 1;
	dp->encoder_turns_mask = (1<<dp->encoder_multiturn_bits) -1;
	dp->encoder_singleturn_bits = dp->encoder_length - dp->encoder_multiturn_bits;
	dp->st_shift_phi_m = (dp->encoder_singleturn_bits - 16);			// 7 for 23 bit position
}

//
// EnDat position read
//
void EnDat_Read_Position( drive_params * dp) {
	int delta_phi;
	unsigned int positionH = 0;
	unsigned int positionL;
	unsigned int base_addr = dp->DOC_ENDAT_BASE_ADDR;
	// Optimization for single turn encoders
	if (dp->encoder_multiturn > 0) {
		positionH = EnDat_Read(base_addr, ENDAT_EMPF1H_ADR);
	}
	positionL = EnDat_Read(base_addr, ENDAT_EMPF1_ADR);
	EnDat_Write(base_addr, ENDAT_STATUS_ADR, 0xFFFFFFFF);	// Acknowledge read of position data

	if (dp->encoder_multiturn > 0) {
		dp->enc_turns = (positionH<<(32 - dp->encoder_singleturn_bits) | positionL>>dp->encoder_singleturn_bits)
							& dp->encoder_turns_mask;
	}
	// Adjust for actual 23 bit position data, masking the data bits
	dp->enc_data = positionL & dp->encoder_mask;
	delta_phi = dp->enc_data - dp->enc_data_prev;
	if (delta_phi > 0x3fffff) {
		delta_phi -= 0x7fffff;
	} else if (delta_phi < -0x3fffff) {
		delta_phi += 0x7fffff;
	}
	dp->enc_data_prev = dp->enc_data;
	// Position difference is 23 bits representing one turn.
	// v = (phi1 - phi0)/2^^23/(t1 - t0)
	// Result in rpm, so v = (phi1 - phi0)/2^^23/(t1 - t0)*60
	// ==>v = delta_phi*k ==> v = delta_phi*(1/8388608)*16000*60
	// ==>v = delta_phi**0.1144
	dp->speed_encoder = (1875*delta_phi)>>(14-SPEED_FRAC_BITS);
	dp->phi_mech = (dp->enc_data >> dp->st_shift_phi_m) & 0xffff;
}

//
// EnDat encoder offset calibration
//
void EnDat_Service(drive_params * dp) {

	unsigned int base_addr = dp->DOC_ENDAT_BASE_ADDR;

    int    index      = 0;
    int    id_phase   = 0;
    int    write_cnt  = 0;
    int e_phase_read  = 0;
    int e_phase_write = 0;
    int EnDat_Value;

    dp->enable_drive  = 0 ;
    dp->mpoles        = 0 ;
    dp->mphase        = 0 ;
    dp->phi_elec      = 0 ;
    dp->i_command_q   = 0 ;
    dp->i_command_d   = 700;

    EnDat_Enc_Parameter_Read(base_addr, 0xA5, 0x5, &EnDat_Value);
	if ( EnDat_Value==0x00A9 ) {
		 debug_printf(DBG_INFO, "---> ------------------------------------\n");
		 debug_printf(DBG_INFO, "     EnDat Version 2.2\n") ;
	} else if( EnDat_Value==0x00FF ) {
		 debug_printf(DBG_INFO, "---> ------------------------------------\n");
		 debug_printf(DBG_INFO, "     EnDat Version 2.1\n") ;
	} else {
		debug_printf(DBG_WARN, "---> ------------------------------------\n");
		debug_printf(DBG_WARN, "     Unknown EnDat Version  %i \n", EnDat_Value) ;
	}

    EnDat_Enc_Parameter_Read(base_addr, 0xAD, 0x1, &e_phase_read);
    debug_printf(DBG_INFO, "     Current commutation Angle  : %X\n",(short)e_phase_read);
	OSTimeDlyHMSM(0, 0, 0, 100);

    EnDat_Enc_Parameter_Read(base_addr, 0xA1, 0xD, &(dp->encoder_length));
    dp->encoder_length &= 0xff ;
    debug_printf(DBG_INFO, "---> ------------------------------------\n");
    debug_printf(DBG_INFO, "---> STATE %i of Motor ( Motor OFF ) \n",IORD_16DIRECT(dp->DOC_SM_BASE_ADDR, SM_STATUS)>> 9);

	OSTimeDlyHMSM(0, 0, 0, 100);
    IOWR_16DIRECT(dp->DOC_ADC_BASE_ADDR, ADC_D, 1);
    IOWR_16DIRECT(dp->DOC_SM_BASE_ADDR, SM_CONTROL , 1 ) ; // Pre-Charge  State
    debug_printf(DBG_INFO, "---> STATE %i of Motor \n",IORD_16DIRECT(dp->DOC_SM_BASE_ADDR, SM_STATUS)>> 9);
	OSTimeDlyHMSM(0, 0, 0, 100);
    IOWR_16DIRECT(dp->DOC_SM_BASE_ADDR, SM_CONTROL , 2 ) ;  // Pre-Run  State
    debug_printf(DBG_INFO, "---> STATE %i of Motor \n",IORD_16DIRECT(dp->DOC_SM_BASE_ADDR, SM_STATUS)>> 9);
	OSTimeDlyHMSM(0, 0, 0, 100);
    dp->enable_drive = 1 ;
    IOWR_16DIRECT(dp->DOC_SM_BASE_ADDR, SM_CONTROL , 3 ) ;  // Run - State
    debug_printf(DBG_INFO, "---> STATE %i of Motor ( Motor ON ) \n",IORD_16DIRECT(dp->DOC_SM_BASE_ADDR, SM_STATUS)>> 9);
    debug_printf(DBG_INFO, "---> Measurement and calculation of angle\n");

	OSTimeDlyHMSM(0, 0, 2/2, 0);
    EnDat_Online(dp);
	OSTimeDlyHMSM(0, 0, 0, 1);

    // Average encoder reading over 40ms to calculate commutation angle
    id_phase = 0;
    for( index=0 ; index<4096 ; index++ ){
        id_phase  += dp->phi_mech ;
		OSTimeDlyHMSM(0, 0, 0, 1);
    }
    id_phase = id_phase >> 12;

	OSTimeDlyHMSM(0, 0, 0, 1);
    // Some dodgy maths mixing un/signed different lengths relying on the way over/underflow will look like
    // a wraparound over a 2pi range
    // Multiply by 4 and mask gives us the electrical offset within a pi/2 quadrant
    e_phase_write  = -((id_phase*4) & 0xFFFF );
    // 0x4000 is 1/4 full scale which represents the pi/2 degree electrical phase offset required for commutation
    e_phase_write  = (e_phase_write + 0x4000) & 0xFFFF;
    EnDat_Init(dp);
	OSTimeDlyHMSM(0, 0, 0, 1);
    e_phase_read = 0 ;
    index=0;
    while(index<5 ){
        if ((short)e_phase_write!=(short)e_phase_read){
            EnDat_Enc_Parameter_Write(base_addr, 0xAD, 0x1, e_phase_write);
			//OSTimeDlyHMSM(0, 0, 1, 0);
			OSTimeDlyHMSM(0, 0, 0, 500);
            EnDat_Enc_Parameter_Read(base_addr, 0xAD, 0x1, &e_phase_read);
			//OSTimeDlyHMSM(0, 0, 1, 0);
			OSTimeDlyHMSM(0, 0, 0, 500);
            write_cnt++;
        }
       index++;
    }

	debug_printf(DBG_INFO, "---> ------------------------------------\n");
	debug_printf(DBG_INFO, "---> <Conrol value of phi>\n");
	debug_printf(DBG_INFO, "---> Phi without pole   : %X\n",id_phase);
	debug_printf(DBG_INFO, "---> Phi with    pole   : %X\n",id_phase*4);
	debug_printf(DBG_INFO, "---> Phi with 16 Bit    : %X\n",(id_phase*4) & 0xFFFF);
	debug_printf(DBG_INFO, "---> Phi invers         : %X\n",(unsigned short)(-((id_phase*4) & 0xFFFF)));
	debug_printf(DBG_INFO, "---> Phi Offset         : %X\n",(unsigned short)(-((id_phase*4) & 0xFFFF)));
	debug_printf(DBG_INFO, "---> ------------------------------------\n");
	debug_printf(DBG_INFO, "---> Try to write       : %i\n",write_cnt);
	debug_printf(DBG_INFO, "---> ------------------------------------\n");
	debug_printf(DBG_INFO, "---> Write offset angle : %X\n",(short)e_phase_write);
	debug_printf(DBG_INFO, "---> Read  offset angle : %X\n",(short)e_phase_read);
	debug_printf(DBG_INFO, "---> ------------------------------------\n");
	if((short)e_phase_write==(short)e_phase_read){
		debug_printf(DBG_INFO, "---> Read and Write of angle in EnDat EEPROM was successful! \n");
	}else if((short)e_phase_write!=(short)e_phase_read){
		debug_printf(DBG_ERROR, "Error: ---> Read and Write of angle in EnDat-EEPROM was not successful! \n");
		debug_printf(DBG_ERROR, "       ---> Encoder is not working! \n");
	}

    IOWR_16DIRECT(dp->DOC_ADC_BASE_ADDR, ADC_D, 0);
    dsm_reset(dp->DOC_SM_BASE_ADDR);
	//OSTimeDlyHMSM(0, 0, 1, 0);
	OSTimeDlyHMSM(0, 0, 0, 500);
    dp->enable_drive = 0 ;

    debug_printf(DBG_INFO, "---> Motor OFF \n");
    debug_printf(DBG_INFO, "---> ------------------------------------\n");
}

/*!
 * @}
 */

/*!
 * @}
 */
