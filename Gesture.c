#include "r_cg_macrodriver.h"
#include "r_cg_serial.h"
#include "taskHandler.h"
#include "Qapp.h"
#include "Gesture.h"
#include "TM1650.h"

uint8_t buffGesture[BuffGestureSize];
uint8_t tailGesture=0;
uint8_t headGesture=0;
uint8_t gestureBuf[2];

uint32_t degreeX=0;

void initGestureChannel(void){
	R_UART1_Receive(gestureBuf,1);
	R_UART1_Start();
	
	
}
	
uint8_t getDegree(void){
	
	static uint8_t step=0;
	static uint8_t remainingChar=0;
	uint8_t digit;
	while(tailGesture!=headGesture){
		switch(step){
			case 0: //read 55
				if (buffGesture[tailGesture]==0x55){
					step=1;
				}
			break;
			case 1: //check which kind of info
				switch (buffGesture[tailGesture]){
				case 0x53:
					step=2;
				break;
				case 0x51:
					step=0;
				break;
				}
			break;
			case 2: //handle 0x53 degree info
				degreeX= (uint16_t)buffGesture[tailGesture];
				step=3;
			break;
			case 3:
				degreeX = degreeX + (uint32_t)(buffGesture[tailGesture]*256);
				degreeX= degreeX*1800/32768;
				step=4;
				remainingChar=7;
				digit= degreeX % 10;
				showDigit(4,0);
				showDigit(3,digit);
				degreeX=degreeX/10;
				digit= degreeX % 10;
				showDigit(2,digit+20);
				digit = degreeX /10;
				showDigit(1,digit);
				//showDigit(1,10);
			break;
			case 4:
				remainingChar--;
				if (remainingChar==0){
					step=0;
				}else{
					step=4;
				}
			
			break;
			case 5:
			break;
			
			case 6:
			break;
			
		}
		tailGesture++;
		tailGesture=tailGesture%BuffGestureSize;
	}
				
			
	
}