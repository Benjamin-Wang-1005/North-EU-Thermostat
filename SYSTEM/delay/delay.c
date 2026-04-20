
#include "delay.h"
static u8  fac_us=0;//us delay base
static u16 fac_ms=0;//ms delay base

//SYSTICK fix as 1/8 system clock
//SYSCLK: system clock
void delay_init(u8 SYSCLK)
{
//	SysTick->CTRL&=0xfffffffb;//clear bit2,choose external clock  HCLK/8
	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);	//choose system clock  HCLK/8
	fac_us=SYSCLK/8;		    
	fac_ms=(u16)fac_us*1000;
}								    
//delay n x ms
//SysTick->LOAD max 24 bit,so MAX delay time is:
//nms<=0xffffff*8*1000/SYSCLK
//SYSCLK is Hz,nms unit is ms
//while crystal 72M,nms<=1864 
void delay_ms(u16 nms)
{	 		  	  
	u32 temp;		   
	SysTick->LOAD=(u32)nms*fac_ms;//load delay time(SysTick->LOAD is 24bit)
	SysTick->VAL =0x00;           //clear counter
	SysTick->CTRL=0x01 ;          //start time count
	do
	{
		temp=SysTick->CTRL;
	}
	while(temp&0x01&&!(temp&(1<<16)));//wait time    
	SysTick->CTRL=0x00;       //Close timer
	SysTick->VAL =0X00;       //Clear counter  	    
}   

//delay nus		    								   
void delay_us(u32 nus)
{		
	u32 temp;	    	 
	SysTick->LOAD=nus*fac_us; //load timer  		 
	SysTick->VAL=0x00;        //Clear counter
	SysTick->CTRL=0x01 ;      //start count
	do
	{
		temp=SysTick->CTRL;
	}
	while(temp&0x01&&!(temp&(1<<16)));//wait time exp   
	SysTick->CTRL=0x00;       //Close counter
	SysTick->VAL =0X00;       //clear counter
}




































