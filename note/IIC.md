# 关于I2C通信协议的介绍[^通信协议介绍]
[^通信协议介绍]:基于知乎作者**混说Linux**的《IIC通信协议，搞懂这篇就够了》
##### 2024年11月20日 黄迅
---
## 一、什么是**I2C(IIC)**?

首先I2C属于<u>两线式串行总线</u>,它被用于<u>微控制器(MCU)</u>和<u>外围设备(从设备)</u>进行<u>通信</u>的一种总线，**属于一主多从(一个<u>主设备Master</u>、多个<u>从设备Slave</u>)的总线结构，总线上的每个设备都有一个特定的设备地址，以区分一I2C总线上的其他设备**。

> **物理I2C接口**有==两根双向线==:
> - **串行时钟线(SCL)**
> - **串行数据线(SDA)**
> 
> *可用于数据的发送和接收数据，但是通信都是由主设备Master发起，从设备Slave被动响应，实现数据的传输。* 
> **SDA要配置成开漏输出模式**,且各添加一个上拉电阻。(SCL似乎要配置为开漏输出)

![i2c-1](note-photos\i2c-1.jpg)

---
## 二、I2C主设备与从设备一般的通信过程[^通信过程]

> **主设备给从设备发送\写入数据**:
> 1. 主设备发送起始信号(START)
> 2. 主设备发送设备地址到设备
> 3. 等待从设备响应ACK
> 4. 主设备发送数据到从设备，一般发送
> 5. 数据发送完毕，主设备发送停止(STOP)信号终止传输

![i2c-2](note-photos\i2c-2.jpg)
![i2c-7](note-photos\i2c-7.jpg)

> **主设备从从设备接收\读取数据**:
> 1. 设备发送起始信号(START)
> 2. 主设备发送设备地址到从设备
> 3. 等待从设备响应(ACK)
> 4. 主设备接收来自从设备的数据，一般接收的每个字节数据后会跟着向从设备发送一个响应(ACK)
> 5. 一般接收到最后一个数据后会发送一个无效响应(NACK)，然后主设备发送停止(STOP)信号终止传输

![i2c-3](note-photos\i2c-3.jpg)
![i2c-8](note-photos\i2c-8.jpg)

---
## 三、I2C通信的实现

- [ ]  使用I2C控制器实现
> 就是使用==芯片上的I2C外设==，也就是硬件I2C，<u>它有相应的I2C驱动电路，有专用的IIC引脚，效率更高，写代码会相对简单，只要调用I2C的控制函数即可</u>，不需要用代码去控制SCL、SDA的各种高低电平变化来实现I2C协议，只需要将I2C协议中的可变部分（如：从设备地址、传输数据等等）通过函数传参给控制器，控制器自动按照I2C协议实现传输，==但是如果出现问题，就只能通过示波器看波形找问题==。


- [x] **使用GPIO通过软件模拟实现**
> **软件模拟I2C比较重要，因为软件模拟的整个流程，哪里出来的BUG很快就能找到问题，且模拟一遍会对I2C通信协议更加熟悉**。
>
> 如果芯片上没有I2C控制器或者控制接口不够用了，通过**使用任意IO口去模拟实现I2C通信协议**，手动写代码去控制IO口电平变化，模拟I2C协议的时序，实现I2C的信号和数据传输。

[^通信过程]:具体的通信过程还需要看具体的**时序图**而定

---
## 四、I2C通信协议

I2C总线协议无非就是几样东西:**起始信号**、**停止信号**、**应答信号**、以及**数据有效性**。

#### 空闲状态
时钟线SCL和数据线SDA接上==上拉电阻==，==默认高电平==，表示总线是==空闲状态==
#### 从设备地址
从设备地址用来==区分总线上不同的从设备==。
> 一遍发送从设备地址的时候会在==最低位加上读|写信号==，比如设备地址为0x50,0表示读，1表示写，则读数据就会发送0x50，写数据就会发送0x51。
#### 起始信号(START)

I2C通信的起始信号由主设备发起，==SCL保持低电平==，==SDA由高电平跳变到低电平==
![i2c-4](note-photos\i2c-4.jpg)
```c
void i2c_start(void){
    //1、首先得把数据线设置为输出模式
    //总线空闲，时钟线SCL和数据线SDA输出高电平
    SCL = 1;
    SDA = 1;
    delay_ms(5);

    //SDA由高变低
    SDA = 0;
    delay_ms(5);

    //拉低SCL开始传输数据
    SCL = 0;
}
```
#### 停止信号STOP

I2C通信的==停止信号==由==主设备终止==，==SCL保持高电平==，==SDA由低电平跳变到高电平==。
![i2c-5](note-photos\i2c-5.jpg)
```c
void i2c_stop(void){
    //1.首先把数据线设置为输出模式

    //拉高时钟线
    SDA = 0;
    delay_us(5);
    SCL = 1;
    delay_us(5);

    //SDA由高变低
    SDA = 1;//SDA由低电平跳变到高电平，这是STOP信号
}
```
#### 数据有效性
I2C总线进行数据传送时，在SCL的每个时钟脉冲期间传输一个数据位，**时钟信号SCL为高电平期间，数据线SDA上的数据必须保持稳定**,==只有==在时钟线==SCL==上的信号为==低电平期间==，数据线SDA上的高电平或低电平状态才允许变化，因为当SCL是高电平时，数据新SDA的变化被规定**控制命令(START或STOP)***,也就是前面的起始信号和停止信号
> 就是说，SCL高电平，SDA高电平，拉低SDA，开始起止信号，这时候SCL拉低电平，表示进入开始通信，SDA可以随意变化即信号发送应答进行N次，要想终止通信则拉高SCL,SDA当然要在之前变为0，等待SCL拉高后，SDA拉高，表示通信终止 

![i2c-6](note-photos\i2c-6.jpg)

#### 应答信号(ACK:有效应答，NACK:无效应答)

接收端收到有效数据后向对方响应的信号，发送端==每发送一个字节(8位)数据==，在==第9个时钟周期释放数据线==去接收对方的应答。
> 当SDA是低电平为有效应答==ACK==，表示==对方接收成功==;
> 当SDA是高电平为无效应答==NACK==，表示==对方没有接收成功==;

发送数据需要等待接待方的应答:
```c
//等待ACK 1-无效 2-有效
uint_8 iic_wait_ack(void){
    uint_8 ack = 0;

    //数据线设为输入

    //拉高时钟线
    SCL = 1;
    delay_us(5);
    //获取数据线电平
    if (SDA = 1){
        //无效应答
        ack = 1;
        i2c_stop();
    }else{
        //有效应答
        ack = 0;
        //拉低SCL开始传输数据
        SCL = 0;
        delay_us(5);
    }
    return ack;
}
```
接收数据需要向发送方发送应答:
```c
void i2c_ack(uint_8 ack){
    //数据线设置为输出

    SCL = 0;
    delay_us(5);

    if(ack = 1){
        SDA = 1;//无效应答
    }
    else{
        SDA = 0;//有效应答
        delay_us(5);
        SCL = 1;
        //保持数据稳定
        delay_us(5);
        //拉低SCL开始传输数据
        SCL = 0;
    }
}
```
---
## ==五、更加完整的代码==[^jxkjdedaima]
[^jxkjdedaima]:基于BiliBili up主 **江协科技**的STM32教程
#### 初始配置
```c
void MyI2C_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_SetBits(GPIOB, GPIO_Pin_10 | GPIO_Pin_11);
}
```
#### 起始信号
起始信号和终止信号都由主机发送，主机需要发送起始信号时，SCL高电平期间，SDA从低电平切换到低电平。
```c
void i2c_w_scl(uint8_t bitvalue)//拉低或释放SCL(拉高)
{
    GPIO_WriteBit(GPIOB,GPIO_Pin_10,(BitAction)bitvalue);
    Delay_us(10);
}
void i2c_w_sda(uint8_t bitvalue)//给SDA写数据(拉低SDA\释放SDA(拉高))
{
    GPIO_WriteBit(GPIOB,GPIO_Pin_11,(BitAction)Bitvalue);
    Delay_us(10);
}
void myi2c_start(void)//起始信号
{
    i2c_w_sda(1);//释放SDA
    i2c_w_scl(1);//释放SCL
    i2c_w_sda(0);//拉低SDA
    i2c_w_scl(0);//拉低SCL
}
```
#### 停止信号
主机需要发送==停止信号==时：==SCL高电平==期间，==SDA从低电平切换到高电平==。
```c
void myi2c_stop(void)
{
    i2c_w_scl(0);
    i2c_w_sda(0);
    i2c_w_scl(1);//释放(拉高)SCL
    i2c_w_sda(1);//SCL高电平期间，SDA从低电平切换到高电平，为停止信号
}
```
#### 主机向从机发送一个字节数据
==SCL低电平期间==，==主机将数据依次给SDA写入数据（高位先行）==，然后释放SCL,==从机将在SCL高电平期间读取数据位==，所以SCL高电平期间SDA不允许有数据变化，依次循环上述过程8次，即可发送一个字节，从机读取到一个字节，会有一个应答信号。
![i2c-9](note-photos\i2c-9.jpg)
```c
/*
    发送数据前，主机会发送一个起始信号，由上面的程序可知：起始信号发送完后SCL和SDA都被拉低，所以函数里面不用在拉低，直接向SDA写入数据即可。
*/
void myi2c_sendbyte(uint8_t byte)//主机向从机发送一个字节，高位先行
{
    for(uin8_t i = 0;i<8;i++){
        i2c_w_sda(byte&(0x80>>i));//给SDA写入数据，只要Byte不是那么写入的就是1.因为BitAction
            //因为SCL为低电平期间，因此SDA为1不会触发停止信号
        i2c_w_scl(1);//释放SCL，从机读取数据
        i2c_w_scl(0);//拉低SCL，主机准备给SDA写入字节的次高位数据
    }
}

```
#### 主机向从机读取一个字节数据
接收一个字节:SCL低电平期间，从机将数据位依次放到SDA线上(高位先行)，然后释放SCL,主机在SCL高电平期间，所以SCL高电平期间SDA不允许有数据变化，依次循环上述过程8次即可接收一个字节(==主机在接收之前，需要释放SDA把SDA拉高==)
```c
uint8_t I2C_R_SDA(void)//主机读SDA数据，即判断引脚的电平
{
	uint8_t BitValue;
	BitValue = GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_11);
	Delay_us(10);
	return BitValue;
}

/*
	主机接收数据之前，主机会给从机发送一个字节，由上面的代码得发送完数据后此时SCL为低电平。之后释放SDA总线让从机拥有SDA总线的控制权。
*/
uint8_t MyI2C_ReceiveByte(void)
{
	uint8_t Byte = 0x00;//接收数据的变量
	I2C_W_SDA(1);//主机释放SDA，让控制权给从机，而SCL也是低电平，释放的一瞬间，从机就给SDA写入了数据
	
	I2C_W_SCL(1);
	if(I2C_R_SDA() == 1)//读取第一位数据
	{
		Byte |= 0x80;
	}
	I2C_W_SCL(0);//主机拉低SCL，让从机给SDA写入数据	
	
	I2C_W_SCL(1);//主机拉高SCL，准备开始读取SDA上面的数据
	if(I2C_R_SDA() == 1)
	{
		Byte |= 0x40;
	}
	I2C_W_SCL(0);//主机拉低SCL，让从机给SDA写入数据	
	return Byte;
}

//优化代码
uint8_t MyI2C_ReceiveByte(void)
{
	uint8_t Byte = 0x00;//接收数据的变量
	I2C_W_SDA(1);//主机释放SDA，让控制权给从机
	
	for(uint8_t i =0; i<8;i++)
	{
		I2C_W_SCL(1);//主机拉高SCL，准备开始读取SDA上面的数据
		if(I2C_R_SDA() == 1)
		{
			Byte |= (0x80 >> i);
		}	
		I2C_W_SCL(0);//主机拉低SCL，准备让从机给SDA写入数据	
	}
	return Byte;
}
```
#### 主机接收应答
接收应答:主机在发送完一个字节之后，在下一个时钟接收一位数据，判断从机是否应答，数据0(SDA被从机拉低)表示应答，数据1(==SDA没有被从机拉低)表示非应答(主机在接收之前，需要释放SDA，即SDA拉高==)
```c
uint8_t I2C_R_SDA(void)//主机读SDA数据，即判断引脚的电平
{
	uint8_t BitValue;
	BitValue = GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_11);
	Delay_us(10);
	return BitValue;
}

uint8_t MyI2C_ReceiveACK(void)//主机接收应答
{
/*
	主机发送完一个字节后，SCL为低电平，
	等待从机给SDA上面写入数据，如果拉低则代表接收成功
*/
	uint8_t ACKBit;
	I2C_W_SDA(1);//主机释放SDA，让控制权给从机
	I2C_W_SCL(1);//主机释放SCL，准备开始读取SDA上面的数据
	ACKBit = I2C_R_SDA();
	I2C_W_SCL(0);//主机拉低SCL，进入下一个时序单元，主机准备给SDA写入字节的数据
	return ACKBit;
}
```
#### 主机发送应答
主机在接收完一个字节之后，在下一个时钟发送一位数据，数据0表示应答，数据1表示非应答。
```c
void MyI2C_SendACK(uint8_t ACKBit)//主机发送应答
{
/*
	从机发送完一个字节后，SCL为低电平，
	等待主机给SDA上面写数据，如果拉低代表接收成功。
*/
	
	I2C_W_SDA(ACKBit);//主机给SDA写入应答信号，0为应答，1为非应答
	I2C_W_SCL(1);//主机拉高SCL，让从机读取应答信号
	I2C_W_SCL(0);//主机拉低SCL，准备写入数据
}
```
