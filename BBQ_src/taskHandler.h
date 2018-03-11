
//Total 16 possible mode specific events, i.e., events that will be processed differently in defferent modes.
#define	KeyShortPressed 	0x0001
#define	KeyLongPressed		0x0002
#define ChangeLightLevel	0x0004
#define evChangeMode	0x0008
#define evChangeLightingMode	0x0010
#define evStartUserTimer	0x0020
#define evUpdateTemperature	0x2000//
#define evUpdateTimeLeft	0x2000//Total 16 possible events

//Common events (Max 16 events). Common events are events that are processed the same way in every modes.
#define	evMeasureTemperature 	0x0001
#define	evStoveHot		0x0002
#define evProb1Hot	0x0004
#define evProb2Hot	0x0008
#define evTimeout	0x0010
#define evADCReady	0x0020
#define evRoutineJob	0x0040
#define evWarm			0x0080
#define evHot			0x0100
#define evToggleDP		0x0800
#define evToggleLight	0x1000

extern uint16_t gModeEvents; //event for specific mode
extern uint16_t gEvents;  //common events

void handleModeTask(void); //Handle task for specific mode
void handleCommonTask(void); //Handle task common to all modes
void handleUITask(void); //Manage how the lights changes (User Interface handler)

extern void handleColorModeEvent(void);
extern void handleLevelModeEvent(void); //Change light level mode
extern void sendEvent(uint16_t event);
extern void clearEvent(uint16_t event);

extern void sendModeEvent(uint16_t event);
extern void clearModeEvent(uint16_t event);


