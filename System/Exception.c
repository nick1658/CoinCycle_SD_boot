#include "s3c2416.h"
#include "Exception.h"

// 定义32个函数指针保存各IRQ中断入口函数
static void (*IRQ_Table[32])(void);


void Undef_Handler(void)
{
//	LOG ("Undef_Handler");
	while(1) {

	}
}

void SWI_Handler(void)
{
//	LOG ("SWI_Handler");
	while(1) {

	}
}

void PAbt_Handler(void)
{
//	LOG ("PAbt_Handler");
	while(1) {

	}
}

void DAbt_Handler(void)
{
//	LOG ("DAbt_Handler");
	while(1) {
	
	}
}

void FIQ_Handler(void)
{
//	LOG ("FIQ_Handler");
	while(1) {
		
	}
}

void IRQ_Register(unsigned char Channel, void (*Func)(void))
{
	if (Channel < 32) { // 32个外设中断
		IRQ_Table[Channel] = Func; // 注册相应中断处理函数
	}
}


void IRQ_ClearInt(unsigned char Channel)
{
	rSRCPND1 |= (1 << Channel);
	rINTPND1 |= (1 << Channel);
}


void IRQ_EnableInt(uint8_t Channel)
{
	if (Channel < 32) {
		rSRCPND1 |= (0x01 << Channel);	// write 1 to clear
		rINTPND1 |= (0x01 << Channel);	// write 1 to clear	
		rINTMOD1 &= ~(1 << Channel); // IRQ 模式
		rINTMSK1 &= ~(1 << Channel); // 开中断		
	}
}

void IRQ_DisableInt(uint8_t Channel)
{
	if (Channel < 32) {
		rINTMSK1 |= (1 << Channel); // 关中断		
	}	
}

void IRQ_Handler(void)
{
	unsigned int Offset;
	if (rINTPND1 != 0) 
	{
		// 第一组引起的中断处理	
//		while(1);		
		Offset = rINTOFFSET1;
		if (IRQ_Table[Offset]) 
		{ // 中断处理需己注册
			(IRQ_Table[Offset])(); // 调用相应的中断处理
		}
		rSRCPND1 |= (0x01<<Offset);	// write 1 to clear
		rINTPND1 |= (0x01<<Offset);	// write 1 to clear
	} else if (rINTPND2 != 0) 
		{
		// 第二组引起的中断处理		
		Offset = rINTOFFSET2;
		rSRCPND2 |= (0x01<<Offset);	// write 1 to clear
		rINTPND2 |= (0x01<<Offset);	// write 1 to clear
		switch (Offset) 
			{
			case 0:
			// INT_2D 中断处理	
				break;
			case 4:
			// INT_PCM0 中断处理	
				break;
			case 6:
			// INT_I2S0 中断处理	
				break;
			default:
				break;
			}
		}
}


__asm void disable_interrupts(void)
{
	LDR		R0, =0x4A000000
	LDR		R1, =0xFFFFFFFF
	STR		R1, [R0, #0x08]
	STR		R1, [R0, #0x48]
	STR		R1, [R0, #0x1C]
	bx lr
}



