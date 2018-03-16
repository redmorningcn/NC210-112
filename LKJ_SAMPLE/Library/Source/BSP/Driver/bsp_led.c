/*******************************************************************************
*   Filename:       bsp_led.c
*   Revised:        All copyrights reserved to Roger-WY.
*   Date:           2014-08-18
*   Revision:       v1.0
*   Writer:	        Roger-WY.
*
*   Description:    LED控制
*
*
*   Notes:
*
*******************************************************************************/

#include "bsp_led.h"
#include "includes.h"
#include "global.h"

#define BSP_LED_EN 1
#if BSP_LED_EN > 0

//============================================================================//

/*******************************************************************************
* 名    称： App_Led_Init
* 功    能： 用户LED初始化
* 入口参数： 无
* 出口参数： 无
* 作　　者： Roger-WY
* 创建日期： 2014-08-18
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
void App_Led_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(LED01_R_RCC, ENABLE);
    GPIO_InitStructure.GPIO_Pin   = LED01_R_PIN ;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(LED01_R_PORT, &GPIO_InitStructure);

    RCC_APB2PeriphClockCmd(LED01_G_RCC, ENABLE);
    GPIO_InitStructure.GPIO_Pin   = LED01_G_PIN ;
    GPIO_Init(LED01_G_PORT, &GPIO_InitStructure);

    RCC_APB2PeriphClockCmd(LED01_B_RCC, ENABLE);
    GPIO_InitStructure.GPIO_Pin   = LED01_B_PIN ;
    GPIO_Init(LED01_B_PORT, &GPIO_InitStructure);
    /***********************************************
     * 描述：LED02的R引脚复用了JTAG的下载引脚，调用下面两句，将JTAG引脚作为普通IO使用
     */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);
    //然后调用GPIO重映射函数，根据需求实现重映射：
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable,ENABLE);

    RCC_APB2PeriphClockCmd(LED02_R_RCC, ENABLE);
    GPIO_InitStructure.GPIO_Pin   = LED02_G_PIN ;
    GPIO_Init(LED02_G_PORT, &GPIO_InitStructure);

    RCC_APB2PeriphClockCmd(LED02_G_RCC, ENABLE);
    GPIO_InitStructure.GPIO_Pin   = LED02_B_PIN ;
    GPIO_Init(LED02_B_PORT, &GPIO_InitStructure);

    RCC_APB2PeriphClockCmd(LED02_B_RCC, ENABLE);
    GPIO_InitStructure.GPIO_Pin   = LED01_B_PIN ;
    GPIO_Init(LED02_B_PORT, &GPIO_InitStructure);

    /***********************************************
    * 描述： 关闭所有LED
    */
    GPIO_SetBits(LED01_R_PORT , LED01_R_PIN );
    GPIO_SetBits(LED01_G_PORT , LED01_G_PIN );
    GPIO_SetBits(LED01_B_PORT , LED01_B_PIN );
    GPIO_SetBits(LED02_R_PORT , LED01_R_PIN );
    GPIO_SetBits(LED02_G_PORT , LED01_G_PIN );
    GPIO_SetBits(LED02_B_PORT , LED01_B_PIN );
}

/*******************************************************************************
* 名    称： App_Led_On
* 功    能： 控制LED开
* 入口参数： Ledx  选择哪个LED  （取值范围：APP_ALLLED/APP_LED01/APP_LED02）
*            color 亮选择的LED的那个颜色（取值范围：Color_All/Color_Red/Color_Green/Color_Bule）
* 出口参数： 无
* 作　　者： Roger-WY
* 创建日期： 2014-08-18
* 修    改：
* 修改日期：
*******************************************************************************/
void App_Led_On(unsigned char Ledx,unsigned char color)
{
    if(Ledx == APP_ALLLED) {
       switch(color)
       {
            case Color_All  :
                GPIO_ResetBits(LED01_R_PORT , LED01_R_PIN );
                GPIO_ResetBits(LED01_G_PORT , LED01_G_PIN );
                GPIO_ResetBits(LED01_B_PORT , LED01_B_PIN );
                GPIO_ResetBits(LED02_R_PORT , LED02_R_PIN );
                GPIO_ResetBits(LED02_G_PORT , LED02_G_PIN );
                GPIO_ResetBits(LED02_B_PORT , LED02_B_PIN );
                break;
            case Color_Red  :
                GPIO_ResetBits(LED01_R_PORT , LED01_R_PIN );
                GPIO_ResetBits(LED02_R_PORT , LED02_R_PIN );
                break;
            case Color_Green:
                GPIO_ResetBits(LED01_G_PORT , LED01_G_PIN );
                GPIO_ResetBits(LED02_G_PORT , LED02_G_PIN );
                break;
            case Color_Bule :
                GPIO_ResetBits(LED01_B_PORT , LED01_B_PIN );
                GPIO_ResetBits(LED02_B_PORT , LED02_B_PIN );
                break;
            default:break;
       }
    } else if (Ledx == APP_LED01) {
        switch(color)
        {
            case Color_All  :
                GPIO_ResetBits(LED01_R_PORT , LED01_R_PIN );
                GPIO_ResetBits(LED01_G_PORT , LED01_G_PIN );
                GPIO_ResetBits(LED01_B_PORT , LED01_B_PIN );
                break;
            case Color_Red  :
                GPIO_ResetBits(LED01_R_PORT , LED01_R_PIN );
                break;
            case Color_Green:
                GPIO_ResetBits(LED01_G_PORT , LED01_G_PIN );
                break;
            case Color_Bule :
                GPIO_ResetBits(LED01_B_PORT , LED01_B_PIN );
                break;
            default:break;
        }
    } else if(Ledx == APP_LED02) {
       switch(color)
       {
            case Color_All  :
                GPIO_ResetBits(LED02_R_PORT , LED02_R_PIN );
                GPIO_ResetBits(LED02_G_PORT , LED02_G_PIN );
                GPIO_ResetBits(LED02_B_PORT , LED02_B_PIN );
                break;
            case Color_Red  :
                GPIO_ResetBits(LED02_R_PORT , LED02_R_PIN );
                break;
            case Color_Green:
                GPIO_ResetBits(LED02_G_PORT , LED02_G_PIN );
                break;
            case Color_Bule :
                GPIO_ResetBits(LED02_B_PORT , LED02_B_PIN );
                break;
            default:break;
       }
    } else {
        //不处理
    }
}

/*******************************************************************************
* 名    称： App_Led_Off
* 功    能： 控制LED开
* 入口参数： Ledx  选择哪个LED  （取值范围：APP_ALLLED/APP_LED01/APP_LED02）
*            color 亮选择的LED的那个颜色（取值范围：Color_All/Color_Red/Color_Green/Color_Bule）
* 出口参数： 无
* 作　　者： Roger-WY
* 创建日期： 2014-08-18
* 修    改：
* 修改日期：
*******************************************************************************/
void App_Led_Off(unsigned char Ledx,unsigned char color)
{
    if(Ledx == APP_ALLLED) {
       switch(color)
       {
            case Color_All  :
                GPIO_SetBits(LED01_R_PORT , LED01_R_PIN );
                GPIO_SetBits(LED01_G_PORT , LED01_G_PIN );
                GPIO_SetBits(LED01_B_PORT , LED01_B_PIN );
                GPIO_SetBits(LED02_R_PORT , LED02_R_PIN );
                GPIO_SetBits(LED02_G_PORT , LED02_G_PIN );
                GPIO_SetBits(LED02_B_PORT , LED02_B_PIN );
                break;
            case Color_Red  :
                GPIO_SetBits(LED01_R_PORT , LED01_R_PIN );
                GPIO_SetBits(LED02_R_PORT , LED02_R_PIN );
                break;
            case Color_Green:
                GPIO_SetBits(LED01_G_PORT , LED01_G_PIN );
                GPIO_SetBits(LED02_G_PORT , LED02_G_PIN );
                break;
            case Color_Bule :
                GPIO_SetBits(LED01_B_PORT , LED01_B_PIN );
                GPIO_SetBits(LED02_B_PORT , LED02_B_PIN );
                break;
            default:break;
       }
    } else if (Ledx == APP_LED01) {
        switch(color)
        {
            case Color_All  :
                GPIO_SetBits(LED01_R_PORT , LED01_R_PIN );
                GPIO_SetBits(LED01_G_PORT , LED01_G_PIN );
                GPIO_SetBits(LED01_B_PORT , LED01_B_PIN );
                break;
            case Color_Red  :
                GPIO_SetBits(LED01_R_PORT , LED01_R_PIN );
                break;
            case Color_Green:
                GPIO_SetBits(LED01_G_PORT , LED01_G_PIN );
                break;
            case Color_Bule :
                GPIO_SetBits(LED01_B_PORT , LED01_B_PIN );
                break;
            default:break;
        }
    } else if(Ledx == APP_LED02) {
       switch(color)
       {
            case Color_All  :
                GPIO_SetBits(LED02_R_PORT , LED02_R_PIN );
                GPIO_SetBits(LED02_G_PORT , LED02_G_PIN );
                GPIO_SetBits(LED02_B_PORT , LED02_B_PIN );
                break;
            case Color_Red  :
                GPIO_SetBits(LED02_R_PORT , LED02_R_PIN );
                break;
            case Color_Green:
                GPIO_SetBits(LED02_G_PORT , LED02_G_PIN );
                break;
            case Color_Bule :
                GPIO_SetBits(LED02_B_PORT , LED02_B_PIN );
                break;
            default:break;
       }
    } else {
        //不处理
    }
}

/*******************************************************************************
* 名    称： App_Led_Toggle
* 功    能： 对Led状态取反
* 入口参数： Ledx  选择哪个LED  （取值范围：APP_ALLLED/APP_LED01/APP_LED02）
*            color 亮选择的LED的那个颜色（取值范围：Color_All/Color_Red/Color_Green/Color_Bule）
* 出口参数： 无
* 作　　者： Roger-WY
* 创建日期： 2014-08-18
* 修    改：
* 修改日期：
*******************************************************************************/
void App_Led_Toggle(unsigned char Ledx,unsigned char color)
{
    if(Ledx == APP_ALLLED) {
       switch(color)
       {
            case Color_All  :
                GPIO_WriteBit(LED01_R_PORT, LED01_R_PIN,(BitAction)((1-GPIO_ReadOutputDataBit(LED01_R_PORT, LED01_R_PIN))));
                GPIO_WriteBit(LED01_G_PORT, LED01_G_PIN,(BitAction)((1-GPIO_ReadOutputDataBit(LED01_G_PORT, LED01_G_PIN))));
                GPIO_WriteBit(LED01_B_PORT, LED01_B_PIN,(BitAction)((1-GPIO_ReadOutputDataBit(LED01_B_PORT, LED01_B_PIN))));
                GPIO_WriteBit(LED02_R_PORT, LED02_R_PIN,(BitAction)((1-GPIO_ReadOutputDataBit(LED02_R_PORT, LED02_R_PIN))));
                GPIO_WriteBit(LED02_G_PORT, LED02_G_PIN,(BitAction)((1-GPIO_ReadOutputDataBit(LED02_G_PORT, LED02_G_PIN))));
                GPIO_WriteBit(LED02_B_PORT, LED02_B_PIN,(BitAction)((1-GPIO_ReadOutputDataBit(LED02_B_PORT, LED02_B_PIN))));
                break;
            case Color_Red  :
                GPIO_WriteBit(LED01_R_PORT, LED01_R_PIN,(BitAction)((1-GPIO_ReadOutputDataBit(LED01_R_PORT, LED01_R_PIN))));
                GPIO_WriteBit(LED02_R_PORT, LED02_R_PIN,(BitAction)((1-GPIO_ReadOutputDataBit(LED02_R_PORT, LED02_R_PIN))));
                break;
            case Color_Green:
                GPIO_WriteBit(LED01_G_PORT, LED01_G_PIN,(BitAction)((1-GPIO_ReadOutputDataBit(LED01_G_PORT, LED01_G_PIN))));
                GPIO_WriteBit(LED02_G_PORT, LED02_G_PIN,(BitAction)((1-GPIO_ReadOutputDataBit(LED02_G_PORT, LED02_G_PIN))));
                break;
            case Color_Bule :
                GPIO_WriteBit(LED01_B_PORT, LED01_B_PIN,(BitAction)((1-GPIO_ReadOutputDataBit(LED01_B_PORT, LED01_B_PIN))));
                GPIO_WriteBit(LED02_B_PORT, LED02_B_PIN,(BitAction)((1-GPIO_ReadOutputDataBit(LED02_B_PORT, LED02_B_PIN))));
                break;
            default:break;
       }
    } else if (Ledx == APP_LED01) {
        switch(color)
        {
            case Color_All  :
                GPIO_WriteBit(LED01_R_PORT, LED01_R_PIN,(BitAction)((1-GPIO_ReadOutputDataBit(LED01_R_PORT, LED01_R_PIN))));
                GPIO_WriteBit(LED01_G_PORT, LED01_G_PIN,(BitAction)((1-GPIO_ReadOutputDataBit(LED01_G_PORT, LED01_G_PIN))));
                GPIO_WriteBit(LED01_B_PORT, LED01_B_PIN,(BitAction)((1-GPIO_ReadOutputDataBit(LED01_B_PORT, LED01_B_PIN))));
                break;
            case Color_Red  :
                GPIO_WriteBit(LED01_R_PORT, LED01_R_PIN,(BitAction)((1-GPIO_ReadOutputDataBit(LED01_R_PORT, LED01_R_PIN))));
                break;
            case Color_Green:
                GPIO_WriteBit(LED01_G_PORT, LED01_G_PIN,(BitAction)((1-GPIO_ReadOutputDataBit(LED01_G_PORT, LED01_G_PIN))));
                break;
            case Color_Bule :
                GPIO_WriteBit(LED01_B_PORT, LED01_B_PIN,(BitAction)((1-GPIO_ReadOutputDataBit(LED01_B_PORT, LED01_B_PIN))));
                break;
            default:break;
        }
    } else if(Ledx == APP_LED02) {
       switch(color)
       {
            case Color_All  :
                GPIO_WriteBit(LED02_R_PORT, LED02_R_PIN,(BitAction)((1-GPIO_ReadOutputDataBit(LED02_R_PORT, LED02_R_PIN))));
                GPIO_WriteBit(LED02_G_PORT, LED02_G_PIN,(BitAction)((1-GPIO_ReadOutputDataBit(LED02_G_PORT, LED02_G_PIN))));
                GPIO_WriteBit(LED02_B_PORT, LED02_B_PIN,(BitAction)((1-GPIO_ReadOutputDataBit(LED02_B_PORT, LED02_B_PIN))));
                break;
            case Color_Red  :
                GPIO_WriteBit(LED02_R_PORT, LED02_R_PIN,(BitAction)((1-GPIO_ReadOutputDataBit(LED02_R_PORT, LED02_R_PIN))));
                break;
            case Color_Green:
                GPIO_WriteBit(LED02_G_PORT, LED02_G_PIN,(BitAction)((1-GPIO_ReadOutputDataBit(LED02_G_PORT, LED02_G_PIN))));
                break;
            case Color_Bule :
                GPIO_WriteBit(LED02_B_PORT, LED02_B_PIN,(BitAction)((1-GPIO_ReadOutputDataBit(LED02_B_PORT, LED02_B_PIN))));
                break;
            default:break;
       }
    } else {
        //不处理
    }
}

#endif


/*************************end of file*****************************************/
