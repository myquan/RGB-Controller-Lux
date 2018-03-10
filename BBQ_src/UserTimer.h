
extern uint8_t gUserTimerCounting; //signify whether user timer is counting
extern uint16_t gUserTimerCount; //when gUserTimerCount reach 0, the time is out.
extern uint16_t gUserTimeLimit;  //4 count = 1 second


extern void checkUserTimeout(void);


