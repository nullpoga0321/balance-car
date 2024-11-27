#ifndef __MOTOR_H__
#define __MOTOR_H__

#include "stm32f1xx_hal.h"

void load(int motor1,int motor2);
int abs(int p);
void limit(int*motor1,int*motor2);

#endif
