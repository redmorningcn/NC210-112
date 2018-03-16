/*******************************************************************************
 *   Filename:       bsp_ht1621b.c
 *   Revised:        All copyrights reserved to Roger-WY.
 *   Date:           2015-01-05
 *   Revision:       v1.0
 *   Writer:	     Roger-WY.
 *
 *   Description:    笔段式液晶控制器HT1621B驱动程序
 *
 *   Notes:
 *
 *
 *******************************************************************************/


#include "bsp_ht1621b.h"
#include "includes.h"



/***********************************************
 * 描述：函数申明
 */
static void HT1621B_GPIO_Config(void);
static void HT1621BWriteData(u8 Data,u8 cnt);
void HT1621B_Init(void);
void HT1621BWrCmd(u8 Cmd);
void HT1621BWrOneData(u8 Addr,u8 Data);
void HT1621BWrStrData(u8 Addr,u8 *p,u8 cnt);
void HT1821B_AllClean(void);

/*******************************************************************************
 * 名    称： HT1621B_GPIO_Config
 * 功    能： HT1621B所用的IO初始化
 * 入口参数： 无
 * 出口参数： 无
 * 作　　者：
 * 创建日期： 2015-01-05
 * 修    改：
 * 修改日期：
 * 备    注：
 *******************************************************************************/
static void HT1621B_GPIO_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(HT1621B_CS_RCC, ENABLE);
	GPIO_InitStructure.GPIO_Pin  = HT1621B_CS_PIN ;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(HT1621B_CS_PORT, &GPIO_InitStructure);

    RCC_APB2PeriphClockCmd(HT1621B_WR_RCC, ENABLE);
	GPIO_InitStructure.GPIO_Pin  = HT1621B_WR_PIN ;
	GPIO_Init(HT1621B_WR_PORT, &GPIO_InitStructure);

    RCC_APB2PeriphClockCmd(HT1621B_DATA_RCC, ENABLE);
	GPIO_InitStructure.GPIO_Pin  = HT1621B_DATA_PIN ;
	GPIO_Init(HT1621B_DATA_PORT, &GPIO_InitStructure);
}


/*******************************************************************************
 * 名    称： HT1621B_Init
 * 功    能： HT1621B初始化
 * 入口参数： 无
 * 出口参数： 无
 * 作　　者：
 * 创建日期： 2015-01-05
 * 修    改：
 * 修改日期：
 * 备    注：
 *******************************************************************************/
void HT1621B_Init(void)
{
    HT1621B_GPIO_Config();  //配置GPIO
    HT1621B_CS_HIGH();
    HT1621B_WR_HIGH();
    HT1621B_DATA_HIGH();

    // _delay_ms(2000);      /*此处需要延时2S使LCD工作电压稳定*/
    BSP_OS_TimeDly(2000);

    HT1621BWrCmd(BIAS);
    HT1621BWrCmd(RC256);    //使用内部振荡器
    HT1621BWrCmd(SYSDIS);   //关振系统荡振器和LCD偏压发生器
    HT1621BWrCmd(WDTDIS);   //禁止看门狗
    HT1621BWrCmd(SYSEN);    //打开系统振荡漾器
    HT1621BWrCmd(LCDON);    //打开LCD偏压
}

/*******************************************************************************
 * 名    称： HT1621BWriteData
 * 功    能： 向HT1621B中写指定位数的数据函数
 * 入口参数： Data为需要传送的数据
 *            cnt为传送数据位数,数据传送为低位在前
 * 出口参数： 无
 * 作　　者：
 * 创建日期： 2015-01-05
 * 修    改：
 * 修改日期：
 * 备    注：
 *******************************************************************************/
static void HT1621BWriteData(u8 Data,u8 cnt)
{
    for (u8 i=0;i<cnt;i++)
    {
        HT1621B_WR_LOW();
        _Nop();     /*延时10US*/
        if(Data&0x80)
        {
            HT1621B_DATA_HIGH();
        }else
        {
            HT1621B_DATA_LOW();
        }
        _Nop();     /*延时10US*/
        HT1621B_WR_HIGH();
        _Nop();     /*延时10US*/
        Data<<=1;
    }

}


/*******************************************************************************
 * 名    称： HT1621BWrCmd
 * 功    能： 向HT1621B中写指令
 * 入口参数： Cmd为需要发送的指令
 * 出口参数： 无
 * 作　　者：
 * 创建日期： 2015-01-05
 * 修    改：
 * 修改日期：
 * 备    注： 写入命令标识位100
 *******************************************************************************/
void HT1621BWrCmd(u8 Cmd)
{
    HT1621B_CS_LOW();
    _Nop();       /*延时10US*/
    HT1621BWriteData(0x80,4); //写入命令标志100
    HT1621BWriteData(Cmd,8);  //写入命令数据
    HT1621B_CS_HIGH();
    _Nop();      /*延时10US*/
}

/*******************************************************************************
 * 名    称： HT1621BWrOneData
 * 功    能： HT1621B在指定地址写入数据函数
 * 入口参数： Addr为写入初始地址，Data为写入数据
 * 出口参数： 无
 * 作　　者：
 * 创建日期： 2015-01-05
 * 修    改：
 * 修改日期：
 * 备    注： 因为HT1621B的数据位4位，所以实际写入数据为参数的后4位 、
 *                HT1621B的地址位6位，所以实际写入数据为参数的后6位 、
 *******************************************************************************/
void HT1621BWrOneData(u8 Addr,u8 Data)
{
    HT1621B_CS_LOW();
    HT1621BWriteData(0xa0,3);    //写入数据标志101
    HT1621BWriteData(Addr<<2,6); //写入地址数据
    HT1621BWriteData(Data<<4,4); //写入数据
    HT1621B_CS_HIGH();
    _Nop();                   /*延时10US*/
}


/*******************************************************************************
 * 名    称： HT1621BWrStrData
 * 功    能： HT1621B连续写入数据函数
 * 入口参数： Addr为写入初始地址，*p为连续写入数据指针，cnt为写入数据总数
 * 出口参数： 无
 * 作　　者：
 * 创建日期： 2015-01-05
 * 修    改：
 * 修改日期：
 * 备    注： HT1621的数据位4位，此处每次数据为8位，写入数据总数按8位计算
 *******************************************************************************/
void HT1621BWrStrData(u8 Addr,u8 *p,u8 cnt)
{
    HT1621B_CS_LOW();
    HT1621BWriteData(0xa0,3); //写入数据标志101
    HT1621BWriteData(Addr << 2,6); //写入地址数据
    for (u8 i=0;i<cnt;i++)
    {
        HT1621BWriteData(*p,8); //写入数据
        p++;
    }
    HT1621B_CS_HIGH();
    _Nop();               /*延时10US*/
}

/*******************************************************************************
 * 名    称： HT1821B_AllClean
 * 功    能： HT1621B清屏函数
 * 入口参数： 无
 * 出口参数： 无
 * 作　　者：
 * 创建日期： 2015-01-05
 * 修    改：
 * 修改日期：
 * 备    注：
 *******************************************************************************/
void HT1821B_AllClean(void)
{
    u8 i;
    for(i=0;i<32;i++)
    {
       HT1621BWrOneData(i,0x00);
    }
}

/*******************************************************************************
 * 名    称： HT1621_TEST
 * 功    能： HT1621B测试函数
 * 入口参数： 无
 * 出口参数： 无
 * 作　　者：
 * 创建日期： 2015-01-05
 * 修    改：
 * 修改日期：
 * 备    注：
 *******************************************************************************/
#if  HT1621B_TEST_EN > 0
void HT1621B_TEST(void)
{
    u8 i,j,t;
    HT1621B_Init(); //上电初始化LCD
    HT1821B_AllClean();
    BSP_OS_TimeDly(1000); //延时一段时间大概1S

    while(1)
 	{
        HT1821B_AllClean(); //清除1621寄存器数据，暨清屏
        for (i=0;i<32;i++)
        {
            t=0x01;
            for (j=0;j<4;j++)
            {
                HT1621BWrOneData(i,t);
                t<<=1;
                t++;
                BSP_OS_TimeDly(500);
            }
        }
    }
}
#endif
