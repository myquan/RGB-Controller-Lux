/***********************************************************************************************************************
* DISCLAIMER
* This software is supplied by Renesas Electronics Corporation and is only intended for use with Renesas products.
* No other uses are authorized. This software is owned by Renesas Electronics Corporation and is protected under all
* applicable laws, including copyright laws. 
* THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIESREGARDING THIS SOFTWARE, WHETHER EXPRESS, IMPLIED
* OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
* NON-INFRINGEMENT.  ALL SUCH WARRANTIES ARE EXPRESSLY DISCLAIMED.TO THE MAXIMUM EXTENT PERMITTED NOT PROHIBITED BY
* LAW, NEITHER RENESAS ELECTRONICS CORPORATION NOR ANY OF ITS AFFILIATED COMPANIES SHALL BE LIABLE FOR ANY DIRECT,
* INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES FOR ANY REASON RELATED TO THIS SOFTWARE, EVEN IF RENESAS OR
* ITS AFFILIATES HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
* Renesas reserves the right, without notice, to make changes to this software and to discontinue the availability 
* of this software. By using this software, you agree to the additional terms and conditions found by accessing the 
* following link:
* http://www.renesas.com/disclaimer
*
* Copyright (C) 2010, 2017 Renesas Electronics Corporation. All rights reserved.
***********************************************************************************************************************/

/***********************************************************************************************************************
* File Name    : r_cg_timer_user.c
* Version      : CodeGenerator for RL78/G1A V2.04.00.02 [19 Apr 2017]
* Device(s)    : R5F10EGC
* Tool-Chain   : CA78K0R
* Description  : This file implements device driver for TAU module.
* Creation Date: 2018/3/2
***********************************************************************************************************************/

/***********************************************************************************************************************
Pragma directive
***********************************************************************************************************************/
#pragma interrupt INTTM00 r_tau0_channel0_interrupt
#pragma interrupt INTTM01 r_tau0_channel1_interrupt
#pragma interrupt INTTM02 r_tau0_channel2_interrupt
#pragma interrupt INTTM03 r_tau0_channel3_interrupt
#pragma interrupt INTTM05 r_tau0_channel5_interrupt
#pragma interrupt INTTM06 r_tau0_channel6_interrupt
#pragma interrupt INTTM07 r_tau0_channel7_interrupt
/* Start user code for pragma. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */

/***********************************************************************************************************************
Includes
***********************************************************************************************************************/
#include "r_cg_macrodriver.h"
#include "r_cg_timer.h"
/* Start user code for include. Do not edit comment generated here */
#include "Qapp.h"
#include "handleKey.h"
#include "taskHandler.h"
#include "UserTimer.h"
#include "r_cg_intc.h"
/* End user code. Do not edit comment generated here */
#include "r_cg_userdefine.h"

/***********************************************************************************************************************
Global variables and functions
***********************************************************************************************************************/
/* Start user code for global. Do not edit comment generated here */
uint8_t toggleFlag=0;
enum {
	KeyStatus_UnPressed,
	KeyStatus_ShortPressed,
	KeyStatus_LongPressed
};
uint8_t gKeyStatus = KeyStatus_UnPressed;
/* End user code. Do not edit comment generated here */

/***********************************************************************************************************************
* Function Name: r_tau0_channel0_interrupt
* Description  : This function is INTTM00 interrupt service routine.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
__interrupt static void r_tau0_channel0_interrupt(void)
{
    /* Start user code. Do not edit comment generated here */
	//*** PWM master ***
    /* End user code. Do not edit comment generated here */
}

/***********************************************************************************************************************
* Function Name: r_tau0_channel1_interrupt
* Description  : This function is INTTM01 interrupt service routine.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
__interrupt static void r_tau0_channel1_interrupt(void)
{
    /* Start user code. Do not edit comment generated here */
    /* End user code. Do not edit comment generated here */
}

/***********************************************************************************************************************
* Function Name: r_tau0_channel2_interrupt
* Description  : This function is INTTM02 interrupt service routine.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
__interrupt static void r_tau0_channel2_interrupt(void)
{
    /* Start user code. Do not edit comment generated here */
	gGodTimerCount++;
	gGodTimerCount = gGodTimerCount % 240; //250ms x 4 x 60 = 1 minutes
	sendEvent(evRoutineJob);
	checkUserTimeout();
    /* End user code. Do not edit comment generated here */
}

/***********************************************************************************************************************
* Function Name: r_tau0_channel3_interrupt
* Description  : This function is INTTM03 interrupt service routine.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
__interrupt static void r_tau0_channel3_interrupt(void)
{
    /* Start user code. Do not edit comment generated here */
    /* End user code. Do not edit comment generated here */
}

/***********************************************************************************************************************
* Function Name: r_tau0_channel5_interrupt
* Description  : This function is INTTM05 interrupt service routine.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
__interrupt static void r_tau0_channel5_interrupt(void)
{
    /* Start user code. Do not edit comment generated here */
	//*** Quan Application Timer ***
	TMIF05 = 0U;
	if (gMeasureTimerCount> MeasureTimerCountLimit - 2){
		//P7.7 = ~P7.7;
		sendEvent(evMeasureTemperature);
		gMeasureTimerCount =0;
	}else{
		gMeasureTimerCount++;
	}
    gAppTimerKick = 1;
	gDelayCounter++;
    /* End user code. Do not edit comment generated here */
}

/***********************************************************************************************************************
* Function Name: r_tau0_channel6_interrupt
* Description  : This function is INTTM06 interrupt service routine.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
__interrupt static void r_tau0_channel6_interrupt(void)
{
    /* Start user code. Do not edit comment generated here */
	//*** Key Scan Timer ***
	    switch (gPressedKey){
	    case FunctionKey:
	    	if (functionKey) { //=== Key was released
				if (gKeyStatus == KeyStatus_ShortPressed){
					sendModeEvent(KeyShortPressed);
				}else if (gKeyStatus == KeyStatus_LongPressed){
					//sendEvent(ChangePlayingMode);
					gPressedKey = NoKey;				
				}else {
					gPressedKey = NoKey;
				}
				//PressedKey = NoKey;
				gKeyStatus = KeyStatus_UnPressed;
				R_INTC1_Start();	//?i¡¦_upKey?¡±??A_¡Âa?O
				R_TAU0_Channel6_Stop();	//?¢G?Y-n|Ascan key status?F.
				gKeyPressedDuration = 0;
			}else{
				if (gKeyPressedDuration <= LongPressDuration){
					if (gKeyPressedDuration > ShortPressDuration){
						if (gKeyPressedDuration == LongPressDuration){
							gKeyStatus = KeyStatus_LongPressed;
							sendModeEvent(KeyLongPressed);
						}else{						
							gKeyStatus = KeyStatus_ShortPressed;
						}
					}else {				
					}				
					gKeyPressedDuration++;
				}else{					
					gKeyStatus = KeyStatus_LongPressed;
				}							
			}
	    break;
	    case UpKey:
	    	if (upKey) { //=== Key was released
				if (gKeyStatus == KeyStatus_ShortPressed){
					sendModeEvent(KeyShortPressed);
				}else if (gKeyStatus == KeyStatus_LongPressed){
					gPressedKey = NoKey;
					//sendEvent(KeyLongPressed);
				}else {
					gPressedKey = NoKey;
				}
				//gPressedKey = NoKey;
				gKeyStatus = KeyStatus_UnPressed;
				R_INTC0_Start();	//?i¡¦_upKey?¡±??A_¡Âa?O
				R_TAU0_Channel6_Stop();	//?¢G?Y-n|Ascan key status?F.
				gKeyPressedDuration = 0;
			}else{
				if (gKeyPressedDuration <= LongPressDuration){
					if (gKeyPressedDuration > ShortPressDuration){
						if (gKeyPressedDuration == LongPressDuration){
							gKeyStatus = KeyStatus_LongPressed;
							sendModeEvent(KeyLongPressed);
						}else{						
							gKeyStatus = KeyStatus_ShortPressed;
						}
					}else {				
					}				
					gKeyPressedDuration++;
				}else{
					
					gKeyStatus = KeyStatus_LongPressed;
				}								
			}

	    break;
	    case DownKey:
	    	if (downKey) { //=== Key was released
				if (gKeyStatus == KeyStatus_ShortPressed){
					sendModeEvent(KeyShortPressed);
					//sendEvent(ChangeLightLevel);
				}else if (gKeyStatus == KeyStatus_LongPressed){
					gPressedKey = NoKey;
				}else {
					gPressedKey = NoKey;
				}
				gKeyStatus = KeyStatus_UnPressed;
				R_INTC2_Start();	//?i¡¦_upKey?¡±??A_¡Âa?O
				R_TAU0_Channel6_Stop();	//?¢G?Y-n|Ascan key status?F.
				gKeyPressedDuration = 0;
			}else{
				if (gKeyPressedDuration <= LongPressDuration){
					if (gKeyPressedDuration > ShortPressDuration){
						if (gKeyPressedDuration == LongPressDuration){
							gKeyStatus = KeyStatus_LongPressed;
							sendModeEvent(KeyLongPressed);
						}else{						
							gKeyStatus = KeyStatus_ShortPressed;
						}
					}else {				
					}				
					gKeyPressedDuration++;
				}else{
					
					gKeyStatus = KeyStatus_LongPressed;
				}				
				
			}
	    
	    break;
	    default:
	    
	    break;
    }
   
    /* End user code. Do not edit comment generated here */
}

/***********************************************************************************************************************
* Function Name: r_tau0_channel7_interrupt
* Description  : This function is INTTM07 interrupt service routine.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
__interrupt static void r_tau0_channel7_interrupt(void)
{
    /* Start user code. Do not edit comment generated here */
	//PWM Slave
    /* End user code. Do not edit comment generated here */
}

/* Start user code for adding. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */
