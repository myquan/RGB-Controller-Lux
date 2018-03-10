
#define EE_ADDR	0xA0
typedef struct {
	uint8_t flag;
	uint8_t workingMode;
	uint8_t lightingMode;
	uint8_t colorIndex;
	uint8_t brightIndex;
	uint8_t temperatureThreshold[3];
} ConfigData_t;

extern ConfigData_t gConfigData;

void saveConfig(void);
MD_STATUS writeEE(uint8_t* data, uint8_t byteNum);
MD_STATUS readEE(uint8_t* data, uint8_t byteNum);