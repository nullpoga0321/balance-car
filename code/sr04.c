#include "sr04.h" 
#include "pid.h"
extern TIM_HandleTypeDef htim3;

uint16_t COUNTER;//用来保存计数器里的值 
float DISTANCE;//用来保存距离

/*
根据我们的配置PA3为输出引脚,
PA2则为中断，EXTI2。
*/
void RCCdelay_us(uint32_t udelay)//利用指令耗时计算延迟
{
  __IO uint32_t Delay = udelay * 72 / 8;//(SystemCoreClock / 8U / 1000000U)
    //见stm32f1xx_hal_rcc.c -- static void RCC_Delay(uint32_t mdelay)
  do
  {
    __NOP();
  }
  while (Delay --);
}

void get_distance(void){
  /*
  由超声波模块原理，可知触发信号是10us的TTL(即高电平),
  然后是模块内部发出信号大概循环发出8个40KHZ的脉冲，
  最后输出回响信号，回响电平输出与检测距离成比例。
  */
  HAL_GPIO_WritePin(GPIOA,GPIO_PIN_3,GPIO_PIN_SET);
  RCCdelay_us(12);
  HAL_GPIO_WritePin(GPIOA,GPIO_PIN_3,GPIO_PIN_RESET);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
	if(GPIO_Pin == GPIO_PIN_2){
  //这就是我们的中断回响函数
  //在中断里面我们要判断是上升沿还是下降沿触发中断
  if( HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_2) == GPIO_PIN_SET ){
    //开启定时器之前我们要把定时器里面的计数器清零
    __HAL_TIM_SetCounter(&htim3,0);
    //继续打开计时器的时钟
  HAL_TIM_Base_Start(&htim3);}
  else //如果检测到的是低电平,即下降沿触发，我们则要让这个定时器停止计时
  { 
  HAL_TIM_Base_Stop(&htim3);
  COUNTER = __HAL_TIM_GetCounter(&htim3);
  DISTANCE = COUNTER/1000000*340*100/2;
  //因为计算是US为单位的所以我们除以10的六次方就得到了秒
  //然后乘上340我们就得到了米
  //再乘上100CM不要忘记除以2因为时间是两倍的(包含了发射和反射)
  //这里的单位是CM/S
  }
  //这里不用返回值的函数是因为我们定义的DISTANCE和COUNTER
  }
  if(GPIO_Pin==GPIO_PIN_5){
		control();
	}
}






