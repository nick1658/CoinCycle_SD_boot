
#include "S3C2416.h"

u16 iapbuf[1024];   
u16 spibuf[2048];  

int AnalyseHEX(char hex[],int len)
{
	char i=1;
	char data;
	int cc=0;
	char temp[3];
	do{
		temp[0] = hex[i++];
		temp[1] = hex[i++];
		temp[2] = '\0';
		data = simple_strtoul(temp, NULL, 16);
		cc += data;
	} while (i<(len-2));
	cc%=256;
	cc=0x100-cc;
	cc &= 0xFF;
	temp[0] = hex[i++];
	temp[1] = hex[i++];
	data = simple_strtoul(temp, NULL, 16);
	return (cc==data)?0:1;
}

int get_hex_struct (s_hex_file *p_hex, char *_data_buf)
{
	char temp[16];
	int i = 0, j;
	p_hex->start = _data_buf[i++];
	temp[0] = _data_buf[i++];
	temp[1] = _data_buf[i++];
	temp[2] = '\0';
	p_hex->len = simple_strtoul(temp, NULL, 16);
	temp[0] = _data_buf[i++];
	temp[1] = _data_buf[i++];
	temp[2] = _data_buf[i++];
	temp[3] = _data_buf[i++];
	temp[4] = '\0';
	p_hex->addr = simple_strtoul(temp, NULL, 16);
	temp[0] = _data_buf[i++];
	temp[1] = _data_buf[i++];
	temp[2] = '\0';
	p_hex->type = simple_strtoul(temp, NULL, 16);
	if (p_hex->len > 0){
		for (j = 0; j < p_hex->len; j++){
			temp[0] = _data_buf[i++];
			temp[1] = _data_buf[i++];
			temp[2] = '\0';
			p_hex->data[j] = simple_strtoul(temp, NULL, 16);
		}
	}
	temp[0] = _data_buf[i++];
	temp[1] = _data_buf[i++];
	temp[2] = '\0';
	p_hex->sum_check = simple_strtoul(temp, NULL, 16);
	return AnalyseHEX(_data_buf, p_hex->len * 2 + 11);
}

#define BUFF_SIZE 512
int iap_func_bin (char *app_name, int opt)
{
	u32 * p_code_addr = (u32 *)0x30000000;
	u32 * p_buf;
	int i = 0, index = 0, app_size = 0;
	FATFS fs;
	FIL file;
	FRESULT Res;	
    FILINFO fno;
	char buf[BUFF_SIZE];
	unsigned int ByteRead;
	
	memset (&fno, 0, sizeof (FILINFO));
	memset ((void *)0x30000000, 0, 512*1024);
	
	cy_println ("Start Update App");
	Res = f_mount(&fs, "0:" , 1);	
	
	Res =  f_open(&file, app_name, FA_READ);
	if (Res != FR_OK){
		cy_print("Open file %s failed, ", app_name);	
		cy_println("Res = %d", Res);
		return -1;
	}else{
		if (opt == 1){
			Res = f_stat(app_name, &fno);
			if(fno.fattrib & AM_RDO){
				cy_println ("No Need to Update!");
				return FR_OK;
			}
		}
		/*
		
        cy_println("Size: %lu", fno.fsize);
        cy_println("Timestamp: %u/%02u/%02u, %02u:%02u",
               (fno.fdate >> 9) + 1980, fno.fdate >> 5 & 15, fno.fdate & 31,
               fno.ftime >> 11, fno.ftime >> 5 & 63);
        cy_println("Attributes: %c%c%c%c%c",
               (fno.fattrib & AM_DIR) ? 'D' : '-',
               (fno.fattrib & AM_RDO) ? 'R' : '-',
               (fno.fattrib & AM_HID) ? 'H' : '-',
               (fno.fattrib & AM_SYS) ? 'S' : '-',
               (fno.fattrib & AM_ARC) ? 'A' : '-');
		strcpy(buf, "/");
		Res = f_opendir(&dir, buf);              
		Res = scan_files(buf);
		get_fileinfo (&dir, &fno);*/
	}
	Res = FR_INVALID_OBJECT;
	for (;;){
		if (f_eof(&file))
			break;
		Res = f_read(&file, (unsigned char *)&buf, BUFF_SIZE, &ByteRead);	
		if (Res != RES_OK) {
			cy_println ("Res = %d", Res);
			Uart0_Printf("Read file error\n\r");	
			return -1;
		}
		index++;
		if (index % 40 == 0){
			LED1_NOT;
		}
		p_buf = (u32 *)buf;
		for (i = 0; i < ByteRead; i += 4){
			*p_code_addr = *p_buf;//buf[i] | buf[i+1] << 8 | buf[i+2] << 16 | buf[i+3] << 24;
			p_code_addr++;
			p_buf++;
			app_size += 4;
		}
		if (ByteRead < BUFF_SIZE){
			Res = FR_OK;
		}
	}
	if (opt == 1){
		f_chmod(app_name, AM_RDO, AM_RDO | AM_ARC);
	}
	f_close(&file);
	if (Res == FR_OK){
		return app_size;
	}else{
		return -1;
	}
}
int iap_func_hex (char *app_name, int opt)
{
	u32 flash_addr = 0;
	
	u32 * p_code_addr = (u32 *)0x30000000;
	int i = 0, index = 0, app_size = 0;
	FATFS fs;
	FIL file;
	FRESULT Res;	
    FILINFO fno;
	char buf[BUFF_SIZE];
	s_hex_file p_hex;
	
	memset (&fno, 0, sizeof (FILINFO));
	memset ((void *)0x30000000, 0, 512*1024);
	
	//W25QXX_Read ((U8*)&spi_flash_info, SPI_FLASH_INFO_ADDR, sizeof(s_spi_file));
	cy_println ("Start Update App");
	Res = f_mount(&fs, "0:" , 1);	
	
	Res =  f_open(&file, app_name, FA_READ);
	if (Res != FR_OK){
		cy_print("Open file %s failed, ", app_name);	
		cy_println("Res = %d", Res);
		return -1;
	}else{
		if (opt == 1){
			Res = f_stat(app_name, &fno);
			if(fno.fattrib & AM_RDO){
				cy_println ("No Need to Update!");
				return FR_OK;
			}
		}
		/*
		
        cy_println("Size: %lu", fno.fsize);
        cy_println("Timestamp: %u/%02u/%02u, %02u:%02u",
               (fno.fdate >> 9) + 1980, fno.fdate >> 5 & 15, fno.fdate & 31,
               fno.ftime >> 11, fno.ftime >> 5 & 63);
        cy_println("Attributes: %c%c%c%c%c",
               (fno.fattrib & AM_DIR) ? 'D' : '-',
               (fno.fattrib & AM_RDO) ? 'R' : '-',
               (fno.fattrib & AM_HID) ? 'H' : '-',
               (fno.fattrib & AM_SYS) ? 'S' : '-',
               (fno.fattrib & AM_ARC) ? 'A' : '-');
		strcpy(buf, "/");
		Res = f_opendir(&dir, buf);              
		Res = scan_files(buf);
		get_fileinfo (&dir, &fno);*/
	}
	Res = FR_INVALID_OBJECT;
	for (;;){
		if (f_eof(&file))
			break;
		f_gets (buf, 64, &file);
		index++;
		if (index % 40 == 0){
			LED1_NOT;
		}
		if (get_hex_struct (&p_hex, buf)){//文件错误
			cy_println ("Load App failed!");
			return -1;
		}
		switch (p_hex.type){
			case 0x00://data
				for (i = 0; i < p_hex.len; i += 4){
					*p_code_addr = p_hex.data[i] | p_hex.data[i+1] << 8 | p_hex.data[i+2] << 16 | p_hex.data[i+3] << 24;
					p_code_addr++;
					app_size += 4;
				}
				break;
			case 0x01://end
				cy_println ("Read Line %d of file %s", index, app_name);
				Res =FR_OK;
				break;
			case 0x04://ex addr
				flash_addr = (p_hex.data[0]<< 8 | p_hex.data[1]) << 16;
				if (flash_addr >= BOOTLOAD_ADDR){//否则会覆盖bootloader
					cy_println ("App Runtime Addr Error!");
				return -1;
				}
				p_code_addr = (u32 *)flash_addr;
				break;
			default:break;
		}
	}
	if (opt == 1){
		f_chmod(app_name, AM_RDO, AM_RDO | AM_ARC);
	}
	f_close(&file);
	if (Res == FR_OK){
		return app_size;
	}else{
		return -1;
	}
}

//#define __BOOT_UBOOT_ 0x33C00000

void start_app (void)
{
	cy_println ("\nNow Start App...");
	memset (iap_code_buf, 0, sizeof(iap_code_buf));
#ifndef __BOOT_UBOOT_
	LoadAppFromAppSpace ((U32)&iap_code_buf, sizeof(iap_code_buf));
#else
	LoadAppFromAppSpace ((U32)__BOOT_UBOOT_, sizeof(iap_code_buf));
#endif
	
#ifndef __BOOT_UBOOT_
	int *magic_num = (int*)(&iap_code_buf[0x3c]);
#else
	int *magic_num = (int*)(__BOOT_UBOOT_ + 0x3C);
#endif
	if (*magic_num == PROGRAM_MAGIC_NUM){
		void (*theKernel)(void);
#ifndef __BOOT_UBOOT_
		theKernel = (void (*)(void))0x30000000;
#else
		theKernel = (void (*)(void))__BOOT_UBOOT_;
#endif
		cy_println ("disable all interrupts");
		disable_interrupts ();
		MMU_DisableDCache(); // 禁用DCache
		MMU_DisableICache(); // 禁用ICache
		MMU_InvalidateDCache(); // 使16K DCache无效
		MMU_InvalidateICache(); // 使16K ICache无效
		//MMU_EnableICache(); // 加快执行MMU_Init
		MMU_DisableMMU(); // 禁用MMU
		MMU_InvalidateTLB(); // 使无效转换表
		theKernel ();
	}else{
		comscreen(Disp_Indexpic[26],Number_IndexpicB);	//触摸屏跳转到提示固件丢失界面
		cy_println ("The app was not valid!!!"); 
	}
}

__asm  int cleanup_before_linux (void)
{
	push	{r4, lr}
//bl	c3e017f8 <disable_interrupts>
	mrc	p15, 0, r3, c1, c0, 0
	bic	r3, r3, #4096	; 0x1000
	bic	r3, r3, #4
	mcr	p15, 0, r3, c1, c0, 0
	mov	r0, #0
	mcr	p15, 0, r0, c7, c7, 0
	pop	{r4, pc}
}






















