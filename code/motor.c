#include "motor.h"

//begin
#define pwm_max 7200
#define pwm_min 7200
//end 


extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim3;

int abs(int p)
{
    if(p>0){
        return p;
    }else
    {
        return -p;
    }
}
void load(int motor1,int motor2)//-7200~7200
{
    if (motor1<0)
    {
        HAL_GPIO_WritePin(GPIOB,GPIO_PIN_13,GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12,GPIO_PIN_RESET);
    }
    else
    {
        HAL_GPIO_WritePin(GPIOB,GPIO_PIN_13,GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12,GPIO_PIN_SET);
    }
    /*然后我们还需要把我们的速度加载进去，即占空比加载到定时器里面*/
    __HAL_TIM_SetCompare(&htim1,TIM_CHANNEL_4,abs(motor1));

    if (motor2<0)
    {
        HAL_GPIO_WritePin(GPIOB,GPIO_PIN_14,GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOB,GPIO_PIN_15,GPIO_PIN_RESET);
    }
    else
    {
        HAL_GPIO_WritePin(GPIOB,GPIO_PIN_14,GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOB,GPIO_PIN_15,GPIO_PIN_SET);
    }
    /*然后我们还需要把我们的速度加载进去，即占空比加载到定时器里面*/
    __HAL_TIM_SetCompare(&htim1,TIM_CHANNEL_1,abs(motor2));
}



/*
FUNCTION:电机限幅函数

PARARMETER:电机1的指针 电机2的指针

NOTE:为什么用指针变量是因为我们这个函数的目的是为了修改我们传进来的
MOTO1和MOTO2的这个参数的值，这两个变量是我们调用在函数的外部，所以我们
想修改这个函数的变量的话，我们只能通过指针的方式来进行修改。
*/
void limit(int*motor1,int*motor2){
  if(*motor1>pwm_max){*motor1=pwm_max;}
  if(*motor1<pwm_min){*motor1=pwm_min;}
  if(*motor2<pwm_max){*motor2=pwm_max;}
  if(*motor2<pwm_min){*motor2=pwm_min;}
}

