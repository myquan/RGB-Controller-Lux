
//=== App related ===
extern uint16_t gDelayCounter;
extern void delay20ms(uint16_t delayTime);
extern uint32_t gTimerCounter;
extern uint8_t gDPOn; //DP is shown or not

//=== Temperature Functions ===
#define MeasureInterval	2  //in second
#define MeasureTimerCountLimit	MeasureInterval * 100
#define TemperatureThresholdInF	85
//#define TemperatureThresholdInC (TemperatureThresholdInF - 32)*5/9

extern uint8_t gDisplayFlag;

extern uint16_t gMeasureTimerCount;
extern uint16_t getTemperature(void);


//=== SlowLightup ===
#define LightupTime 3
#define PartyModeColorInterval	50
#define PartyMode2Speed			2
//=== Pin Defines ===
#define functionKey P5.0
#define upKey	P13.7
#define downKey P5.1
#define lite	P3.0
#define buzzer	P7.5
#define gas20	P7.0
#define gas40	P7.1
#define gas60	P7.2
#define gas80	P7.3
#define gas99	P7.4
//Indicator LED. Total 5 LEDs for 5 working modes.
#define indLight	P2.2
#define indStove	P2.3
#define indTimer	P2.4
#define indProbe1	P2.5
#define indProbe2	P2.6
#define igniterControl	P2.7

//ADC input for each probe
#define ProbeStove	22
#define Probe1		18
#define Probe2		19
//=== RGB related ===
//*=== setting 1 ===
	#define blueControl TDR07
	#define greenControl TDR03
	#define redControl TDR01
	#define RedColorIndex	0
	#define GreenColorIndex	1
	#define BlueColorIndex	2
	#define YellowColorIndex	3
#define WhiteColorIndex		4

#define DefaultColorIndex	4
#define DefaultBrightIndex	0

//=== PWM Defines ===
#define Lightness20P	0x0960
#define Lightness30P	0x0E10
#define Lightness35P	0x1068
#define Lightness40P	0x12C0
#define Lightness50P	0x1770
#define Lightness65P	0x1E78
#define Lightness100P	0x2EE0

#define TotalBrightLevels 	4
#define TotalColors			6
#define TotalParty1Colors	3

//=== Global Variables ===
enum {
	LightMode=0,
	StoveMode,
	TimerMode,
	Probe1Mode,
	Probe2Mode	
}; //Working Modes

enum {
	MonoMode,
	PartyMode
}; //Working Modes

typedef struct {
	uint16_t red;
	uint16_t green;
	uint16_t blue;
} ColorType;

extern uint8_t gWorkingMode;
extern uint8_t gDefaultBrightIndex;
extern uint8_t gBrightIndex; //Index of Light level arry
extern uint8_t gColorIndex;
extern uint16_t BrightLevels[TotalBrightLevels]; //4 levels of lightness
extern ColorType gColors[TotalColors];
extern uint8_t gAppTimerKick; //
extern uint8_t appTimerKickCount;
extern uint8_t gSettingDirty; //Set when anyone of the setting is changed. Application will use this flag to do EE saving.
extern uint8_t gGodTimerCount;

extern uint8_t gStoveHot; //the PT1000 is hot
extern uint8_t gProbe1Hot; //the NTC is hot
extern uint8_t gProbe2Hot; //the NTC is hot
extern uint8_t gTimerIsOn; //the User Timer in on.
extern uint8_t gLightingMode; //the lighting mode.
extern uint8_t gProbe; //which probe to be measured.

extern void init_variables(void);
extern void init_app(void);
extern void slowLightup(void);
extern void adjustLight(uint8_t colorIndex, uint16_t lightLevel); //adjust LED's color and lightness according to gBrightIndex and gLightLevel
extern void slowlyChangeLightC(void);
extern void showStoveHot(void);
extern void delayms(uint8_t deleyTime);
extern void disPlayDigits(void);
