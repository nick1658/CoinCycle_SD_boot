#define CH00  1
#define CH01  1<<1
#define CH02  1<<2
#define CH03  1<<3
#define CH04  1<<4
#define CH05  1<<5
#define CH06  1<<6
#define CH07  1<<7
#define CH08  1<<8
#define CH09  1<<9
#define CH10  1<<10
#define CH11  1<<11
#define CH12  1<<12
#define CH13  1<<13
#define CH14  1<<14
#define CH15  1<<15
#define CH16  1<<16
#define CH17  1<<17
#define CH18  1<<18
#define CH19  1<<19
#define CH20  1<<20
#define CH21  1<<21
#define CH22  1<<22
#define CH23  1<<23
#define CH24  1<<24
#define CH25  1<<25
#define CH26  1<<26
#define CH27  1<<27
#define CH28  1<<28
#define CH29  1<<29
#define CH30  1<<30
#define CH31  0x80000000
/*---------------------------------------------------------------------------*/
/*ʱ�䶨��*/
#define PerCent		250				/*10����*/
#define TenPerCent	PerCent * 10 	/*0.1��*/
#define Secend		TenPerCent * 10 /*1��*/
#define TenSecend	TenPerCent * 100 /*10��*/
#define Motor220Wait TenPerCent*8 /*0.8�뽻����������̷�ת�𻵵�ʱ����*/
#define Motor220_1_Max  Secend*8 /*1��220V����������ת��ʱ�䣬��ֵӦ��ʵ���г��Գ�0.5�룬����Ϊ�˰�ȫ*/
#define Motor220_2_Max  Secend*8 /*2��220V����������ת��ʱ�䣬��ֵӦ��ʵ���г��Գ�0.5�룬����Ϊ�˰�ȫ*/
#define Motor220_3_Max  Secend*8 /*3��220V����������ת��ʱ�䣬��ֵӦ��ʵ���г��Գ�0.5�룬����Ϊ�˰�ȫ*/
#define Motor220_4_Max  Secend*8 /*4��220V����������ת��ʱ�䣬��ֵӦ��ʵ���г��Գ�0.5�룬����Ϊ�˰�ȫ*/
#define Motor220_5_Max  Secend*8 /*5��220V����������ת��ʱ�䣬��ֵӦ��ʵ���г��Գ�0.5�룬����Ϊ�˰�ȫ*/
#define Motor220_6_Max  Secend*8 /*6��220V����������ת��ʱ�䣬��ֵӦ��ʵ���г��Գ�0.5�룬����Ϊ�˰�ȫ*/
#define Motor220_7_Max  Secend*8 /*7��220V����������ת��ʱ�䣬��ֵӦ��ʵ���г��Գ�0.5�룬����Ϊ�˰�ȫ*/
#define Motor220_8_Max  Secend*8 /*8��220V����������ת��ʱ�䣬��ֵӦ��ʵ���г��Գ�0.5�룬����Ϊ�˰�ȫ*/
#define Motor220_9_Max  Secend*8 /*9��220V����������ת��ʱ�䣬��ֵӦ��ʵ���г��Գ�0.5�룬����Ϊ�˰�ȫ*/
/*---------------------------------------------------------------------------*/
/*���ڴ������Ķ���2011-06-15*/
#define SeperaterOnStart  CH00 /*���������ֿ�Ƭ��λ      �ߺ�08*/
#define TapeOnStart       CH01 /*��������ֽ����λ        �ߺ�07*/
#define HostOnTapeStage   CH03 /*����������������ֽλ    �ߺ�05*/
#define HostOnHeatStage   CH02 /*���������������̺�λ    �ߺ�06*/
#define Tape2Ready        CH04 /*��������ֽ������        �ߺ�04*/
#define Tape1Ready        CH05 /*��������ֽ������        �ߺ�03*/
#define TapeRight         CH06 /*��������ֽ�����        �ߺ�02*/
#define TapeLeft          CH07 /*��������ֽ�����        �ߺ�01*/ 

#define BoardOverLow      CH08 /*���������г�������λ    �ߺ�16*/
#define BoardOverTop      CH09 /*���������г�������λ    �ߺ�15*/
#define HostOnLow         CH10 /*����������������λ      �ߺ�14*/
#define HostOnStart       CH11 /*����������������λ      �ߺ�13*/
#define HostOnTop         CH12 /*����������������λ      �ߺ�12*/
#define TurnerOnStart     CH13 /*����������ת��0λ       �ߺ�11*/
#define TapeWrapedRight   CH14 /*��������ֽ��������      �ߺ�10*/
#define TapeWrapedLeft    CH15 /*��������ֽ��������      �ߺ�09*/
#define ValueNumber	128
//#define HostOverLow       CH11 /*�����������������λ(΢������)   �ߺ�13*/
//#define HostOverTop       CH12 /*�����������������λ(΢������)   �ߺ�12*/

#define TapeHozontal      CH16 /*��������ֽ��ˮƽλ      �ߺ�24*/
#define StartBotton       CH17 /*������ť��              �ߺ�23*/
#define TapeOutBotton     CH18 /*��ֽ����ť��            �ߺ�22*/
#define TapeInBotton      CH19 /*��ֽ����ť��            �ߺ�21*/
#define MoneyOnBoard      CH20 /*�Ӻ��ն������Ĵ������źţ��г�������Ǯ��  �ߺ�20*/
#define WrapperWaitSensor CH21 /*�Ӻ��ն������Ĵ������źţ���������ʱ����  �ߺ�19*/
#define MoneyOnTape       CH22 /*�Ӻ��ն������Ĵ������źţ����ʹ���Ǯ��    �ߺ�18*/
#define BoxOnStart        CH23 /*����������������λ     �ߺ�17*/

#define PullStop          CH29 /*�Ʊ����׻ص���ʼλ        �ߺ�27*/
#define PushStop          CH30 /*�Ʊ����׵�ֹͣλ        �ߺ�26*/
#define TapeOnSeperator   CH31 /*ֽ���ڷֿ�Ƭλ��        �ߺ�25*/

/*---------------------------------------------------------------------------*/
/*ʱ�䳣���趨*/
#define WrapperWait  15000*TimerSpeed /*��������ʱ������ʱ��*/
#define CombineTime  89900 /*�ֿ�צ�ϲ���ʱ��*/
#define HostGoDownTime   40000*TimerSpeed /*��������ѹ��ѹ��10��Ǯ��ʱ��*/
#define BoardDownStep 65 /*200---340--280�г��彵һ�� ��Ӧ��������Ĳ���*/
#define BoardSpeed    6  /*�г����ٶ�*/
#define SpeedOfTapeIn 10  /*��ֽ�ٶ�*/
#define SpeedTapeWrap 4*TimerSpeed  /*��ֽ�ٶ�*/
#define AmountOfTapeBack 80 /*=80��ֽ�����е���ֽ��*/

#define AmountOfTapeIn 1050 /*Ԥ��ֽ��1000--1400--1230-1310-1120*/

#define TapeInMax  12000*TimerSpeed /*��ֽ�������ִ��ʱ�䣬������*/
#define TapeRunMax 63000*TimerSpeed /*��ֽ�������ִ��ʱ�䣬������   5000=1S*/
#define AfterHeat  15000 /*��ͷֹͣ���Ⱥ�ȴ�ʱ��        33000ʱ ��⵽��ͷ��ѹ��̧���ܹ�  4.78S����   20000   ��2.89  */
#define BoxGoBackMax 25000 /*�������˻ع���Ҫ�޶��ڴ�ʱ���ڣ���ʱ�ᱨ����Ӧ��>=20000*/
/*---------------------------------------------------------------------------*/
/*����LCD����Ķ���*/
#define LCD_RESET P031 /*P0.31��*/
#define LCD_SCLK  P028 /*P0.28��*/
#define LCD_CS    P029 /*P0.29��*/
#define LCD_SID   P030 /*P0.30��*/

/*����220V����Ķ���*/
#define P220_0 CH17 /*�������*/
#define P220_1 CH19 /*�������*/
#define P220_2 CH31 /*��*/
#define P220_3 CH28 /*��ͷ*/

/* --------------------------------------------------   */

#define P24_00 CH30 /*�ưѱ����׵�ŷ�1 ���ʹ���һ��Ǯ������        ���ϵ�������1��CH30�����ߺ�CY1*/
#define P24_08 CH27 /*�ưѱ����׵�ŷ�2 ���ʹ���һ��Ǯ������        ���ϵ�������2��CH27�����ߺ�CY2*/
#define P24_01 CH29 /*��צ���ף�                ���ϵ�������3���ߺ�CY3*/
#define P24_03 CH24 /*����C1�����̺�ֽ�����д������ϵ�������4���ߺ�CY4*/
#define P24_02 CH22 /*�������ף�                ���ϵ�������5���ߺ�CY5*/
#define P24_04 CH23    /*������������           ���ϵ�������6���ߺ�CY6,��10�ѱ�����*/
#define P24_09 CH11 /*����·��ŷ�              ���ϵ�������7���ߺ�CY7*/
#define P24_05 CH10  /*�գ�                      ���ϵ�������8��*/
#define P24_06 CH08 /*ȥ���ƹ�1                 ���ϵ�������9��        �ߺ�50*/
#define P24_07 CH07 /*ȥ���ƹ�2                 ���ϵ�������10��       �ߺ�51*/
#define P24_10 CH04
#define P24_11 CH01

/* --------------------------------------------------   */
/*���ڲ�������Ķ���2011-06-14����*/
/*�ֿ�Ƭ���*/
#define PU00 CH09 /*�������00������*/
#define DR00 CH06 /*�������00�ķ���*/
#define StepMotor00_ms 10; /*�������03һ���������ں�����*/
/*��ֽ���������*/
#define PU01 CH13 /*�������01������*/
#define DR01 CH15 /*�������01�ķ���*/
#define StepMotor01_ms 10; /*�������02һ���������ں�����*/
/*�г�����*/
#define PU02 CH26 /*�������02������*/
#define DR02 CH25 /*�������02�ķ���*/
#define StepMotor02_ms 10; /*�������05һ���������ں�����*/
/*��ֽ�е��*/
#define PU03 CH20 /*�������03������*/
#define DR03 CH18 /*�������03�ķ���*/
#define StepMotor03_ms 10; /*�������04һ���������ں�����*/

#define PU04 CH03 /*�������04������*/
#define DR04 CH02 /*�������04�ķ���*/
#define StepMotor04_ms 10; /*�������01һ���������ں�����*/

#define PU05 CH05 /*�������05������*/
#define DR05 CH00 /*�������05�ķ���*/
#define StepMotor05_ms 10; /*�������00һ���������ں�����*/

/*---------------------------------------------------------------------------*/
/*��������*/
/*��32λ���������뺯��*/
//unsigned int InputLow32(void);
//void ErrorReport(unsigned int ErrorNumber);
/*ͨ�ò������ת����������������������š����أ�ʣ��������*/
//unsigned int StepMotorRun(unsigned int Motor);
/*ͨ�ò���������ú�������������������ţ�����������������������������أ�1��ȷ��0��������Ŵ���*/
//unsigned int StepMotorSet(unsigned int Motor,unsigned int drection,long Pulse,unsigned int ms);
/*220V���״̬���������ޡ����أ�0��ת��P220_0����P220_1��*/
//unsigned int Motor220Status(void);
/*220V�������������ת��ʱ�䡣���أ�1��ʼת����0���ڽ���ȴ�*/
//unsigned int Motor220RunUp(unsigned int ms);
/*220V�������������ת��ʱ�䡣���أ�1��ʼת����0��������ȴ�*/
//unsigned int Motor220RunDown(unsigned int ms);
/*220V���ֹͣ*/
//void Motor220Stop(void);

/*�������г���(ʹ�ò������02)�ĳ�ʼ��*/
//void Process02Format(void);
/*��������ֽ��ʼ��*/
//void ProcessTapeIn(void);
/*---------------------------------------------------------------------------*/
/*������붨��*/
#define Err220OnTop       1001    /*������Ϣ��������������*//*����:���������λ*/  
#define Err220OnLow       1002    /*������Ϣ��������������*/
#define ErrPowerOut         1007    /*��ʾ:���ļ�ͣ��ť*/
#define ErrStepNumer        1003    /*������Ϣ��������ַǱ�׼�����*/
#define Err220TimeRunOut    1004    /*������Ϣ�������ת����ʱ*/
#define ErrTurnerOnStart    1005    /*����:��ת����û�лع���λ*/
#define ErrBoxOnStart		1008 	/*����:������û�лع���λ(������ѹ����)*/
#define	ErrBoardOverTop     1010  	/*����:�г��峬ʱδ������λ*/
#define	ErrTapeOnStart		1012 	/*����:��ֽ�г�ʱδ����λ*/ /*��ֽ������������Ҳû��0λ*/
#define ErrSeperaterOnStart 1013  	/*����:�ֿ�Ƭ��ʱδ����λ*/ /*����������Ҳû��0λ*/
#define ErrTapeReady 		1016 	/*����:ֽ��δ����λ(�����ǳ���������)*/ /*����������Ҳû�н�ֽ��λ*//*���ܿ�ֽ*/
#define ErrTapeEquip 		1018 	/*����:ֽ������*/


#define	ErrTapeWraped		1022	/*����:ֽ��δ����*/
#define	ErrTapeRun  		1023	/*����:��ֽ��ʱδ���*/
#define	ErrBoardOverLow		1024	/*����:�г���δ������λ ����������Ҳû������λ*/
#define ErrBoardOverLow2    1026	/*����:�г�����ǰ������λ,��������*/
#define ErrTapeOut          1027    /*����:�뻻ֽ��*/
#define ErrMoneyUnwrap      1028    /*����:Ǯ��ɢ��*/
#define ErrMoneyLess        1030    /*����:Ǯ�Ѳ���*/
#define ErrPush             1035    /*����30:���鷭ת������λ���г�������λ*/
#define ErrPush2            1037    /*������������δ����*/
#define ErrTapeGet          1041    /*����:��ֽ�����嶪ʧ*/

#define ErrMoneyOnBoard     1080    /*����:�����Ǯ�ѹ���*/
                                        /*����:���������Ǯ�������Ƿ�Ϊ��*/
#define ErrBoard            1081    /*����:��ȡ���г����ڵ�Ǯ��*/

/*---------------------------------------------------------------------------*/
