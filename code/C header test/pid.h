#ifndef __PID_H__
#define __PID_H__

#include "stm32f1xx_hal.h"
int  vertical(float med,float angle,float gyro_y);
int velocity(int target,int encoder_l,int encoder_r);
int turn(float gyro_z,float target_turn);

#endif
