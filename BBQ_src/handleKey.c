#include "r_cg_macrodriver.h"
#include "handleKey.h"


uint8_t gPressedKey = NoKey; //記錄那一支按鍵被按下
uint8_t gKeyPressedDuration = 0;
uint8_t gDoubleKeyFlag=0;  //This variable is used to detect double key pressed for left and right keys. If this flag is true. Then double key is pressed.
