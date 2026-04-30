#ifndef MOTOR_H
#define MOTOR_H

#include "ee14lib.h"

EE14Lib_Err motor_init(TIM_TypeDef* const timer);
EE14Lib_Err motor_move_to(TIM_TypeDef* const timer, const EE14Lib_Pin pin, const unsigned int duty);

EE14Lib_Err motor_open(TIM_TypeDef* const timer, const EE14Lib_Pin pin);
EE14Lib_Err motor_close(TIM_TypeDef* const timer, const EE14Lib_Pin pin);

#endif