#include "pid.h"

//begin
float vertical_kp;  //比例
float vertical_kd;  //微分

float velocity_ki;  //积分
float velocity_kp;  //比例

float turn_kp;
float turn_kd;

uint8_t stop;
//end

//直力环PD控制器
//参数:期望角度、真实角度、角速度值
int  vertical(float med,float angle,float gyro_y)
{
    int temp;
    temp = vertical_kp * (med - angle) + vertical_kd * gyro_y;
    /*
    直立环输出 a = kp*(C-C1) + kd*C'
    其中C为小车当前的倾角，C'为倾角的微分，即角速度
    */
    return temp;
}

//速度环PI控制器
//输入:期望速度、真实速度(我们是由编码器进行测速的，所以包含两个速度值)
int velocity(int target,int encoder_l,int encoder_r)
{
    //1、计算偏差值
    int err;
    err = (encoder_l+encoder_r)-target;

    //2、低通滤波
    /*
    读传感器的时候滤波的操作是必不可少的
    在电路中高频的噪声是非常多的
    然后这种会使得我们的信号质量非常的差
    所以我们要通过一个低通滤波消除这种杂波
    */
    int err_lowout;
    static int err_lowout_last;
    static float a = 0.7;//典型的数据
    err_lowout = (1-a)*err + a*err_lowout_last;//误差值
    err_lowout_last = err_lowout;
        //直立环不用滤波是因为MPU6050已经进行了滤波操作

    //3、积分
    //在连续函数中，对于离散数据求积分其实就是求和(STM32中数据基本上都是离散的)
    static int encoder_s;
    encoder_s = encoder_s + err_lowout;

    //4、积分限幅
    encoder_s = encoder_s>20000?20000:(encoder_s<(-20000)?-20000:encoder_s);

    if(stop==1){
        encoder_s = 0;
        stop = 0;
    }

    //5、速度环的计算
    int temp;
    temp = velocity_kp*err_lowout + velocity_ki*encoder_s;
    return temp;
}

//转向环PD控制器
//输入:角速度、角度值(遥控时想要转的角度)
int turn(float gyro_z,float target_turn)
{
    int temp;
    temp = turn_kp*target_turn + turn_kd*gyro_z;
    return temp;
}

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


