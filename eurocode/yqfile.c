
///////本函数包函  显示函数  报错函数  打印函数  读文件函数

#include "s3c2416.h"


 U8 coinchoose = 0;    // 币种选择
 /*******************************
设置的 踢币延时  踢币时间 进币电机入币时间 停止时间,长时间没币停止时间
**********************************/

/*************
	PRINTER  打印 
***************/
#define PINTNUM 60
U8 dgus_readt[6]= {0xA5,0x5A,0x03,0x81,0x20, 0x10};  // only need y-m-d w h:m:s  读时间
void print_func(void)     // 打印 
{
	uart1_init();
	Uart1_Printf("\r\n");
	Uart1_Printf("\r\n");
	Uart1_Printf("\r\n");

	if(coinchoose == CN0)    //中文
	{
		while(1)     // read time 
		{
			comscreen(dgus_readt,6);	//read time
			while(touch_flag ==0){;}
			if (touchnum[7]>0)
			{
				Uart1_Printf("日期:20%02x-%02x-%02x %02x:%02x\r\n",touchnum[6],touchnum[7],touchnum[8],touchnum[10],touchnum[11]);
				touch_flag = 0;
				break;
			}
			else
			{
				touch_flag = 0;
			}
		}
		/*
		//Uart1_Printf("工号: %d \r\n",gh_now);
 		//Uart1_Printf("   1分     %4d     %d.%d%d\r\n",coin_num[8],((coine[coinchoose][8]*coin_num[8])/100),(((coine[coinchoose][8]*coin_num[8])%100)/10),(((coine[coinchoose][8]*coin_num[8])%100)%10));
		//Uart1_Printf("   2分     %4d     %d.%d%d\r\n",coin_num[7],((coine[coinchoose][7]*coin_num[7])/100),(((coine[coinchoose][7]*coin_num[7])%100)/10),(((coine[coinchoose][7]*coin_num[7])%100)%10));
		//Uart1_Printf("   5分     %4d     %d.%d%d\r\n",coin_num[6],((coine[coinchoose][6]*coin_num[6])/100),(((coine[coinchoose][6]*coin_num[6])%100)/10),(((coine[coinchoose][6]*coin_num[6])%100)%10));
 		Uart1_Printf("   1角     %4d     %d.%d%d\r\n",(coin_num[3]+coin_num[4]+coin_num[5]),((coine[coinchoose][3]*(coin_num[3]+coin_num[4]+coin_num[5]))/100),(((coine[coinchoose][3]*(coin_num[3]+coin_num[4]+coin_num[5]))%100)/10),(((coine[coinchoose][3]*(coin_num[3]+coin_num[4]+coin_num[5]))%100)%10));
		Uart1_Printf("   5角     %4d     %d.%d%d\r\n",(coin_num[1]+coin_num[2]),((coine[coinchoose][1]*(coin_num[1]+coin_num[2]))/100),(((coine[coinchoose][1]*(coin_num[1]+coin_num[2]))%100)/10),(((coine[coinchoose][1]*(coin_num[1]+coin_num[2]))%100)%10));
		Uart1_Printf("   1元     %4d     %d.%d%d\r\n",coin_num[0],((coine[coinchoose][0]*coin_num[0])/100),(((coine[coinchoose][0]*coin_num[0])%100)/10),(((coine[coinchoose][0]*coin_num[0])%100)%10));
		Uart1_Printf("   币种  数量(枚)  金额(元)\r\n");
		Uart1_Printf("   详细信息:  \r\n");
		Uart1_Printf("\r\n");
//		Uart1_Printf("    异币:        %d 枚\r\n",processed_coin_info.total_ng);
		Uart1_Printf("   金额:     %d.%d%d 元\n",(processed_coin_info.total_money/100),((processed_coin_info.total_money%100)/10),((processed_coin_info.total_money%100)%10));
		Uart1_Printf("   总数:     %d 枚\r\n",processed_coin_info.total_good);
		Uart1_Printf("   本次清分:\r\n");*/
		
 		Uart1_Printf("   1角     %4d     %d\r\n",*pre_value.country[COUNTRY_ID].coin[4].data.p_coinval, *pre_value.country[COUNTRY_ID].coin[4].data.p_coinval * 5);
		Uart1_Printf("   5角     %4d     %d\r\n",*pre_value.country[COUNTRY_ID].coin[2].data.p_coinval, *pre_value.country[COUNTRY_ID].coin[2].data.p_coinval * 25);
		Uart1_Printf("   1元     %4d     %d\r\n",*pre_value.country[COUNTRY_ID].coin[0].data.p_coinval, *pre_value.country[COUNTRY_ID].coin[0].data.p_coinval * 50);
		Uart1_Printf("   币种  数量(卷)  金额(元)\r\n");
		Uart1_Printf("   详细信息:  \r\n");
		Uart1_Printf("\r\n");
		Uart1_Printf("   金额:     %d.%d%d 元\n",(processed_coin_info.total_money/100),((processed_coin_info.total_money%100)/10),((processed_coin_info.total_money%100)%10));
		Uart1_Printf("   总数:     %d 枚\r\n",processed_coin_info.total_good);
		Uart1_Printf("   本次清分:\r\n");

	}

	else if(coinchoose == CN1)//其他语种1 例如英文
	{
		while(1)     // read time 
		{
			comscreen(dgus_readt,6);	//read time
			while(touch_flag ==0){;}
			if (touchnum[7]>0)
			{
				Uart1_Printf("DATE:20%02x-%02x-%02x %02x:%02x\r\n",touchnum[6],touchnum[7],touchnum[8],touchnum[10],touchnum[11]);
				touch_flag = 0;
				break;
			}
			else
			{
				touch_flag = 0;
			}
		}
//		Uart1_Printf("Worker: %d\r\n",gh_now);
		Uart1_Printf(" 1CENT    %d      %d.%d%d\r\n",coin_num[7],((coine[coinchoose][7]*coin_num[7])/100),(((coine[coinchoose][7]*coin_num[7])%100)/10),(((coine[coinchoose][7]*coin_num[7])%100)%10));
		Uart1_Printf(" 2CENT    %d      %d.%d%d\r\n",coin_num[6],((coine[coinchoose][6]*coin_num[6])/100),(((coine[coinchoose][6]*coin_num[6])%100)/10),(((coine[coinchoose][6]*coin_num[6])%100)%10));
		Uart1_Printf(" 5CENT    %d      %d.%d%d\r\n",coin_num[5],((coine[coinchoose][5]*coin_num[5])/100),(((coine[coinchoose][5]*coin_num[5])%100)/10),(((coine[coinchoose][5]*coin_num[5])%100)%10));
		Uart1_Printf(" 10CENT   %d      %d.%d%d\r\n",coin_num[4],((coine[coinchoose][4]*coin_num[4])/100),(((coine[coinchoose][4]*coin_num[4])%100)/10),(((coine[coinchoose][4]*coin_num[4])%100)%10));
		Uart1_Printf(" 20CENT   %d      %d.%d%d\r\n",coin_num[3],((coine[coinchoose][3]*coin_num[3])/100),(((coine[coinchoose][3]*coin_num[3])%100)/10),(((coine[coinchoose][3]*coin_num[3])%100)%10));
		Uart1_Printf(" 50CENT   %d      %d.%d%d\r\n",coin_num[2],((coine[coinchoose][2]*coin_num[2])/100),(((coine[coinchoose][2]*coin_num[2])%100)/10),(((coine[coinchoose][2]*coin_num[2])%100)%10));
		Uart1_Printf(" 1EURO    %d      %d.%d%d\r\n",coin_num[1],((coine[coinchoose][1]*coin_num[1])/100),(((coine[coinchoose][1]*coin_num[1])%100)/10),(((coine[coinchoose][1]*coin_num[1])%100)%10));
		Uart1_Printf(" 2EURO    %d      %d.%d%d\r\n",coin_num[0],((coine[coinchoose][0]*coin_num[0])/100),(((coine[coinchoose][0]*coin_num[0])%100)/10),(((coine[coinchoose][0]*coin_num[0])%100)%10));
		Uart1_Printf(" Coin  Aount(Pcs)  Money(EURO)\r\n");
		Uart1_Printf(" list:\r\n");
		Uart1_Printf("\r\n");
//		Uart1_Printf("		Fake:      %d Pcs\n",processed_coin_info.total_ng);
		Uart1_Printf("    Money:   %d.%d%d EURO\n",(processed_coin_info.total_money/100),((processed_coin_info.total_money%100)/10),((processed_coin_info.total_money%100)%10));
		Uart1_Printf("    Amount:       %d Pcs\n",processed_coin_info.total_good);
		Uart1_Printf("    Current Count:\r\n");
	


	}
		
	else if(coinchoose == CN2)//其他语种2
	{
		while(1)     // read time 
		{
			comscreen(dgus_readt,6);	//read time
			while(touch_flag ==0){;}
			if (touchnum[7]>0)
			{
				Uart1_Printf("DATE:20%02x-%02x-%02x %02x:%02x\r\n",touchnum[6],touchnum[7],touchnum[8],touchnum[10],touchnum[11]);
				touch_flag = 0;
				break;
			}
			else
			{
				touch_flag = 0;
			}
		}
//		Uart1_Printf("Worker: %d\n",gh_now);
		Uart1_Printf("  1CENT    %d       %d.%d%d\r\n",coin_num[5],((coine[coinchoose][5]*coin_num[5])/100),(((coine[coinchoose][5]*coin_num[5])%100)/10),(((coine[coinchoose][5]*coin_num[5])%100)%10));
		Uart1_Printf("  5CENT    %d       %d.%d%d\r\n",coin_num[4],((coine[coinchoose][4]*coin_num[4])/100),(((coine[coinchoose][4]*coin_num[4])%100)/10),(((coine[coinchoose][4]*coin_num[4])%100)%10));
		Uart1_Printf("  10CENT   %d       %d.%d%d\r\n",coin_num[3],((coine[coinchoose][3]*coin_num[3])/100),(((coine[coinchoose][3]*coin_num[3])%100)/10),(((coine[coinchoose][3]*coin_num[3])%100)%10));
		Uart1_Printf("  25CENT   %d       %d.%d%d\r\n",coin_num[2],((coine[coinchoose][2]*coin_num[2])/100),(((coine[coinchoose][2]*coin_num[2])%100)/10),(((coine[coinchoose][2]*coin_num[2])%100)%10));
		Uart1_Printf("  50CENT   %d       %d.%d%d\r\n",coin_num[1],((coine[coinchoose][1]*coin_num[1])/100),(((coine[coinchoose][1]*coin_num[1])%100)/10),(((coine[coinchoose][1]*coin_num[1])%100)%10));
		Uart1_Printf("  1Dollar  %d       %d.%d%d\r\n",coin_num[0],((coine[coinchoose][0]*coin_num[0])/100),(((coine[coinchoose][0]*coin_num[0])%100)/10),(((coine[coinchoose][0]*coin_num[0])%100)%10));
		Uart1_Printf("  Coin   Aount(Pcs) Money(Dollar)\r\n");
		Uart1_Printf("	list:\r\n");
		Uart1_Printf("\r\n");
//		Uart1_Printf("		Fake:	   %d Pcs\n",processed_coin_info.total_ng);
		Uart1_Printf("    Money:		%d.%d%d Dollar\n",(processed_coin_info.total_money/100),((processed_coin_info.total_money%100)/10),((processed_coin_info.total_money%100)%10));
		Uart1_Printf("    Amount: 	 %d Pcs\n",processed_coin_info.total_good);
		Uart1_Printf("    Current Count: \n");

		Uart1_Printf("\r\n");

		
	}
	else if(coinchoose == CN3)//其他语种2
	{
		while(1)     // read time 
		{
			comscreen(dgus_readt,6);	//read time
			while(touch_flag ==0){;}
			if (touchnum[7]>0)
			{
				Uart1_Printf("DATE:20%02x-%02x-%02x %02x:%02x\r\n",touchnum[6],touchnum[7],touchnum[8],touchnum[10],touchnum[11]);
				touch_flag = 0;
				break;
			}
			else
			{
				touch_flag = 0;
			}
		}
		//Uart1_Printf("Worker: %d\r\n",gh_now);
		Uart1_Printf("	1PENCY	  %d      %d.%d\r\n",coin_num[7],((coine[coinchoose][7]*coin_num[7])/100),((coine[coinchoose][7]*coin_num[7])%100));
		Uart1_Printf("	2PENCE    %d      %d.%d\r\n",coin_num[6],((coine[coinchoose][6]*coin_num[6])/100),((coine[coinchoose][6]*coin_num[6])%100));
		Uart1_Printf("	5PENCE    %d      %d.%d\r\n",coin_num[5],((coine[coinchoose][5]*coin_num[5])/100),((coine[coinchoose][5]*coin_num[5])%100));
		Uart1_Printf("	10PENCE   %d      %d.%d\r\n",coin_num[4],((coine[coinchoose][4]*coin_num[4])/100),((coine[coinchoose][4]*coin_num[4])%100));
		Uart1_Printf("	20PENCE   %d      %d.%d\r\n",coin_num[3],((coine[coinchoose][3]*coin_num[3])/100),((coine[coinchoose][3]*coin_num[3])%100));
		Uart1_Printf("	50PENCE   %d      %d.%d\r\n",coin_num[2],((coine[coinchoose][2]*coin_num[2])/100),((coine[coinchoose][2]*coin_num[2])%100));
		Uart1_Printf("	1POUND	  %d      %d.%d\r\n",coin_num[1],((coine[coinchoose][1]*coin_num[1])/100),((coine[coinchoose][1]*coin_num[1])%100));
		Uart1_Printf("	2POUNDS	  %d      %d.%d\r\n",coin_num[0],((coine[coinchoose][0]*coin_num[0])/100),((coine[coinchoose][0]*coin_num[0])%100));
		Uart1_Printf("	Coin   Aount(Pcs) Money(POUND)\n");
		Uart1_Printf("	list:\r\n");
		Uart1_Printf("\r\n");
//		Uart1_Printf("		Fake:	   %d Pcs\r\n",processed_coin_info.total_ng);
		Uart1_Printf("		Money:		%d.%d%d POUND\n",(processed_coin_info.total_money/100),((processed_coin_info.total_money%100)/10),((processed_coin_info.total_money%100)%10));
		Uart1_Printf("		Amount: 	 %d Pcs\r\n",processed_coin_info.total_good);
		Uart1_Printf("		Current Count:\r\n");


	}

	Uart1_Printf("\r\n");
	Uart1_Printf("\r\n");
	Uart1_Printf("\r\n");
	Uart1_Printf("\r\n");
	Uart1_Printf("\r\n");
	Uart1_Printf("\r\n");
	Uart1_Printf("\r\n");
	Uart1_Printf("\r\n");
	Uart0_Printf("finished print\r\n");	
}				




