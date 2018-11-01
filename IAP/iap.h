#ifndef	_IAP_H
#define _IAP_H

#define HEX_DATA_START 9

#define BOOTLOAD_ADDR		0x33E00000  	//第一个应用程序起始地址(存放在FLASH)
#define SPI_FLASH_INFO_ADDR		(12*1024*1024)  	//第一个应用程序起始地址(存放在FLASH)
#define SPI_FLASH_APP1_ADDR		(13*1024*1024)  	//第一个应用程序起始地址(存放在FLASH)


#define PROGRAM_MAGIC_NUM 0x55aa1a25


#define DEFAULT_APP_FILE_NAME "0:/S3C2416.hex"
#define DEFAULT_APP_BIN_FILE_NAME "0:/S3C2416.bin"

typedef  void (*iapfun)(void);				//定义一个函数类型的参数.
typedef struct{
	U8 start;
	U8 len;
	u16 addr;
	U8 type;
	char data[16];
	U8 sum_check;
}s_hex_file;

typedef struct{
	char app_name[16];
	u32 app_size;
	char reg_info[16];
	U16 SAVE_DATA[128];
}s_spi_file;



void iap_load_app(u32 appxaddr);			//执行flash里面的app程序
int iap_func_hex (char *app_name, int opt);
int iap_func_bin (char *app_name, int opt);
void start_app (void);
int cleanup_before_linux (void);
int load_spi_app (u32 app_addr);



#endif

 
