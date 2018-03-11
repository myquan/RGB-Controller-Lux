#include "r_cg_macrodriver.h"
#include "r_cg_serial.h"
#include "taskHandler.h"
#include "handleKey.h"
#include "Qapp.h"
#include "TM1650.h"
#include "r_cg_pclbuz.h"
#include "temperature.h"
#include "EEPROM.h"
#include "UserTimer.h"

uint16_t gEvents=0;
uint16_t gModeEvents=0;

void handleLightModeTask(void);
void handleProbe1ModeTask(void);
void handleProbe2ModeTask(void);
void handleStoveModeTask(void);
void handleTimerModeTask(void);

void showDigits(uint16_t number);

void handleModeTask(void){
	switch (gWorkingMode){
		case LightMode:
			handleLightModeTask();
			break;
		case StoveMode:
			handleStoveModeTask();
			break;
		case TimerMode:
			handleTimerModeTask();
			break;
		case Probe1Mode:
			handleProbe1ModeTask();
			break;
		case Probe2Mode:
			handleProbe2ModeTask();
			break;
	}	
}

//***
//*** Handle Task common to all working modes ***
//***
void handleCommonTask(void){
	if (gEvents & evMeasureTemperature){
		//P0.3=1;
		clearEvent(evMeasureTemperature);
		readTemperature(gProbe);
		gProbe++;
		gProbe = gProbe%3;
//P0.3=0;
	}else if (gEvents & evTimeout){
		clearEvent(evTimeout);
		gTimerIsOn=1;
		gDoLightFlash=1;
	}else if (gEvents & evRoutineJob){
		clearEvent(evRoutineJob);
		if ((gGodTimerCount % 16)==0){ //check to save every 2 seconds
			if (gSettingDirty){
				gConfigData.colorIndex = gColorIndex;
				gConfigData.workingMode = gWorkingMode;
				gConfigData.lightingMode = gLightingMode;
				gConfigData.brightIndex = gBrightIndex;
				gConfigData.userTimeLimit = gUserTimeLimit;
				gConfigData.temperatureUnit = gTemperatureUnit;
				gConfigData.temperatureThreshold[0]= stoveThreshold;
				gConfigData.temperatureThreshold[1]= probe1Threshold;
				gConfigData.temperatureThreshold[2]= probe2Threshold;
				saveConfig();	
				gSettingDirty=0;
			}
		}
		R_PCLBUZ0_Stop();;
		if ((gGodTimerCount % 2)==0){
			if (gDoLightFlash){
				if (gLightOn==1){
					gLightOn=0;
					R_PCLBUZ0_Start();;
				}else{
					gLightOn=1;
				}
			}else{
				gLightOn=1;
			}
		}
	}
}

//===================================================
//= Handle how lights changes and Display shows 	=
//===================================================
void handleUITask(void){
	if (gStoveHot==1){
		if (gAppTimerKick){ //AppTimer will kick every 20ms
			gAppTimerKick = 0;
			appTimerKickCount++;
		}	
		if (appTimerKickCount > PartyMode2Speed){ //After n * 20ms, the color need to be changed.
			if (gLightOn){
				showStoveHot();
				appTimerKickCount = 0; //reset timing
			}else{
				redControl = 0;
				greenControl = 0;
				blueControl =0;
			}
		}
	}else{		
		switch(gLightingMode){
			case MonoMode:
				if (gLightOn){
					redControl = gColors[gColorIndex].red * BrightLevels[gBrightIndex];
					greenControl = gColors[gColorIndex].green * BrightLevels[gBrightIndex];
					blueControl = gColors[gColorIndex].blue * BrightLevels[gBrightIndex];
				}else{
					redControl = 0;
					greenControl = 0;
					blueControl =0;
				}
			break;
			case PartyMode:
				if (gAppTimerKick){ //AppTimer will kick every 20ms
					gAppTimerKick = 0;
					appTimerKickCount++;
				}	
				if (appTimerKickCount > PartyMode2Speed){ //After n * 20ms, the color need to be changed.
					if (gLightOn){
						slowlyChangeLightC();
					}else{
						redControl = 0;
						greenControl = 0;
						blueControl =0;
					}				
					appTimerKickCount = 0; //reset timing
				}

			break;
		}
	}
	switch(gWorkingMode){
		case LightMode:
			indLight=IndicatorLEDOn;
			indStove=indTimer=indProbe1=indProbe2=IndicatorLEDOff;
			showDigit(1, Code_C);
			showDigit(2, Code_O);
			showDigit(3, Code_L);
			showDigit(4, Code_R);
		break;
		case StoveMode:
			indStove=IndicatorLEDOn;
			indLight=indTimer=indProbe1=indProbe2=IndicatorLEDOff;
		
		break;
		case TimerMode:
			indTimer=IndicatorLEDOn;
			indStove=indLight=indProbe1=indProbe2=IndicatorLEDOff;
			//showDigit(1,11);//Adp
			if (gUserTimerCounting){
				showTimer(gUserTimerCount*4);
			}else{
				showTimer(gUserTimeLimit*4);
			}
		break;
		case Probe1Mode:
			//showDigit(1,99);//clear digit
			indProbe1=IndicatorLEDOn;
			indLight=indTimer=indStove=indProbe2=IndicatorLEDOff;			
		break;
		case Probe2Mode:
			indProbe2=IndicatorLEDOn;
			indLight=indTimer=indStove=indProbe1=IndicatorLEDOff;
		break;
	}
	
}

//=======================================
//= The first mode: Light setting mode
//=======================================
void handleLightModeTask(void){
	if (gModeEvents & KeyShortPressed){
		clearModeEvent(KeyShortPressed);
		switch (gPressedKey){
		case UpKey:
			gColorIndex++;
			gColorIndex = gColorIndex%TotalColors;
			gSettingDirty=1;
			if (igniterControl){
				igniterControl=0;
			}else{
				igniterControl=1;
			}
			break;
		case DownKey:
			if (gColorIndex>0){
				gColorIndex--;			
			}else{
				gColorIndex= TotalColors-1;
			}
			gSettingDirty=1;
			break;
		case FunctionKey: //switch light mode: mono/party
			//gBrightIndex++;
			//gBrightIndex=gBrightIndex%4;
			gWorkingMode = StoveMode;
			gProbe=0;
			sendEvent(evMeasureTemperature);

			//R_PCLBUZ0_Start();
			break;
		}
		gPressedKey = NoKey;			
	}else if (gModeEvents & KeyLongPressed){
		switch (gPressedKey){
		case UpKey:
			break;
		case DownKey:
			break;
		case FunctionKey: //switch light mode: mono/party
			if (gLightingMode == MonoMode){
				//delay20ms(1);
				gLightingMode=PartyMode;
			}else{
				gLightingMode=MonoMode;
			}
			break;
		}
		clearModeEvent(KeyLongPressed);
	}else if (gModeEvents & evToggleLight){
		clearModeEvent(evToggleLight);
		if (gBrightIndex ==0){
			gBrightIndex = 3;
		}else{
			gBrightIndex =0;
		}
		adjustLight(gColorIndex, gBrightIndex);
	}	
}

//===========================================
//= The first mode: Stove Probe setting mode
//===========================================
void handleStoveModeTask(void){
	if (gModeEvents & KeyShortPressed){
		clearModeEvent(KeyShortPressed);
		switch (gPressedKey){
		case UpKey:
			if (gDoubleKeyFlag==1){
				gDoubleKeyFlag=0;
				if (gTemperatureUnit==0){
					gTemperatureUnit=1;
				}else{
					gTemperatureUnit=0;
				}
				gSettingDirty=1;
				gProbe=0;
				sendEvent(evMeasureTemperature);				
			}else{
				if (stoveThreshold<251){
					stoveThreshold +=1;
				}
				//gStoveHot +=128; //escape warning;
			}
			break;
		case DownKey:
			if (gDoubleKeyFlag==1){
				gDoubleKeyFlag=0;
				if (gTemperatureUnit==0){
					gTemperatureUnit=1;
				}else{
					gTemperatureUnit=0;
				}
				gSettingDirty=1; //mark to save into EEPROM
				gProbe=0;
				sendEvent(evMeasureTemperature);
			}else{
				if (stoveThreshold>1){
					stoveThreshold--;			
				}
			}
			break;
		case FunctionKey: //switch light mode: mono/party
			//gBrightIndex++;
			//gBrightIndex=gBrightIndex%4;
			gWorkingMode = TimerMode;
			gProbe=1;
			sendEvent(evMeasureTemperature);
			R_PCLBUZ0_Stop();

			break;
		}
		gPressedKey = NoKey;			
	}else if (gModeEvents & KeyLongPressed){
		switch (gPressedKey){
		case UpKey:
			break;
		case DownKey:
			break;
		case FunctionKey: //switch light mode: mono/party
			if (gLightingMode == MonoMode){
				//delay20ms(1);
				gLightingMode=PartyMode;
			}else{
				gLightingMode=MonoMode;
			}
			break;
		}
		clearModeEvent(KeyLongPressed);
	}else if (gModeEvents & evToggleLight){
		clearModeEvent(evToggleLight);
		if (gBrightIndex ==0){
			gBrightIndex = 3;
		}else{
			gBrightIndex =0;
		}
		adjustLight(gColorIndex, gBrightIndex);
	}	
}

void handleTimerModeTask(void){
	if (gModeEvents & KeyShortPressed){
		clearModeEvent(KeyShortPressed);
		switch (gPressedKey){
		case UpKey:
			if (gDoubleKeyFlag==1){
				gDoubleKeyFlag=0;
				//toggle User Timer
				gTimerIsOn=0;
				gDoLightFlash=0;
				if (gUserTimerCounting){
					gUserTimerCounting=0;
				}else{
					gUserTimerCounting=1;
					//gUserTimerCount=gUserTimeLimit;
					startUserTimer();
				}
			}else{
				if (gUserTimerCounting){
				//we shouldn't change TimeLimit when timer is active.	
				}else{
					if (gUserTimeLimit<20000){
						if (gUserTimeLimit<60){
							gUserTimeLimit++;
						}else if (gUserTimeLimit< 60*60){ //Less than 1 hour
							gUserTimeLimit+=60;
						}
					}
					gSettingDirty=1;
				}
			}
			//showTimer(gUserTimeLimit*4);

			break;
		case DownKey:
			if (gDoubleKeyFlag==1){
				gDoubleKeyFlag=0;
				gTimerIsOn=0;
				gDoLightFlash=0;
				if (gUserTimerCounting){
					gUserTimerCounting=0;
				}else{
					gUserTimerCounting=1;
					//gUserTimerCount=gUserTimeLimit;
					startUserTimer();
				}
			}else{
				if (gUserTimerCounting){
				//we shouldn't change TimeLimit when timer is active.	
				}else{
					if (gUserTimeLimit >1){
						if (gUserTimeLimit<=60){
							gUserTimeLimit--;
						}else if (gUserTimeLimit< 60*60){ //Less than 1 hour
							gUserTimeLimit-=60;
						}
						gSettingDirty=1;
					}
				}
			}
			//showTimer(gUserTimeLimit*4);
			break;
		case FunctionKey: //This will never happen.
			gWorkingMode = Probe1Mode;
			gProbe=1;
			sendEvent(evMeasureTemperature);
			break;
		}
		gPressedKey = NoKey;
	}else if (gModeEvents & KeyLongPressed){
	}
}
//=== Process event in Timeer Mode ===
void handleTimerModeTask2(void){
	if (gEvents & KeyShortPressed){
		clearEvent(KeyShortPressed);
		switch (gPressedKey){
		case UpKey:
			break;
		case DownKey:
			break;
		case FunctionKey: //This will never happen.
			gWorkingMode = Probe1Mode;
			break;
		}
		gPressedKey = NoKey;
	}else if (gEvents & evStartUserTimer){ //Begin User Timer Countdown
		clearEvent(evStartUserTimer);
		//lite = !lite;
		//gPressedKey = NoKey;
	}else if (gEvents & evTimeout){
		clearEvent(evTimeout);
		//gColorIndex = GreenColorIndex; //set to color RED. R-G-B-Y-W
		gBrightIndex = 3;
		adjustLight(gColorIndex, gBrightIndex);
		showDigit(4,0);
	}else if (gEvents & evToggleDP){
		clearEvent(evToggleDP);
		if (gDPOn){
			showDigit(1, 10);
			gDPOn=0;
		}else{
			showDigit(1,11); //show A with dot-point
			gDPOn=1;
		}
		sendEvent(evUpdateTimeLeft);
	}else if (gEvents & evToggleLight){
		clearEvent(evToggleLight);
		if (gBrightIndex ==0){
			gBrightIndex = 3;
		}else{
			gBrightIndex =0;
		}
		adjustLight(gColorIndex, gBrightIndex);
		
	}else if (gEvents & evHot){
		clearEvent(evHot);
		gColorIndex = RedColorIndex; //set to color RED. R-G-B-Y-W
		adjustLight(gColorIndex, gBrightIndex);
	}else if (gEvents & evWarm){
		clearEvent(evWarm);
		gColorIndex = YellowColorIndex;
		adjustLight(gColorIndex, gBrightIndex);
	}else if (gEvents & evUpdateTimeLeft){
		uint16_t timeLeft;
		clearEvent(evUpdateTimeLeft);
		if (gTimerCounter<=500){
			timeLeft = 0;
		}else{
			timeLeft = (gTimerCounter-500) / 3000 +1;
		}
		showDigits(timeLeft);
	}	
}



void handleProbe1ModeTask(void){
	if (gModeEvents & KeyShortPressed){
		clearModeEvent(KeyShortPressed);
		switch (gPressedKey){
		case UpKey:
			gColorIndex++;
			gColorIndex = gColorIndex%TotalColors;
			break;
		case DownKey:
			if (gColorIndex>0){
				gColorIndex--;			
			}else{
				gColorIndex= TotalColors-1;
			}
			break;
		case FunctionKey: //switch light mode: mono/party
			//gBrightIndex++;
			//gBrightIndex=gBrightIndex%4;
			gWorkingMode = Probe2Mode;
			gProbe=2;
			sendEvent(evMeasureTemperature);

			break;
		}
		gPressedKey = NoKey;			
	}else if (gModeEvents & KeyLongPressed){
		switch (gPressedKey){
		case UpKey:
			break;
		case DownKey:
			break;
		case FunctionKey: //switch light mode: mono/party
			if (gLightingMode == MonoMode){
				//delay20ms(1);
				gLightingMode=PartyMode;
			}else{
				gLightingMode=MonoMode;
			}
			break;
		}
		clearModeEvent(KeyLongPressed);
	}else if (gModeEvents & evToggleLight){
		clearModeEvent(evToggleLight);
		if (gBrightIndex ==0){
			gBrightIndex = 3;
		}else{
			gBrightIndex =0;
		}
		adjustLight(gColorIndex, gBrightIndex);
	}	
}

void handleProbe2ModeTask(void){
	if (gModeEvents & KeyShortPressed){
		clearModeEvent(KeyShortPressed);
		switch (gPressedKey){
		case UpKey:
			gColorIndex++;
			gColorIndex = gColorIndex%TotalColors;
			break;
		case DownKey:
			if (gColorIndex>0){
				gColorIndex--;			
			}else{
				gColorIndex= TotalColors-1;
			}
			break;
		case FunctionKey: //switch light mode: mono/party
			//gBrightIndex++;
			//gBrightIndex=gBrightIndex%4;
			gWorkingMode = LightMode;

			break;
		}
		gPressedKey = NoKey;			
	}else if (gModeEvents & KeyLongPressed){
		switch (gPressedKey){
		case UpKey:
			break;
		case DownKey:
			break;
		case FunctionKey: //switch light mode: mono/party
			if (gLightingMode == MonoMode){
				//delay20ms(1);
				gLightingMode=PartyMode;
			}else{
				gLightingMode=MonoMode;
			}
			break;
		}
		clearModeEvent(KeyLongPressed);
	}else if (gModeEvents & evToggleLight){
		clearModeEvent(evToggleLight);
		if (gBrightIndex ==0){
			gBrightIndex = 3;
		}else{
			gBrightIndex =0;
		}
		adjustLight(gColorIndex, gBrightIndex);
	}	
}


void sendEvent(uint16_t event){
	gEvents |= event;
}

void clearEvent(uint16_t event){
	uint16_t t_event= event^0xffff;
	
	gEvents &= t_event;
}
void sendModeEvent(uint16_t event){
	gModeEvents |= event;
}

void clearModeEvent(uint16_t event){
	uint16_t t_event= event^0xffff;	
	gModeEvents &= t_event;
}


//=== showDigits ===
void showDigits(uint16_t number){
	uint16_t digit;
	digit = number % 10;
	showDigit(4, digit);
	//delay20ms(1);
	digit = (number/10) %10;
	showDigit(3, digit);
	//delay20ms(1);
	digit = (number/100) %10;
	showDigit(2, digit);
	//delay20ms(1);
}
