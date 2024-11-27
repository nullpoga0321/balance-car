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

