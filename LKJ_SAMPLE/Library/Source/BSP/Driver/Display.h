#ifndef  _Display_h_
#define  _Display_h_

enum
{
DISPLAY_TIME = 0,
DISPLAY_BAT_VOL,
DISPLAY_SPEED,
DISPLAY_PHASE_DIFF,
DISPLAY_DUTY_RATIO,
DISPLAY_DTC,
DISPLAY_HISTORY_NUMBER,
DISPLAY_STEP_MAX,
};


/**-------------------------------------------------------
  * @函数名 LED_DIS_Config
  * @功能   初始化LED的端口
  * @参数   无
  * @返回值 无
***------------------------------------------------------*/
extern void LED_DIS_Config(void);

//-------------------------------------------------------------------
//函数名称:     Init7219 ()
//功    能:     7219初始化
//入口参数:     无
//出口参数:     无
//-------------------------------------------------------------------
extern void Init7219 (void);

//-----------------------------------------------------------------------------------
//函数名称:               DisplayNum ()
//功    能:               用数码管将整型数据显示出来
//入口参数:               number
//出口参数:               无
//------------------------------------------------------------------------------------
extern void    DisplayNum (uint32_t Unmber);

//-----------------------------------------------------------------------------------
//函数名称:               DisplayNum_WithZero ()
//功    能:               用数码管将整型数据显示出来,高位的0也显示出来
//入口参数:               number
//出口参数:               无
//------------------------------------------------------------------------------------
extern void    DisplayNum_WithZero (uint32_t Unmber);

//-----------------------------------------------------------------------------
//函数名称:             MovDisplayChar ()
//功    能:             用数码管显示字符即已定字符
//入口参数:             character: 字符代码
//出口参数:             无
//显示内容:             -   E   H   L   P   Black
//内容代码:     	    A	B	C	D	E	F	  
//-----------------------------------------------------------------------------
extern void    MovDisplayChar (uint8_t Character);

//--------------------------------------------------------------------
//void          DisplayNumWithDoit (uint32 Unmber)
//功    能:     用数码管将整型数据显示出来
//入口参数:     number
//出口参数:     无
//-------------------------------------------------------------------
extern void    DisplayNumWithDot (uint32_t Unmber);

//------------------------------------------------------
//函数名称:     DisplayString ()
//功    能:     用数码管显示字符串代码
//入口参数:     String: 字符代码
//出口参数:     无
//显示内容:     String
//内容代码:	    调用格式  DisplayString("ABCD");
//-------------------------------------------------------
extern void     DisplayString(char *String);
extern void     DisplayString_WithDot(char * String);

extern void     DisplayABC_2(uint_least8_t *String);

extern void     Dis_Test(void);

#endif


