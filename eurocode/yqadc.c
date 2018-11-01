////
#include "s3c2416.h"

s_coin_env coin_env;// (std_ad0-40)  //波形下降 阀值  990   950  当前基准下降40  


void adcsininget(U16 ad0,U16 ad1,U16 ad2)
{
	
	coin_env.std_down_value0= (ad0-50);  //波形下降 阀值 40  	
	coin_env.std_up_value0= (ad0-30);  //波形恢复 20
	//my_println( "up: %d", coin_env.std_down_value0);
	//my_println( "dn: %d", coin_env.std_up_value0);
}

void setStdValue (void)
{
	U16 is;
		////////////////////////////////////
	U16 ad0_std[AD0STDNUM];
	U16 ad2_std[AD2STDNUM];
	U16 ad1_std[AD1STDNUM];

	for(is=0;is<AD0STDNUM;is++)
	{
		ad0_std[is] = ReadAdc0();
	}
	std_ad0 = (ad0_std[2]  +ad0_std[4]+ad0_std[5]+ad0_std[7]+ad0_std[9])/5;

	
/////////////////////////
	for(is =0;is<AD2STDNUM;is++)
	{
		ad2_std[is] = ReadAdc2();	
	}
	std_ad2 = (ad2_std[2]+ad2_std[3]+ad2_std[4] +ad2_std[5] +ad2_std[6] +ad2_std[7]+ad2_std[8] +ad2_std[9])/8;
	/////////////////////////////////////////
	for(is=0;is<AD1STDNUM;is++)
	{
		ad1_std[is] = ReadAdc1();
	}
	std_ad1  = (ad1_std[2] +ad1_std[3] +ad1_std[4]+ad1_std[5]+ad1_std[6]+ad1_std[7]+ad1_std[8]+ad1_std[9])/8;
//	Uart0_Printf("A0 :%d   A1 :%d  A2 :%d  \r\n",std_ad0,std_ad1,std_ad2);
	
	/////////////////////////每个硬币 单独进行补偿 
	
	adcsininget(std_ad0,std_ad1,std_ad2);//AD 波形进入 的阀值
}

volatile U32 adtime = 0;    //定时中断里 计时

//////////////////////////////////////////////////////
 U16 ad0_mintemp = 0;    // = AD0STDVALUE;		//作为 与ad1_ad_value 值进行比较的值，为了取得波形最高值
 U16 ad0_maxtemp = 0;   //波形恢复  参考比较值
 U16 wave0down_flagone = 0;
 U16 wave0up_flagone = 0;    //波形上升 参考比较值
 U16 wave0up_flag = 0;
 U16 wave0down_flagtwo =0;   //双峰时，用于判断第二个峰
 U32 ch0_count =0;  //通道0 通过的硬币计数 
 U32 ch0_pre_count =0;  //通道0 来硬币 标记  
 U32 ch0_coin_come =0;  //通道 来硬币 标记
 U16 ch0_coin_leave = 0;
 volatile U16 blockflag = 0;      //使用鉴伪传感器 报堵币
 U16 ad0_min = 0;     //贮存每枚硬币过去后的 最大值
 
 
//S16 coin_env.ad0_averaged_value = 0;    //在运行过程中 是ADSAMPNUM个 value的求和值
 S16 ad0_ad_value = 0;  //在运行过程中 是ADSAMPNUM个 value的和值的平均值


 S16 ad0_temp_value = 0;  //在求平均值时用到的临时 变量
 S16 ad0_samp_number = 0;   // ad value sample times 
 S16 ad1_temp_value = 0;  //在求平均值时用到的临时 变量
 S16 ad1_samp_number = 0;   // ad value sample times 
 S16 ad2_temp_value = 0;  //在求平均值时用到的临时 变量
 S16 ad2_samp_number = 0;   // ad value sample times 
 
 S16 ad0_ad_value_buf[ADSAMPNUM0];	//ad value sample  ADSAMPNUM times 
 S16 ad1_ad_value_buf[ADSAMPNUM0];	//ad value sample  ADSAMPNUM times 
 S16 ad2_ad_value_buf[ADSAMPNUM0];	//ad value sample  ADSAMPNUM times 



 U16 ad1_ad_value= 0;	//ad value sample  ADSAMPNUM times 
 U16 ad1_mintemp = 0;// = AD1STDVALUE;		//作为 与ad1_ad_value 值进行比较的值，为了取得波形最高值
 U16 ad1_maxtemp = 0;   //波形恢复  参考比较值
 U16 wave1down_flagone = 0;
 U16 wave1up_flagone = 0;    //波形上升 参考比较值
 U16 wave1up_flagtwo = 0;
 U16 wave1down_flagtwo = 0;   //双峰时，用于判断第二个峰
 U32 ch1_count =0;  //通道1 通过的硬币计数 
// U32 ch1_come =0;  //通道 来硬币 标记
 U16 ad1_min = 0;     //贮存每枚硬币过去后的 最大值
 U32 ch1_coin_come =0;  //通道0 来硬币 标记


 U16 ad2_ad_value = 0;	//ad value sample  ADSAMPNUM times 
 U16 ad2_mintemp = 0;		//作为 与ad2_ad_value 值进行比较的值，为了取得波形最高值
 U16 wave2up_flagone =0;    //波形上升 参考比较值
 U32 ch2_count =0;  //通道2 通过的硬币计数 
 U32 ch2_coin_come =0;  //通道0 来硬币 标记
// U32 ch2_chonetemp =0;  //通道1 来硬币 标记
 U16 ad2_min = 0;     //贮存每枚硬币过去后的 最大值
 U32 ch1_count1temp =0;  //通道1 来硬币 标记  


//模拟转换  0  函数
//目前采用算术平均滤波算法进行处理 N= 10 后加一个消抖计数滤波器进行处理，判断波形的趋势
//////可以尝试一下中位值平均滤波处理
//////或者尝试一下中位值滤波处理

void coin_env_init (void)
{
	int i;
	S8 *p = (S8 *) &coin_env;
	for (i = 0; i < sizeof(s_coin_env); i++)
	{
		*(p++) = 0;
	}	
}

void AD_Sample_All (void)
{
	//Read AD 0 ////////////////////////////////////////////////////////////////////////////////////////
	rADCMUX = 0;		//setup channel
	rADCCON|=0x1;									//start ADC
	while(rADCCON & 0x1);							//check if Enable_start is low
	
	//PreProcess AD 0 Start*********************************************************************
	ad0_temp_value = ad0_ad_value_buf[ad0_samp_number];//old ad0_samp_number = 0，第一次采样值,步骤2
	coin_env.ad0_averaged_value -= ad0_temp_value;//求和减去第一次AD值
	//PreProcess AD 0 End**********************************************************************
	
	while(!(rADCCON & 0x8000));						//check if EC(End of Conversion) flag is high
	ad0_ad_value_buf[ad0_samp_number] = ((int)rADCDAT0 & 0x3ff);
	
	//Read AD 1 ////////////////////////////////////////////////////////////////////////////////////////
	rADCMUX = 0x01;		//setup channel
	rADCCON|=0x1;									//start ADC
	while(rADCCON & 0x1);							//check if Enable_start is low
	
	//Process AD 0 Start ***********************************************************************
	/*coin_env.ad0_averaged_value 求和数*/
	coin_env.ad0_averaged_value += ad0_ad_value_buf[ad0_samp_number];//coin_env.ad0_averaged_value 均值后的数				
	ad0_ad_value = (coin_env.ad0_averaged_value)/ADSAMPNUM0;   //求当前数组总和后求平均值
	Detect_AD_Value_buf[detect_sample_data_buf_index].AD0 = ad0_ad_value; // save ad to buf
	ad0_samp_number++;
	if( (ad0_samp_number > (ADSAMPNUM0-1)) )   //如果采集够了ADSAMPNUM个数据10
	{
		ad0_samp_number = 0;
	}
	//Process AD 0 End ***********************************************************************
	
	//PreProcess AD 1 Start*********************************************************************
	ad1_temp_value = ad1_ad_value_buf[ad1_samp_number];//old ad1_samp_number = 0，第一次采样值,步骤2
	coin_env.ad1_averaged_value -= ad1_temp_value;//求和减去第一次AD值
	//PreProcess AD 1 End**********************************************************************
	
	while(!(rADCCON & 0x8000));						//check if EC(End of Conversion) flag is high

	ad1_ad_value_buf[ad1_samp_number] = ((int)rADCDAT0 & 0x3ff);
	
	//Read AD 2 ////////////////////////////////////////////////////////////////////////////////////////
	rADCMUX = 0x02;		//setup channel
	rADCCON|=0x1;									//start ADC
	while(rADCCON & 0x1);							//check if Enable_start is low
	
	//Process AD 1 Start ***********************************************************************
	/*coin_env.ad1_averaged_value 求和数*/
	coin_env.ad1_averaged_value += ad1_ad_value_buf[ad1_samp_number];//coin_env.ad1_averaged_value 均值后的数				
	ad1_ad_value = (coin_env.ad1_averaged_value)/ADSAMPNUM0;   //求当前数组总和后求平均值
	Detect_AD_Value_buf[detect_sample_data_buf_index].AD1 = ad1_ad_value; // save ad to buf
	ad1_samp_number++;
	if( (ad1_samp_number > (ADSAMPNUM0-1)) )   //如果采集够了ADSAMPNUM个数据10
	{
		ad1_samp_number = 0;
	}
	//Process AD 1 End ***********************************************************************
	
	//PreProcess AD 2 Start*********************************************************************
	ad2_temp_value = ad2_ad_value_buf[ad2_samp_number];//old ad2_samp_number = 0，第一次采样值,步骤2
	coin_env.ad2_averaged_value -= ad2_temp_value;//求和减去第一次AD值
	//PreProcess AD 2 End**********************************************************************
	
	while(!(rADCCON & 0x8000));						//check if EC(End of Conversion) flag is high
	ad2_ad_value_buf[ad2_samp_number] = ((int)rADCDAT0 & 0x3ff);
	
	//Process AD 2 Start ***********************************************************************
	/*coin_env.ad2_averaged_value 求和数*/
	coin_env.ad2_averaged_value += ad2_ad_value_buf[ad2_samp_number];//coin_env.ad2_averaged_value 均值后的数				
	ad2_ad_value = (coin_env.ad2_averaged_value)/ADSAMPNUM0;   //求当前数组总和后求平均值
	Detect_AD_Value_buf[detect_sample_data_buf_index].AD2 = ad2_ad_value; // save ad to buf
	ad2_samp_number++;
	if( (ad2_samp_number > (ADSAMPNUM0-1)) )   //如果采集够了ADSAMPNUM个数据10
	{
		ad2_samp_number = 0;
	}
	//Process AD 2 End ***********************************************************************
	
	detect_sample_data_buf_index++;
	if (detect_sample_data_buf_index >= DATA_BUF_LENGTH){
		detect_sample_data_buf_index = 0;
	}
}

U32 coin_cross_time = 0;
	
void my_ad0_valueget(void)
{
	//S16 std0_offset, std1_offset, std2_offset;
	//U16 is;
	switch(coin_env.ad0_step)
	{ 
/*预制延迟,初始赋值*/
		case 0: 
		{
			cmd();
			my_println ("Start Detect");
			cmd();
			
			ch0_count = 0;
			ch0_counttemp = ch0_count;
			ch0_pre_count = ch0_count;
			
			ch0_coin_leave = 0;
			ch0_coin_come = 0;
			
			wave0down_flagone = 0;
			wave0up_flagone = 0;
			wave0down_flagtwo = 0;	
			
			detect_sample_data_buf_index = 0;
			
			ad0_samp_number = 0;
			coin_env.ad0_averaged_value = 0;

			ad0_ad_value = AD0STDSET;
			ad0_mintemp = AD0STDSET;//900
			ad0_min = AD0STDSET;//900
			
			blockflag = ADBLOCKT;	   //使用鉴伪传感器 报堵币 堵币时间5
			coin_env.ad0_step = 2;
			break;
		}
/*读取通道0,ad0_ad_value采样10次作为均值*/
		case 2:
		{
			//my_println ("Start Sample AD0");
			ad0_ad_value_buf[ad0_samp_number] = ReadAdc0();
			coin_env.ad0_averaged_value += ad0_ad_value_buf[ad0_samp_number];   //求当前数组总和
			ad0_samp_number++;
			blockflag = ADBLOCKT;//堵币时间5
			if( (ad0_samp_number > (ADSAMPNUM0-1)) )   //如果采集够了ADSAMPNUM个数据 10
			{
				ad0_samp_number = 0;
				ad0_ad_value = coin_env.ad0_averaged_value/ADSAMPNUM0;   //求当前数组的平均值
				coin_env.ad0_step = 3; //10 次才会跳到步骤3
					
				dbg ("go to  coin_env.ad0_step %d", coin_env.ad0_step);
			}
			break;
		}
/*采样对比,旧采样10次值-旧采样某次值+当前采样值/均次*/
		case 3: 
		{
			AD_Sample_All ();
			blockflag = ADBLOCKT;	   //使用鉴伪传感器 报堵币
			coin_env.ad0_step = 6;
			ch0_coin_leave = 0;
			/* 
			if ((detect_sample_data_buf_index) > para_set_value.data.adj_offset_position){
				detect_sample_data_buf_index -= 3000;
//					Uart0_Printf("%d	%d	%d	\n",
//								Detect_AD_Value_buf[detect_sample_data_buf_index].AD0, 
//								Detect_AD_Value_buf[detect_sample_data_buf_index].AD1,
//								Detect_AD_Value_buf[detect_sample_data_buf_index].AD2);
				for (is = 0; is < COIN_TYPE_NUM; is++){ //补偿值
					std0_offset = Detect_AD_Value_buf[detect_sample_data_buf_index].AD0 - pre_value.country[coinchoose].coin[is].data.std0;
					std1_offset = Detect_AD_Value_buf[detect_sample_data_buf_index].AD1 - pre_value.country[coinchoose].coin[is].data.std1;
					std2_offset = Detect_AD_Value_buf[detect_sample_data_buf_index].AD2 - pre_value.country[coinchoose].coin[is].data.std2;
				#ifdef SAMPLE_METHOD_0
					coin_cmp_value[is].compare_max0 = (pre_value.country[coinchoose].coin[is].data.max0 + pre_value.country[coinchoose].coin[is].data.offsetmax0 + 
													  std0_offset);
					coin_cmp_value[is].compare_min0 = (pre_value.country[coinchoose].coin[is].data.min0 + pre_value.country[coinchoose].coin[is].data.offsetmin0 + 
													  std0_offset);
					
					coin_cmp_value[is].compare_max1 = (pre_value.country[coinchoose].coin[is].data.max1 + pre_value.country[coinchoose].coin[is].data.offsetmax1 + 
													  std1_offset - std0_offset);
					coin_cmp_value[is].compare_min1 = (pre_value.country[coinchoose].coin[is].data.min1 + pre_value.country[coinchoose].coin[is].data.offsetmin1 + 
													  std1_offset - std0_offset);
					
					coin_cmp_value[is].compare_max2 = (pre_value.country[coinchoose].coin[is].data.max2 + pre_value.country[coinchoose].coin[is].data.offsetmax2 + 
													  std2_offset - std1_offset);
					coin_cmp_value[is].compare_min2 = (pre_value.country[coinchoose].coin[is].data.min2 + pre_value.country[coinchoose].coin[is].data.offsetmin2 + 
													  std2_offset - std1_offset);
				#endif
				#ifdef SAMPLE_METHOD_1
					coin_cmp_value[is].compare_max0 = (pre_value.country[coinchoose].coin[is].data.max0 + pre_value.country[coinchoose].coin[is].data.offsetmax0 + 
													  std0_offset);
					coin_cmp_value[is].compare_min0 = (pre_value.country[coinchoose].coin[is].data.min0 + pre_value.country[coinchoose].coin[is].data.offsetmin0 + 
													  std0_offset);
					
					coin_cmp_value[is].compare_max1 = (pre_value.country[coinchoose].coin[is].data.max1 + pre_value.country[coinchoose].coin[is].data.offsetmax1 + 
													  std1_offset);
					coin_cmp_value[is].compare_min1 = (pre_value.country[coinchoose].coin[is].data.min1 + pre_value.country[coinchoose].coin[is].data.offsetmin1 + 
													  std1_offset);
					
					coin_cmp_value[is].compare_max2 = (pre_value.country[coinchoose].coin[is].data.max2 + pre_value.country[coinchoose].coin[is].data.offsetmax2 + 
													  std2_offset);
					coin_cmp_value[is].compare_min2 = (pre_value.country[coinchoose].coin[is].data.min2 + pre_value.country[coinchoose].coin[is].data.offsetmin2 + 
													  std2_offset);
				#endif
				}
				detect_sample_data_buf_index = 0;
			}*/
			break;
		}
/*获取均值与参考值对比及变化的步骤执行*/
		case 6: 					//read in ad value ADSAMPNUM times 
				{
					blockflag = ADBLOCKT;	   //使用鉴伪传感器 报堵币5
					/*当前步骤3的采样值小于标准参考值时 coin_env.std_down_value0 ad0-50 ,ad0 采样8次均值*/
					if( (ad0_ad_value < coin_env.std_down_value0)	)  //std_value	standard value for detect  从平均值的监测改为当前值的监测
					{
						wave0down_flagone++;    //小于参考值  则表示有硬币过来
					}
					/*均值大于参考值时*/
					else if((ad0_ad_value > coin_env.std_down_value0))
					{
						wave0down_flagone = 0;
					}
					/*采样值小于参考值,持续性WAVEUP0次以上（消抖处理）*/	
					if(  wave0down_flagone > WAVEUP0)     //WAVEUP0 4
					{
						coin_env.ad0_step  = 9;
						coin_cross_time = 0;//开始计时
						//my_println ("go to  coin_env.ad0_step %d", coin_env.ad0_step);
						wave0down_flagone = 0;
						detect_sample_data_buf_index = 0;
						break;
					}
					/*返回步骤3采集*/
					else
					{	
						coin_env.ad0_step  = 3;	
						detect_sample_data_buf_index = 0;
						
						//my_println ("go to  coin_env.ad0_step %d", coin_env.ad0_step);					
					}
					break;
				} 
/*波谷保持,假设最小值*/
		case 9:   ///此处不能去掉，因为双峰会跳到这里
			{
				ad0_mintemp = ad0_ad_value;
				ch0_coin_come++;  //通道 0 来硬币  通知其它通道
				coin_env.ad0_step  = 10;
				//my_println ("go to  coin_env.ad0_step %d", coin_env.ad0_step);
				break;
			}
/*再次求和均值*/
		case 10:						//ad0 value get
				{
					AD_Sample_All ();
					coin_env.ad0_step = 16;
					//my_println ("go to  coin_env.ad0_step %d", coin_env.ad0_step);
					break;
				}
/*确定最小值,波形回升*/
		case 16:						//read in ad value ADSAMPNUM times 
				{

					//step9 ad0_mintemp 假定最小值
					if( ad0_mintemp > ad0_ad_value )  //波形持续  下降中  此处使用的 比较参数非常重要，原来是ad0_ad_value_buf[ad0_samp_number] 试一下ad0_ad_value
					{
						ad0_mintemp =ad0_ad_value;	//;ad0_ad_value_buf[ad0_samp_number],交换最小值
						coin_env.AD_min_index[0] = detect_sample_data_buf_index;
						wave0up_flagone = 0;
					}
					else if((ad0_ad_value > ad0_mintemp )&&((ad0_ad_value - ad0_mintemp )>50))     //波形    回升  
					{
						wave0up_flagone++;
					}

					/*持续性8次,波形回升*/
					if( wave0up_flagone  > WAVEMAX0)     //确认波形峰值  WAVEMAX0  8
					{
						coin_env.ad0_step = 19;	
						//my_println ("go to  coin_env.ad0_step %d", coin_env.ad0_step);
						wave0up_flagone = 0;
					}
					/*返回step10 采样均值步骤*/
				    else
					{
						coin_env.ad0_step = 10;
						//my_println ("go to  coin_env.ad0_step %d", coin_env.ad0_step);
					}
					break;
				}
/*已经确认最小值，肯定有经过一枚硬币，假设最大值*/
		case 19:				
				{
					ad0_min = ad0_mintemp;
//					my_println("adtempcount0 = %d",ch0_counttemp);
					ch0_count++;    //if ch0_count != ch_counttemp ,则表示通道0采集到一枚
					ad0_maxtemp = ad0_ad_value ;//ad0_ad_value求和均值
					coin_env.ad0_step = 25; ///25
//					my_println ("go to  coin_env.ad0_step %d", coin_env.ad0_step);
					break;
				}	
/*读取AD转换值,跟均值*/
		case 25:						//read in ad value ADSAMPNUM times 
				{
					AD_Sample_All ();
					
					coin_env.ad0_step = 31; ///25
					//my_println ("go to  coin_env.ad0_step %d", coin_env.ad0_step);
					break;
				}
/*判断是否跟参考值相同或相近,确定最大采样值,波形恢复到参考值*/
		case 31:						//ad0 value get
				{
					/*均值大于参考值,表示一枚已经过去*/
					if( (ad0_ad_value >= coin_env.std_up_value0) ){    //如果 波形恢复到参考值，表示一枚结束
						wave0up_flag++;
						wave0down_flagtwo = 0;
						ad0_maxtemp = ad0_ad_value ;
					}else if( (ad0_ad_value < coin_env.std_down_value0)&&(ad0_maxtemp>ad0_ad_value)&&((ad0_maxtemp-ad0_ad_value)>70) ){ /*考虑连币情况*/ //连币时，尖峰过去后还会再来一个尖峰，ad0_ad_value_buf[ad0_samp_number] 
						wave0down_flagtwo++;
					}else if(ad0_ad_value >= ad0_maxtemp){/*如果波形还在上升，更新确定最大值*/		
						wave0down_flagtwo = 0;
						ad0_maxtemp = ad0_ad_value ;
					}
					
					/*已经恢复参考值,返回初始测量*/
					if( wave0up_flag > WAVE0fall){// WAVE0fall 2
						coin_env.ad0_step = 3; //
						ch0_coin_leave = 1;
						my_env.coin_cross_time = coin_cross_time;
						blockflag = ADBLOCKT;      //使用鉴伪传感器 报堵币 5
						wave0up_flag =0;
						wave0down_flagtwo = 0;	
						break;
					}
						/*连币双波谷,返回状态9*/
					if( (wave0down_flagtwo > WAVEUPT0)) {    //表示双峰来临 WAVEUPT0 8
						coin_env.ad0_step = 9;
						//my_println ("go to  coin_env.ad0_step %d", coin_env.ad0_step);
						//my_println("more than two coin come");
//						coin_env.ad0_step = 38; //
						blockflag = ADBLOCKT;      //使用鉴伪传感器 报堵币 5
						wave0up_flag =0;
						wave0down_flagtwo = 0;	
						break;
					}else{/*即没有恢复也没有连币则继续测量*/
						coin_env.ad0_step = 25;	
						//my_println ("go to  coin_env.ad0_step %d", coin_env.ad0_step);
					}								
					break;
				}

	}
	return;
}

////////////////////////////////////////////////////////////////////////////////


//模拟转换  1  函数
void my_ad1_valueget(void)
{					
	switch(coin_env.ad1_step)
	{
		case 0: 
		{
			ch1_count = 0;
			ch1_counttemp = ch1_count;
			
			ch1_coin_come = ch0_coin_come;
			
			wave1down_flagone = 0;
			wave1up_flagone =0;
			wave1down_flagtwo =0;
			
			ad1_ad_value = AD1STDSET;
			ad1_mintemp = AD1STDSET;
			ad1_min = AD1STDSET;
			
			ad1_samp_number = 0;
			coin_env.ad1_averaged_value = 0;

			coin_env.ad1_step = 2;
			break;
		}
		case 2:
		{
			//**********************************************************************
			//my_println ("Start Sample AD1");
			//*************************************************************************
			ad1_ad_value_buf[ad1_samp_number] = ReadAdc1();
			coin_env.ad1_averaged_value = coin_env.ad1_averaged_value + ad1_ad_value_buf[ad1_samp_number];   //求当前数组总和
			ad1_samp_number++;
			if( (ad1_samp_number > (ADSAMPNUM0-1)) )   //如果采集够了ADSAMPNUM个数据 10
			{
				ad1_samp_number = 0;
				ad1_ad_value = coin_env.ad1_averaged_value/ADSAMPNUM0;   //求当前数组的平均值
				coin_env.ad1_step = 3; //10 次才会跳到步骤3
				dbg ("go to  coin_env.ad1_step %d", coin_env.ad0_step);
			}
			break;
		}
		case 3: 						//进入硬币检测
		{						
			if(( ch1_coin_come != ch0_coin_come))//硬币来了,+1值不一样
			{
				coin_env.ad1_step = 5;
				ad1_mintemp =  ad1_ad_value;
				ch1_coin_come  =  ch0_coin_come;
				break;
			}
			break;
		}
		case 5: 						//进入硬币检测
		{						

			if(ad1_mintemp >ad1_ad_value)
			{
				ad1_mintemp =  ad1_ad_value;
				coin_env.AD_min_index[1] = detect_sample_data_buf_index;
			}
			/**/
 			if((ch0_pre_count != ch0_count))	 //如果0通道出来了。
			{
				coin_env.ad1_step = 3;
  				ad1_min = ad1_mintemp;
	//			Uart0_Printf("1min %d\r\n",ad1_mintemp);
				ad1_mintemp = AD1STDSET;
				ch1_count++;    //if ch1_count != ch_counttemp ,则表示通道1采集到一枚
				break;
			}
 			break;
		}
	}
}

//模拟转换  2  函数
#define AD2CHANG 6
void my_ad2_valueget(void)
{
	switch(coin_env.ad2_step)
	{
		/*初始赋值*/
		case 0: 
		{	
			ch2_count = 0;
			ch2_counttemp = ch2_count;
			
			ch2_coin_come = ch0_coin_come;
			
			ad2_ad_value = AD2STDSET;//900
			ad2_mintemp = AD2STDSET;//900
			ad2_min = AD2STDSET;//900
			
			ad2_samp_number = 0;
			coin_env.ad2_averaged_value = 0;
			
			coin_env.ad2_step = 2;
			break;
		}
		case 2:
		{
			//**********************************************************************
			//my_println ("Start Sample AD2");
			ad2_ad_value_buf[ad2_samp_number] = ReadAdc2();
			coin_env.ad2_averaged_value = coin_env.ad2_averaged_value + ad2_ad_value_buf[ad2_samp_number];   //求当前数组总和
			ad2_samp_number++;
			if( (ad2_samp_number > (ADSAMPNUM0-1)) )   //如果采集够了ADSAMPNUM个数据 10
			{
				ad2_samp_number = 0;
				ad2_ad_value = coin_env.ad2_averaged_value/ADSAMPNUM0;   //求当前数组的平均值
				coin_env.ad2_step = 3; //10 次才会跳到步骤3
					
				dbg ("go to  coin_env.ad2_step %d", coin_env.ad0_step);
			}
			//*************************************************************************
			break;
		}
		case 3: 	    //进入硬币 检测 					
		{			
			if(( ch2_coin_come != ch0_coin_come))
			{
				coin_env.ad2_step	 = 5;
				ad2_mintemp =ad2_ad_value; 
				ch2_coin_come = ch0_coin_come;
				break;
			}
			break;
		}
		case 5: 	    //进入硬币 检测 					
		{

			if( ad2_mintemp > ad2_ad_value ) 
			{
				ad2_mintemp =ad2_ad_value;
				coin_env.AD_min_index[2] = detect_sample_data_buf_index;
			}
				
			if((ch0_pre_count != ch0_count)){
				coin_env.ad2_step	 = 3;
				ad2_min =  ad2_mintemp;
				ch2_count++;	//if ch2_count != ch_counttemp ,则表示通道2采集到一枚
				ad2_mintemp = AD2STDSET;//900
				ch0_pre_count = ch0_count;
				if (my_env.print_wave_to_pc == 0){
					detect_sample_data_buf_index = 0;
				}
				break;
			}
			break;
		}

	}
	return;
}

	
////////////////////////////////////////////////////////////////////////////////
S16 std_ad0 = 0;
S16 std_ad1 = 0;
S16 std_ad2 = 0;
S16 std_ad3= 0;

 int temperstd = 281;   //20度  20*10 +600  = 800MV;  800/3300 *1024 = 248.24
 int temperpre = 250;
 int dexvaule = 13;   //  相当于1.3倍
#define GETTEMPERCP 0 //std_ad3 - temperstd) // 0  //(((std_ad3 - temperstd)*10)/22)
#define GETTEMPER2CP 0 //(std_ad3 - temperstd)  //0  //(((std_ad3 - temperstd)*10)/40) //((( std_ad3 - temperpre)*15)/10)


//测试没有硬币通过时 DA0 DA1 DA2 通道的当前值是否符合检测的要求
U16 adstd_test(void)
{
	unsigned int is;
	int adstdtest0,adstdtest1,adstdtest2;
	int ad2_std[AD2STDNUM];
	int ad1_std[AD1STDNUM];
	int ad0_std[AD0STDNUM];

	std_ad3 = ReadAdc3();//温度AD采样数值
	/*基准值*/
	/**********************************************************************
	**计算AD2的平均值
	**去掉最初采样的两个AD值(刚开始的值可能不准确)
	**********************************************************************/
	for(is =0;is<AD2STDNUM;is++) //AD2STDNUM 10
	{
		ad2_std[is] = ReadAdc2();
	}
	std_ad2 = (ad2_std[2]+ad2_std[3]+ad2_std[4] +ad2_std[5] +ad2_std[6] +ad2_std[7]+ad2_std[8] +ad2_std[9])/8; 
	Uart0_Printf("std_ad2:%d \r\n",std_ad2);
	/////////////////////////////////////////
	/**********************************************************************
	**计算AD1的平均值
	**去掉最初采样的两个AD值(刚开始的值可能不准确)
	**********************************************************************/
	for(is=0;is<AD1STDNUM;is++)
	{
		ad1_std[is] = ReadAdc1();
	}
	/*基准值*/
	std_ad1 = (ad1_std[2] +ad1_std[3] +ad1_std[4]+ad1_std[5]+ad1_std[6]+ad1_std[7]+ad1_std[8]+ad1_std[9])/8;
	Uart0_Printf("std_ad1:%d \r\n",std_ad1);
	////////////////////////////////////
	/**********************************************************************
	**计算AD0的平均值
	**去掉最初采样的两个AD值(刚开始的值可能不准确)
	**********************************************************************/
	for(is=0;is<AD0STDNUM;is++)
	{
		ad0_std[is] = ReadAdc0();
	}
	/*基准值*/
	std_ad0 = (ad0_std[2] +ad0_std[3] +ad0_std[4]+ad0_std[5]+ad0_std[6]+ad0_std[7]+ad0_std[8]+ad0_std[9])/8;
	Uart0_Printf("std_ad0:%d \r\n",std_ad0);
	//温度补偿以后 的值在 930  950 之间才可以运行
	adstdtest0  = std_ad0;
	adstdtest1  = std_ad1;
	adstdtest2  = std_ad2;
	adcsininget(std_ad0,std_ad1,std_ad2);//AD 波形进入 的阀值
	/*偏移基准值需要重新调整传感器设置使之符合要求后才能开始进行检测*/
	if( (adstdtest0 > (pre_value.country[coinchoose].coin[0].data.std0 + ADOFFSET)) || (adstdtest0< (pre_value.country[coinchoose].coin[0].data.std0 - ADOFFSET)) ||
		(adstdtest1 > (pre_value.country[coinchoose].coin[0].data.std1 + ADOFFSET)) || (adstdtest1< (pre_value.country[coinchoose].coin[0].data.std1 - ADOFFSET)) ||
		(adstdtest2 > (pre_value.country[coinchoose].coin[0].data.std2 + ADOFFSET)) || (adstdtest2< (pre_value.country[coinchoose].coin[0].data.std2 - ADOFFSET)))
		{
			Uart0_Printf("Pleas check the sensor\r\n");
			return 1;
		}
	return 1;
	////////////////////////////////////
							
}

///////////////////////////////////////

// 通道扩展的范围 人民币   欧元  美元   根据币种决定
 U8 cn0copmaxc0[COINCNUM] = {15,33,40,10}; //20 //鉴别值 100元真  200欧  100美元  200英镑 
 U8 cn0copminc0[COINCNUM] = {30,65,40,20};  //20 
 U8 cn0copmaxc1[COINCNUM] = {15,40,40,10};  //60
 U8 cn0copminc1[COINCNUM] = {30,50,40,20}; //30
 U8 cn0copmaxc2[COINCNUM] = {15,40,40,10}; //40
 U8 cn0copminc2[COINCNUM] = {50,40,40,20}; //40
//1元真   补偿范围值
 short int offset0_cnn0[5];
 short int offset1_cnn0[5];
 short int offset2_cnn0[5];

 S32 compcoin0max0 = 0;			  //compensate //鉴别范围值 0 100元真  200欧  100美元  200英镑
 S32 compcoin0min0 = 0;			 
 S32 compcoin0max1 = 0;	
 S32 compcoin0min1 = 0;			 
 S32 compcoin0max2 = 0;	
 S32 compcoin0min2 = 0;	

 s_coin_compare_value coin_cmp_value[COIN_TYPE_NUM];
//
 
//////
 U8  cn1copmaxc0[COINCNUM] = {70,40,40,20}; //鉴别值 50铜    100便  50美分  100便士 
 U8  cn1copminc0[COINCNUM] = {35,34,40,40};  
 U8  cn1copmaxc1[COINCNUM] = {40,40,40,20};  
 U8  cn1copminc1[COINCNUM] = {40,40,40,40};  
 U8  cn1copmaxc2[COINCNUM] = {50,50,40,20}; 
 U8  cn1copminc2[COINCNUM] = {40,50,40,80};   
 
 short int offset0_cnn1[5];
 short int offset1_cnn1[5];
 short int offset2_cnn1[5];

 S32 compcoin1max0 = 0;			  //compensate //鉴别范围值 1 5角铜 
 S32 compcoin1min0 = 0;			 
 S32 compcoin1max1 = 0;	
 S32 compcoin1min1 = 0;			 
 S32 compcoin1max2 = 0;	
 S32 compcoin1min2 = 0;	

///////

 U8 cn2copmaxc0[COINCNUM] = {20,20,20,20}; //鉴别值    50钢   50便  25美分 50便士
 U8 cn2copminc0[COINCNUM] = {65,65,40,10};  
 U8 cn2copmaxc1[COINCNUM] = {40,20,20,20};  
 U8 cn2copminc1[COINCNUM] = {40,40,40,20};  
 U8 cn2copmaxc2[COINCNUM] = {45,40,45,20};  
 U8 cn2copminc2[COINCNUM] = {45,60,40,20};  

 short int offset0_cnn2[5];
 short int offset1_cnn2[5];
 short int offset2_cnn2[5];

 S32 compcoin2max0 = 0;			  //compensate //鉴别范围值 2 
 S32 compcoin2min0 = 0;			 
 S32 compcoin2max1 = 0;	
 S32 compcoin2min1 = 0;			 
 S32 compcoin2max2 = 0;	
 S32 compcoin2min2 = 0;	
//////
 U8 cn3copmaxc0[COINCNUM] = {20,40,50,20};//鉴别值 10  大铝   20便  50美分 50便士
 U8 cn3copminc0[COINCNUM] = {20,25,60,20}; 
 U8 cn3copmaxc1[COINCNUM] = {20,40,40,20}; 
 U8 cn3copminc1[COINCNUM] = {20,26,30,20}; 
 U8 cn3copmaxc2[COINCNUM] = {80,60,80,20}; 
 U8 cn3copminc2[COINCNUM] = {80,60,80,20}; 

 short int offset0_cnn3[5];
 short int offset1_cnn3[5];
 short int offset2_cnn3[5];

 S32 compcoin3max0 = 0;			  //compensate //鉴别范围值 3 
 S32 compcoin3min0 = 0;			 
 S32 compcoin3max1 = 0;	
 S32 compcoin3min1 = 0;			 
 S32 compcoin3max2 = 0;	
 S32 compcoin3min2 = 0;	
///////
 S8  cn4copmaxc0[COINCNUM] = {30,50,40,20}; //50 //鉴别值 1角小钢   2便  25美分  1英镑 
 S8  cn4copminc0[COINCNUM] = {20,18,30,20};
 S8  cn4copmaxc1[COINCNUM] = {20,60,40,20};
 S8  cn4copminc1[COINCNUM] = {20,18,40,20};
 S8  cn4copmaxc2[COINCNUM] = {30,30,40,20};
 S8  cn4copminc2[COINCNUM] = {20,30,40,20};
 short int offset0_cnn4[5];
 short int offset1_cnn4[5];
 short int offset2_cnn4[5];

 S32 compcoin4max0 = 0;			  //compensate //鉴别范围值 4 
 S32 compcoin4min0 = 0;			 
 S32 compcoin4max1 = 0;	
 S32 compcoin4min1 = 0;			 
 S32 compcoin4max2 = 0;	
 S32 compcoin4min2 = 0;	
///////
 U8 cn5copmaxc0[COINCNUM] = {20,40,40,20};//15//鉴别值 1角小铝   1便  10美分   20便士 
 U8 cn5copminc0[COINCNUM] = {25,40,20,20};
 U8 cn5copmaxc1[COINCNUM] = {15,40,50,20};
 U8 cn5copminc1[COINCNUM] = {20,40,25,20};//30
 U8 cn5copmaxc2[COINCNUM] = {40,70,40,20}; //20
 U8 cn5copminc2[COINCNUM] = {40,40,40,20}; //30

 short int offset0_cnn5[5];
 short int offset1_cnn5[5];
 short int offset2_cnn5[5];

 S32 compcoin5max0 = 0;			  //compensate //鉴别范围值 5 
 S32 compcoin5min0 = 0;			 
 S32 compcoin5max1 = 0;	
 S32 compcoin5min1 = 0;			 
 S32 compcoin5max2 = 0;	
 S32 compcoin5min2 = 0;	
//////
 U8 cn6copmaxc0[COINCNUM] = {20,30,30,20};//鉴别值 5分铝        10便士
 U8 cn6copminc0[COINCNUM] = {20,20,20,20};
 U8 cn6copmaxc1[COINCNUM] = {20,40,40,20};
 U8 cn6copminc1[COINCNUM] = {20,20,20,20};
 U8 cn6copmaxc2[COINCNUM] = {70,60,60,20};//60
 U8 cn6copminc2[COINCNUM] = {70,60,60,20};//60  

 short int offset0_cnn6[5];
 short int offset1_cnn6[5];
 short int offset2_cnn6[5];

 S32 compcoin6max0 = 0;			  //compensate //鉴别范围值 6 
 S32 compcoin6min0 = 0;			 
 S32 compcoin6max1 = 0;	
 S32 compcoin6min1 = 0;			 
 S32 compcoin6max2 = 0;	
 S32 compcoin6min2 = 0;	
/////
 U8 cn7copmaxc0[COINCNUM] = {30,15,0,20};//鉴别值 2分   10便  1便 
 U8 cn7copminc0[COINCNUM] = {40,40,0,20};//
 U8 cn7copmaxc1[COINCNUM] = {20,20,0,20};//
 U8 cn7copminc1[COINCNUM] = {70,40,0,20};//
 U8 cn7copmaxc2[COINCNUM] = {50,30,0,20};//
 U8 cn7copminc2[COINCNUM] = {40,30,0,20};//

 short int offset0_cnn7[5];
 short int offset1_cnn7[5];
 short int offset2_cnn7[5];

 S32 compcoin7max0 = 0;			  //compensate //鉴别范围值 7 
 S32 compcoin7min0 = 0;			 
 S32 compcoin7max1 = 0;	
 S32 compcoin7min1 = 0;			 
 S32 compcoin7max2 = 0;	
 S32 compcoin7min2 = 0;	
////
 U8 cn8copmaxc0[COINCNUM] = {30,0,0,0};//鉴别值 1分   
 U8 cn8copminc0[COINCNUM] = {20,0,0,0}; 
 U8 cn8copmaxc1[COINCNUM] = {30,0,0,0}; 
 U8 cn8copminc1[COINCNUM] = {20,0,0,0}; 
 U8 cn8copmaxc2[COINCNUM] = {40,0,0,0}; 
 U8 cn8copminc2[COINCNUM] = {40,0,0,0}; 

 short int offset0_cnn8[5];
 short int offset1_cnn8[5];
 short int offset2_cnn8[5];

S32 compcoin8max0 = 0;			  //compensate //鉴别范围值 8 
S32 compcoin8min0 = 0;			 
S32 compcoin8max1 = 0;	
S32 compcoin8min1 = 0;			 
S32 compcoin8max2 = 0;	
S32 compcoin8min2 = 0;	
#define CHUSHU (3)
 
void print_std_value(void)    //  检测 基准值   有不大偏差进行补偿
{
	U16 is = 0;
	U16 ad2_std[AD2STDNUM];
	U16 ad1_std[AD1STDNUM];
	U16 ad0_std[AD0STDNUM];
	U16 ad2countis = 10;
	std_ad3 = ReadAdc3();

	for(is =0;is<ad2countis;is++)
	{
		ad2_std[is] = ReadAdc2(); 
	}	
	std_ad2 = 0;
	for(is =0;is<ad2countis;is++)
	{
		std_ad2 = std_ad2+ad2_std[is]; 
	}	
	
	std_ad2 =std_ad2/ad2countis;
	/////////////////////////////////////////
	for(is=0;is<AD1STDNUM;is++)
	{
		ad1_std[is] = ReadAdc1();
	}
	std_ad1 = (ad1_std[2] +ad1_std[3] +ad1_std[4]+ad1_std[5]+ad1_std[6]+ad1_std[7]+ad1_std[8]+ad1_std[9])/8;
	////////////////////////////////////
	for(is=0;is<AD0STDNUM;is++)
	{
		ad0_std[is] = ReadAdc0();
	}
	std_ad0 = (ad0_std[2] +ad0_std[3] +ad0_std[4]+ad0_std[5]+ad0_std[6]+ad0_std[7]+ad0_std[8]+ad0_std[9])/8;
}

U16 adstd_offset()    //  检测 基准值   有不大偏差进行补偿
{
	U16 is;
		////////////////////////////////////
	U16 ad0_std[AD0STDNUM];
	U16 ad2_std[AD2STDNUM];
	U16 ad1_std[AD1STDNUM];
	
	S16 std0_offset, std1_offset, std2_offset;


	for(is=0;is<AD0STDNUM;is++){
		ad0_std[is] = ReadAdc0();
	}
	std_ad0 = (ad0_std[2] +ad0_std[3] +ad0_std[4]+ad0_std[5]+ad0_std[6]+ad0_std[7]+ad0_std[8]+ad0_std[9])/8;
//	std_ad0 = (ad0_std[2]  +ad0_std[4]+ad0_std[5]+ad0_std[7]+ad0_std[9])/5;

	
/////////////////////////
	for(is =0;is<AD2STDNUM;is++){
		ad2_std[is] = ReadAdc2();	
	}
	std_ad2 = (ad2_std[2]+ad2_std[3]+ad2_std[4] +ad2_std[5] +ad2_std[6] +ad2_std[7]+ad2_std[8] +ad2_std[9])/8;
	/////////////////////////////////////////
	for(is=0;is<AD1STDNUM;is++){
		ad1_std[is] = ReadAdc1();
	}
	std_ad1  = (ad1_std[2] +ad1_std[3] +ad1_std[4]+ad1_std[5]+ad1_std[6]+ad1_std[7]+ad1_std[8]+ad1_std[9])/8;
//	Uart0_Printf("A0 :%d   A1 :%d  A2 :%d  \r\n",std_ad0,std_ad1,std_ad2);
	//offset_ad0 = std_ad0 - AD0STDSET;
	/////////////////////////每个硬币 单独进行补偿 
	std_ad3 = ReadAdc3();
	if( ( ((std_ad0-GETTEMPERCP) < (pre_value.country[coinchoose].coin[0].data.std0 + OFFSETMAX))) && 
		  ((std_ad0-GETTEMPERCP) > (pre_value.country[coinchoose].coin[0].data.std0 - OFFSETMIN)) ){        //如果  在990  1020之间 可以进行补偿	
	}else {
		return 2;
	}	

	
	/////////////   
	my_println ("real     std0 = %4d          std1 = %4d     std2 = %4d", std_ad0, std_ad1, std_ad2);
	for (is = 0; is < COIN_TYPE_NUM; is++){ //补偿值
		std0_offset = std_ad0 - pre_value.country[coinchoose].coin[is].data.std0;
		std1_offset = std_ad1 - pre_value.country[coinchoose].coin[is].data.std1;
		std2_offset = std_ad2 - pre_value.country[coinchoose].coin[is].data.std2;
	#ifdef SAMPLE_METHOD_0
		coin_cmp_value[is].compare_max0 = (pre_value.country[coinchoose].coin[is].data.max0 + pre_value.country[coinchoose].coin[is].data.offsetmax0 + 
										  std0_offset);
		coin_cmp_value[is].compare_min0 = (pre_value.country[coinchoose].coin[is].data.min0 + pre_value.country[coinchoose].coin[is].data.offsetmin0 + 
										  std0_offset);
		
		coin_cmp_value[is].compare_max1 = (pre_value.country[coinchoose].coin[is].data.max1 + pre_value.country[coinchoose].coin[is].data.offsetmax1 + 
										  std1_offset - std0_offset);
		coin_cmp_value[is].compare_min1 = (pre_value.country[coinchoose].coin[is].data.min1 + pre_value.country[coinchoose].coin[is].data.offsetmin1 + 
										  std1_offset - std0_offset);
		
		coin_cmp_value[is].compare_max2 = (pre_value.country[coinchoose].coin[is].data.max2 + pre_value.country[coinchoose].coin[is].data.offsetmax2 + 
										  std2_offset - std1_offset);
		coin_cmp_value[is].compare_min2 = (pre_value.country[coinchoose].coin[is].data.min2 + pre_value.country[coinchoose].coin[is].data.offsetmin2 + 
										  std2_offset - std1_offset);
	#endif
	#ifdef SAMPLE_METHOD_1
		coin_cmp_value[is].compare_max0 = (pre_value.country[coinchoose].coin[is].data.max0 + pre_value.country[coinchoose].coin[is].data.offsetmax0 + 
										  std0_offset);
		coin_cmp_value[is].compare_min0 = (pre_value.country[coinchoose].coin[is].data.min0 + pre_value.country[coinchoose].coin[is].data.offsetmin0 + 
										  std0_offset);
		
		coin_cmp_value[is].compare_max1 = (pre_value.country[coinchoose].coin[is].data.max1 + pre_value.country[coinchoose].coin[is].data.offsetmax1 + 
										  std1_offset);
		coin_cmp_value[is].compare_min1 = (pre_value.country[coinchoose].coin[is].data.min1 + pre_value.country[coinchoose].coin[is].data.offsetmin1 + 
										  std1_offset);
		
		coin_cmp_value[is].compare_max2 = (pre_value.country[coinchoose].coin[is].data.max2 + pre_value.country[coinchoose].coin[is].data.offsetmax2 + 
										  std2_offset);
		coin_cmp_value[is].compare_min2 = (pre_value.country[coinchoose].coin[is].data.min2 + pre_value.country[coinchoose].coin[is].data.offsetmin2 + 
										  std2_offset);
	
	#endif
	}
	adcsininget(std_ad0,std_ad1,std_ad2);//AD 波形进入 的阀值

	return 1;
	////////////////////////////////////
}


volatile U32 start_sample = 0;
static		U16  minTempAD0 = 1000;
static		U16  minTempAD1 = 1000;
static		U16  minTempAD2 = 1000;



volatile AD_Value AD_Value_buf[DATA_BUF_LENGTH];
volatile AD_Value Detect_AD_Value_buf[DATA_BUF_LENGTH];
//volatile AD_Value Adj_AD_Value_buf[ADJ_BUF_LENGTH];
volatile AD_Value NG_value_buf[NG_BUF_LENGTH];
volatile AD_Value GOOD_value_buf[NG_BUF_LENGTH];
volatile U32 ng_value_index = 0;
volatile U32 good_value_index = 0;
volatile U32  sample_data_buf_index = 0;
volatile U32 detect_sample_data_buf_index = 0;

/////////////////////////////////

extern void Uart_SendByte(int data);

int L_H_min = 1024;
int L_M_min = 1024;
int M_H_min = 1024;
	
int L_H_max = 0;
int L_M_max = 0;
int M_H_max = 0;

void send_sample_data (volatile AD_Value ad_value_buf[DATA_BUF_LENGTH], int counter)
{
	int i = 0;
	int L_H_Temp = 0;
	int L_M_Temp = 0;
	int M_H_Temp = 0;
	
	int H_min_index = 0;
	int M_min_index = 0;
	int L_min_index = 0;
	//if (start_sample)
	{
		Uart0_Printf ("start\n");
		for (i = 0; i < counter; i++)
		{
			if (minTempAD0 > ad_value_buf[i].AD0)
			{
				minTempAD0 = ad_value_buf[i].AD0;
				H_min_index = i;
			}
			if (minTempAD1 > ad_value_buf[i].AD1)
			{
				minTempAD1 = ad_value_buf[i].AD1;
				M_min_index = i;
			}
			if (minTempAD2 > ad_value_buf[i].AD2)
			{
				minTempAD2 = ad_value_buf[i].AD2;
				L_min_index = i;
			}
			Uart0_Printf("%d	%d	%d	\n",ad_value_buf[i].AD0, ad_value_buf[i].AD1, ad_value_buf[i].AD2);
		}
		
		Uart0_Printf ("end\n");
		dgus_tf1word(ADDR_STDH, (minTempAD0)); //	high frequence
		dgus_tf1word(ADDR_STDM,  minTempAD1); //	middle frequence
		dgus_tf1word(ADDR_STDL, (minTempAD2)); //	low frequence	
//		dgus_tf1word(ADDR_STDT,((std_ad3*3300)/1024-600)); //	low frequence	
		
		
		if ( minTempAD1 > 20)
		{
			L_H_Temp = ad_value_buf[H_min_index].AD2 - ad_value_buf[H_min_index].AD0;
			L_M_Temp = ad_value_buf[H_min_index].AD2 - ad_value_buf[H_min_index].AD1;
			M_H_Temp = ad_value_buf[H_min_index].AD1 - ad_value_buf[H_min_index].AD0;
		}
		else if (minTempAD0 >20)
		{
			L_H_Temp = ad_value_buf[M_min_index].AD2 - ad_value_buf[M_min_index].AD0;
			L_M_Temp = ad_value_buf[M_min_index].AD2 - ad_value_buf[M_min_index].AD1;
			M_H_Temp = ad_value_buf[M_min_index].AD1 - ad_value_buf[M_min_index].AD0;
		}
		else
		{
			L_H_Temp = ad_value_buf[L_min_index].AD2 - ad_value_buf[L_min_index].AD0;
			L_M_Temp = ad_value_buf[L_min_index].AD2 - ad_value_buf[L_min_index].AD1;
			M_H_Temp = ad_value_buf[L_min_index].AD1 - ad_value_buf[L_min_index].AD0;
		}
		
		if (L_H_Temp < L_H_min)
		{
			L_H_min = L_H_Temp;
		}
		if (L_H_Temp > L_H_max)
		{
			L_H_max = L_H_Temp;
		}
		
		if (L_M_Temp < L_M_min)
		{
			L_M_min = L_M_Temp;
		}
		if (L_M_Temp > L_M_max)
		{
			L_M_max = L_M_Temp;
		}
		
		if (M_H_Temp < M_H_min)
		{
			M_H_min = M_H_Temp;
		}
		if (M_H_Temp > M_H_max)
		{
			M_H_max = M_H_Temp;
		}
		
		minTempAD0 = 1024;
		minTempAD1 = 1024;
		minTempAD2 = 1024;
	}
}
U16 adstd_adj(void)    //基准值调试  
{
	U16 is = 0;
	U16 ad2_std[AD2STDNUM];
	U16 ad1_std[AD1STDNUM];
	U16 ad0_std[AD0STDNUM];
//	U16 ad2countis = 10;

	for(is=0;is<AD0STDNUM;is++)
	{
		ad0_std[is] = ReadAdc0();
		ad1_std[is] = ReadAdc1();
		ad2_std[is] = ReadAdc2();
	}
	std_ad0 = (ad0_std[2] +ad0_std[3] +ad0_std[4]+ad0_std[5]+ad0_std[6]+ad0_std[7]+ad0_std[8]+ad0_std[9])/8;
	std_ad1 = (ad1_std[2] +ad1_std[3] +ad1_std[4]+ad1_std[5]+ad1_std[6]+ad1_std[7]+ad1_std[8]+ad1_std[9])/8;
	std_ad2 = (ad2_std[2] +ad2_std[3] +ad2_std[4]+ad2_std[5]+ad1_std[6]+ad2_std[7]+ad2_std[8]+ad2_std[9])/8;
	
	/////////////
	//hzf change begin 
	//if(adtime ==0)
	if ((std_ad0 < 850) && (start_sample == 0))
	{
		//Uart0_Printf ("here %d sample_data_buf_index %d\n", start_sample, sample_data_buf_index);
		start_sample = 1;
		sample_data_buf_index = 0;
	}
	else if (start_sample)
	{
		if ((sample_data_buf_index < 1200))
		{
			//Uart0_Printf ("here1 %d sample_data_buf_index %d\n", start_sample, sample_data_buf_index);
			AD_Value_buf[sample_data_buf_index].AD0 = std_ad0;
			AD_Value_buf[sample_data_buf_index].AD1 = std_ad1;
			AD_Value_buf[sample_data_buf_index].AD2 = std_ad2;
			//adtime = ADADJACTERTIME;
			adtime = 0;
			sample_data_buf_index++;
			if (sample_data_buf_index >= DATA_BUF_LENGTH){
				sample_data_buf_index = 0;
			}
		}
		else
		{
			send_sample_data (AD_Value_buf, sample_data_buf_index);
			start_sample = 0;
			sample_data_buf_index = 0;

		}
	}
	//hzf change end
	
	
		
//	Uart0_Printf("adstd_adjtime = %d\r\n",adstd_adjtime);
	return 1;
}

U16 adstd_adj_old(void)    //?????  
{
	U16 is = 0;
	U16 ad2_std[AD2STDNUM];
	U16 ad1_std[AD1STDNUM];
	U16 ad0_std[AD0STDNUM];
	U16 ad2countis = 10;
	std_ad3 = ReadAdc3();

	for(is =0;is<ad2countis;is++)
	{
		ad2_std[is] = ReadAdc2(); 
	}	
	std_ad2 = 0;
	for(is =0;is<ad2countis;is++)
	{
		std_ad2 = std_ad2+ad2_std[is]; 
	}	
	
	std_ad2 =std_ad2/ad2countis;
	/////////////////////////////////////////
	for(is=0;is<AD1STDNUM;is++)
	{
		ad1_std[is] = ReadAdc1();
	}
	std_ad1 = (ad1_std[2] +ad1_std[3] +ad1_std[4]+ad1_std[5]+ad1_std[6]+ad1_std[7]+ad1_std[8]+ad1_std[9])/8;
	////////////////////////////////////
	for(is=0;is<AD0STDNUM;is++)
	{
		ad0_std[is] = ReadAdc0();
	}
	std_ad0 = (ad0_std[2] +ad0_std[3] +ad0_std[4]+ad0_std[5]+ad0_std[6]+ad0_std[7]+ad0_std[8]+ad0_std[9])/8;
	/////////////
	if(adtime ==0)
		{
			dgus_tf1word(ADDR_STDH,(std_ad0 - GETTEMPERCP)); //	high frequence
			dgus_tf1word(ADDR_STDM,std_ad1); //	middle frequence
			dgus_tf1word(ADDR_STDL,(std_ad2+GETTEMPER2CP)); //	low frequence	
//			dgus_tf1word(ADDR_STDT,((std_ad3*3300)/1024-600)); //	low frequence	
			my_println ("\nNick-Cmd:%4d %4d %4d %4d",std_ad0,std_ad1,std_ad2,std_ad3);
			adtime = ADADJACTERTIME;
		}
//	Uart0_Printf("adstd_adjtime = %d\r\n",adstd_adjtime);
	return 1;
}

////////////////////////////////////////////////
U16 adstd_ini(void)    //基准值调试  
{
	
	U16 ad3_std[6];

//	ad0_mintemp = AD0STDVALUE;		//作为 与ad1_ad_value 值进行比较的值，为了取得波形最高值
//	ad1_mintemp = AD1STDVALUE;		//作为 与ad1_ad_value 值进行比较的值，为了取得波形最高值

	while( 1)
	{
		ad3_std[0] = ReadAdc3();
		if(ad3_std[0] >100)
		{		
			temperpre = ( ad3_std[0] - temperstd);
				
			Uart0_Printf("%d	%d\r\n",temperpre,ad3_std[0]);
			return (ad3_std[0]);
		}			
	}
}

