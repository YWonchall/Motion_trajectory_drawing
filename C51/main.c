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
	SCON = 0x50;		//8λ����,�ɱ䲨����
	AUXR &= 0xBF;		//��ʱ��1ʱ��ΪFosc/12,��12T
	AUXR &= 0xFE;		//����1ѡ��ʱ��1Ϊ�����ʷ�����
	TMOD &= 0x0F;		//�趨��ʱ��1Ϊ16λ�Զ���װ��ʽ
	TL1 = 0xE8;		//�趨��ʱ��ֵ
	TH1 = 0xFF;		//�趨��ʱ��ֵ
	ET1 = 0;		//��ֹ��ʱ��1�ж�
	TR1 = 1;		//������ʱ��1
}

void configUART2()
{
	S2CON = 0x50;		//8λ����,�ɱ䲨����
	AUXR &= 0xFB;		//��ʱ��2ʱ��ΪFosc/12,��12T
	T2L = 0xE8;		//�趨��ʱ��ֵ
	T2H = 0xFF;		//�趨��ʱ��ֵ
	AUXR |= 0x10;		//������ʱ��2
}

void Timer0Init(void)		//50����@11.0592MHz
{
	AUXR &= 0x7F;		//��ʱ��ʱ��12Tģʽ
	TMOD &= 0xF0;		//���ö�ʱ��ģʽ
	TL0 = 0x00;		//���ö�ʱ��ֵ
	TH0 = 0x4C;		//���ö�ʱ��ֵ
	TF0 = 0;		//���TF0��־
	TR0 = 1;		//��ʱ��0��ʼ��ʱ
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
		if(counter==0&&Re_buf[0]!=0x55) return;      //��0�����ݲ���֡ͷ          
	  counter++; 
	  if(counter==11)             //���յ�11������
	  {    
			counter=0;//���¸�ֵ��׼����һ֡���ݵĽ���        
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