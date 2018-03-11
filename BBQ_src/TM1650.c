#include "r_cg_macrodriver.h"

#include "QApp.h"
#include "TM1650.h"
//#include "r_cg_intc.h"
#include "r_cg_timer.h"
#include "r_cg_serial.h"

#include "math.h"

#define Digit0	0b00111111
#define Digit1	0b00000110
#define Digit2	0b01011011
#define Digit3	0b01001111
#define Digit4	0b01100110

#define Digit5	0b01101101
#define Digit6	0b01111101
#define Digit7	0b00000111
#define Digit8	0b01111111
#define Digit9	0b01101111
#define DigitA	0b01110111
#define DigitAdp	0b11110111
#define DigitDP	0b10000000

#define DigitC	0b00111001
#define DigitO	0b00111111
#define DigitL	0b00111000
#define DigitR	0b01110111
#define DigitF	0b01110001

uint8_t digitData[16];

void  showDigit(uint8_t place, uint8_t digit);

//void R_IICA0_Create(void);
 void openTM1650(void){
	digitData[0]=0x67;
	digitData[1] = 5;
	//P7.0=1;
	gI2CEnd=0;
	//R_IIC20_Master_Send(0x48, digitData,1, 2);
	R_IIC20_Master_Send(0x48, digitData,1);
	while(!gI2CEnd);
	//P7.0=0;
}

void showTemperature(uint8_t unit, uint32_t mTemperature){
	uint8_t D0,D1,D2,D3;
	uint32_t temperature=0;
	if (unit==1){
		temperature = mTemperature *9 /50 +32;
	}else{
		temperature = mTemperature/10;
	}
	D0= (uint8_t)(temperature %10);
	temperature = temperature / 10;
	D1= (uint8_t) (temperature %10);
	temperature = temperature / 10;
	D2= (uint8_t) (temperature %10);
	showDigit(3,D0);
	showDigit(2,D1);
	if (D2==0){
		showDigit(1,99);
	}else{
		showDigit(1,D2);
	}
	if (unit==0){
		showDigit(4,Code_C); 
	}else{		
		showDigit(4,Code_F); 
	}
}

//**********************
//* shwoTimer(timeCount)
//* parameter: timeCount is 250ms
//*
//***********************
void showTimer(uint16_t timeCount){
	uint8_t seconds=0;
	uint16_t minutes=0;
	if (timeCount < 60*4){ //less than 1 minute 00:NN
		seconds = (uint8_t) (timeCount/4);
		showDigit(1,0);
		showDigit(2,0+20);
		showDigit(3, seconds/10);
		showDigit(4, seconds%10);
	}else if (timeCount <3600*4){ //more than 1 minute, but less than 1 hour.  NN:00
		minutes =  (timeCount/240);
		if (minutes >9){
			showDigit(1,minutes / 10);
		}else{
			showDigit(1,99); //show nothing
		}
		showDigit(2,minutes % 10+20);
		seconds = timeCount % 240;
		seconds = seconds/4;
		showDigit(3, seconds / 10);
		showDigit(4, seconds % 10);
		
	}else{ //more than 1 hour // NN:MM
		uint8_t minutes2;
		uint16_t minutes = timeCount/240;
		uint8_t hours = (uint8_t) (minutes /60);
		minutes2 = (uint8_t) (minutes % 60);
		showDigit(1,hours / 10);
		showDigit(2,hours % 10 + 20);
		showDigit(3, minutes2/10);
		showDigit(4, minutes2%10);
	}
	
}




void  showDigit(uint8_t place, uint8_t digit){
	uint8_t address;
	uint8_t i;
	switch(place){
		case 1:
			address = 0x68;
		break;
		case 2:
			address = 0x6A;
		break;
		case 3:
			address = 0x6C;
		break;
		case 4:
			address = 0x6E;
		break;
		default:
		break;
	}
	switch (digit){
		case 0:			
			digitData[0]=Digit0;
			break;
		case 1:			
			digitData[0]=Digit1;
			break;
		case 2:			
			digitData[0]=Digit2;
			break;
		case 3:			
			digitData[0]=Digit3;
			break;
		case 4:			
			digitData[0]=Digit4;
			break;
		case 5:			
			digitData[0]=Digit5;
			break;
		case 6:			
			digitData[0]=Digit6;
			break;
		case 7:			
			digitData[0]=Digit7;
			break;
		case 8:			
			digitData[0]=Digit8;
			break;
		case 9:			
			digitData[0]=Digit9;
			break;
		case 10:			
			digitData[0]=DigitA;
			break;
		case 11:			
			digitData[0]=DigitAdp;
			break;
		case 12:			
			digitData[0]=DigitDP;
			break;
		case 13:			
			digitData[0]=DigitC;
			break;
		case 14:			
			digitData[0]=DigitL;
			break;
		case 15:			
			digitData[0]=DigitR;
			break;
		case 16:			
			digitData[0]=DigitO;
			break;
		case 17:			
			digitData[0]=DigitF;
			break;
		case 20:			
			digitData[0]=Digit0+0x80;
			break;
		case 21:			
			digitData[0]=Digit1+0x80;
			break;
		case 22:			
			digitData[0]=Digit2+0x80;
			break;
		case 23:			
			digitData[0]=Digit3+0x80;
			break;
		case 24:			
			digitData[0]=Digit4+0x80;
			break;
		case 25:			
			digitData[0]=Digit5+0x80;
			break;
		case 26:			
			digitData[0]=Digit6+0x80;
			break;
		case 27:			
			digitData[0]=Digit7+0x80;
			break;
		case 28:			
			digitData[0]=Digit8+0x80;
			break;
		case 29:			
			digitData[0]=Digit9+0x80;
			break;
		default:
			digitData[0]=0;
			break;
	}		
	gI2CEnd=0;
	//P7.0=1;
	//R_IIC20_Master_Send(address, digitData,1, 2);
	R_IIC20_Master_Send(address, digitData,1);
	while(!gI2CEnd);
	for(i=0; i<255; i++);
	//P7.0=0;
	//MD_STATUS R_IICA0_Master_Receive(uint8_t adr, uint8_t * const rx_buf, uint16_t rx_num, uint8_t wait);

}
 