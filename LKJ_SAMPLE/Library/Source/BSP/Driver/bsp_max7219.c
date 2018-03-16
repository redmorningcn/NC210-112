/*******************************************************************************
 *   Filename:      bsp_max7219.c
 *   Revised:       $Date: 2015-12-07$
 *   Revision:      $
 *	 Writer:		Wuming Shen.
 *
 *   Description:
 *   Notes:
 *					QQ:276193028
 *     				E-mail:shenchangwei945@163.com
 *
 *   All copyrights reserved to Wuming Shen.
 *
 *******************************************************************************/

/*******************************************************************************
* INCLUDES
*/
#include <includes.h>
#include <global.h>
#include <app_lib.h>
#include <string.h>
#include <stdlib.h> 

#include <bsp_max7219.h>

#define BSP_MAX7219_MODULE_EN 1
#if BSP_MAX7219_MODULE_EN > 0
/*******************************************************************************
 * CONSTANTS
 */
/***********************************************
* 描述： max7219寄存器地址定义
*/
#define MAX7279_NON_OPT         0x00            // 空操作寄存器
#define Digit0                  0x01            // 数码管1寄存器
#define Digit1                  0x02            // 数码管2寄存器
#define Digit2                  0x03            // 数码管3寄存器
#define Digit3                  0x04            // 数码管4寄存器
#define Digit4                  0x05            // 数码管5寄存器
#define Digit5                  0x06            // 数码管6寄存器
#define Digit6                  0x07            // 数码管7寄存器
#define Digit7                  0x08            // 数码管8寄存器
/**/
#define MAX7279_DECODE_MODE     0x09            // 译码模式寄存器
#define MAX7279_BRIGHTNESS      0x0a            // 亮度寄存器
#define MAX7279_SCAN            0x0b            // 扫描位数寄存器
#define MAX7279_LOW_PWR         0x0c            // 低功耗模式寄存器
#define MAX7279_DISP_TEST       0x0f            // 显示测试寄存器

/***********************************************
* 描述： max7219控制寄存器命令
*/
#define MAX7279_LOW_PWR_MODE    0x00            // 低功耗方式
#define MAX7279_NORMAL_MODE     0x01            // 正常操作方式
#define MAX7279_DECODE_SET      0x00            // 译码设置，8位均为BCD码;对8个数都编码
#define MAX7279_8_DIGIT_SCAN    0x07            // 扫描位数设置，显示8位数码管
#define MAX7279_4_DIGIT_SCAN    0x03            // 扫描位数设置，显示4位数码管
//#define MAX7279_BRIGHTNESS_LEVEL    0x00      // 亮度级别设置
#define MAX7279_BRIGHTNESS_LEVEL 0X0A           // 亮度级别设置
//#define MAX7279_BRIGHTNESS_LEVEL    0xf       // 亮度级别设置
#define MAX7279_TEST_ENTER      0x01            // 显示测试模式
#define MAX7279_TEST_EXIT       0x00            // 显示测试结束，恢复正常工作模式


/***********************************************
* 描述： 
*/

/***********************************************
* 描述： 
*/
StrLedDisp  LedDispCtrl[DISPLAY_FRAME_NUM]  = {0};
uint8       LedDispBuf[DISPLAY_LED_NUM]     = {Digit0,Digit1,Digit2,Digit3,Digit4,Digit5,Digit6,Digit7};
/***********************************************
* 描述： 
*/
const uint8 LED_SEG_CODE[]    = {
    //
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    //空格 !    "    #    $    %    &    '    (     )    *    +    ,    -    .   /
    0x00,0xE5,0xE5,0xE5,0xE5,0xE5,0xE5,0xE5,0xE5,0xE5,0xE5,0xE5,0xE5,0x01,0xE5,0xE5,
    // 0    1    2      3        4    5    6    7    8    9
    0x7E,0x30,0x6D,0x79,0x33,0x5B,0x5F,0x70,0x7F,0x7B,
    // :    ;    <    =     >   ?    @
    0xE5,0xE5,0xE5,0xE5,0xE5,0xE5,0xE5,
    //  A    B   C    D     E    F    G    H   I    J    K    L    M
    0x77,0x1F,0x4E,0x3d,0x4F,0x47,0x5F,0x37,0x30,0x3c,0x87,0x0e,0xE5,
    //  N    O    P   Q    R     S    T    U    V   W    X    Y    Z
    0x76,0x7E,0x67,0xFE,0x66,0x5B,0x0F,0x3E,0x3E,0xE5,0x37,0x33,0x6D,
    // [    \    ]    ^    _    `
    0xE5,0xE5,0xE5,0xE5,0xE5,0xE5,
    // a    b   c    d     e    f    g    h    i    j     k    l    m
    0x77,0x1F,0x0D,0x3D,0x4F,0x47,0x5F,0x17,0x30,0x3c,0x87,0x0e,0xE5,
    // n    o   p    q     r    s    t    u    v   w    x    y    z
    0x15,0x1D,0x67,0x73,0x05,0x5B,0x0F,0x1C,0x1C,0xD3,0x37,0x33,0x6D,
    // {    |    }    ~       灭
    0xE5,0xE5,0xE5,0xE5,0xE5,0x00
        
};

/*******************************************************************************
 * MACROS
 */
/***********************************************
* 描述： IO,时钟等定义
*/
#define SPI_CS_PIN            GPIO_Pin_12             /* PB.12 */
#define SPI_CS_PIN_NUM        12 
#define SPI_CS_GPIO_PORT      GPIOB
#define SPI_CS_GPIO_CLK       RCC_APB2Periph_GPIOB

#define SPI_SCK_PIN           GPIO_Pin_13             /* PB.13 */
#define SPI_SCK_PIN_NUM       13          
#define SPI_SCK_GPIO_PORT     GPIOB 
#define SPI_SCK_GPIO_CLK      RCC_APB2Periph_GPIOB

#define SPI_MOSI_PIN          GPIO_Pin_15             /* PB.15 */
#define SPI_MOSI_PIN_NUM      15 
#define SPI_MOSI_GPIO_PORT    GPIOB
#define SPI_MOSI_GPIO_CLK     RCC_APB2Periph_GPIOB

/***********************************************
* 描述： IO操作定义
*/
#define SPI_CS_LOW()     	  GPIO_ResetBits(SPI_CS_GPIO_PORT, SPI_CS_PIN)
#define SPI_CS_HIGH()    	  GPIO_SetBits(SPI_CS_GPIO_PORT, SPI_CS_PIN)

#define SPI_SCK_LOW()    	  GPIO_ResetBits(SPI_SCK_GPIO_PORT,SPI_SCK_PIN)
#define SPI_SCK_HIGH()   	  GPIO_SetBits(SPI_SCK_GPIO_PORT,SPI_SCK_PIN)

#define SPI_MOSI_LOW()        GPIO_ResetBits(SPI_MOSI_GPIO_PORT,SPI_MOSI_PIN)
#define SPI_MOSI_HIGH()       GPIO_SetBits(SPI_MOSI_GPIO_PORT,SPI_MOSI_PIN)

/*******************************************************************************
 * TYPEDEFS
 */

/*******************************************************************************
 * LOCAL VARIABLES
 */

/*******************************************************************************
 * GLOBAL VARIABLES
 */

/*******************************************************************************
 * LOCAL FUNCTIONS
 */

/*******************************************************************************
 * GLOBAL FUNCTIONS
 */

/*******************************************************************************
 * EXTERN VARIABLES
 */

/*******************************************************************************
 * EXTERN FUNCTIONS
 */

/*******************************************************************************
* 名    称： SPI_GpioInit
* 功    能： ADC GPIO初始化
* 入口参数： 无
* 出口参数： 无
* 作　 　者： wumingshen.
* 创建日期： 2015-12-07
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
static void SPI_GpioInit(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(SPI_CS_GPIO_CLK | SPI_MOSI_GPIO_CLK |
                           SPI_SCK_GPIO_CLK, ENABLE);

    /*!< Configure SPI pin: Chip CS pin */
    GPIO_InitStructure.GPIO_Pin     = SPI_CS_PIN;
    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_Out_PP;
    GPIO_Init(SPI_CS_GPIO_PORT, &GPIO_InitStructure);
    
    /*!< Configure SPI pins: SCK */
    GPIO_InitStructure.GPIO_Pin     = SPI_SCK_PIN;
    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_Out_PP;
    GPIO_Init(SPI_SCK_GPIO_PORT, &GPIO_InitStructure);

    /*!< Configure SPI pins: MOSI */
    GPIO_InitStructure.GPIO_Pin     = SPI_MOSI_PIN;
    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_Out_PP;
    GPIO_Init(SPI_MOSI_GPIO_PORT, &GPIO_InitStructure);

    SPI_CS_HIGH();
}

/*******************************************************************************
* 名    称： SPI_SendByte
* 功    能： 通过SPI总线发送一个字节数据(同时接收一个字节数据)
* 入口参数： 要写入的一个字节数据
* 出口参数： 在发数据时，MISO信号线上接收的一个字节
* 作　 　者： wumingshen.
* 创建日期： 2015-12-07
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
static void SPI_SendByte(uint8_t byte)
{
    uint8_t i;
    
    /***********************************************
    * 描述： 
    */
    for(i=0;i<8;i++) {		
		SPI_SCK_LOW();
		Delay_Nus(10);
		if(byte & 0x80)
			SPI_MOSI_HIGH();
		else
			SPI_MOSI_LOW();
		byte = byte << 1;
		Delay_Nus(5);
		SPI_SCK_HIGH();
		Delay_Nus(10);
	}
}	

/*******************************************************************************
* 名    称： BSP_MAX7219Init
* 功    能： MAX7219初始化
* 入口参数： 无
* 出口参数： 无
* 作　 　者： wumingshen.
* 创建日期： 2015-12-07
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
void BSP_MAX7219Init( void )
{
	SPI_GpioInit(); 
    
	SPI_CS_HIGH();
	SPI_SCK_HIGH();
    
	Delay_Nus(30);
}

/*******************************************************************************
* 名    称： SPI_SendData
* 功    能： 向7219发送1Byte数据或命令
* 入口参数：-addr:   数据接收地址
*           -num:   要存的数据
* 出口参数： 
* 作　 　者： wumingshen.
* 创建日期： 2015-12-08
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
void SPI_SendData(uint8 Addr,uint8 Num)
{    
    SPI_CS_LOW();                                  //7219允许接接收
    SPI_SendByte( Addr );                          //送地址
    SPI_SendByte( Num );                           //送数据
    SPI_CS_HIGH();                                 //关7219接收
}

/*******************************************************************************
* 名    称： BSP_DispWrite
* 功    能： 将显示数据写入显示缓存
* 入口参数： - Num         显示数据
*           - Fmt         显示数据格式。如："e%",显示字母‘e’和数据Num
*           - Align       对齐方式 ‘1’为左对齐，’0‘为右对齐
*           - FmtType     格式化模式。’1‘前面显示，’0‘后面显示
*           - DotPosition 显示点的位置
*           - Cycle       闪烁次数
*           - Duty    亮的时间
*           - OffX100ms   灭的时间
*           - Index       显示序号
* 出口参数： 
* 作　 　者： wumingshen.
* 创建日期： 2015-12-08
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
void BSP_DispWrite( int     Num,
                    char    *Fmt,
                    char    Align,
                    uint8   FmtType,
                    uint8   DotPosition,
                    uint8   Cycle,
                    uint8   Duty,
                    // uint8   OffX100ms,
                    uint8   Index)
{

    char    i               = 0,    Nflag   = 0,    b       = 0;
    char    NbrCnt          = 0,    Chcnt   = 0;
    char    NumPosition     = 0;
    int     temp            = 0;
    char    flag            = 0;
    uint8   BSP_DispBuff[8] = {0};
    uint8   CharBuffer[8]   = {0};
    uint8   NumbBuffer[8]   = {0};
    uint8   buf[DISPLAY_LED_NUM]          = {128,128,128,128,128,128,128,128};
            temp            = Num;

    /***********************************************
    * 描述： 
    */
    if(Index > sizeof(LedDispCtrl)/sizeof(StrLedDisp)-1) {
        return;
    }
    
    LedDispCtrl[Index].Flag         = 1;
    LedDispCtrl[Index].Cycle        = Cycle;
    LedDispCtrl[Index].Duty         = Duty;
    // LedDispCtrl[Index].OffX100ms     = OffX100ms;

    /***********************************************
    * 描述： 
    */
    for(i = 0;i < strlen(Fmt);i++) {
        if(Fmt[i] == '%') {
            if ( FmtType == 0 ) {
                if(temp < 0) {
                    Nflag = 1;
                    temp = abs(temp);
                    NumbBuffer[0] = '-';
                    NbrCnt++;
                }
                
                do {
                    NumbBuffer[NbrCnt] = temp%10 + '0';
                    temp /= 10;
                    NbrCnt++;
                } while((NbrCnt <= 8)&&temp);
            } else {
                do {
                    NumbBuffer[NbrCnt] = temp%10 + '0';
                    temp /= 10;
                    NbrCnt++;
                } while((NbrCnt <= 8)&&temp);
                
                if(temp < 0) {
                    Nflag = 1;
                    temp = abs(temp);
                    NumbBuffer[NbrCnt] = '-';
                    NbrCnt++;
                }
            
            }
            
            switch(i) {
            case 0: NumPosition = 0;break;
            case 1: NumPosition = 1;break;
            case 2: NumPosition = 2;break;
            case 3: NumPosition = 3;break;
            }
            
            if(Nflag == 0) {
                flag = 1;
            } else {
                flag = 0;
            }
            
            for(b = 0;b < (NbrCnt - Nflag)/2;b++) {
                temp = NumbBuffer[b + Nflag];
                NumbBuffer[b + Nflag] = NumbBuffer[NbrCnt -flag- Nflag - b];
                NumbBuffer[NbrCnt - flag - Nflag - b] = temp;
            }
        } else {
            CharBuffer[Chcnt] = Fmt[i];
            Chcnt++;
        }
    }
    /***********************************************
    * 描述： 
    */
    switch(NumPosition) {
    case 0: 
        for(i = 0;i < 8;i++) {
            BSP_DispBuff[i] = NumbBuffer[i];
        }
        for(i = NbrCnt;i < 8;i++) {
            BSP_DispBuff[i] = CharBuffer[i - NbrCnt];
        }
        break;
    case 1: 
        BSP_DispBuff[0] = CharBuffer[0];
        for(i = 1;i < 8;i++) {
            BSP_DispBuff[i] = NumbBuffer[i - 1];
        }
        for(i = (NbrCnt + 1);i < 8;i++) {
            BSP_DispBuff[i] = CharBuffer[i - NbrCnt];
        }
    break;
    case 2: 
        BSP_DispBuff[0] = CharBuffer[0];
        BSP_DispBuff[1] = CharBuffer[1];
        for(i = 2;i < 8;i++) {
            BSP_DispBuff[i] = NumbBuffer[i - 2];
        }
        for(i = (NbrCnt + 2);i < 8;i++) {
            BSP_DispBuff[i] = CharBuffer[i - NbrCnt];
        }
    break;
    case 3: 
        for(i = 0;i < 3;i++) {
            BSP_DispBuff[i] = CharBuffer[i];
        }
        for(i = 3;i < 8;i++) {
            BSP_DispBuff[i] = NumbBuffer[i - 3];
        }
    }
    
    for(i = 0;i < DISPLAY_LED_NUM;i++) {
        buf[i] = 128;
    }
    
    /***********************************************
    * 描述： 
    */
	if(Align == 1) {
        for(i = 0;(i < (NbrCnt + Chcnt))&&(i < DISPLAY_LED_NUM);i++) {
            buf[i]      = BSP_DispBuff[i];
        }
    } else {
        for(i = 0;(i < (NbrCnt + Chcnt))&&(i < DISPLAY_LED_NUM);i++) {
            buf[3 - i]  = BSP_DispBuff[NbrCnt + Chcnt -1 -i];
        }
    }
    
    /***********************************************
    * 描述： 
    */
    for(i = 0;i < DISPLAY_LED_NUM;i++) {
        if(DotPosition & (0x01 << (3 - i))) {
            LedDispCtrl[Index].Code[i] = LED_SEG_CODE[buf[i]]|0x80;
        } else {
            LedDispCtrl[Index].Code[i] = LED_SEG_CODE[buf[i]];
        }
    }
}

/*******************************************************************************
* 名    称： BSP_Disp
* 功    能： 根据序号，给7219送相应的数据
* 入口参数： 
* 出口参数： 
* 作　 　者： wumingshen.
* 创建日期： 2015-12-08
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
void WIN_DispMsg(const char *msg)
{
    char i,j;
    LedDispCtrl[0].Flag         = 1;
    LedDispCtrl[0].Cycle        = 1;
    LedDispCtrl[0].Duty         = 20;
    /***********************************************
    * 描述： 
    */
    for(i = 0,j = 0; j < DISPLAY_LED_NUM; j++,i++) {
        if(msg[i] == '.') 
	{
		i++;
		if( j>0 )
		{
			LedDispCtrl[0].Code[j-1] |=  0x80;
		}
        }
//		else 
        {
            LedDispCtrl[0].Code[j] = LED_SEG_CODE[msg[i]];
        }
/*
        if(msg[i] == '.') {
            i++;
            LedDispCtrl[0].Code[j] = LED_SEG_CODE[msg[i]] | 0x80;
        } else {
            LedDispCtrl[0].Code[j] = LED_SEG_CODE[msg[i]];
        }
*/
    }
    
    //BSP_DispSetBrightness(Ctrl.Para.dat.DispLevel);
    BSP_DispEvtProcess();
}

/*******************************************************************************
* 名    称： BSP_Disp
* 功    能： 根据序号，给7219送相应的数据
* 入口参数： 
* 出口参数： 
* 作　 　者： wumingshen.
* 创建日期： 2015-12-08
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
void BSP_Disp(uint8 Index)
{
    uint8 i;
    if(Index > sizeof(LedDispCtrl)/sizeof(StrLedDisp)-1) {
        return;
    }
    //BSP_DispInit();
    for(i = 0; i<DISPLAY_LED_NUM ; i++) {
        SPI_SendData(LedDispBuf[i],LedDispCtrl[Index].Code[i]);
    }
}

/*******************************************************************************
* 名    称： BSP_DispOff
* 功    能： 通过给数码管送消隐段码来清除显示
* 入口参数： 
* 出口参数： 
* 作　 　者： wumingshen.
* 创建日期： 2015-12-08
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
void BSP_DispOff(void)
{
    uint8 i;
    BSP_DispInit();
    for(i = 0; i<DISPLAY_LED_NUM ; i++) {
        SPI_SendData(LedDispBuf[i],0);
    }
}

/*******************************************************************************
* 名    称： BSP_DispEvtProcess
* 功    能： 用数码管将整型数据显示出来
* 入口参数： 
* 出口参数： 
* 作　 　者： wumingshen.
* 创建日期： 2015-12-08
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
void BSP_DispEvtProcess()
{    
    if(LedDispCtrl[0].Flag != 0 ) {
		BSP_Disp(0);
    }
}

/*******************************************************************************
* 名    称： BSP_DispClr
* 功    能： 清除显示缓冲区,包括显示标志位和显示内容
* 入口参数： 第几个显示缓冲区
* 出口参数： 
* 作　 　者： wumingshen.
* 创建日期： 2015-12-08
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
void BSP_DispClr(uint8 order)
{
    if( order < sizeof(LedDispCtrl) / sizeof(StrLedDisp) ) {
        LedDispCtrl[order].Flag = 0;
    }
}

/*******************************************************************************
* 名    称： BSP_DispClrAll
* 功    能： 清除所有显示缓冲区内容
* 入口参数： 
* 出口参数： 
* 作　 　者： wumingshen.
* 创建日期： 2015-12-08
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
void BSP_DispClrAll(void)
{
    uint8 i;
    
    for(i=0;i<sizeof(LedDispCtrl)/sizeof(StrLedDisp);i++) {
        LedDispCtrl[i].Flag = 0;
    }
}

/*******************************************************************************
* 名    称： BSP_DispSetBrightness
* 功    能： 7219设置亮度
* 入口参数： 
* 出口参数： 
* 作　 　者： wumingshen.
* 创建日期： 2015-12-12
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
void BSP_DispSetBrightness ( u8 bl )
{    
    static u8   last;
    
    if ( bl > 15 )
        bl  = 15;
    if ( last != bl ) {
        last    = bl;
        SPI_SendData( MAX7279_BRIGHTNESS,bl );      // 设置亮度
    }
}

/*******************************************************************************
* 名    称： BSP_DispInit
* 功    能： 7219初始化
* 入口参数： 
* 出口参数： 
* 作　 　者： wumingshen.
* 创建日期： 2015-12-08
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
void BSP_DispInit (void)
{
    BSP_MAX7219Init();                                              // 初始化硬件
    
    SPI_SendData( MAX7279_DISP_TEST,0x00 );                         // 设置工作模式
    SPI_SendData( MAX7279_SCAN,MAX7279_8_DIGIT_SCAN );              // 设置扫描界限
    SPI_SendData( MAX7279_DECODE_MODE,MAX7279_DECODE_SET );         // 设置译码模式
    SPI_SendData( MAX7279_BRIGHTNESS,MAX7279_BRIGHTNESS_LEVEL );    // 设置亮度
    SPI_SendData( MAX7279_LOW_PWR,MAX7279_NORMAL_MODE );            // 设置为正常工作模式
    
    BSP_DispClrAll();                                               // 清屏
}

/*******************************************************************************
* 				                end of file                                    *
*******************************************************************************/
#endif
