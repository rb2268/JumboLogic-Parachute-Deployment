#ifndef BUTTONS_H
#define BUTTONS_H
#include "ee14lib.h"


void setupButtons(EE14Lib_Pin input1, EE14Lib_Pin input2);
void runButtons(EE14Lib_Pin input);
void turnMotor(EE14Lib_Pin input);

#endif