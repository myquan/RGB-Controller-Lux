#include "r_cg_macrodriver.h"
#include "EEPROM.h"
#include "taskHandler.h"

#include "QApp.h"
#include "r_cg_intc.h"
#include "r_cg_timer.h"

uint8_t gUserTimerCounting=0; //signify whether user timer is counting
uint16_t gUserTimerCount=0; //when gUserTimerCount reach 0, the time is out.
uint16_t gUserTimeLimit=60;  //1 count = 1 second, so when user timer is started, initail internalUserTimerCount= gUserTimerLimit x 4
static uint16_t internalUserTimerCount;

void startUserTimer(void){
	internalUserTimerCount= gUserTimeLimit * 4;
	
}

void checkUserTimeout(void){

	if (internalUserTimerCount==0){
		sendEvent(evTimeout);
	}else{
		internalUserTimerCount--;		
	}
			
}





