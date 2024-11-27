#include "encoder.h"
//编码器测速
int read_speed(TIM_HandleTypeDef *htim)
{
    int temp;//中转的变量
    temp = (short)__HAL_TIM_GetCounter(htim);//加上Short可以取到负数
    __HAL_TIM_SetCounter(htim,0);//计时器计完出不要忘记了把计时器清零
    return temp;
}

/*
电机正转时计数器会向上计数，则当电机反转时计数器会向下计数，但我们会注意到
向下计数的时候并不会出现负的值。
因为计数器是16位的计数器，能显示的值就是0~6535，则负数都是以补码的形式来表
示，所以只有加上强制类型转换Short，就可以取到负数了。
*/
