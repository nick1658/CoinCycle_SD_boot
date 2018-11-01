#ifndef __YQDRIVE_H__
#define __YQDRIVE_H__


#define _MY_DEBUG_NO_DOOR_CHECK_
#define _MY_DEBUG_NO_PAN_BLOCK_ERROR_
#define _MY_DEBUG_NO_CHANEL_BLOCK_ERROR_
#define _MY_DEBUG_NO_KICK_ERROR_
//输出 端口
#define STARTRUN 0
#define STOPRUN 1

////////////////////////////////////new pcb open this
//////////////////////////

	
#define OUT7(x)		(rGPGDAT = (rGPGDAT & (~0x8)) |(x<<3))	//EINT11/GPG3
#define OUT6(x) 	(rGPBDAT = (rGPBDAT & (~0x2)) |(x<<1))	 	//TOUT1/GPB1
#define OUT5(x) 	(rGPBDAT = (rGPBDAT & (~0x1)) |(x))	 	//TOUT0/GPB0
#define OUT4(x) 	(rGPHDAT = (rGPHDAT & (~0x2000)) |(x<<13))	//CLKOUT0/GPH13	
#define OUT3(x) 	(rGPHDAT = (rGPHDAT & (~0x4000)) |(x<<14))	//CLKOUT1/GPH14	
#define OUT2(x) 	(rGPGDAT = (rGPGDAT & (~0x80)) |(x<<7))	//EINT15/GPG7 
#define OUT1(x) 	(rGPFDAT = (rGPFDAT & (~0x2)) |(x<<1))	//EINT1/GPF1/sd0_CDN
#define OUT0(x) 	(rGPEDAT = (rGPEDAT & (~0x20)) |(x<<5))	//SD0_CLK/GPE5
				  
#define  PAN_MOTOR_LEFT(x) 	 OUT7(x)			//P2^0; 	//EINT11/GPG3 //  转盘反转
#define  PAN_MOTOR_RIGHT(x)  OUT6(x)			//P2^1; TOUT1/GPB1  //  转盘正转 PAN_MOTOR_RIGHT(x)
#define  PRESS_MOTOR(x) 	 OUT5(x)			//P2^2; TOUT0/GPB0  //压币电机

#define  STORAGE_MOTOR(x) 	 OUT0(x)			//P2^3; CLKOUT0/GPH13  //斗送入电机
#define  STORAGE_DIR(x) 	 OUT1(x)			//P2^3; CLKOUT0/GPH13  //斗送入电机转向
#define  EMKICK1(x) 		 OUT3(x)			//P2^4; CLKOUT1/GPH14      // kick out 
#define  EMKICK2(x) 		 OUT2(x)			//P2^5; //EINT15/GPG7   //kick back
//#define  STOPCOIN_ELECMAG(x) OUT1(x) 			//P2^6; EINT1/GPF1/SD0_CDn  // 转盘阻币电磁铁

#define STORAGE_MOTOR_STARTRUN() STORAGE_MOTOR(0)
#define STORAGE_MOTOR_STOPRUN() STORAGE_MOTOR(1)


#define STORAGE_DIR_P() STORAGE_DIR(0) //正转
#define STORAGE_DIR_N() STORAGE_DIR(1) //反转



#define ALL_STOP()	STORAGE_MOTOR_STOPRUN(); \
					STORAGE_DIR_P(); \
					OUT2(STOPRUN); \
					OUT3(STOPRUN); \
					OUT4(STOPRUN); \
					OUT5(STOPRUN); \
					OUT6(STOPRUN); \
					OUT7(STOPRUN)

//输入 端口
#define DUINOWU 1       //对射 无物 为电平 0  槽型无物为 1
#define DUIHAVE 0       //对射 有物 为电平 1 

#define A0IN7 	((rGPEDAT & 0x40)>>6)	//SD0_CMD/GPE6
#define A0IN6 	((rGPEDAT & 0x80)>>7)	//SD0_DAT0/GPE7
#define A0IN5 	((rGPEDAT & 0x100)>>8)	//SD0_DAT1/GPE8
#define A0IN4 	((rGPEDAT & 0x200)>>9)	//SD0_DAT2/GPE9
#define A0IN3 	((rGPEDAT & 0x400)>>10)	//SD0_DAT3/GPE10

////////////////////////////////////old pcb open this
//#define A0IN0 	((rGPGDAT & 0x10)>>4)	//EINT12/GPG4
//#define A0IN1 	((rGPGDAT & 0x20)>>5)	//EINT13/GPG5
//#define A0IN0 	((rGPGDAT & 0x40)>>6)	//EINT14/GPG6
////////////////////////////////////new pcb open this
#define A0IN2 	((rGPGDAT & 0x40)>>6)	//EINT14/GPG6
#define A0IN1 	((rGPGDAT & 0x20)>>5)	//EINT13/GPG5
#define A0IN0 	((rGPGDAT & 0x10)>>4)	//EINT12/GPG4


#ifdef _MY_DEBUG_NO_DOOR_CHECK_
	#define COIN_DETECT      1	//P0^0;  //转盘槽型传感器       EINT12/GPG4  	
	#define PANCOIN_OVER  0	//P0^1;  //转盘满币对射传感器   EINT13/GPG5 
	#define DC_COINOVER   1	//P0^2;  // 清分 满币 传感器    EINT14/GPG6 
	#define DOOR_MAG      0	//P0^3;  //压币带门 传感器      SD0_DAT3/GPE10
	#define DOOR_PAN      0	//P0^4;  //转盘门 传感器  	    SD0_DAT2/GPE9
	#define PRESS_MICROSW 0	//P0^5; //压币带压合传感器   	SD0_DAT1/GPE8
	#define DT_METAL      1	//P0^6; //真假币金属传感器   	SD0_DAT0/GPE7
	#define DT_NOMETAL    1	//P0^7 ;  // 间隙金属传感器  	SD0_CMD/GPE6
#else
	#define COIN_DETECT      A0IN0	//P0^0;  //转盘槽型传感器       EINT12/GPG4  	
	#define PANCOIN_OVER  A0IN1	//P0^1;  //转盘满币对射传感器   EINT13/GPG5 
	#define DC_COINOVER   A0IN2	//P0^2;  // 清分 满币 传感器    EINT14/GPG6 
	#define DOOR_MAG      A0IN3	//P0^3;  //压币带门 传感器      SD0_DAT3/GPE10
	#define DOOR_PAN      A0IN4	//P0^4;  //转盘门 传感器  	    SD0_DAT2/GPE9
	#define PRESS_MICROSW A0IN5	//P0^5; //压币带压合传感器   	SD0_DAT1/GPE8
	#define DT_METAL      A0IN6	//P0^6; //真假币金属传感器   	SD0_DAT0/GPE7
	#define DT_NOMETAL    A0IN7	//P0^7 ;  // 间隙金属传感器  	SD0_CMD/GPE6
#endif

////////////////////////////
void deviceinit(void);	//开机先把通道上的币挡下去 

#define METALSTATIONNO  1
#define METALSTATIONYES 0

#define PRETIME 50//50*20MS			//挡币 给电  持续时间  有就是当转盘停止后压币带的再停止的时间
#define PANLEFTNUM 40  //转盘倒转的次数
#define TESTNOCOINTIME 2 //如果不是堵币 没币 倒转的次数
#define STOPCYTIME 2    //转盘电磁铁 动作时间	2*20MS
#define PANDCTESTTIME 12   //转盘槽型检测堵转时间 12*20MS
#define COINTESTSECOND 100    //长时间没币  倒转后再走的时间 100*20MS
#define PANSTOPTIME 41   //转盘 停顿时间   41*20ms
#define ADTESTTIME 75 //通知基准 实时检测 间隔  75*20MS
#define NOCOINADBEGIN  3   //这么长时间里 间隙金属传感器都没有检测到硬币 就进行AD纠偏

#define STOP_TIME para_set_value.data.motor_idle_t//无币停机时间10秒



extern volatile U32 runtime;   // // 执行件 用的计时
extern volatile U32 coininmttime;   //入币电机用的计时
extern volatile U32 panruntime;   //转盘用的 计时
extern volatile U32 pressnocointime;    //长时间无币通过的 计时
extern volatile U32 nocoindtmetal ;  //2金属传感器长时间没币  

extern unsigned short int runstep;  //  部件动作步骤号
extern unsigned short int panlefttime;  //转盘左转 时间变量
extern unsigned int cannotputcoin; // 清分 斗 满币 状态提示
 
extern unsigned short int kickstep;
extern unsigned short int forgeflag;
extern volatile U32 time;   // //踢币电磁铁 用的计时



void coin_detect_func(void);    //  对射电眼
void runfunction(void);   //部件动作函数
void detectforge(void);	 //踢币程序










#endif /*__YQDRIVE_H__*/
