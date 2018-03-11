#include "r_cg_macrodriver.h"
#include "EEPROM.h"

#include "QApp.h"
#include "r_cg_intc.h"
#include "r_cg_timer.h"
#include "r_cg_serial.h"
#include "r_cg_userdefine.h"

typedef struct {
	uint8_t addr;
	ConfigData_t data;
} DataToWrite_t;

DataToWrite_t data2Write;


void saveConfig(void){
	uint16_t i;
	data2Write.addr=0;
	data2Write.data = gConfigData;
	gIICA0Done=0;
	writeEE(&data2Write,17);
	while(!gIICA0Done);
	for(i=0; i<25000; i++){
		NOP();
	}

}
MD_STATUS writeEE(uint8_t* data, uint8_t byteNum){
	MD_STATUS result;
	result= R_IICA0_Master_Send(EE_ADDR, data, byteNum, 16);
	return result;
}

MD_STATUS readEE(uint8_t* data, uint8_t byteNum){
	MD_STATUS result;
	result = R_IICA0_Master_Receive(EE_ADDR, data, byteNum, 16);
	return result;
}

