#define Code_C	13
#define Code_L	14
#define Code_R	15
#define Code_O	16
#define Code_F	17


extern void openTM1650(void);
extern void showDigit(uint8_t place, uint8_t digit);
extern void showTemperature(uint8_t unit, uint32_t temperature);
extern void showTimer(uint16_t timeCount);
	