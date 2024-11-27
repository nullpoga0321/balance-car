#include "control.h"

//INCLUEDS BEGIN
#include "encoder.h"
#include "inv_mpu.h"
#include "inv_mpu_dmp_motion_driver.h"
#include "mpu6050.h"
#include "pid.h"
#include "motor.h"
//INCLUEDS END 

//EXTERN BEGIN
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim4;
//EXTERN END

//PARAM BEGIN
int encoder_left,encoder_right;//编码器
float pitch,roll,yaw;//陀螺仪角度(MPU6050)
short gyrox,gyroy,gyroz;//角速度值
short aacx,aacy,aacz;//角加速度值

int vertical_out,velocity_out,turn_out;//直力环、速度环、转向环输出

int target_speed;//速度环的期望速度
int target_turn;//转向环的期望值

float med_angle;//平衡时角度值的偏移量

int moto1,moto2;//电机速度值(即占空比)
//PARAM END


/*
FUNCTION:PID控制
PARAMETER:~
NOTE:最好每隔10ms调用一次(然后想产生精准的延时效果采用软件延时肯定是
不准确的，因此我们用陀螺仪自带的输出引脚来触发中断)。
*/
void control(void){
    int pwm_out;

    //1、读取编码器和陀螺仪的数据
    encoder_right = read_speed(&htim2);
    encoder_right = -read_speed(&htim4);

    //2、读取陀螺仪的数据
    mpu_dmp_get_data(&pitch,&roll,&yaw);//读取角度
    MPU_Get_Gyroscope(&gyrox,&gyroy,&gyroz);//读取角速度
    MPU_Get_Accelerometer(&aacx,&aacy,&aacz);//读取加速度

    //3、将数据传给PID控制器，计算输出结果,即左右电机转速值
    velocity_out = velocity(target_speed,encoder_left,encoder_right);   //速度环(输出角度结果)
    vertical_out = vertical(velocity_out+med_angle,roll,gyrox);   //直力环
        /*
        这里速度环输出的角度结果加上med_angle是因为平衡小车在安装的时候，由于机械结构不对称等等因素
        ，当平衡小车平衡时陀螺仪的的角度可能不是0度所以要加上偏移量，这个偏移量我们可以在调参的时候测
        试出来。
        */
    turn_out = turn(gyroz,target_turn);     //转向环
    pwm_out = vertical_out;//传给电机pwm占空比即直力环的输出结果
    //转向赋予参数则是通过差速进行转向的
    moto1 = pwm_out - turn_out; 
    moto2 = pwm_out + turn_out;
        //当target_turn为0时则转向环的输出为0则moto1与moto2仅由直立环的输出控制


    limit(&moto1,&moto2);//限幅函数 用来限制电机的速度
    load(moto1,moto2);
}

