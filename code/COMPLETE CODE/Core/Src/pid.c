#include "pid.h"
#include "encoder.h"
#include "inv_mpu.h"
#include "inv_mpu_dmp_motion_driver.h"
#include "mpu6050.h"
#include "motor.h"

//���������ݱ���
int Encoder_Left,Encoder_Right;
float pitch,roll,yaw;
short gyrox,gyroy,gyroz;
short	aacx,aacy,aacz;

//�ջ������м����
int Vertical_out,Velocity_out,Turn_out,Target_Speed,Target_turn,MOTO1,MOTO2;
float Med_Angle=3;//ƽ��ʱ�Ƕ�ֵƫ��������е��ֵ��
//����
float Vertical_Kp=-120,Vertical_Kd=-0.75;			//ֱ���� ��������Kp��0~1000��Kd��0~10��
float Velocity_Kp=-1.1,Velocity_Ki=-0.0055;		//�ٶȻ� ��������Kp��0~1��
float Turn_Kp=10,Turn_Kd=0.l;											//ת��

uint8_t stop;

extern TIM_HandleTypeDef htim2,htim4;
extern float distance;
extern uint8_t Fore,Back,Left,Right;
#define SPEED_Y 12 //����(ǰ��)����趨�ٶ�
#define SPEED_Z 150//ƫ��(����)����趨�ٶ� 

//ֱ����PD������
//���룺�����Ƕȡ���ʵ�Ƕȡ����ٶ�
int Vertical(float Med,float Angle,float gyro_Y)
{
	int temp;
	temp=Vertical_Kp*(Angle-Med)+Vertical_Kd*gyro_Y;
	return temp;
}

//�ٶȻ�PI������
//���룺�����ٶȡ�����������ұ�����
int Velocity(int Target,int encoder_L,int encoder_R)
{
	static int Err_LowOut_last,Encoder_S;
	static float a=0.7;
	int Err,Err_LowOut,temp;
	Velocity_Ki=Velocity_Kp/200;
	//1������ƫ��ֵ
	Err=(encoder_L+encoder_R)-Target;
	//2����ͨ�˲�
	Err_LowOut=(1-a)*Err+a*Err_LowOut_last;
	Err_LowOut_last=Err_LowOut;
	//3������
	Encoder_S+=Err_LowOut;
	//4�������޷�(-20000~20000)
	Encoder_S=Encoder_S>20000?20000:(Encoder_S<(-20000)?(-20000):Encoder_S);
	if(stop==1)Encoder_S=0,stop=0;
	//5���ٶȻ�����
	temp=Velocity_Kp*Err_LowOut+Velocity_Ki*Encoder_S;
	return temp;
}


//ת��PD������
//���룺���ٶȡ��Ƕ�ֵ
int Turn(float gyro_Z,int Target_turn)
{
	int temp;
	temp=Turn_Kp*Target_turn+Turn_Kd*gyro_Z;
	return temp;
}

void Control(void)	//ÿ��10ms����һ��
{
	int PWM_out;
	//1����ȡ�������������ǵ�����
	Encoder_Left=Read_Speed(&htim2);
	Encoder_Right=-Read_Speed(&htim4);
	mpu_dmp_get_data(&pitch,&roll,&yaw);
	MPU_Get_Gyroscope(&gyrox,&gyroy,&gyroz);
	MPU_Get_Accelerometer(&aacx,&aacy,&aacz);
	//ң��
	if((Fore==0)&&(Back==0))Target_Speed=0;//δ���ܵ�ǰ������ָ��-->�ٶ����㣬����ԭ��
	if(Fore==1)
	{
		if(distance<50)
			Target_Speed++;
		else
			Target_Speed--;
	}
	if(Back==1){Target_Speed++;}//
	Target_Speed=Target_Speed>SPEED_Y?SPEED_Y:(Target_Speed<-SPEED_Y?(-SPEED_Y):Target_Speed);//�޷�
	
	/*����*/
	if((Left==0)&&(Right==0))Target_turn=0;
	if(Left==1)Target_turn+=30;	//��ת
	if(Right==1)Target_turn-=30;	//��ת
	Target_turn=Target_turn>SPEED_Z?SPEED_Z:(Target_turn<-SPEED_Z?(-SPEED_Z):Target_turn);//�޷�( (20*100) * 100   )
	
	/*ת��Լ��*/
	if((Left==0)&&(Right==0))Turn_Kd=0.6;//��������ת��ָ�����ת��Լ��
	else if((Left==1)||(Right==1))Turn_Kd=0;//������ת��ָ����յ�����ȥ��ת��Լ��

	
	//2�������ݴ���PID�������������������������ҵ��ת��ֵ
	Velocity_out=Velocity(Target_Speed,Encoder_Left,Encoder_Right);
	Vertical_out=Vertical(Velocity_out+Med_Angle,roll,gyrox);
	Turn_out=Turn(gyroz,Target_turn);
	PWM_out=Vertical_out;
	MOTO1=PWM_out-Turn_out;
	MOTO2=PWM_out+Turn_out;
	Limit(&MOTO1,&MOTO2);
	Load(MOTO1,MOTO2);
	Stop(&Med_Angle,&roll);//��ȫ���
}
