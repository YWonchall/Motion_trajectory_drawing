## 运动轨迹绘制

## 写在前面

​	这个是当时某项作业的一小部分，并没有花太多时间仔细研究，很多地方仅仅是理论可行，实际存在很大误差，仅供参考。作图部分不准确，可能是作图的代码不适合或者传感器误差太大，更大的原因个人觉得是对于传感器的数据处理不到位。本项目仅仅实现了六轴传感器、单片机、PC之间通过串口的通信，在数据处理及绘图方面仅仅做了中学物理水平的计算。具体细节还需自己研究一下。本项目分为两部分：

- 51单片机：读取串口数据、对数据处理、数据通过向PC端发送
- PC：一个python程序读取单片机传送的数据、进行简单处理及图像绘制

## 单片机部分：

主要就是两个串口的配置，主逻辑：串口A不停的接收六轴传来的数据，每接收一组

进行一次数据处理，然后通过串口B发送

代码如下：

```c
#include <stc8.h>
#include <math.h>
#define time 0.05
int i,j;
float b = 10.151;
float c = 5.00;
unsigned char testNum[3][5] = {
{0x51,1,1,1,1},{0x52,2,2,2,2},{0x53,3,3,3,3}
};
unsigned char str[32]= {0};
unsigned char *ptrTxd;
unsigned char cntTxd = 0;
unsigned char Re_buf[11];
unsigned char counter=0;
unsigned char ucStra[6];
float aBefore[3] = {0.0, 0.0, 0.0};
float aAfter[3] = {0.0, 0.0, 0.0};
float v[3] = {0.0, 0.0, 0.0};
float s[3] = {0.0, 0.0, 0.0};
int m =5000;
void configUART1();
void configUART2();
void Timer0Init(void);
void sendData(float* dat);
void main()
{
	EA = 1;
	ES = 1;
	ET0 = 1;
	IE2 = 0x01;
	configUART1();
	configUART2();
	Timer0Init();
	while(1)
	{	
		aAfter[0] = ((short)(ucStra[1]<<8| ucStra[0]))/32768.0*16.0;
		aAfter[1] = ((short)(ucStra[3]<<8| ucStra[2]))/32768.0*16.0+0.25;
		aAfter[2] = ((short)(ucStra[5]<<8| ucStra[4]))/32768.0*16.0-0.55;
		for(i=0;i<3;i++)
		{		
			if(fabs(aAfter[i])<0.1)
				aAfter[i] = 0.0;
		}
		
		for(j=0;j<3;j++)
		{
			for(i=0;i<4;i++)
			{
				testNum[j][i+1] = ((unsigned char*)&s[j])[i];;
			}
			ptrTxd = testNum[j];
			cntTxd = sizeof(testNum[j]);
			TI = 1;
			while(cntTxd!=0);
			while(m--);
			m=5000;
		}
	}
}


void configUART1()
{
	SCON = 0x50;		//8位数据,可变波特率
	AUXR &= 0xBF;		//定时器1时钟为Fosc/12,即12T
	AUXR &= 0xFE;		//串口1选择定时器1为波特率发生器
	TMOD &= 0x0F;		//设定定时器1为16位自动重装方式
	TL1 = 0xE8;		//设定定时初值
	TH1 = 0xFF;		//设定定时初值
	ET1 = 0;		//禁止定时器1中断
	TR1 = 1;		//启动定时器1
}

void configUART2()
{
	S2CON = 0x50;		//8位数据,可变波特率
	AUXR &= 0xFB;		//定时器2时钟为Fosc/12,即12T
	T2L = 0xE8;		//设定定时初值
	T2H = 0xFF;		//设定定时初值
	AUXR |= 0x10;		//启动定时器2
}

void Timer0Init(void)		//50毫秒@11.0592MHz
{
	AUXR &= 0x7F;		//定时器时钟12T模式
	TMOD &= 0xF0;		//设置定时器模式
	TL0 = 0x00;		//设置定时初值
	TH0 = 0x4C;		//设置定时初值
	TF0 = 0;		//清除TF0标志
	TR0 = 1;		//定时器0开始计时
}

void interruptTimer0() interrupt 1
{
	TF0 = 0;	
	for(i=0;i<3;i++)
	{
		s[i] = s[i] + v[i]*time + 0.25*(aAfter[i]+aBefore[i])*time;
		v[i] = v[i] + (aBefore[i]+aAfter[i])*0.5*time;
		aBefore[i] = aAfter[i];
	}
	
}

void interruptUART1() interrupt 4
{
	if(RI)
	{
		RI = 0;
		SBUF = SBUF;
	}
	if(TI)
	{
		TI = 0;
		
		if(cntTxd > 0)
		{
			SBUF = *ptrTxd;
			cntTxd--;
			ptrTxd++;
	  }
		
  }
}

void interruptUART2() interrupt 8
{
	if(S2CON&0x02)
	{
		S2CON &= ~0x02;
	}
	if(S2CON&0x01)
	{
		S2CON&=~0x01;
		Re_buf[counter] = S2BUF;
		if(counter==0&&Re_buf[0]!=0x55) return;      //第0号数据不是帧头          
	  counter++; 
	  if(counter==11)             //接收到11个数据
	  {    
			counter=0;//重新赋值，准备下一帧数据的接收        
			if(Re_buf [1] == 0x51)
			{
				ucStra[0]=Re_buf[2];
				ucStra[1]=Re_buf[3];
				ucStra[2]=Re_buf[4];
				ucStra[3]=Re_buf[5];
				ucStra[4]=Re_buf[6];
				ucStra[5]=Re_buf[7];
			} 
	  }
	}
}
```

## PC部分：

这部分比较简单，逻辑为：接收单片机串口数据（加速度），进行简单的物理计算坐标，图像绘制。

代码如下：

```
import datetime
import matplotlib.pyplot as plt
import serial
import struct
import binascii

now = datetime.datetime.now()
end1 = datetime.datetime.now()+datetime.timedelta(seconds=4)
end2 = datetime.datetime.now()+datetime.timedelta(seconds=6)
x=[]
y=[]
z=[]
data = [0,0,0]
num = [31,133,135,64,12]
def packData(data):
    num.pop(4)
    data.append(struct.unpack('<f', struct.pack('4B', *num))[0])
    num.append(0)
t = serial.Serial('com3',9600)

i=0
j=0
k=0
while(1):
    str = t.read()
    a = binascii.b2a_hex(str).decode()
    num[i] = eval('0x'+a)
    if(i==0 and num[0]!=0x51 and num[0]!=0x52 and num[0]!=0x53):
        pass
    else:
        i+=1
        if(i==5):
            i=0
            j+=1
            num.reverse()
            if(num[4]==0x51):
                packData(x)
            elif(num[4]==0x52):
                packData(y)
            else:
                packData(z)
            if(end1<=datetime.datetime.now()<=end2):
                break

# 打开画图窗口1，在三维空间中绘图
fig = plt.figure(1)
ax = fig.gca(projection='3d')
# 给出点（0，0，0）和（100，200，300）
# 将数组中的前两个点进行连线
figure = ax.plot(x, y, z, c='r')
plt.show()
```

整个工程已发布在github：https://github.com/YWonchall/Motion_trajectory_drawing

感谢阅读！