#include "r_cg_macrodriver.h"

#include "QApp.h"
#include "r_cg_intc.h"
#include "r_cg_timer.h"
#include "r_cg_serial.h"
#include "r_cg_pclbuz.h"
#include "r_cg_adc.h"
#include "math.h"
#include "TM1650.h"
#include "taskHandler.h"
#include "EEPROM.h"
#include "Gesture.h"

//=== App related ===
uint8_t gWorkingMode = LightMode;
uint16_t gDelayCounter = 0;
uint8_t gTimerLimit = 3; //in Minutes
uint8_t gTimerCountdown=0; //1: Timer counting down. 0:Stop countdown
uint32_t gTimerCounter=0;
uint8_t gDPOn=0; //DP is shown or not
uint8_t appTimerKickCount = 0;
uint8_t gSettingDirty=0; //Set when anyone of the setting is changed. Application will use this flag to do EE saving.
uint8_t gGodTimerCount=0;

//=== Temperature related ===
uint16_t gMeasureTimerCount = 0;
//uint16_t gCurrentTemperature = 0; //in Celsius
uint8_t gStoveHot=0; //the PT1000 is hot
uint8_t gProbe1Hot=0; //the NTC is hot
uint8_t gProbe2Hot=0; //the NTC is hot
uint8_t gProbe=0;

//=== RGB light related ===
uint8_t gBrightIndex= 0;
uint8_t gColorIndex= 0;
uint16_t BrightLevels[TotalBrightLevels] = {10,30,55,85};
uint8_t gLightingMode=MonoMode; //the lighting mode.
uint8_t gAppTimerKick = 0;
uint8_t brightnessIndex =0;
uint8_t variationStage = 0; //only used in change color lightness
uint8_t gParty1ColorIndex=0;

//=== Timer related ===
uint8_t gTimerIsOn=0; //the User Timer in on.

ColorType gColors2[TotalColors] = 	{{0,0,0},
									{90,130,100},
									{100,125,130},
									{100,125,125},
									{100,120,120}};
ColorType gColors[TotalColors] = 	{{0,0,320},
									{0,320,0},
									{150,0,170},
									{150,170,0},
									{90,135,115},
									{0,0,0}	};
ColorType gColors_Party1[TotalParty1Colors] = 	{
									{0,0,320},
									{150,0,170},
									{90,135,105}};
uint8_t gExpTable[20] = {1,1,1,1,1,2,3,4,6,8,11,15,20,27};		
ColorType ColorBlue 	={0,0,320};
ColorType ColorPurple	={150,170,0};
ColorType ColorWhite ={90,135,115};
ColorType ColorRed ={320,0,0};
ColorType ColorGreen ={0,320,0};
ColorType ColorYellow ={140,170,0};

uint8_t configData[10]={1,1,0,3,4,5,6,7,8};
uint16_t lightnessCurve(uint16_t lightLevel);
ConfigData_t gConfigData;

	
void init_variables(void){
	uint16_t i;
	gIICA0Done=0;
	configData[0]=1; //start from 2nd byte
	if (MD_OK == writeEE(configData,1)){
		for(i=0; i<25000; i++){
			NOP();
		}
		while(!gIICA0Done);
		gIICA0Done=0;
		readEE(&gConfigData,8);
		for(i=0; i<25000; i++){
			NOP();
		}
		while(!gIICA0Done);
		if (gConfigData.flag != 0x88){
			//=== New Chip. Initialize config data.
			gConfigData.flag=0x88;
			gConfigData.workingMode=1;  //light mode
			gConfigData.lightingMode=0; //Mono
			gConfigData.colorIndex =	DefaultColorIndex; 
			gConfigData.brightIndex =	DefaultBrightIndex; //birghtness
			saveConfig();
		}
	}
	
	gWorkingMode = gConfigData.workingMode;
	
	//gWorkingMode = StoveMode;
	gLightingMode =	gConfigData.workingMode;
	gLightingMode =	gConfigData.lightingMode;
	gColorIndex = 	gConfigData.colorIndex;
	gBrightIndex =	gConfigData.brightIndex;
	//gCurrentTemperatureThreshold = (TemperatureThresholdInF -32)*5/9 ;
	gas20=gas40=gas60=gas80=gas99=1;
	indLight=indStove=indTimer=indProbe1=indProbe2=1;
	buzzer=0;
	gProbe1Hot=0;
	gProbe2Hot=0;
	gStoveHot=0;
	//P0.2=P0.3=0;
	gas20=0;
}

void init_app(void){
	uint16_t i;
	init_variables();
	//R_PCLBUZ0_Start();
	buzzer=0;
	//gas99=0;
//	CS_Max31856 = 1;
	//adjustLight(gColorIndex, gBrightIndex);
	R_INTC0_Start();	//Function Key
	R_INTC1_Start();	//Up Key	
	R_INTC2_Start();	//DownKey
	R_TAU0_Channel0_Start(); //PWM
	R_TAU0_Channel5_Start(); //internal Timer
	R_TAU0_Channel2_Start(); //internal Timer for GodTimer doing God Common Jobs.	
	//lite = 1;
	//readConfig();
#if (1)
	blueControl= 0;
	greenControl = 0;
	redControl = 0;
#else
	blueControl= 32000;
	greenControl = 32000;
	redControl = 32000;

#endif
#if 0
	if (gLightingMode == PartyMode){
		redControl = gColors_Party1[gParty1ColorIndex].red * BrightLevels[gBrightIndex];
		greenControl = gColors_Party1[gParty1ColorIndex].green * BrightLevels[gBrightIndex];
		blueControl = gColors_Party1[gParty1ColorIndex].blue * BrightLevels[gBrightIndex];
	}else{
		redControl = gColors[gColorIndex].red * BrightLevels[gBrightIndex];
		greenControl = gColors[gColorIndex].green * BrightLevels[gBrightIndex];
		blueControl = gColors[gColorIndex].blue * BrightLevels[gBrightIndex];
	}
#endif
	R_ADC_Start();
	R_PCLBUZ0_Stop();
	slowLightup();
	while(0){
		if (gEvents & evMeasureTemperature){
			clearEvent(evMeasureTemperature);
			break;
		}
	}
	openTM1650();
	initGestureChannel();
}

void slowLightup(void){
	uint8_t finishSlowLightup = 0;
	uint8_t waitCount =0;
	uint16_t lightLevel =0;
	//uint8_t levelStep = gLightLevel/5;
	//gLightLevel = gDefaultBrightIndex;
	uint16_t redStep = gColors[gColorIndex].red * BrightLevels[gBrightIndex]/100;
	uint16_t greenStep = gColors[gColorIndex].green * BrightLevels[gBrightIndex]/100;
	uint16_t blueStep = gColors[gColorIndex].blue * BrightLevels[gBrightIndex]/100;
	uint16_t red =0;
	uint16_t green = 0;
	uint16_t blue = 0;

	gAppTimerKick = 0;
	while(!finishSlowLightup){
		if (gAppTimerKick){ //Will increase waitCount every AppTimerKick 20mS
			waitCount++;
			gAppTimerKick = 0; //wait for next kick
		}
		if (waitCount >= LightupTime){
			red += redStep*gExpTable[lightLevel];
			green += greenStep*gExpTable[lightLevel];
			blue += blueStep*gExpTable[lightLevel];
			redControl = red;
			greenControl = green;
			blueControl = blue;
			//redControl = lightnessCurve(red);
			//greenControl = lightnessCurve(green);
			//blueControl = lightnessCurve(blue);
			waitCount = 0; //recount, will wait for systemkicktime X 5 ms
			lightLevel++;
		}else{
			//waitCount++;
		}
		if (lightLevel >= 14){
			finishSlowLightup =1;
		}
	}
}

//=== Adjust light color and lightness according to color index and light level
void adjustLight(uint8_t colorIndex, uint16_t lightLevel){
	redControl = gColors[colorIndex].red * BrightLevels[lightLevel];
	greenControl = gColors[colorIndex].green * BrightLevels[lightLevel];
	blueControl = gColors[colorIndex].blue * BrightLevels[lightLevel];
	
}

uint16_t lightnessCurve(uint16_t lightLevel){
	uint32_t result;
	uint16_t result16;
	result = lightLevel;
	result = result * lightLevel /12000 * lightLevel / 12000 ;
	result16 = result;
	return result16;
}
uint16_t lightnessCurve2(uint16_t lightLevel){
	uint32_t result;
	uint16_t result16;
	result = lightLevel;
	result = result * lightLevel /12000 * lightLevel / 12000 ;
	result16 = result;
	return result16;
}

void slowlyChangeLightC(void){ 
	static uint16_t redStep;
	static uint16_t greenStep;
	static uint16_t blueStep;
	static uint16_t red =0;
	static uint16_t green = 0;
	static uint16_t blue = 0;
	static uint8_t flashStage=0;
	uint16_t nowRead, nextRed;
	uint16_t nowGreen, nextGreen;
	uint16_t nowBlue, nextBlue;
	static uint8_t redDir, greenDir, blueDir; //direction for PWM increament.
		switch (variationStage){
			case 0: //initialize variables
				
				gParty1ColorIndex++;
				gParty1ColorIndex %= TotalParty1Colors;
				red = redControl;
				green=greenControl;
				blue=blueControl;
				
				nextRed = gColors_Party1[gParty1ColorIndex].red*BrightLevels[gBrightIndex];
				nextGreen = gColors_Party1[gParty1ColorIndex].green*BrightLevels[gBrightIndex];
				nextBlue = gColors_Party1[gParty1ColorIndex].blue*BrightLevels[gBrightIndex];
				
				if (red > nextRed){
					redDir=0;
					redStep = (red- nextRed)/100;
				}else{
					redDir=1;
					redStep = (nextRed-red)/100;					
				}
				if (green > nextGreen){
					greenDir = 0;
					greenStep = (green- nextGreen)/100;
				}else{
					greenDir=1;
					greenStep = (nextGreen-green)/100	;				
				}
				if (blue > nextBlue){
					blueDir = 0;
					blueStep = (blue- nextBlue)/100;
				}else{
					blueDir=1;
					blueStep = (nextBlue-blue)/100;					
				}
				variationStage = 1;
				brightnessIndex = 50; //number of step
				break;
			case 1: //switching 
				brightnessIndex--;
				if(redDir){
					red += redStep*2;
				}else{
					red -=redStep*2;
				}
				if (greenDir){
					green += greenStep*2;
				}else{
					green -= greenStep*2;
				}
				if (blueDir){
					blue += blueStep*2;
				}else{
					blue -= blueStep*2;
				}					
				redControl = red;
				greenControl = green;
				blueControl = blue;
				if (brightnessIndex==0){
					brightnessIndex=20;
					variationStage = 2;
				}					
			break;
			case 2: //buffer stage
				redControl = gColors_Party1[gParty1ColorIndex].red * BrightLevels[gBrightIndex];
				greenControl = gColors_Party1[gParty1ColorIndex].green * BrightLevels[gBrightIndex];
				blueControl = gColors_Party1[gParty1ColorIndex].blue * BrightLevels[gBrightIndex];
				brightnessIndex--;
				if (brightnessIndex==0){
					brightnessIndex=10;
					red = redControl;
					green = greenControl;
					blue = blueControl;
					if (gProbe1Hot){
						brightnessIndex=3; //This is how long the dark period lasts.
						variationStage = 3;
						redControl = 0;
						greenControl = 0;
						blueControl = 0;
					}else{
						if (gProbe2Hot){
							brightnessIndex=5;
							variationStage=5;
							flashStage = 0;
						}else{
							variationStage = 0;
							flashStage=0;
						}
					}
				}
				break;
			case 3: //dark Period
				brightnessIndex--;
				if (brightnessIndex<=0){
					brightnessIndex =5;
					variationStage=4;
				}
			break;
			case 4: //light up
				//brightnessIndex++;
				redControl = red;
				greenControl = green;
				blueControl = blue;
				brightnessIndex--;
				if (brightnessIndex<=0){
					if (gProbe2Hot){
						brightnessIndex=25;
						variationStage = 5;
						flashStage=0;
					}else{
						variationStage=0; //start again
					}
				}
			break;
			case 5: //probe2Hot, now bright for some time
				redControl = red;
				greenControl = green;
				blueControl = blue;
				brightnessIndex--;
				if (brightnessIndex<=0){
					brightnessIndex=3;
					variationStage=6;
				}
			break;
			case 6: //light off for sometime
				redControl = 0;
				greenControl = 0;
				blueControl = 0;
				brightnessIndex--;
				if (brightnessIndex<=0){
					redControl = red;
					greenControl = green;
					blueControl = blue;
					brightnessIndex=1; //lite up for 2 period long
					variationStage = 7;
				}
			break;
			case 7:			
				brightnessIndex--;
				if (brightnessIndex<=0){
					if (flashStage==1){
						variationStage=0; //start again
					}else{
						brightnessIndex=5;
						variationStage = 5; //flash on more time
					}
					flashStage++;
				}
				break;		
		}
}//slowlyChangeLightC()

void showStoveHot(void){ 
	static uint16_t redStep;
	static uint16_t greenStep;
	static uint16_t blueStep;
	static uint16_t red =0;
	static uint16_t green = 0;
	static uint16_t blue = 0;
	static uint8_t flashStage=0;
	uint16_t nowRead, nextRed;
	uint16_t nowGreen, nextGreen;
	uint16_t nowBlue, nextBlue;
	static uint8_t redDir, greenDir, blueDir; //direction for PWM increament.
		switch (variationStage){
			case 0: //initialize variables
				
				gParty1ColorIndex++;
				gParty1ColorIndex %= TotalParty1Colors;
				red = redControl;
				green=greenControl;
				blue=blueControl;
				
				redControl = ColorRed.red * BrightLevels[gBrightIndex];
				greenControl = ColorRed.green * BrightLevels[gBrightIndex];
				blueControl = ColorRed.blue * BrightLevels[gBrightIndex];
				
				variationStage = 1;
				brightnessIndex = 50; //number of step
				break;
			case 1: //switching 
				brightnessIndex--;
				redControl = red;
				greenControl = green;
				blueControl = blue;
				if (brightnessIndex==0){
					brightnessIndex=20;
					variationStage = 2;
				}					
			break;
			case 2: //buffer stage
				redControl = ColorRed.red * BrightLevels[gBrightIndex];
				greenControl = ColorRed.green * BrightLevels[gBrightIndex];
				blueControl = ColorRed.blue * BrightLevels[gBrightIndex];
				brightnessIndex--;
				if (brightnessIndex==0){
					brightnessIndex=10;
					red = redControl;
					green = greenControl;
					blue = blueControl;
					if (gProbe1Hot){
						brightnessIndex=3; //This is how long the dark period lasts.
						variationStage = 3;
						redControl = 0;
						greenControl = 0;
						blueControl = 0;
					}else{
						if (gProbe2Hot){
							brightnessIndex=5;
							variationStage=5;
							flashStage = 0;
						}else{
							variationStage = 0;
							flashStage=0;
						}
					}
				}
				break;
			case 3: //dark Period
				brightnessIndex--;
				if (brightnessIndex<=0){
					brightnessIndex =5;
					variationStage=4;
				}
			break;
			case 4: //light up
				//brightnessIndex++;
				redControl = red;
				greenControl = green;
				blueControl = blue;
				brightnessIndex--;
				if (brightnessIndex<=0){
					if (gProbe2Hot){
						brightnessIndex=25;
						variationStage = 5;
						flashStage=0;
					}else{
						variationStage=0; //start again
					}
				}
			break;
			case 5: //probe2Hot, now bright for some time
				redControl = red;
				greenControl = green;
				blueControl = blue;
				brightnessIndex--;
				if (brightnessIndex<=0){
					brightnessIndex=3;
					variationStage=6;
				}
			break;
			case 6: //light off for sometime
				redControl = 0;
				greenControl = 0;
				blueControl = 0;
				brightnessIndex--;
				if (brightnessIndex<=0){
					redControl = red;
					greenControl = green;
					blueControl = blue;
					brightnessIndex=1; //lite up for 2 period long
					variationStage = 7;
				}
			break;
			case 7:			
				brightnessIndex--;
				if (brightnessIndex<=0){
					if (flashStage==1){
						variationStage=0; //start again
					}else{
						brightnessIndex=5;
						variationStage = 5; //flash on more time
					}
					flashStage++;
				}
				break;		
		}
}//stoveHot()


void delay20ms(uint16_t deleyTime){
	gDelayCounter=0;
	while(gDelayCounter < deleyTime){
	}
}

void disPlayDigits(void){
	openTM1650();
	showDigit(4,8);
	showDigit(3,1);
	showDigit(2,0);
	showDigit(1,2);
}