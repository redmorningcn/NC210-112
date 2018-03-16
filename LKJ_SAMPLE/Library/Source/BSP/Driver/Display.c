#include "stm32f10x.h"
#include <stdio.h>
#include "SZ_STM32F107VC_LIB.h"
#include "DS3231.h"
#include "I2C_CLK.h"
#include "Display.h"
#include "delay.h"
#ifdef PHOTOELECTRIC_VELOCITY_MEASUREMENT
#include <Speed_sensor.h>
#include "app_task_mater.h"
#else
#include <power_macro.h>
#endif
#include "SPI_CS5463_AC.h"
 
#include 	"Display.h"  
#include 	"string.h"

#include <includes.h>

//定义端口及功能引脚---------------------------------------------------------------
//#define     DDR_7219    DDRB 
//#define     PINT_7219   PINB
//#define     PORT_7219   PORTB  
//#define     CS_7219     4                       //数据锁存

//max7219寄存器地址定义-------------------------------------------------------------
#define     NoOp        0x00                    // 空操作寄存器
#define     Digit0      0x01                    // 数码管1寄存器
#define     Digit1      0x02                    // 数码管2寄存器
#define     Digit2      0x03                    // 数码管3寄存器
#define     Digit3      0x04                    // 数码管4寄存器
#define     Digit4      0x05                    // 数码管5寄存器
#define     Digit5      0x06                    // 数码管6寄存器
#define     Digit6      0x07                    // 数码管7寄存器
#define     Digit7      0x08                    // 数码管8寄存器
/**/
#define     DecodeMode  0x09                    // 译码模式寄存器
#define     Intensity   0x0a                    // 亮度寄存器
#define     ScanLimit   0x0b                    // 扫描位数寄存器
#define     ShutDown    0x0c                    // 低功耗模式寄存器
#define     DisplayTest 0x0f                    // 显示测试寄存器
//max7219控制寄存器命令
#define     ShutdownMode    0x00                // 低功耗方式
#define     NormalOperation 0x01                // 正常操作方式
//#define     DecodeDigit     0xff                // 译码设置，8位均为BCD码;对8个数都编码   
#define     NoDecodeDigit   0x00                // 译码设置，对8个数都不编码   
#define     ScanDigit       0x07                // 扫描位数设置，显示8位数码管
//#define   ScanDigit 0x03                      // 扫描位数设置，显示8位数码管
#define   	IntensityGrade  0x2a                // 亮度级别设置
//#define     IntensityGrade  0x03              // 亮度级别设置
#define     TestMode    0x01                    // 显示测试模式 
#define     TextEnd     0x00                    // 显示测试结束，恢复正常工作模式


#define RCC_LED_DIS      		RCC_APB2Periph_GPIOD
#define MOSI_DIS                GPIO_Pin_8
#define SCK_DIS                 GPIO_Pin_10
#define CS_DIS                  GPIO_Pin_9
#define LED_DIS_PORT            GPIOD


/**-------------------------------------------------------
  * @函数名 LED_DIS_Config
  * @功能   初始化LED的端口
  * @参数   无
  * @返回值 无
***------------------------------------------------------*/
void LED_DIS_Config(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  /* Enable GPIOB, GPIOC and AFIO clock */
  RCC_APB2PeriphClockCmd(RCC_LED_DIS , ENABLE);
    
  /* LEDs pins configuration */
  GPIO_InitStructure.GPIO_Pin = MOSI_DIS | SCK_DIS | CS_DIS;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(LED_DIS_PORT, &GPIO_InitStructure);
}


//------------------------------------------------------------------------
//  名  称 ：void WriteCS_DIS( uint8_t temp )
//  功  能 ：设置 CS_DIS
// 入口参数：无
// 出口参数：无
//------------------------------------------------------------------------
void WriteCS_DIS(uint8_t temp)
{
	if( temp )						// 端口输出高电平
	{
		GPIO_SetBits(LED_DIS_PORT, CS_DIS);				//PD2 置1
	}
	else
	{
		GPIO_ResetBits(LED_DIS_PORT, CS_DIS);			//PD2 置0
	}
}
//------------------------------------------------------------------------
//  名  称 ：void WriteSCK1( uint8_t temp )
//  功  能 ：设置 SCK0
// 入口参数：无
// 出口参数：无
//------------------------------------------------------------------------
void WriteSCK1(uint8_t temp)
{
	if( temp )						// 端口输出高电平
	{
		GPIO_SetBits(LED_DIS_PORT, SCK_DIS);
	}
	else
	{
		GPIO_ResetBits(LED_DIS_PORT, SCK_DIS);
	}
}


//------------------------------------------------------------------------
//  名  称 ：void WriteMOSI1( uint8_t temp )
//  功  能 ：设置 MOSI1
// 入口参数：无
// 出口参数：无
//------------------------------------------------------------------------
void WriteMOSI1(uint8_t temp)
{	
	if( temp )					
	{
		GPIO_SetBits(LED_DIS_PORT, MOSI_DIS);
	}
	else
	{
		GPIO_ResetBits(LED_DIS_PORT, MOSI_DIS);
	}
}


//---------------------------------------------------------------------------------
//函数名称:             SentByte_SPI()
//功    能:             SPI发送一字节数据
//入口参数:             Data8
//出口参数:             无
//说    明:				以模拟SPI总线方式发送数据
//----------------------------------------------------------------------------------
void SentByte_SPI1(uint8_t ch)
{
	uint8_t i,temp;   
	
	WriteSCK1 ( 0 );
	
    for (i=0;i<8;i++)
    {
		temp=ch&0x80;
    	ch=ch<<1;
        if(temp)
        {
        	WriteMOSI1( 1 );	

        	WriteSCK1 ( 0 );

//            NOP();
  
        	WriteSCK1 ( 1 );	        	
        }
        else
        {
        	WriteMOSI1( 0 );	

            WriteSCK1 ( 0 );

//            NOP();
            
        	WriteSCK1 ( 1 );
        }
    }
    
//    NOP();
    
    WriteSCK1 ( 0 );
}

//----------------------------------------------------------------------------
//函数名称:   SentByteTo7219()
//功    能:   向7219发送1Byte数据或命令
//入口参数:   addr:   数据接收地址
//            Data:   要存的数据
//出口参数:   无
//-----------------------------------------------------------------------------
void SentByteTo7219(uint8_t Addr,uint8_t Data) 
{
  	 WriteCS_DIS( 0 );			   					   		
  	 SentByte_SPI1( Addr );		                //送地址											
  	 SentByte_SPI1( Data );                    	//送数据
  	 WriteCS_DIS( 1 );							//数据锁存,关7219接收		  									
}


//-------------------------------------------------------------------
//函数名称:     Init7219 ()
//功    能:     7219初始化
//入口参数:     无
//出口参数:     无
//-------------------------------------------------------------------
void Init7219 (void)
{
//	InitIOForSPI1();								//SPI端口初始化IO口
	
    SentByteTo7219 (DisplayTest,TextEnd);         	// 设置工作模式
	SentByteTo7219 (ScanLimit,ScanDigit);         	// 设置扫描界限
    SentByteTo7219 (DecodeMode,NoDecodeDigit);      // 设置不译码模式
    SentByteTo7219 (Intensity,IntensityGrade);    	// 设置亮度
    SentByteTo7219 (ShutDown,NormalOperation);    	// 设置为正常工作模式
}

uint8_t LedNumCode[]={0x7E,0x30,0x6D,0x79,0x33,0x5B,0x5F,0x70,0x7F,0x7B,0x77,
                   // 0    1    2    3    4    5    6    7    8    9    A 
                     0x1F,0x4E,0x3D,0x4F,0x47};  
                   // b   C    d     E    F      


	 /*			b
			  -----
			 |     |
		   g |	   | c
			 |	h  |
			  -----
			 |     |
		   f |	   | d
			 |	   |
			  -----	 .a
				e
          编码规则： abcd efgh

	 */


/*                   
uint8_t LedCharCode[]={0x77,0x1F,0x4E,0x3D,0x4F,0x47,0x7B,0x37,0x30,0x3c,0x87,0x0e,0x01,
                   //   A    B    C    d    E    F    g    H     I   J    K    L   Minus    
                        0x76,0x7E,0x67,0xfE,0x66,0x5B,0x0f,0x3e,0x3e,0x00,0x37,0x33,0x6D};  
                        //n   O    P    Q     r    S    t    U    V    W   X    Y    Z   
*/                        
uint8_t LedCharCode[] = {0x00,0xE5,0xE5,0xE5,0xE5,0xE5,0xE5,0xE5,0xE5,0xE5,0xE5,0xE5,0xE5,0x01,0xE5,0xE5,
				  	   //  ''   !    "    #    $    %    &    '    (     )   *    +    ,    -    .   /
					  0x7E,0x30,0x6D,0x79,0x33,0x5B,0x5F,0x70,0x7F,0x7B, 
					 // 0    1   2	  3	    4    5    6    7    8    9	
					  0xE5,0xE5,0xE5,0xE5,0xE5,0xE5,0xE5,
					 // :    ;    <    =     >   ?   @
					  0x77,0x1F,0x4E,0x3d,0x4F,0x47,0x5F,0x37,0x30,0x3c,0x87,0x0e,0xE5,
                    //  A    B   C    D     E    F    G    H   I    J    K    L    M    
                      0x76,0x7E,0x67,0xFE,0x66,0x5B,0x0F,0x3E,0x3E,0xE5,0x37,0x33,0x6D,
                    //  N    O    P   Q    R     S    T    U    V   W    X    Y    Z     
                      0xE5,0xE5,0xE5,0xE5,0x08,0xE5,
                     // [    \    ]    ^    _    `
                      0x77,0x1F,0x0D,0x3D,0x4F,0x47,0x5F,0x17,0x30,0x3c,0x87,0x0e,0xE5,
                     // a    b   c    d     e    f    g    h    i    j     k    l    m
					  0x15,0x1D,0x67,0x73,0x05,0x5B,0x0F,0x1C,0x1C,0xD3,0x37,0x33,0x6D,
                     // n    o   p    q     r    s    t    u    v   w    x    y    z     
                      0xE5,0xE5,0xE5,0xE5,0xE5,0x00//
                      // {    |    }    ~       灭
					   };     
           
#define     LED_CHAR_NNM    16                  //数码管显示编码个数 
#define     DISPLAY_LED_NUM 8                   //数码管个数   
        
//--------------------------------------------------------------------
//void          DisplayNum (uint32_t Unmber)
//功    能:     用数码管将整型数据显示出来
//入口参数:     number
//出口参数:     无
//-------------------------------------------------------------------
void    DisplayNum (uint32_t Unmber)
{
    unsigned char i  = DISPLAY_LED_NUM;
	unsigned char digit[8] = {Digit0,Digit1,Digit2,Digit3,Digit4,Digit5,Digit6,Digit7};
	
	Init7219();                                //显示初始化
	
	do
	{ 		
		i--;
		SentByteTo7219(digit[i],LedNumCode[Unmber % 10] );       //从低位到高位显示
		Unmber /= 10;                       
	} while(i && Unmber);										 //高位的0不显示
	while(i)
	{
        i--;
        SentByteTo7219(digit[i],0x00);            //将没有用位的显示清除        
    }	  
}

//--------------------------------------------------------------------
//void          DisplayNum_WithZero (uint32_t Unmber)
//功    能:     用数码管将整型数据显示出来
//入口参数:     number
//出口参数:     无
//-------------------------------------------------------------------
void    DisplayNum_WithZero (uint32_t Unmber)
{
    unsigned char i  = DISPLAY_LED_NUM;
	unsigned char digit[8] = {Digit0,Digit1,Digit2,Digit3,Digit4,Digit5,Digit6,Digit7};
	
	Init7219();                                //显示初始化
	
	do
	{ 		
		i--;
		SentByteTo7219(digit[i],LedNumCode[Unmber % 10] );       //从低位到高位显示
		Unmber /= 10;                       
	}while(i);													 //高位的0也显示出来	
	while(i)
	{
        i--;
        SentByteTo7219(digit[i],0x00);            //将没有用位的显示清除        
    }	  
}

//--------------------------------------------------------------------
//void          DisplayNumWithDoit (uint32_t Unmber)
//功    能:     用数码管将整型数据显示出来
//入口参数:     number
//出口参数:     无
//-------------------------------------------------------------------
void    DisplayNumWithDot (uint32_t Unmber)
{
    unsigned char i  = DISPLAY_LED_NUM;
	unsigned char digit[8] = {Digit0,Digit1,Digit2,Digit3,Digit4,Digit5,Digit6,Digit7};

	do
	{ 		
		i--;
		SentByteTo7219(digit[i],LedNumCode[Unmber % 10] + 0x80);       //从低位到高位显示
		Unmber /= 10;                       
	}while(i && Unmber);
		
	while(i)
	{
        i--;
        SentByteTo7219(digit[i],0x00);            //将没有用位的显示清除        
    }	  
}

//------------------------------------------------------
//函数名称:     DisplayString ()
//功    能:     用数码管显示字符串代码
//入口参数:     String: 字符代码
//出口参数:     无
//显示内容:     String
//内容代码:	    调用格式  DisplayString("ABCD");
//-------------------------------------------------------
void DisplayString(char * String)
{
	uint8_t   i = 0;
	uint8_t   digit[8] = {Digit0,Digit1,Digit2,Digit3,Digit4,Digit5,Digit6,Digit7};
	uint8_t   CharTemp;
	
	while( i < DISPLAY_LED_NUM)
	{
        SentByteTo7219(digit[i],0x00);                      //将没有用位的显示清除  
          
        i++;    
    }	
	
	i = 0;
	while(i < DISPLAY_LED_NUM && strlen((char *)String) - i > 0)
	{ 		
		i++;
		if(String[strlen((char *)String) - i] >= ' ')					//显示字母
		{
			CharTemp = String[strlen((char *)String)- i] - ' ';                       	//求显示字符位置,LedCharCode位置
			SentByteTo7219(digit[DISPLAY_LED_NUM - i],LedCharCode[CharTemp]);  	//从低位到高位显示
//			SentByteTo7219(digit[i],LedCharCode[CharTemp]);  	//从高位到低位显示       
		}   
		else													//显示数字
		{
			CharTemp = String[strlen((char *)String) - i] - '0';                      	//求显示字符位置,LedCharCode位置
			SentByteTo7219(digit[DISPLAY_LED_NUM - i],LedNumCode[CharTemp]);   	//从低位到高位显示 
//			SentByteTo7219(digit[i],LedNumCode[CharTemp]);   	//从高位到低位显示     			
		}        
	}
}


//------------------------------------------------------
//函数名称:     DisplayString_WithDot ()
//功    能:     用数码管显示字符串代码,带小数点
//入口参数:     String: 字符代码
//出口参数:     无
//显示内容:     String
//内容代码:	    调用格式  DisplayString("ABCD");
//-------------------------------------------------------
void DisplayString_WithDot(char * String)
{
	uint8_t   i = 0;
	uint8_t   digit[8] = {Digit0,Digit1,Digit2,Digit3,Digit4,Digit5,Digit6,Digit7};
	uint8_t   CharTemp;
	
	while( i < DISPLAY_LED_NUM)
	{
        SentByteTo7219(digit[i],0x00);                      //将没有用位的显示清除  
          
        i++;    
    }	
	
	i = 0;
	while(i < DISPLAY_LED_NUM && strlen((char *)String) - i > 0)
	{ 		
		i++;
		if(i == 4)
		{
			if(String[strlen((char *)String) - i] >= ' ')					//显示字母
			{
				CharTemp = String[strlen((char *)String)- i] - ' ';                       	//求显示字符位置,LedCharCode位置
				SentByteTo7219(digit[DISPLAY_LED_NUM - i],LedCharCode[CharTemp] + 0x80);  	//从低位到高位显示
//				SentByteTo7219(digit[i],LedCharCode[CharTemp]);  	//从高位到低位显示       
			}   
			else													//显示数字
			{
				CharTemp = String[strlen((char *)String) - i] - '0';                      	//求显示字符位置,LedCharCode位置
				SentByteTo7219(digit[DISPLAY_LED_NUM - i],LedNumCode[CharTemp] + 0x80);   	//从低位到高位显示 
//				SentByteTo7219(digit[i],LedNumCode[CharTemp]);   	//从高位到低位显示     			
			}
		}
		else
		{
			if(String[strlen((char *)String) - i] >= ' ')					//显示字母
			{
				CharTemp = String[strlen((char *)String)- i] - ' ';                       	//求显示字符位置,LedCharCode位置
				SentByteTo7219(digit[DISPLAY_LED_NUM - i],LedCharCode[CharTemp]);  	//从低位到高位显示
//				SentByteTo7219(digit[i],LedCharCode[CharTemp]);  	//从高位到低位显示       
			}   
			else													//显示数字
			{
				CharTemp = String[strlen((char *)String) - i] - '0';                      	//求显示字符位置,LedCharCode位置
				SentByteTo7219(digit[DISPLAY_LED_NUM - i],LedNumCode[CharTemp]);   	//从低位到高位显示 
//				SentByteTo7219(digit[i],LedNumCode[CharTemp]);   	//从高位到低位显示     			
			}
		}			        
	}
}

//------------------------------------------------------
//函数名称:     DisplayABC_2 ()
//功    能:     用数码管显示字符串代码E1、E2、E3、E4
//入口参数:     String: 字符代码
//出口参数:     无
//显示内容:     String
//内容代码:	    调用格式  DisplayString("ABCD");
//-------------------------------------------------------
void DisplayABC_2(uint_least8_t * String)
{
	uint8_t   i = 0;
	uint8_t   digit[8] = {Digit0,Digit1,Digit2,Digit3,Digit4,Digit5,Digit6,Digit7};
	uint8_t   CharTemp;
	
	while( i < DISPLAY_LED_NUM)
	{
        SentByteTo7219(digit[i],0x00);                      //将没有用位的显示清除  
          
        i++;    
    }	
	
	i = 0;
	while(i < DISPLAY_LED_NUM && strlen((char *)String) - i > 0)
	{ 		
		i++;
		if(String[strlen((char *)String) - i] >= ' ')					//显示字母
		{
			CharTemp = String[strlen((char *)String)- i] - ' ';                       	//求显示字符位置,LedCharCode位置
			SentByteTo7219(digit[i-1],LedCharCode[CharTemp]);  	//从低位到高位显示
//			SentByteTo7219(digit[i],LedCharCode[CharTemp]);  	//从高位到低位显示       
		}   
		else													//显示数字
		{
			CharTemp = String[strlen((char *)String) - i] - '0';                      	//求显示字符位置,LedCharCode位置
			SentByteTo7219(digit[i-1],LedNumCode[CharTemp]);   	//从低位到高位显示 
//			SentByteTo7219(digit[i],LedNumCode[CharTemp]);   	//从高位到低位显示     			
		}        
	}
}


//------------------------------------------------------
//函数名称:MovDisplayChar ()
//功    能:用数码管显示字符即已定字符
//入口参数:character: 字符代码
//出口参数:无
//显示内容:   -   E   H   L   P   Black
//内容代码:	  A	  B	  C	  D	  E	  F	  
//-------------------------------------------------------
void MovDisplayChar (uint8_t character)
{
	uint8_t i = DISPLAY_LED_NUM ,j;
	uint8_t digit[8] = {Digit0,Digit1,Digit2,Digit3,Digit4,Digit5,Digit6,Digit7};
		
	for(j = 0;j < 12; j++)
	{
	 	 i = DISPLAY_LED_NUM;
		 while(i)
		 {
            i--;
            SentByteTo7219(digit[i],0x00);            	//将没有用位的显示清除
         }	
		 
		SentByteTo7219(digit[j%4],LedNumCode[character%LED_CHAR_NNM]);       //从低位到高位显示   
//		DelayX10ms(20);
		Delay(2000000);                
	}
}

void Dis_Test(void)
{
	uint8_t i = 0;
	for(i=0;i<10;i++)
	{
		DisplayNum_WithZero(i*11111111);
		//Delay(400000);
        BSP_OS_TimeDly(OS_TICKS_PER_SEC/2);
	}
}


