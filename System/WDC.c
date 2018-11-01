/**************************************************************
The initial Watch Dog
**************************************************************/
#include "S3C2416.h"


#define WatchDogTimer 30000
/*---------------------------------------------------------------------------*/
//��ʼ�����Ź�
void watchdog_reset(void)      //���Ź���λ����
{
//	Uart_Printf( "WatchDog setup! %dS\n",WatchDogTimer/10000);
	rWTCON=((37<<8)|(3<<3)); 
	/*---------------------------------------------------*/
			//���Ź�ʱ������ T = WTCNT * t_watchdog
			//���Ź�ι��
	rWTDAT=WatchDogTimer;
	rWTCNT=WatchDogTimer;
	/*---------------------------------------------------*/
	rWTCON &= ~(3<<1);    //��ֹ���Ź��ж�

	rWTCON|=((1<<5)|(1<<0));   //�������Ź���ʱ��������λ
//	while(1);
}
/*---------------------------------------------------------------------------*/
//���Ź�ι�� 
void SetWatchDog(void)
{
	rWTDAT=WatchDogTimer;
	rWTCNT=WatchDogTimer;
}
/*---------------------------------------------------------------------------*/