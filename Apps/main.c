//;----------------------------------------------------------

//;----------------------------------------------------------
#include "s3c2416.h"
#include "Nand.h"

__align(4) U32 code_flag __attribute__((at(0x33104000), zero_init));
//主函数

extern void TftpTrm250ms(void); //250MS调用一次
extern u32_t sys_now(void);
void LwIP_Periodic_Handle(uint32_t localtime)
{
	static uint32_t pre_time = 0;
  /* TCP periodic process every 250 ms */
	if (localtime - pre_time >= 250/*TCP_TMR_INTERVAL*/){
		pre_time =  localtime;
		//tcp_tmr();
		TftpTrm250ms();
	}
}
int main(void)
{
	rWTCON = 0;	// 关闭开门狗
 	port_Init();
	uart_init();//115200bps
	system_env_init ();
	sys_env.uart0_cmd_flag = 2;
	//////////////  
 
	////////////////

	uart1_init();//串口打印机
	uart2_init();//屏幕
	cy_println ("\n#####   Coin_SD_boot Program For YQ ##### ");
	Timer_Init ();
	//watchdog_reset();/*初始化看门狗,T = WTCNT * t_watchdog*/
    rNF_Init();
	// Initilaize the LwIP stack
	lwip_init();	
	// ip address 192, 168, 1, 20
	ethernetif_config();	
	httpd_init();	
	tftp_init ();
	//Hsmmc_Init ();
	//cy_println ("Hsmmc_init_flag is %d", Hsmmc_exist ());
	
	if (code_flag == 0x55555555){
		U8 r_code;
		cy_println ("Code_flag = 0x%x, Begin Write code...", code_flag);
		code_flag = 0;
		r_code = WriteCodeToNand ();
		if (r_code == 0)
			cy_println ("write code to nand flash block 0 page 0 nand addr 0 completed");   
		else
			cy_println ("write code to nand flash block 0 page 0 nand addr 0 failed");  
	}
	
	cmd ();
	
	sys_env.uart0_cmd_flag = 0;
	sys_env.boot_delay = 50*3; // 启动延时3秒
	while (sys_env.boot_delay > 0 || sys_env.boot_stay == 0x55){
		if (sys_env.tty_online_ms == 1){
			sys_env.tty_online_ms = 0;
			sys_env.boot_stay = 0x55;
			update_finish (sys_env.update_flag);
		}
		if (sys_env.uart0_cmd_flag == 1){
			sys_env.boot_stay = 0x55;
			vTaskCmdAnalyze ();
			sys_env.uart0_cmd_flag = 0;
		}
		if( touch_flag ==1){
			touchresult();//判断触摸 状态的函数
			touch_flag =0;
		}
		if (DM9000_GetReceiveStatus()){
			ethernetif_input(&DM9000_netif);
		}
		if (sys_env.net_task == 1){
			sys_env.net_task = 0;
			run_command ("reset");
		}
		LwIP_Periodic_Handle(sys_now ());
		// Handle timeouts
		sys_check_timeouts();
	}
	start_app ();
	while (1){
		if (sys_env.tty_online_ms == 1){
			sys_env.tty_online_ms = 0;
			update_finish (sys_env.update_flag);
		}
		if (sys_env.uart0_cmd_flag == 1){
			vTaskCmdAnalyze ();
			sys_env.uart0_cmd_flag = 0;
		}
		if( touch_flag ==1){
			touchresult();//判断触摸 状态的函数
			touch_flag =0;
		}
		if (DM9000_GetReceiveStatus()){
			ethernetif_input(&DM9000_netif);
		}
		// Handle timeouts
		sys_check_timeouts();
	}
}
