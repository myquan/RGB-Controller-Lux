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
	}else if (gEvents & evRoutineJob){
		if ((gGodTimerCount % 8)==0){ //check to save every 2 seconds
			if (gSettingDirty){
				gConfigData.colorIndex = gColorIndex;
				gConfigData.workingMode = gWorkingMode;
				gConfigData.lightingMode = gLightingMode;
				gConfigData.brightIndex = gBrightIndex;
				gConfigData.temperatureThreshold[0]= stoveThreshold;
				saveConfig();	
				gSettingDirty=0;
			}
		}
	}
}

//=============================
//= Handle how lights changes =
//=============================
void handleLightTask(void){
	if (gStoveHot==1){
		if (gAppTimerKick){ //AppTimer will kick every 20ms
			gAppTimerKick = 0;
			appTimerKickCount++;
		}	
		if (appTimerKickCount > PartyMode2Speed){ //After n * 20ms, the color need to be changed.
			showStoveHot();
			appTimerKickCount = 0; //reset timing
		}
	}else{		
		switch(gLightingMode){
			case MonoMode:
				redControl = gColors[gColorIndex].red * BrightLevels[gBrightIndex];
				greenControl = gColors[gColorIndex].green * BrightLevels[gBrightIndex];
				blueControl = gColors[gColorIndex].blue * BrightLevels[gBrightIndex];
			break;
			case PartyMode:
				if (gAppTimerKick){ //AppTimer will kick every 20ms
					gAppTimerKick = 0;
					appTimerKickCount++;
				}	
				if (appTimerKickCount > PartyMode2Speed){ //After n * 20ms, the color need to be changed.
					slowlyChangeLightC();
					appTimerKickCount = 0; //reset timing
				}

			break;
		}
	}
	switch(gWorkingMode){
		case LightMode:
			indLight=0;
			indStove=indTimer=indProbe1=indProbe2=1;
			showDigit(1, Code_C);
			showDigit(2, Code_O);
			showDigit(3, Code_L);
			showDigit(4, Code_R);
		break;
		case StoveMode:
			indStove=0;
			indLight=indTimer=indProbe1=indProbe2=1;
		
		break;
		case TimerMode:
			indTimer=0;
			indStove=indLight=indProbe1=indProbe2=1;
			//showDigit(1,11);//Adp
			if (gUserTimerCounting){
				showTimer(gUserTimerCount);
			}else{
				showTimer(gUserTimeLimit*4);
			}
		break;
		case Probe1Mode:
			//showDigit(1,99);//clear digit
			indProbe1=0;
			indLight=indTimer=indStove=indProbe2=1;
			
		break;
		case Probe2Mode:
		P0.3=0;
			indProbe2=0;
			indLight=indTimer=indStove=indProbe1=1;
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
			gStoveHot +=128; //escape warning;
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
			if (gUserTimeLimit<20000){
				gUserTimeLimit++;
			}
			showTimer(gUserTimeLimit*4);

			break;
		case DownKey:
			if (gUserTimeLimit >1){
				gUserTimeLimit--;
			}
			showTimer(gUserTimeLimit*4);
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
