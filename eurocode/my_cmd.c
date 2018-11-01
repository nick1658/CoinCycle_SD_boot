#include "def.h"
#include "S3C2416.h"

cmd_analyze_struct cmd_analyze; 

void printf_hello(int32_t argc, void *cmd_arg);
void handle_arg(int32_t argc, void *cmd_arg);
void do_go(int32_t argc, void *cmd_arg);
void do_set(int32_t argc, void *cmd_arg);
void do_print(int32_t argc, void *cmd_arg);
void do_help(int32_t argc, void *cmd_arg);
void do_read(int32_t argc, void *cmd_arg);
void do_write(int32_t argc, void *cmd_arg);
void do_erase(int32_t argc, void *cmd_arg);
int do_reset_cpu (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
void do_db(int32_t argc, void *cmd_arg);
int cmd_usage(cmd_tbl_t *cmdtp);
/*命令表*/  
const cmd_list_struct cmd_list[]={  
/*   命令    参数数目    处理函数        帮助信息                         */     
{"help",		0,	do_help,		"help"},  
{"write",		8,	do_write,		"write <to> <nand> <block> <page>"}, 
{"erase",		8,	do_erase,		"erase <to> <nand> <block> <page>"}, 
{"read",		8,	do_read,		"read <from> <nand> <block> <page>"}, 
};

s_system_env sys_env;

__align(4) char iap_code_buf[APP_NAND_SIZE] __attribute__((at(0x30000000), zero_init)); // 程序缓冲区256K
void system_env_init (void)
{
	int i;
	S8 *p = (S8 *) &sys_env;
	for (i = 0; i < sizeof(s_system_env); i++)
	{
		*(p++) = 0;
	}
	sys_env.coin_index = sys_env.coin_index;
	sys_env.save_ng_data = 1;
	sys_env.save_good_data = 1;
	sys_env.workstep = 102;
		
}

/** 
* 使用SecureCRT串口收发工具,在发送的字符流中可能带有不需要的字符以及控制字符, 
* 比如退格键,左右移动键等等,在使用命令行工具解析字符流之前,需要将这些无用字符以 
* 及控制字符去除掉. 
* 支持的控制字符有: 
*   上移:1B 5B 41 
*   下移:1B 5B 42 
*   右移:1B 5B 43 
*   左移:1B 5B 44 
*   回车换行:0D 0A 
*  Backspace:08 
*  Delete:7F 
*/  
static uint32_t get_true_char_stream(char *dest, const char *src)  
{  
   uint32_t dest_count=0;  
   uint32_t src_count=0;  
     
    while(src[src_count] != '\0')//0x0D && src[src_count+1]!=0x0A)  
    {  
       if(isprint(src[src_count]))  
       {  
           dest[dest_count++]=src[src_count++];  
       }  
       else  
       {  
           switch(src[src_count])  
           {  
                case    0x08:                          //退格键键值  
                {  
                    if(dest_count>0)  
                    {  
                        dest_count --;  
                    }  
                    src_count ++;  
                }break;  
                case    0x1B:  
                {  
                    if(src[src_count+1]==0x5B)  
                    {  
                        if(src[src_count+2]==0x41 || src[src_count+2]==0x42)  
                        {  
                            src_count +=3;              //上移和下移键键值  
                        }  
                        else if(src[src_count+2]==0x43)  
                        {  
                            dest_count++;               //右移键键值  
                            src_count+=3;  
                        }  
                        else if(src[src_count+2]==0x44)  
                        {  
                            if(dest_count >0)           //左移键键值  
                            {  
                                dest_count --;  
                            }  
                           src_count +=3;  
                        }  
                        else  
                        {  
                            src_count +=3;  
                        }  
                    }  
                    else  
                    {  
                        src_count ++;  
                    }  
                }break;  
                default:  
                {  
                    src_count++;  
                }break;  
           }  
       }  
    }  
   dest[dest_count++]=src[src_count++];   
    return dest_count;  
} 

unsigned long simple_strtoul(const char *cp,char **endp,unsigned int base)
{
	unsigned long result = 0,value;

	if (*cp == '0') {
		cp++;
		if ((*cp == 'x') && isxdigit(cp[1])) {
			base = 16;
			cp++;
		}
		if (!base) {
			base = 8;
		}
	}
	if (!base) {
		base = 10;
	}
	while (isxdigit(*cp) && (value = isdigit(*cp) ? *cp-'0' : (islower(*cp)
	    ? toupper(*cp) : *cp)-'A'+10) < base) {
		result = result*base + value;
		cp++;
	}
	if (endp)
		*endp = (char *)cp;
	return result;
}

long simple_strtol(const char *cp,char **endp,unsigned int base)
{
	if(*cp=='-')
		return -simple_strtoul(cp+1,endp,base);
	return simple_strtoul(cp,endp,base);
}




/*字符串转10/16进制数*/                                                              
static int32_t string_to_dec(uint8_t *buf)                              
{                                                                                    
   uint32_t i=0;                                                                     
   uint32_t base=10;       //基数                                                    
   int32_t  neg=1;         //表示正负,1=正数                                         
   int32_t  result=0;                                                                
                                                                                     
    if((buf[0]=='0')&&(buf[1]=='x'))                                                 
    {                                                                                
       base=16;                                                                      
       neg=1;                                                                        
       i=2;                                                                          
    }                                                                                
    else if(buf[0]=='-')                                                             
    {                                                                                
       base=10;                                                                      
       neg=-1;                                                                       
       i=1;                                                                          
    }                                                                                
    for(; buf[i] != 0; i++)                                                                  
    {                                                                                
       if(buf[i]==0x20 || buf[i]==0x0D || buf[i] == '\0')    //为空格                                  
       {                                                                             
           break;                                                                    
       }                                                                             
                                                                                     
       result *= base;                                                               
       if(isdigit(buf[i]))                 //是否为0~9                               
       {                                                                             
           result += buf[i]-'0';                                                     
       }                                                                             
       else if(isxdigit(buf[i]))           //是否为a~f或者A~F                        
       {                                                                             
            result+=tolower(buf[i])-87;                                              
       }                                                                             
       else                                                                          
       {                                                                             
           result += buf[i]-'0';                                                     
       }                                                                             
    }                                                                                
   result *= neg;                                                                    
                                                                                     
    return result ;                                                                  
}           

                                                    
static uint32_t rec_count=0;  

/**                                                                                                                            
* 命令参数分析函数,以空格作为一个参数结束,支持输入十六进制数(如:0x15),支持输入负数(如-15)                                      
* @param rec_buf   命令参数缓存区                                                                                              
* @param len       命令的最大可能长度                                                                                          
* @return -1:       参数个数过多,其它:参数个数                                                                                 
*/                                                                                                                             
static int32_t cmd_arg_analyze(const char *rec_buf, unsigned int len)                                                                
{                                                                                                                              
   uint32_t i;                                                                                                                 
   uint32_t blank_space_flag=0;    //空格标志                                                                                  
   uint32_t arg_num=0;             //参数数目                                                                                  
   uint32_t index[ARG_NUM];        //有效参数首个数字的数组索引                                                                
                                                                                                                               
    /*先做一遍分析,找出参数的数目,以及参数段的首个数字所在rec_buf数组中的下标*/                                                
    for(i=0;i<len;i++)                                                                                                         
    {                                                                                                                          
       if(rec_buf[i]==0x20)        //为空格                                                                                    
       {                                                                                                                       
           blank_space_flag=1;                                                                                                 
           continue;                                                                                                           
       }                                                                                                                       
       else if(rec_buf[i]==0x0D || rec_buf[i] == '\0')   //换行                                                                                     
       {                                                                                                                       
           break;                                                                                                              
       }                                                                                                                       
       else                                                                                                                    
       {                                                                                                                       
           if(blank_space_flag==1)                                                                                             
           {                                                                                                                   
                blank_space_flag=0;                                                                                            
                if(arg_num < ARG_NUM)                                                                                          
                {                                                                                                              
                   index[arg_num]=i;                                                                                           
                    arg_num++;                                                                                                 
                }                                                                                                              
                else                                                                                                           
                {                                                                                                              
                    return -1;      //参数个数太多                                                                             
                }                                                                                                              
           }                                                                                                                   
       }                                                                                                                       
    }                                                                                                                          
                                                                                                                               
    for(i=0;i<arg_num;i++)                                                                                                     
    {                                                                                                                          
        cmd_analyze.cmd_arg[i]=string_to_dec((unsigned char *)(rec_buf+index[i]));                            
    }                                                                                                                          
    return arg_num;                                                                                                            
}                                                                                                                              
  



                                                                         
void run_command (char * _cmd_str)
{
	int i = 0;
	
	SetWatchDog(); //看门狗喂狗
	
	while (*(_cmd_str + i) )
	{
		if ( (*(_cmd_str + i) ) == 0x0a)
			break;
		cmd_analyze.rec_buf[i] = *(_cmd_str + i);
		//cy_print ("%02x ", cmd_analyze.rec_buf[i]);
		i++;
	}
	if (i == 0)
		return;
	cmd_analyze.rec_buf[i] = '\0';
	//cmd_analyze.rec_buf[i + 1] = 0x0A;
	vTaskCmdAnalyze ();
}
	
	/*命令行分析任务*/  
void vTaskCmdAnalyze( void )  
{  
   uint32_t i;  
   int32_t rec_arg_num;  
    char cmd_buf[CMD_LEN];        
    uint32_t rec_num;     
	
                                                                                                                      
	rec_num=get_true_char_stream(cmd_analyze.processed_buf, cmd_analyze.rec_buf);  
		
	//cy_println ("rec_buf = %s", cmd_analyze.rec_buf);
	//cy_println ("rec_num = %d", rec_num);
	//cy_println ("processed_buf = %s", cmd_analyze.processed_buf);
                                                                                                                                       
      /*从接收数据中提取命令*/                                                                                                         
      for(i=0;i<CMD_LEN;i++)                                                                                                           
      {                                                                                                                                
          if((i > 0) && ((cmd_analyze.processed_buf[i]==' ') || (cmd_analyze.processed_buf[i]==0x0D)  || (cmd_analyze.processed_buf[i] == 0) ))                                       
          {                                                                                                                            
               cmd_buf[i]='\0';        //字符串结束符                                                                                  
               break;                                                                                                                  
          }                                                                                                                            
          else                                                                                                                         
          {                                                                                                                            
               cmd_buf[i]=cmd_analyze.processed_buf[i];                                                                                
          }                                                                                                                            
      }                                                                                                                                
                                                                                                                                       
      rec_arg_num=cmd_arg_analyze(&cmd_analyze.processed_buf[i], rec_num);                                                              
                                                                                                                                       
      for(i=0;i<sizeof(cmd_list)/sizeof(cmd_list[0]);i++)                                                                              
      {                                                                                                                                
          if(!strcmp(cmd_buf,cmd_list[i].cmd_name))       //字符串相等                                                                 
          {                                                                                                                            
               if(rec_arg_num<0 || rec_arg_num>cmd_list[i].max_args)                                                                   
               {                                                                                                                       
                   cy_println("Too Much arg\n");                                                                                             
               }                                                                                                                       
               else                                                                                                                    
               {                                                                                                                       
					cmd_list[i].handle(rec_arg_num,(void *)cmd_analyze.cmd_arg);  
					//cy_println ("<End cmd_list[%d].%s>", i, cmd_list[i].cmd_name);	
					cy_println ();
               }                                                                                                                       
               break;                                                                                                                  
          }                                                                                                                            
                                                                                                                                       
      }                                                                                                                                
	if(i>=sizeof(cmd_list)/sizeof(cmd_list[0]))                                                                                      
	{                                                                                                                                
		//cy_println("Unsurport Cmd: %s", cmd_buf);     
		//cmd();
		strcpy (cmd_buf, cmd_analyze.rec_buf);	
		if (my_run_command (cmd_buf, 0) >= 0)
		{
		}	
	}    
	cmd ();	                                                                                                                            
	sys_env.uart0_cmd_flag = 0;
}



U8 Disp_Indexpic[Number_IndexpicA][Number_IndexpicB]=         //MCU控制图片寄存器数据  切换 屏图片 指令数组
{
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x00},   //显示 索引 0		
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x01},	//显示 索引 1
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x02},	//显示 索引 2		
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x03},	//显示 索引 3
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x04},	//显示 索引 4	   	
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x05},	//显示 索引 5
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x06},   //显示 索引 6		
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x07},	//显示 索引 7 
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x08},	//显示 索引 8   	
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x09},	//显示 索引 9   
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x0A},	//显示 索引 10  	
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x0B},	//显示 索引 11 
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x0C},	//显示 索引 12 	
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x0D},	//显示 索引 13  
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x0E},	//显示 索引 14  	
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x0F},	//显示 索引 15 
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x10},	//显示 索引 16	
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x11},	//显示 索引 17
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x12},	//显示 索引 18	
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x13},	//显示 索引 19
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x14},	//显示 索引 20	
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x15},	//显示 索引 21
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x16},	//显示 索引 22	
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x17},	//显示 索引 23
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x18},	//显示 索引 24	
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x19},	//显示 索引 25
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x1A},	//显示 索引 26	
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x1B},	//显示 索引 27
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x1C},	//显示 索引 28	
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x1D},	//显示 索引 29
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x1E},	//显示 索引 30	
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x1F},	//显示 索引 31
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x20},	//显示 索引 32	
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x21},	//显示 索引 33
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x22},	//显示 索引 34	
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x23},	//显示 索引 35
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x24},	//显示 索引 36	
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x25},	//显示 索引 37
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x26},	//显示 索引 38	
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x27},	//显示 索引 39
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x28},	//显示 索引 40	
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x29},	//显示 索引 41
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x2A},	//显示 索引 42	
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x2B},	//显示 索引 43
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x2C},	//显示 索引 44	
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x2D},	//显示 索引 45
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x2E},	//显示 索引 46  
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x2F},	//显示 索引 47
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x30},	//显示 索引 48	
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x31},	//显示 索引 49
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x32},	//显示 索引 50	
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x33},	//显示 索引 51
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x34},	//显示 索引 52	
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x35},	//显示 索引 53
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x36},	//显示 索引 54	
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x37},	//显示 索引 55
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x38},	//显示 索引 56	
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x39},	//显示 索引 57
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x3a},	//显示 索引 58	
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x3b},	//显示 索引 59
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x3c},	//显示 索引 60	
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x3d},	//显示 索引 61
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x3e},	//显示 索引 62	
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x3f},	//显示 索引 63
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x40},	//显示 索引 64	
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x41},	//显示 索引 65
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x42},	//显示 索引 66	
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x43},	//显示 索引 67
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x44},	//显示 索引 68	
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x45},	//显示 索引 69
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x46},	//显示 索引 70	
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x47},	//显示 索引 71
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x48},	//显示 索引 72	
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x49},	//显示 索引 73
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x4a},	//显示 索引 74	
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x4b},	//显示 索引 75
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x4c},	//显示 索引 76	
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x4d},	//显示 索引 77
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x4e},	//显示 索引 78	
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x4f},	//显示 索引 79
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x50},	//显示 索引 80	
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x51},	//显示 索引 81
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x52},	//显示 索引 82  
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x53},	//显示 索引 83
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x54},	//显示 索引 84	
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x55},	//显示 索引 85
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x56},	//显示 索引 86	
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x57},	//显示 索引 87
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x58},	//显示 索引 88	
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x59},	//显示 索引 89
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x5a},	//显示 索引 90	
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x5b},	//显示 索引 91
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x5c},	//显示 索引 92	
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x5d},	//显示 索引 93
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x5e},	//显示 索引 94	
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x5f},	//显示 索引 95
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x60},	//显示 索引 96	
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x61},	//显示 索引 97
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x62},	//显示 索引 98	
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x63},	//显示 索引 99
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x64},	//显示 索引 100	
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x65},	//显示 索引 101
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x66},	//显示 索引 102	
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x67},	//显示 索引 103
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x68},	//显示 索引 104	
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x69},	//显示 索引 105
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x6a},	//显示 索引 106	
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x6b},	//显示 索引 107
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x6c},	//显示 索引 108	
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x6d},	//显示 索引 109
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x6e},	//显示 索引 110	
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x6f},	//显示 索引 111
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x70},	//显示 索引 112	
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x71},	//显示 索引 113
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x72},	//显示 索引 114	
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x73},	//显示 索引 115
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x74},	//显示 索引 116	
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x75},	//显示 索引 117
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x76},	//显示 索引 118   
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x77},	//显示 索引 119	
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x78},	//显示 索引 120	
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x79},	//显示 索引 121	
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x7A},	//显示 索引 122	
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x7B},	//显示 索引 123	
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x7C},	//显示 索引 124	
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x7D},	//显示 索引 125
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x7E},	//显示 索引 126	
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x7F},	//显示 索引 127
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x80},	//显示 索引 128	
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x81},	//显示 索引 129
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x82},	//显示 索引 130	
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x83},	//显示 索引 131
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x84},	//显示 索引 132	
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x85},	//显示 索引 133
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x86},	//显示 索引 134	
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x87},	//显示 索引 135	
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x88},	//显示 索引 136	
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x89},	//显示 索引 137	
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x8A},	//显示 索引 138	
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x8B},	//显示 索引 139	
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x8C},	//显示 索引 140	
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x8D},	//显示 索引 141
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x8E},	//显示 索引 142	
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x8F},	//显示 索引 143
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x90},	//显示 索引 144	
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x91},	//显示 索引 145
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x92},	//显示 索引 146	
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x93},	//显示 索引 147
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x94},	//显示 索引 148	
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x95},	//显示 索引 149
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x96},	//显示 索引 150	
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x97},	//显示 索引 151	
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x98},	//显示 索引 152	
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x99},	//显示 索引 153	
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x9A},	//显示 索引 154	
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x9B},	//显示 索引 155	
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x9C},	//显示 索引 156	
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x9D},	//显示 索引 157
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x9E},	//显示 索引 158	
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0x9F},	//显示 索引 159
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0xA0},	//显示 索引 160	
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0xA1},	//显示 索引 161
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0xA2},	//显示 索引 162	
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0xA3},	//显示 索引 163
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0xA4},	//显示 索引 164	
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0xA5},	//显示 索引 165
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0xA6},	//显示 索引 166	
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0xA7},	//显示 索引 167	
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0xA8},	//显示 索引 168	
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0xA9},	//显示 索引 169	
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0xAA},	//显示 索引 170	
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0xAB},	//显示 索引 171	
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0xAC},	//显示 索引 172	
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0xAD},	//显示 索引 173
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0xAE},	//显示 索引 174	
	{0xA5,0x5A,0x04,0x80,0x03,0x00,0xAF},	//显示 索引 175
};
volatile U8 touchnum[TSGET_NUM] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
volatile U16 uartcount = 0;  // 串口2接收 字节 计数
volatile U16 touch_flag =0;  // 串口2接收 标志位
void touchget_from_uart2(void)    //receive the touch from dgus at real time 
{

	U8 temp;
	while(!(rUTRSTAT2&0x1));
	temp = rURXH2;//读取寄存器值
	
	if(touch_flag ==0)
	{
		if( (uartcount == 0)&& (temp == 0xA5))//0xa5 引索
		{
			touchnum[uartcount] = temp;
//			cy_print("%d:%x ",uartcount,touchnum[uartcount]);
			uartcount++;
		}
		else if( (uartcount == 1)&& (temp == 0x5A))//0x5a 引索
		{
			touchnum[uartcount] = temp;
//			cy_print("%d:%x ",uartcount,touchnum[uartcount]);
			uartcount++;
		}
		else if( (uartcount == 2))//字节数
		{
			touchnum[uartcount] = temp;
//			cy_print("%d:%x ",uartcount,touchnum[uartcount]);
			uartcount++;
		}
		//总共要接受的字节数 + 3是因为有数据帧头A5 5A 和长度共三个字节
		else if( (uartcount > 2)&& (uartcount < (touchnum[2]+3)))
		{
			touchnum[uartcount] = temp;
//			cy_print("%d:%x ",uartcount,touchnum[uartcount]);
			if((uartcount ==  (touchnum[2]+2)))
			{
				touch_flag =1; //触摸屏数据接收完成
				////////////////////////////////////////////////////////////////////////////////////////
				/////////////////////////////////////////////////////////////////////////////////////
				uartcount = 0;
				return;
			}
			uartcount++;
		}
		else
		{
			cy_print("U %d ",uartcount);
			uartcount = 0;
		}
	}
	else 
	{
		uartcount = 0;
	}
	return;
}



void touchresult(void)      //根据接收到的  数 来决定 执行的任务
{
	U16 addr, value;
	addr = (touchnum[4] << 8) | (touchnum[5]);
	value = (touchnum[7] << 8) | (touchnum[8]);
	///////////////A5 5A 06 83 00 06 01 00 0x:1 2/////////////////////////
	switch (addr){
		case ADDR_RESET:  //
			if (value == 0x0123){
				comscreen(Disp_Indexpic[0],Number_IndexpicB);
				cy_println ("Software Reset...");
				run_command ("reset");	
			}
			break;
		default:break;
	}
}


void comscreen(U8* str,S16 length)  //发送 指令至液晶屏57600bps
{
	U32 i;
	U8 temp;
	for(i = 0;i<length;i++)
	{
		temp = 	*str;
		Uart2_sendchar(temp);	
		str++;
	}	
	
}

u8 auchCRCHi[]=
{
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01,
0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81,
0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01,
0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01,
0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01,
0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
0x40

};

u8 auchCRCLo[] =
{
0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06, 0x07, 0xC7, 0x05, 0xC5, 0xC4,
0x04, 0xCC, 0x0C, 0x0D, 0xCD, 0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09,
0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A, 0x1E, 0xDE, 0xDF, 0x1F, 0xDD,
0x1D, 0x1C, 0xDC, 0x14, 0xD4, 0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,
0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3, 0xF2, 0x32, 0x36, 0xF6, 0xF7,
0x37, 0xF5, 0x35, 0x34, 0xF4, 0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A,
0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9, 0x29, 0xEB, 0x2B, 0x2A, 0xEA, 0xEE,
0x2E, 0x2F, 0xEF, 0x2D, 0xED, 0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60, 0x61, 0xA1, 0x63, 0xA3, 0xA2,
0x62, 0x66, 0xA6, 0xA7, 0x67, 0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F,
0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68, 0x78, 0xB8, 0xB9, 0x79, 0xBB,
0x7B, 0x7A, 0xBA, 0xBE, 0x7E, 0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,
0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71, 0x70, 0xB0, 0x50, 0x90, 0x91,
0x51, 0x93, 0x53, 0x52, 0x92, 0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C,
0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B, 0x99, 0x59, 0x58, 0x98, 0x88,
0x48, 0x49, 0x89, 0x4B, 0x8B, 0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42, 0x43, 0x83, 0x41, 0x81, 0x80,
0x40
}; 

u16 CRC16(char * _data_buf,int len)
{
	u8 uchCRCHi=0xff;
	u8 uchCRCLo=0xff;
	u16 uindex;
	while(len--){
		uindex = uchCRCHi^ *_data_buf++;
		uchCRCHi = uchCRCLo^auchCRCHi[uindex];
		uchCRCLo = auchCRCLo[uindex];
	}
	return (uchCRCHi<<8|uchCRCLo);
}

void update_finish (e_update_flag flag)
{
	u16 crc = 0;
	switch (flag)
	{
		case UART_UPDATE:
			if (rec_count > 1){	
				crc = CRC16 (iap_code_buf, rec_count - 2);//CRC 校验
				if (crc == ((iap_code_buf[rec_count - 2] << 8) | iap_code_buf[rec_count - 1])){//最后两个字节是校验码
					cy_println("OK");//校验通过
					cmd();
					run_command ("write flash");
				}else{
					cy_println("ERROR");//校验失败请求重发
					cmd();
					comscreen(Disp_Indexpic[27],Number_IndexpicB);	//触摸屏跳转到提示固件丢失界面
				}
				rec_count = 0;
				sys_env.tty_mode = 0;
				memset (cmd_analyze.rec_buf, 0, sizeof(cmd_analyze.rec_buf));
			}
		break;
		case NET_UPDATE:
			run_command ("write flash");
			break;
		default:break;
	}
	sys_env.update_flag = NULL_UPDATE;
}
/*提供给串口中断服务程序，保存串口接收到的单个字符*/                                   
void fill_rec_buf(char data)                                                           
{                                                                                      
    //接收数据                     
	if (sys_env.tty_mode == 0x55){ //程序下载
		sys_env.tty_online_ms = TTY_ONLINE_TIME;
		iap_code_buf[rec_count++] = data;
		if (rec_count == 1){
			cy_print ("\n");
		}
		if (rec_count % 1024 == 0){
			cy_print (".");
		}
		if (rec_count % (1024*64) == 0){
			cy_print ("\n");
		}
		if (rec_count >= APP_NAND_SIZE){
			rec_count=0; 
			sys_env.tty_mode = 0;
			cy_println("ERROR");//数组越界，表示接收失败
		}
	}else if (sys_env.uart0_cmd_flag == 0){
		if (rURXH0 == '\b'){
			if (rec_count > 0){
				Uart0_sendchar('\b');
				Uart0_sendchar(' ');
				Uart0_sendchar('\b');
				Uart0_sendchar('\b');
				if (rec_count > 1){
					Uart0_sendchar(cmd_analyze.rec_buf[rec_count - 2]);
				}else{
					Uart0_sendchar(':');
				}
				rec_count--;
				cmd_analyze.rec_buf[rec_count] = ' ';
			}
			return;
		}
		
		Uart0_sendchar(data);		
		if(0x0D == data){// && 0x0D==cmd_analyze.rec_buf[rec_count-1]) 
			if (rec_count > 0) 
			{				
				cmd_analyze.rec_buf[rec_count] = '\0';  
				//cy_println ("rec_count = %d", rec_count);  
				rec_count=0;    
			}	  
			Uart0_sendchar('\n');
			sys_env.uart0_cmd_flag = 1; 	
			//cy_println ("rec_count = %d", rec_count);
			//vTaskCmdAnalyze (); 		                          
		}                                                                                  
		else                                                                               
		{                                    
			cmd_analyze.rec_buf[rec_count] = data;                                                 
			rec_count++;                                                                   																   
		   /*防御性代码，防止数组越界*/                                                    
		   if(rec_count>=CMD_BUF_LEN)                                                      
		   {                                                                               
			   rec_count=0;                                                                
		   }                                                                               
		}     
	}
	else
	{
		rec_count = 0;
	}
}                                                                                      
 

/*打印每个参数*/                                                                
void handle_arg(int32_t argc, void * cmd_arg)                                
{                                                                               
   uint32_t i;                                                                  
   int32_t  *arg=(int32_t *)cmd_arg;                                            
                                                                                
    if(argc==0)                                                                 
    {                                                                           
       cy_println("There is no arg");                                  
    }                                                                           
    else                                                                        
    {                                                                           
       for(i=0;i<argc;i++)                                                      
       {                                                                        
			cy_println("The %dst arg is :%d",i+1,arg[i]);            
       }                                                                        
    } 
}                                                                               

 


/*打印字符串:Hello world!*/                                    
 void do_help(int32_t argc, void *cmd_arg)              
 {  
	int i;
	cy_println("\n----------------------------------------------------");   
	for (i = 0; i < (sizeof (cmd_list) / sizeof (cmd_list_struct)); i++)
	{
		cy_println("--%s			%s", cmd_list[i].cmd_name, cmd_list[i].help);     
	}	
	cy_println("----------------------------------------------------\n");   	
 }   


 
void write_para_1 (int32_t arg[])
{
	U8 r_code;
	if (arg[0] == string_to_dec((uint8 *)("code"))){ 
		r_code = WriteCodeToNand ();
		if (r_code == 0)
			cy_println ("write code to nand flash block 0 page 0 nand addr 0 completed");   
		else
			cy_println ("write code to nand flash block 0 page 0 nand addr 0 failed");  
	}else if (arg[0] == string_to_dec((uint8 *)("reset"))){
		sys_env.tty_mode = 0;
		rec_count = 0; 
		memset (iap_code_buf, 0, sizeof(iap_code_buf));
	}else if (arg[0] == string_to_dec((uint8 *)("start"))){
		sys_env.tty_mode = 0x55;
		sys_env.update_flag = UART_UPDATE;
		rec_count = 0;
		comscreen(Disp_Indexpic[22],Number_IndexpicB);	//触摸屏跳转到更新界面 
	}else if (arg[0] == string_to_dec((uint8 *)("flash"))){
		int *magic_num = (int*)(&iap_code_buf[0x3c]);
		if (*magic_num == PROGRAM_MAGIC_NUM){
			r_code = WriteAppToAppSpace ((U32)&iap_code_buf, sizeof(iap_code_buf));
			if (r_code == 0)
				cy_println ("write iap_code_buf to nand flash block 10 page 0 nand addr 0 completed");   
			else
				cy_println ("write iap_code_buf to nand flash block 10 page 0 nand addr 0 failed"); 
			comscreen(Disp_Indexpic[23],Number_IndexpicB);	//触摸屏跳转到更新完成界面 
		}else{
			cy_println ("The app was not valid!!!"); 
			comscreen(Disp_Indexpic[27],Number_IndexpicB);	//触摸屏跳转到提示非法程序界面 
		}
	}else if (arg[0] == string_to_dec((uint8 *)("coin-config"))){ 
	}else{                                                                                                    
		cy_println ("write_para_1 arg Error!");                                                    
		return;                                                          
	}
}
 void do_write(int32_t argc, void *cmd_arg)
{
	switch (argc)
	{
		case 1:
			write_para_1 ((int32_t *)cmd_arg);
		break;
		default: 
			cy_println("%d arg", argc); break;
	}
}

#define BUFF_SIZE 512
void read_para_1(int32_t arg[])
{
}

void read_para_2(int32_t arg[])
{
	U8 read_page_buf[2048];
	int i, j;
	U8 r_code;
	if (arg[0]  == string_to_dec((uint8 *)("nand")))                                                   
	{     
	}                                                                                                  
	else                                                                                                 
	{                                                                                                    
		cy_println ("device must be nand or sd");                                                    
		return;                                                          
	} 
	r_code = Nand_ReadPage (arg[1], 0, read_page_buf);
	if (r_code == 0)
	{
		cy_println ("Read nand block %d page 0", arg[1]);  
		for(i = 0; i < 16; i++)
		{
			for(j = 0; j < 16; j++)
			{
				cy_print("%02x ", read_page_buf[i*16+j]);
			}
			cy_println();
		}
	}
	else if (r_code == 1)
	{
		cy_println (" Parameter Error, Buf must be not null!!!");  
	}
	else if (r_code == 2)
	{
		cy_println ("ECC Error, Read Op failed!!!");  
	}
}

void read_para_3 (int32_t arg[])
{
	uint32_t nand_addr;	
	U8 read_page_buf[2048];
	U8 r_code;
	int i, j;
	if (arg[0]  == string_to_dec((uint8 *)("nand")))                                                   
	{     					  //block * 2048 * 64 +   page * 2048
		nand_addr = (uint32_t)(arg[1] * 2048 * 64 + arg[2] * 2048);
	}                                                                                                  
	else                                                                                                 
	{                                                                                                    
		cy_println ("device must be nand or ddr");                                                    
		return;                                                          
	} 
	r_code = Nand_ReadSkipBad (nand_addr, read_page_buf, 2048);
	if (r_code == 0)
	{
		cy_println ("Read nand block %d page %d", arg[1], arg[2]);  
		for(i = 0; i < 16; i++)
		{
			for(j = 0; j < 16; j++)
			{
				cy_print("%02x ", read_page_buf[i*16+j]);
			}
			cy_println();
		}
	}
	else if (r_code == 1)
	{
		cy_println (" Parameter Error, Buf must be not null!!!");  
	}
	else if (r_code == 2)
	{
		cy_println ("ECC Error, Read Op failed!!!");  
	}
}

void do_read(int32_t argc, void *cmd_arg)
{	
	switch (argc)
	{    
		case 1:
			read_para_1 ((int32_t *)cmd_arg);
			break;
		case 2:
			read_para_2 ((int32_t *)cmd_arg);
			break;
		case 3:
			read_para_3 ((int32_t *)cmd_arg);
			break;
		default: 
			cy_println("%d arg", argc); break;
	}
}


//int do_run (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
//{	
//	switch (argc){
//		case 1:
//			cy_println ("Now Start app");
//			start_app ();	
//			break;
//		case 2:
//			if (iap_func_hex (argv[1], 0) != 0){
//			}else{
//				start_app ();
//			}
//			break;
//		default: cmd_usage (cmdtp);break;
//	}
//	return 0;
//}

//MY_CMD(
//	run,	4,	1,	do_run,
//	"run - run app\n",
//	"run\n"
//);


MY_CMD(
	reset,	4,	1,	do_reset_cpu,
	"reset - reset system\n",
	"reset\n"
);




int do_set_run (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{	
	switch (argc){
		case 1:
			cy_println ("hello world");
			break;
		default: cmd_usage (cmdtp);break;
	}
	return 0;
}
MY_CMD(
	set_run,	4,	1,	do_set_run,
	"set - set app\n",
	"set\n"
);

int do_print_version  (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{	
	cy_println("\n----------------------------------------------------");   
	PRINT_VERSION()
	cy_println("----------------------------------------------------");   
	return 0;
}
MY_CMD(
	version,	4,	1,	do_print_version,
	"version - print version information\n",
	"version\n"
);

void erase_para_2 (int32_t arg[])
{	
	if (arg[0]  == string_to_dec((uint8 *)("nand")))                                                   
	{     
	}                                                                                                  
	else                                                                                                 
	{                                                                                                    
		cy_println ("device must be nand or ddr");                                                    
		return;                                                          
	} 
	if (arg[1] < 20){                         
		cy_println ("can not erase program block %d", arg[1]);
	}else{
		Nand_EraseBlock (arg[1]);                                                    
		cy_println ("erase block %d completed", arg[1]);
	}
}

void do_erase(int32_t argc, void *cmd_arg)
{
	switch (argc)
	{               
		case 2:
			erase_para_2 ((int32_t *)cmd_arg);
			break;
		default: 
			cy_println("%d arg", argc); break;
	}
}

/*
 * reset the cpu by setting up the watchdog timer and let him time out
 */
int do_reset_cpu (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	cy_print("reset... \n\n\n");
	//LOG ("Reset System...");
	rSWIRST = 0x533C2416;

	/* loop forever and wait for reset to happen */
	while (1);
}

int cmd_usage(cmd_tbl_t *cmdtp)
{
	cy_print("%s", cmdtp->usage);
#ifdef	CONFIG_SYS_LONGHELP
	cy_print("Usage:\n");

	if (!cmdtp->help) {
		puts ("- No additional help available.\n");
		return 1;
	}

	my_puts (cmdtp->help);
	//my_putc ('\n');
	//my_putc ('\n');
#endif	/* CONFIG_SYS_LONGHELP */
	return 0;
}

 //遍历文件
 //path:路径
 //返回值:执行结果
FILINFO fileinfo;	//文件信息
DIR dir;  			//目录
U8 mf_scan_files(U8 * path)
{
	FRESULT res;	  
    char *fn;   /* This function is assuming non-Unicode cfg. */
#if _USE_LFN
 	fileinfo.lfsize = _MAX_LFN * 2 + 1;
	fileinfo.lfname = malloc(fileinfo.lfsize);
#endif		  

    res = f_opendir(&dir,(const TCHAR*)path); //打开一个目录
    if (res == FR_OK) 
	{	
		cy_print("\r\n"); 
		while(1)
		{
	        res = f_readdir(&dir, &fileinfo);                   //读取目录下的一个文件
	        if (res != FR_OK || fileinfo.fname[0] == 0) break;  //错误了/到末尾了,退出
	        //if (fileinfo.fname[0] == '.') continue;             //忽略上级目录
#if _USE_LFN
        	fn = *fileinfo.lfname ? fileinfo.lfname : fileinfo.fname;
#else							   
        	fn = fileinfo.fname;
#endif	                                              /* It is a file. */
			cy_print("%s/", path);//打印路径	
			cy_print("%s\r\n",  fn);//打印文件名	  
		} 
		cy_print("\r\n"); 
    }	 
#if _USE_LFN
	free(fileinfo.lfname);
#endif	 
    return res;	  
}
int do_ls (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{	
	FATFS *fs;
	FRESULT Res;	
	fs = malloc (sizeof (FATFS));
	if (fs == NULL){
		cy_println ("malloc (sizeof (FATFS)) failed!");
		return -1;
	}
	switch (argc){
		case 1:
			Res = f_mount(fs, "0:" , 0);
			if (Res != FR_OK){
				cy_println ("f_mount disk \"0:\" failed!");
			}else{
				mf_scan_files ("0:");
			}
			break;
		case 2:
			if (strcmp (argv[1], "sd。") == 0){
				Res = f_mount(fs, "0:" , 0);
				if (Res != FR_OK){
					cy_println ("f_mount disk \"0:\" failed!");
				}else{
					mf_scan_files ("0:");
				}
				break;
			}else if (strcmp (argv[1], "flash") == 0){
				Res = f_mount(fs, "1:" , 0);
				if (Res != FR_OK){
					cy_println ("f_mount disk \"1:\" failed!");
				}else{
					mf_scan_files ("1:");
				}
				break;
			}else{
				cy_println ("disk must be sd or flash");
			}
			cmd_usage (cmdtp);
			break;
		default: cmd_usage (cmdtp);break;
	}
	if (fs != NULL)
		free (fs);
	return 0;
}
MY_CMD(
	ls,	4,	1,	do_ls,
	"ls - list the disk file on current path\n",
	"ls disk path\n"
);



/* test if ctrl-c was pressed */
static int ctrlc_disabled = 0;	/* see disable_ctrl() */
static int ctrlc_was_pressed = 0;
int ctrlc (void)
{
//	if (!ctrlc_disabled && gd->have_console) {
//		if (tstc ()) {
//			switch (getc ()) {
//			case 0x03:		/* ^C - Control C */
//				ctrlc_was_pressed = 1;
//				return 1;
//			default:
//				break;
//			}
//		}
//	}
	return 0;
}

int do_my_help (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);

MY_CMD(
	help,	CFG_MAXARGS,	0,	do_my_help,
 	"help    - print online help\n",
 	"[command ...]\n"
 	"    - show help information (for 'command')\n"
 	"'help' prints online help for the monitor commands.\n\n"
 	"Without arguments, it prints a short usage message for all commands.\n\n"
 	"To get detailed help information for specific commands you can type\n"
  "'help' with one or more command names as arguments.\n"
);

/* This do not ust the U_BOOT_CMD macro as ? can't be used in symbol names */
#ifdef  CFG_LONGHELP
cmd_tbl_t __u_my__cmd_question_mark Struct_Section = {
	"?",	CFG_MAXARGS,	0,	do_my_help,
 	"?       - alias for 'help'\n",
	NULL
};
#else
cmd_tbl_t __u_my__cmd_question_mark Struct_Section = {
	"?",	CFG_MAXARGS,	0,	do_my_help,
 	"?       - alias for 'help'\n"
};
#endif /* CFG_LONGHELP */
int do_my_help (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int i;
	int rcode = 0;
	cmdtp = &__my_cmd_help; //加上这一句防止my_cmd_sec这个Section 被编译器优化掉
	if (argc == 1)
	{	/*show list of commands */

		int cmd_items = (unsigned int)MY_CMD_RO_SEC_LENGTH / sizeof (cmd_tbl_t);	/* pointer arith! */
		cmd_tbl_t *cmd_array[cmd_items];
		int i, j, swaps;

		/* Make array of commands from .uboot_cmd section */
		cmdtp = (cmd_tbl_t *)MY_CMD_RO_SEC_START;
		for (i = 0; i < cmd_items; i++) {
			cmd_array[i] = cmdtp++;
		}

		/* Sort command list (trivial bubble sort) */
		for (i = cmd_items - 1; i > 0; --i) {
			swaps = 0;
			for (j = 0; j < i; ++j) {
				if (strcmp (cmd_array[j]->name,
					    cmd_array[j + 1]->name) > 0) {
					cmd_tbl_t *tmp;
					tmp = cmd_array[j];
					cmd_array[j] = cmd_array[j + 1];
					cmd_array[j + 1] = tmp;
					++swaps;
				}
			}
			if (!swaps)
				break;
		}

		/* print short help (usage) */
		for (i = 0; i < cmd_items; i++) {
			const char *usage = cmd_array[i]->usage;

			/* allow user abort */
			if (ctrlc ())
				return 1;
			if (usage == NULL)
				continue;
			my_puts (usage);
		}
		return 0;
	}
	/*
	 * command help (long version)
	 */
	for (i = 1; i < argc; ++i) {
		if ((cmdtp = find_cmd (argv[i])) != NULL) {
#ifdef	CFG_LONGHELP
			/* found - print (long) help info */
			my_puts (cmdtp->name);
			my_putc (' ');
			if (cmdtp->help) {
				my_puts (cmdtp->help);
			} else {
				my_puts ("- No help available.\n");
				rcode = 1;
			}
			my_putc ('\n');
#else	/* no long help available */
			if (cmdtp->usage)
				my_puts (cmdtp->usage);
#endif	/* CFG_LONGHELP */
		} else {
			cy_print ("Unknown command '%s' - try 'help'"
				" without arguments for list of all"
				" known commands\n\n", argv[i]
					);
			rcode = 1;
		}
	}
	return rcode;
}


/***************************************************************************
 * find command table entry for a command
 */
cmd_tbl_t *find_cmd (const char *cmd)
{
	cmd_tbl_t *cmdtp;
	cmd_tbl_t *cmdtp_temp = (cmd_tbl_t *)MY_CMD_RO_SEC_START;	/*Init value */
	const char *p;
	int len;
	int n_found = 0;

	/*
	 * Some commands allow length modifiers (like "cp.b");
	 * compare command name only until first dot.
	 */
	len = ((p = strchr(cmd, '.')) == NULL) ? strlen (cmd) : (p - cmd);

	for (cmdtp = (cmd_tbl_t *)MY_CMD_RO_SEC_START;
	     cmdtp != (cmd_tbl_t *)MY_CMD_RO_SEC_END;
	     cmdtp++) {
		if (strncmp (cmd, cmdtp->name, len) == 0) {
			if (len == strlen (cmdtp->name))
				return cmdtp;	/* full match */

			cmdtp_temp = cmdtp;	/* abbreviated command ? */
			n_found++;
		}
	}
	if (n_found == 1) {			/* exactly one match */
		return cmdtp_temp;
	}

	return NULL;	/* not found or ambiguous command */
}

/* pass 1 to disable ctrlc() checking, 0 to enable.
 * returns previous state
 */
int disable_ctrlc (int disable)
{
	int prev = ctrlc_disabled;	/* save previous state */

	ctrlc_disabled = disable;
	return prev;
}

int had_ctrlc (void)
{
	return ctrlc_was_pressed;
}

void clear_ctrlc (void)
{
	ctrlc_was_pressed = 0;
}
/****************************************************************************/

int parse_line (char *line, char *argv[])
{
	int nargs = 0;

#ifdef DEBUG_PARSER
	cy_print ("parse_line: \"%s\"\n", line);
#endif
	while (nargs < CFG_MAXARGS) {

		/* skip any white space */
		while ((*line == ' ') || (*line == '\t')) {
			++line;
		}

		if (*line == '\0') {	/* end of line, no more args	*/
			argv[nargs] = NULL;
#ifdef DEBUG_PARSER
		cy_print ("parse_line: nargs=%d\n", nargs);
#endif
			return (nargs);
		}

		argv[nargs++] = line;	/* begin of argument string	*/

		/* find end of string */
		while (*line && (*line != ' ') && (*line != '\t')) {
			++line;
		}

		if (*line == '\0') {	/* end of line, no more args	*/
			argv[nargs] = NULL;
#ifdef DEBUG_PARSER
		cy_print ("parse_line: nargs=%d\n", nargs);
#endif
			return (nargs);
		}

		*line++ = '\0';		/* terminate current arg	 */
	}

	cy_print ("** Too many args (max. %d) **\n", CFG_MAXARGS);

#ifdef DEBUG_PARSER
	cy_print ("parse_line: nargs=%d\n", nargs);
#endif
	return (nargs);
}

/****************************************************************************/

static void process_macros (const char *input, char *output)
{
	char c, prev;
	const char *varname_start = NULL;
	int inputcnt = strlen (input);
	int outputcnt = CFG_CBSIZE;
	int state = 0;		/* 0 = waiting for '$'  */

	/* 1 = waiting for '(' or '{' */
	/* 2 = waiting for ')' or '}' */
	/* 3 = waiting for '''  */
#ifdef DEBUG_PARSER
	char *output_start = output;

	cy_print ("[PROCESS_MACROS] INPUT len %d: \"%s\"\n", strlen (input),
		input);
#endif

	prev = '\0';		/* previous character   */

	while (inputcnt && outputcnt) {
		c = *input++;
		inputcnt--;

		if (state != 3) {
			/* remove one level of escape characters */
			if ((c == '\\') && (prev != '\\')) {
				if (inputcnt-- == 0)
					break;
				prev = c;
				c = *input++;
			}
		}

		switch (state) {
		case 0:	/* Waiting for (unescaped) $    */
			if ((c == '\'') && (prev != '\\')) {
				state = 3;
				break;
			}
			if ((c == '$') && (prev != '\\')) {
				state++;
			} else {
				*(output++) = c;
				outputcnt--;
			}
			break;
		case 1:	/* Waiting for (        */
			if (c == '(' || c == '{') {
				state++;
				varname_start = input;
			} else {
				state = 0;
				*(output++) = '$';
				outputcnt--;

				if (outputcnt) {
					*(output++) = c;
					outputcnt--;
				}
			}
			break;
		case 2:	/* Waiting for )        */
			if (c == ')' || c == '}') {
				int i;
				char envname[CFG_CBSIZE], *envval;
				int envcnt = input - varname_start - 1;	/* Varname # of chars */

				/* Get the varname */
				for (i = 0; i < envcnt; i++) {
					envname[i] = varname_start[i];
				}
				envname[i] = 0;

				/* Get its value */
				envval = getenv (envname);

				/* Copy into the line if it exists */
				if (envval != NULL)
					while ((*envval) && outputcnt) {
						*(output++) = *(envval++);
						outputcnt--;
					}
				/* Look for another '$' */
				state = 0;
			}
			break;
		case 3:	/* Waiting for '        */
			if ((c == '\'') && (prev != '\\')) {
				state = 0;
			} else {
				*(output++) = c;
				outputcnt--;
			}
			break;
		}
		prev = c;
	}

	if (outputcnt)
		*output = 0;

#ifdef DEBUG_PARSER
	cy_print ("[PROCESS_MACROS] OUTPUT len %d: \"%s\"\n",
		strlen (output_start), output_start);
#endif
}

/****************************************************************************
 * returns:
 *	1  - command executed, repeatable
 *	0  - command executed but not repeatable, interrupted commands are
 *	     always considered not repeatable
 *	-1 - not executed (unrecognized, bootd recursion or too many args)
 *           (If cmd is NULL or "" or longer than CFG_CBSIZE-1 it is
 *           considered unrecognized)
 *
 * WARNING:
 *
 * We must create a temporary copy of the command since the command we get
 * may be the result from getenv(), which returns a pointer directly to
 * the environment data, which may change magicly when the command we run
 * creates or modifies environment variables (like "bootp" does).
 */

int my_run_command (const char *cmd, int flag)
{
	cmd_tbl_t *cmdtp;
	char cmdbuf[CFG_CBSIZE];	/* working copy of cmd		*/
	char *token;			/* start of token in cmdbuf	*/
	char *sep;			/* end of token (separator) in cmdbuf */
	char finaltoken[CFG_CBSIZE];
	char *str = cmdbuf;
	char *argv[CFG_MAXARGS + 1];	/* NULL terminated	*/
	int argc, inquotes;
	int repeatable = 1;
	int rc = 0;

#ifdef DEBUG_PARSER
	cy_print ("[RUN_COMMAND] cmd[%p]=\"", cmd);
	my_puts (cmd ? cmd : "NULL");	/* use my_puts - string may be loooong */
	my_puts ("\"\n");
#endif

	clear_ctrlc();		/* forget any previous Control C */

	if (!cmd || !*cmd) {
		return -1;	/* empty command */
	}

	if (strlen(cmd) >= CFG_CBSIZE) {
		my_puts ("## Command too long!\n");
		return -1;
	}

	strcpy (cmdbuf, cmd);

	/* Process separators and check for invalid
	 * repeatable commands
	 */

#ifdef DEBUG_PARSER
	cy_print ("[PROCESS_SEPARATORS] %s\n", cmd);
#endif
	while (*str) {

		/*
		 * Find separator, or string end
		 * Allow simple escape of ';' by writing "\;"
		 */
		for (inquotes = 0, sep = str; *sep; sep++) {
			if ((*sep=='\'') &&
			    (*(sep-1) != '\\'))
				inquotes=!inquotes;

			if (!inquotes &&
			    (*sep == ';') &&	/* separator		*/
			    ( sep != str) &&	/* past string start	*/
			    (*(sep-1) != '\\'))	/* and NOT escaped	*/
				break;
		}

		/*
		 * Limit the token to data between separators
		 */
		token = str;
		if (*sep) {
			str = sep + 1;	/* start of command for next pass */
			*sep = '\0';
		}
		else
			str = sep;	/* no more commands for next pass */
#ifdef DEBUG_PARSER
		cy_print ("token: \"%s\"\n", token);
#endif

		/* find macros in this token and replace them */
		process_macros (token, finaltoken);

		/* Extract arguments */
		if ((argc = parse_line (finaltoken, argv)) == 0) {
			rc = -1;	/* no command at all */
			continue;
		}

		/* Look up command in command table */
		if ((cmdtp = find_cmd(argv[0])) == NULL) {
			cy_print ("Unknown my_cmd command '%s' - now try 'vTaskCmdAnalyze'\n", argv[0]);
			rc = -1;	/* give up after bad command */
			continue;
		}

		/* found - check max args */
		if (argc > cmdtp->maxargs) {
			cy_print ("Usage:\n%s\n", cmdtp->usage);
			rc = -1;
			continue;
		}

#if (CONFIG_COMMANDS & CFG_CMD_BOOTD)
		/* avoid "bootd" recursion */
		if (cmdtp->cmdhandle == do_bootd) {
#ifdef DEBUG_PARSER
			cy_print ("[%s]\n", finaltoken);
#endif
			if (flag & CMD_FLAG_BOOTD) {
				my_puts ("'bootd' recursion detected\n");
				rc = -1;
				continue;
			} else {
				flag |= CMD_FLAG_BOOTD;
			}
		}
#endif	/* CFG_CMD_BOOTD */

		/* OK - call function to do the command */
		if ((cmdtp->cmdhandle) (cmdtp, flag, argc, argv) != 0) {
			rc = -1;
		}

		repeatable &= cmdtp->repeatable;

		/* Did the user stop this? */
		if (had_ctrlc ())
			return 0;	/* if stopped then not repeatable */
	}

	return rc ? rc : repeatable;
}


                                                                












