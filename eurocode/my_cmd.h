#ifndef _MY_CMD_
#define _MY_CMD_

#include "def.h"
#define BOOTLOAD_VERSION "V1.0\n(Compile Time:", __DATE__, __TIME__, ")"
#define PRINT_VERSION() cy_println ("Bootloader Version : %s %s %s%s", BOOTLOAD_VERSION);  
#define HEX_TO_DEC(X) (X >> 4) * 10 + (X & 0xf)


/*
 * Monitor Command Table
 */
 
#define CFG_CBSIZE	64		/* Console I/O Buffer Size	*/
 #define CFG_MAXARGS		16	/* max number of command args   */
#define CFG_LONGHELP				/* undef to save memory		*/
//#define CONFIG_AUTO_COMPLETE	1       /* add autocompletion support   */

struct cmd_tbl_s {
	char		*name;		/* Command Name			*/
	int		maxargs;	/* maximum number of arguments	*/
	int		repeatable;	/* autorepeat allowed?		*/
					/* Implementation function	*/
	int		(*cmdhandle)(struct cmd_tbl_s *, int, int, char *[]);
	char		*usage;		/* Usage message	(short)	*/
#ifdef	CFG_LONGHELP
	char		*help;		/* Help  message	(long)	*/
#endif
#ifdef CONFIG_AUTO_COMPLETE
	/* do auto completion on the arguments */
	int		(*complete)(int argc, char *argv[], char last_char, int maxv, char *cmdv[]);
#endif
};

typedef struct cmd_tbl_s	cmd_tbl_t;


extern cmd_tbl_t  __u_boot_cmd_start;
extern cmd_tbl_t  __u_boot_cmd_end;

#define Struct_Section __attribute__ ((unused, section("my_cmd_sec")))


#define MY_CMD(name,maxargs,rep,cmd,usage,help) \
 __align(4) cmd_tbl_t __my_cmd_##name Struct_Section = {#name, maxargs, rep, cmd, usage, help}
 
extern cmd_tbl_t __my_cmd_task_manager;


typedef struct 
{  
	char const *cmd_name;                        //�����ַ���  
	int32_t max_args;                            //��������Ŀ  
	void (*handle)(int argc, void * cmd_arg);     //����ص�����  
	char  *help;                                 //������Ϣ  
}cmd_list_struct; 


#define ARG_NUM     8          //�����������Ĳ�������  
#define CMD_LEN     20         //������ռ�õ�����ַ�����  
#define CMD_BUF_LEN 60         //��������󳤶�  
       
typedef struct 
{  
	char rec_buf[CMD_BUF_LEN];            //�����������  
	char processed_buf[CMD_BUF_LEN];      //�洢�ӹ��������(ȥ�������ַ�)  
	int32_t cmd_arg[ARG_NUM];             //��������Ĳ���  
}cmd_analyze_struct;  


typedef enum {
  NULL_UPDATE = 0,
  UART_UPDATE = 1,
  NET_UPDATE = 2,
  NET_UPDATEING = 3
} e_update_flag;

typedef struct
{
	S16 print_wave_to_pc;
	S16 auto_stop;
	S16 auto_clear;
	S16 sim_count_flag;
	S16 save_ng_data;
	S16 save_good_data;
	
	S16 country_index;
	S16 coin_index;
	S16 workstep;
	S16 tty_mode;
	S16 pre_workstep;
	S16 uart0_cmd_flag;
	S16 hmi_debug_flag;
	U16 stop_time;
	U32 coin_cross_time;
	U32 tty_online_ms;
	U32 boot_delay;
	U32 boot_stay;
	e_update_flag update_flag;
	U32 net_task;
}s_system_env;


extern s_system_env sys_env;
extern uint32_t time_out;


extern void Delay_ms(uint32_t Count);	
extern void Delay_us(uint32_t Count);

void system_env_init (void);
void print_ng_data (S16 index);
void print_system_env_info (void);
void update_finish (e_update_flag flag);
void fill_rec_buf(char data);
void vTaskCmdAnalyze( void );
void run_command (char * _cmd_str);
int my_run_command (const char *cmd, int flag);
unsigned long simple_strtoul(const char *cp,char **endp,unsigned int base);

S16 is_repeate (S16 _coin_index);//  �б� �ǲ����رҵĺ���

#define TTY_ONLINE_TIME 50

extern char iap_code_buf[APP_NAND_SIZE];


extern unsigned int Image$$MY_CMD_RO_SEC$$Base;
extern unsigned int Image$$MY_CMD_RO_SEC$$Length;

#define MY_CMD_RO_SEC_LENGTH 		(&Image$$MY_CMD_RO_SEC$$Length)
#define MY_CMD_RO_SEC_START 		(&Image$$MY_CMD_RO_SEC$$Base)
#define MY_CMD_RO_SEC_END 			((unsigned int)(&Image$$MY_CMD_RO_SEC$$Base) + (unsigned int)MY_CMD_RO_SEC_LENGTH)
	
cmd_tbl_t *find_cmd (const char *cmd);




#endif