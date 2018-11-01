
#ifndef __TQ2416_YQCOIN_H__
#define __TQ2416_YQCOIN_H__



#include "def.h"
#include "yqsqlite3.h"


typedef struct
{
	U32 total_money;
	U32 total_good;
	U32 total_ng;
	U32 total_coin;
	U32 coinnumber;
	S16 ad2_averaged_value;
}s_processed_coin_info;

extern s_processed_coin_info processed_coin_info;

#define COINCNUM 4     //目前 可以 计数的 国家币种  的总数量 
//#define RATIONCOINNUM 9   //每个国家币种最多9种
//extern U8 coine[COINCNUM][9];  // 币种决定 
extern int alertname[COIN_TYPE_NUM];// 币种决定 

extern U32 coin_num[COIN_TYPE_NUM];    //各币种 计数常量
void coincount(void);	// get average value and print the value,then compare who is who 
void my_coincount(void);	// get average value and print the value,then compare who is who 
void my_precoincount(void);	// get average value and print the value,then compare who is who 
extern  U16 ccstep;


//extern unsigned int temp_coinnum;   //for compare the coin number, check 
#define NUMTIME 5  //通道与通道间的 报错时间

//#define ADTIMETWO   1400  //在这段时间里 只有两个AD发生变化，则踢除此币  800
//#define ADTIMEALL 1600     //如果在这段时间里，只有一个发生变化，清除变化 1000

extern  U32 ch0_counttemp;  // 前一次通道0 通过的硬币计数 
extern  U32 ch1_counttemp;  // 前一次通道1 通过的硬币计数 
extern  U32 ch2_counttemp;  // 前一次通道2 通过的硬币计数 
extern  U32 coinlearnnumber;

void coinlearn(void);   //自学习的程序
void my_coinlearn(void);   //自学习的程序

extern  S16 coin_maxvalue0;
extern  S16 coin_minvalue0;
extern  S16 coin_maxvalue1;
extern  S16 coin_minvalue1;
extern  S16 coin_maxvalue2;
extern  S16 coin_minvalue2;


extern  U16 coine[COINCNUM][COIN_TYPE_NUM];

extern  U32 zs_precount;   //当前预置计数 总数
extern  U32 ze_precount;   //当前预置计数 总额
void precoincount(void);     //   预置 计数

#define DT_STORE 	0x01	//0x80	// manbi   ch02 duishe   EINT1/GPF1
#define DT_PAN  	0x02  	//caoxing   ch01 zhuanpan   EINT0/GPF0  
#define DT_PUSH  	0x04	//0x04	// yabitiao  ch07 EINT11/GPG3
#define DT_PANDOOR  0x80	//0x08	// zhuanpan men  ch05  EINT3/GPF3
#define DT_DOOR	 	0x40	//64	// men     ch03   EINT2/GPF2
#define DT_FORGE  	0x20	//32	// weibi   ch04  EINT4/GPF4
#define DT_COIN 	0x08		//0x80 // coin  cho5  
void detect_read(void);   

/*****************
报错宏定义    函数
*****************/

#define SWITCHERROR 11      //门没有关上
#define STORAGEERROR 12
#define DUIERROR 13     // 币斗满 
#define PANBLOCKEDERROR 14     //转盘赌币
#define KICKCOINERROR 15
#define NEGATIVEERROR 16
#define PRESSMLOCKED  17      //压币带堵币
#define ADSTDEEROR    20       //表示传感器下有币
#define RTCREEROR    21       //表示READ time wrong
#define READOUTDATA    22       // 读出  数据  内存满
#define COUNT_FINISHED 18  	// 1 MCU接到 有清分斗满币 停止	


#define ALERT0 0  //向地址ADDR_TTS 发送"请调基准值"
#define ALERT1 1   // //向地址ADDR_TTS 发送"请调基准值"
#define ALERT3 3  //向地址ADDR_TTS 发送"转盘堵币"
#define ALERT4 4   // //向地址ADDR_TTS 发送"压币带堵币"
#define ALERT5 5   // 踢币异常
#define ALERT6 6  //向地址ADDR_TTS 发送"币斗满"
#define ALERT7 7   // //向地址ADDR_TTS 发送"确认需长按1秒生效"
#define ALERT8 8   // //向地址ADDR_TTS 发送"请导出数据，内存已满"
#define ALERT9 9   // //向地址ADDR_TTS 发送"清分斗满币 请取出"


#define ALERT20 20   // //向地址ADDR_TTS 发送"一元预置值已到"
#define ALERT21 21   // //向地址ADDR_TTS 发送"0.5元预置值已到"
#define ALERT22 22   // //向地址ADDR_TTS 发送"0.1元预置值已到"
#define ALERT23 23   // //向地址ADDR_TTS 发送"0.05元预置值已到"
#define ALERT24 24   // //向地址ADDR_TTS 发送"0.02元预置值已到"
#define ALERT25 25   // //向地址ADDR_TTS 发送"0.01元预置值已到"
#define ALERT26 26   // //向地址ADDR_TTS 发送"10元预置值已到"
#define ALERT27 27   // //向地址ADDR_TTS 发送"5元预置值已到"

extern  U16 prepic_prenum;      // 用于记录 报错前的界面 
void alertfuc(U16 alertflag); //报错






#endif
