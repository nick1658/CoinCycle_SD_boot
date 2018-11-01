#include "s3c2416.h"
#define WUBITIMEBEISHU 50  //20m 一次定时 长时间无币定时为1S，则倍数为 50
#define RUBIYANSHIBEISHU 10 //	 20m 一次定时 ,入币斗延时 一格代表20*10m，则总数为10 
void deviceinit(void)	//开机先把通道上的币挡下去 
{
	int i = 0;
	int good_coin;
	
	my_println("begin init %d ...", coin_env.full_stack_num);
	for (i = 0; i < coin_env.full_stack_num; i++){
		my_println("begin init i %d", i);
		good_coin = coin_env.full_coin_stack[i];
		if (coin_num[good_coin] >= *pre_value.country[COUNTRY_ID].coin[good_coin].data.p_pre_count_set){
			coin_num[good_coin] = 0;
			*(pre_value.country[COUNTRY_ID].coin[good_coin].data.p_pre_count_current) = 0;
			*pre_value.country[COUNTRY_ID].coin[good_coin].data.p_pre_count_full_flag = 0;
		}
	}
	my_println("finish init 0");	
	disp_allcount();
	coin_env.full_stack_num = 0;
	ch0_count =0;
	ch1_count =0;
	ch2_count =0;
	coin_env.ad0_step = 0;		
	coin_env.ad1_step = 0;
	coin_env.ad2_step = 0;
	ccstep = 0;
	processed_coin_info.coinnumber = 0;
	blockflag = ADBLOCKT;
	kickstep = 0;   //踢币检测函数 步骤号
	runstep =0; //正常工作步骤号	
	my_println("finish init 1");		
}

volatile U32 runtime = 0;   // // 执行件 用的计时
volatile U32 coininmttime = 0;   //入币电机用的计时
volatile U32 panruntime = 0;    //转盘用的 计时
volatile U32 pressnocointime = 0;    //长时间无币通过的 计时
volatile U32 nocoindtmetal = 0;



unsigned short int runstep = 0;   //  部件动作步骤号
unsigned short int panlefttime =0;   //转盘左转 时间变量
unsigned int cannotputcoin = 0; // 清分 斗 满币 状态提示
unsigned short int testnocoinflag = 0;	 //无币 倒转次数 计数
unsigned char panprestate = 0;   //转盘前一个状态
unsigned char metalprestate = 0;   //金属传感器 前一个状态
 
unsigned short int kickstep = 0;	 //踢币程序 步骤号
unsigned short int forgeflag = 0;  //假币 踢币程序标志位
volatile U32 time = 0;   // //踢币电磁铁 用的计时

#define FCNOCOMET   195		//宏定义  这段时间里金属传感器没检测到币报踢币异常   1MS定时
#define BACKTIME   38	//踢币电磁铁  收回 给电时间	  1MS定时



void detectforge(void)
{
	//switch(coin_env.kickstep){
		//case 0:
	if(forgeflag == 2){  
		forgeflag = 0;
		if (coin_env.kick_Q[coin_env.kick_Q_index] == 0){
			coin_env.kick_Q[coin_env.kick_Q_index] = para_set_value.data.kick_start_delay_t1;
			Timer3_Start();//开定时器3  	TIME在里面定时
			coin_env.kick_remain++;
			coin_env.kick_total++;
			coin_env.kick_Q_index++;
			if (coin_env.kick_Q_index >= KICK_Q_LEN){
				coin_env.kick_Q_index = 0;
			}
		}else{
			my_println ("kick1 error %s, %d", __FILE__, __LINE__);
		}
	}
}

unsigned int coinintime = 0;	//斗入币时间

void coin_detect_func(void)   
{
 	if(COIN_DETECT == 0){        //对射电眼检测到硬币
		if(coin_env.coin_Q[coin_env.coin_Q_index] == 2){  
			if (coin_env.full_kick_Q[coin_env.full_kick_Q_index] == 0){
				coin_env.full_kick_Q[coin_env.full_kick_Q_index] = para_set_value.data.kick_start_delay_t2;
				Timer3_Start();//开定时器3  	TIME在里面定时
				coin_env.full_kick_remain++;
				coin_env.full_kick_total++;
				coin_env.full_kick_Q_index++;
				if (coin_env.full_kick_Q_index >= KICK_Q_LEN){
					coin_env.full_kick_Q_index = 0;
				}
			}else{
				my_println ("kick2 error %s, %d", __FILE__, __LINE__);
			}
		}
		coin_env.coin_Q[coin_env.coin_Q_index] = 0;
		coin_env.coin_Q_index++;
		if (coin_env.coin_Q_remain >= KICK_Q_LEN){
			coin_env.coin_Q_remain = 0;
			if (coin_env.coin_Q_remain == (KICK_Q_LEN - 1)){
				alertflag = KICKCOINERROR;
				my_env.workstep =88;
			}
		}
		if (coin_env.coin_Q_index > coin_env.coin_Q_remain){
			alertflag = KICKCOINERROR;
			my_env.workstep =88;
		}
	}else{
	}
}

void runfunction(void)   //部件动作函数
{
	switch (runstep){
		case 0:
			STORAGE_MOTOR(STARTRUN);	//  转盘电机
			runstep = 1;
			break;
		case 1:
			break;
		case 20:
			STORAGE_MOTOR(STOPRUN);	//  转盘电机
			runtime = 100; // 2秒
			runstep = 100;
			break;
		case 40:
			STORAGE_MOTOR(STOPRUN);	//  转盘电机
			runtime = 100; // 2秒
			runstep = 120;
			break;
		case 100:
			if (runtime == 0){
				prepic_num = JSJM;
				alertflag = COUNT_FINISHED;
				my_env.workstep =88;
			}
			break;
		case 120:
			if (runtime == 0){
				comscreen(Disp_Indexpic[JSJM],Number_IndexpicB);	 // back to the  picture before alert
				my_env.workstep = 0;
			}
			break;
		default:break;
	}
}


