#include "r_cg_macrodriver.h"
#include "Qapp.h"
#include "TM1650.h"


uint8_t gADCReadyFlag = 0;		

uint16_t stoveTemperature = 0;
uint16_t probe1Temperature = 0;
uint16_t probe2Temperature = 0;

uint16_t stoveThreshold=60;
uint16_t probe1Threshold=50;
uint16_t probe2Threshold=50;
uint16_t rec[50];
uint8_t head=0;
uint8_t tail=0;
uint8_t gTemperatureUnit=0; //Celsius or Fahrenheit

uint16_t NTC[26]=	{2949,2922,2882,2825,2745,2638,2514,2339,2147,1957,
					1709,1490,1281,1089,917,767,638,529,439,365,
					304,255,214,182,154,131};

uint16_t PT1000[31] = {1091,1114,1136,1157,1177,1197,1216,1234,1252,1269,
1286,1302,1318,1333,1347,1362,1375,1389,1402,1414,
1427,1438,1450,1461,1472,1483,1494,1504,1514,1523,
1533};

void readTemperature(uint8_t probe){
	
	uint8_t i;
	uint8_t digit;
	uint32_t tempTableIdx=0; //temperature table index
	uint8_t D0, D1,D2,D3;
	uint32_t temperature;
	uint16_t adcvalue=0;
	uint32_t adcVoltage;
	switch (probe){
		case 0:
			ADS= ProbeStove;//select ADC input
			gADCReadyFlag=0; //clear flag first
			ADCS=1; //trigger new conversion
			while (!gADCReadyFlag);
			adcvalue= ADCR;
			adcVoltage = adcvalue;
			adcVoltage = adcVoltage*3280/4096;
			rec[head]=adcVoltage;
			head++;
			head=head%50;
			for (tempTableIdx=0; tempTableIdx<30; tempTableIdx++){
				if (adcVoltage<PT1000[tempTableIdx]){
					temperature = tempTableIdx*10 - 10*(PT1000[tempTableIdx]-adcVoltage)/(PT1000[tempTableIdx] - PT1000[tempTableIdx-1]);
					break;
				}
			}
			if (tempTableIdx==30){
				temperature = 300;
			}
			stoveTemperature=(uint16_t)(temperature);
			if (gWorkingMode==StoveMode){
				showTemperature(gTemperatureUnit, stoveTemperature);
			}
			if (gStoveHot & 0x80){
				//User have escape temperature sesing. just do nothing.
			}else{
				if (stoveTemperature <=300){ //we will not handle temperature greater than 380 Celius
					if (stoveTemperature >stoveThreshold) {
						gStoveHot=1;
					}else if (stoveTemperature <(stoveThreshold -5)) {
						gStoveHot=0;		
						//redControl = 2000;
					}else{
						//redControl =0;
						//gStoveHot=0;
					}
				}
			}
		break;
		case 1:
			ADS= Probe1;//select ADC input
			gADCReadyFlag=0; //clear flag first
			ADCS=1; //trigger new conversion
			while (!gADCReadyFlag);
			adcvalue= ADCR;
			adcVoltage = adcvalue;
			adcVoltage = adcVoltage*3280/4096;
			if (adcVoltage>=NTC[0]){
				temperature =0;
			}else{
				for (tempTableIdx=1; tempTableIdx<25; tempTableIdx++){
					if (adcVoltage>NTC[tempTableIdx]){
						temperature = tempTableIdx*10 - 10*(adcVoltage-NTC[tempTableIdx])/(NTC[tempTableIdx-1] - NTC[tempTableIdx]);
						break;
					}
				}
			}
			if (tempTableIdx==25){
				temperature = 250;
			}
			probe1Temperature=(uint16_t)(temperature);
			if (gWorkingMode==Probe1Mode){
				showTemperature(gTemperatureUnit, probe1Temperature);
			}
			if (probe1Temperature <380){ //we will not handle temperature greater than 380 Celius
				if (probe1Temperature >probe1Threshold) {
					gProbe1Hot=1;
				}else if (probe1Temperature <(probe1Threshold -5)) {
					gProbe1Hot=0;		
					//redControl = 2000;
				}else{
					//redControl =0;
					//gStoveHot=0;
				}
			}		
		
		break;
		case 2:
			ADS= Probe2;//select ADC input
			gADCReadyFlag=0; //clear flag first
			ADCS=1; //trigger new conversion
			while (!gADCReadyFlag);
			adcvalue= ADCR;
			adcVoltage = adcvalue;
			adcVoltage = adcVoltage*3280/4096;
			temperature = 0;
			if (adcVoltage>=NTC[0]){
				temperature =0;
			}else{
				for (tempTableIdx=1; tempTableIdx<25; tempTableIdx++){
					if (adcVoltage>NTC[tempTableIdx]){
						temperature = tempTableIdx*10 - 10*(adcVoltage-NTC[tempTableIdx])/(NTC[tempTableIdx-1] - NTC[tempTableIdx]);
						break;
					}
				}
			}
			if (tempTableIdx==25){
				temperature = 250;
			}else if (tempTableIdx==0){
				temperature=0;
			}
			probe2Temperature=(uint16_t)(temperature);
			if (gWorkingMode==Probe2Mode){
				showTemperature(gTemperatureUnit, probe2Temperature);
			}
		
		break;		
	}	
}