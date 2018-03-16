/*******************************************************************************
 *   Filename:       bsp_sht11.C
 *   Revised:        All copyrights reserved to Roger-WY.
 *   Revision:       v1.0
 *   Writer:	     Roger-WY.
 *
 *   Description:    STH11温湿度传感器驱动模块
 *                   该驱动同样适用 与SHT10/SHT20。
 *   Notes:
 *
 *     				 E-mail:261313062@qq.com
 *
 *******************************************************************************/

#include "bsp_sht11.h"
#include "includes.h"

/******************************************************************************/
GPIO_InitTypeDef    GPIO_InitStructure;



/************************* Sensirion Provide **********************************/

void  s_transstart(void);
void  s_connectionreset(void);
char  s_write_byte(unsigned char value);
char  s_read_byte(unsigned char ack);
char  s_softreset(void);
char  s_read_statusreg(unsigned char *p_value, unsigned char *p_checksum);
char  s_write_statusreg(unsigned char *p_value);
char  s_measure(unsigned char *p_value, unsigned char *p_checksum, unsigned char mode);
void  calc_sth11(float *p_humidity ,float *p_temperature);
float calc_dewpoint(float h,float t);


/*******************************************************************************
 * 名    称： SHT_Delay
 * 功    能： SHT11操作间延时 t*10(us)
 * 入口参数： 无
 * 出口参数： 无
 * 作　　者： Roger-WY.
 * 创建日期： 2015-06-25
 * 修    改：
 * 修改日期：
 * 备    注： 该函数只能在本文件中调用
 *******************************************************************************/
static void SHT_Delay(unsigned char t)
{
    unsigned int tmp;
    while(t--)
    {
        tmp = 90;
        while(tmp--);
    }
}

/*******************************************************************************
 * 名    称： Bsp_ShtInit
 * 功    能： SHT11初始化操作(包括IO引脚初始化和复位操作)
 * 入口参数： 无
 * 出口参数： 无
 * 作　　者： Roger-WY.
 * 创建日期： 2015-06-25
 * 修    改：
 * 修改日期：
 * 备    注：
 *******************************************************************************/
void Bsp_ShtInit(void)
{
    /***********************************************
    * 描述： SHTxx使用的IO初始化
    */
    SHT_INIT();
    /***********************************************
    * 描述： 复位连接SHTxx
    */
    s_connectionreset();
}

/*******************************************************************************
 * 名    称： s_write_byte
 * 功    能： 初始化滴答定时器，清空所有软定时器
 * 入口参数： 向SHT11写1个字节并检测SHT11是否应答
 * 出口参数： error=1表明SHT11未应答
 * 作　　者： Roger-WY.
 * 创建日期： 2015-06-25
 * 修    改：
 * 修改日期：
 * 备    注：
 *******************************************************************************/
char s_write_byte(unsigned char value)
{
    unsigned char i,error=0;

    SHT_DATA_OUPUT(); // DATA设置为输出

    /* 从高到低逐位发送 */
    for (i=0x80;i>0;i/=2) {                                   // 移位掩码
        if(i & value) {
            SHT_DATA_H();
        } else {
            SHT_DATA_L();
        }
        SHT_CLK_H();
        SHT_Delay(5);
        SHT_CLK_L();
        SHT_Delay(5);
    }
    SHT_DATA_H();                                             // 释放DATA线
    SHT_CLK_H();                                              // 第9个SCK
    SHT_Delay(5);
    SHT_DATA_INPUT();
    error = GPIO_ReadInputDataBit(SHT_PORT,GPIO_Pin_SHT_DAT); // 检查应答 (SHT11将拉底DATA作为应答)
    SHT_Delay(5);
    SHT_CLK_L();
    return error;                                             // error=1表明SHT11未应答
}

/*******************************************************************************
 * 名    称： s_read_byte
 * 功    能： 从SHT11读1个字节并当输入参数ack=1时给出应答
 * 入口参数： ack  应答标志
 * 出口参数： error=1表明SHT11未应答
 * 作　　者： Roger-WY.
 * 创建日期： 2015-06-25
 * 修    改：
 * 修改日期：
 * 备    注：
 *******************************************************************************/
char s_read_byte(unsigned char ack)
{
    unsigned char i,val=0;

    SHT_DATA_OUPUT();                                       // DATA设置为输出
    SHT_DATA_H();                                           // 释放DATA线
    SHT_DATA_INPUT();

    /* 从高到低逐位读取 */
    for (i=0x80;i>0;i/=2) {                                 // 移位掩码
       SHT_CLK_H();
        if (GPIO_ReadInputDataBit(SHT_PORT,GPIO_Pin_SHT_DAT)) {
            val=(val | i);
        }
        SHT_CLK_L();
    }
    SHT_DATA_OUPUT();         // DATA设置为输出
    if(ack) {
        SHT_DATA_L();           // 当ack=1(即需要使用应答)时拉底DATA线
    } else {
        SHT_DATA_H();
    }
    SHT_CLK_H();                                            // 第9个SCK
    SHT_Delay(5);
    SHT_CLK_L();
    SHT_DATA_H();                                           // 释放DATA线
    return val;
}

/*******************************************************************************
 * 名    称： s_transstart
 * 功    能： 发送一个"启动传输"序列
 *                 _____         ________
 *           DATA:      |_______|
 *                     ___     ___
 *           SCK : ___|   |___|   |______
 *
 * 入口参数： 无
 * 出口参数： 无
 * 作　　者： Roger-WY.
 * 创建日期： 2015-06-25
 * 修    改：
 * 修改日期：
 * 备    注：
 *******************************************************************************/
void s_transstart(void)
{
    SHT_DATA_OUPUT();   // DATA设置为输出

    SHT_DATA_H();
    SHT_CLK_L();   // 初始状态

    SHT_Delay(10);
    SHT_CLK_H();

    SHT_Delay(10);
    SHT_DATA_L();

    SHT_Delay(10);
    SHT_CLK_L();

    SHT_Delay(30);
    SHT_CLK_H();

    SHT_Delay(10);
    SHT_DATA_H();

    SHT_Delay(10);
    SHT_CLK_L();

    SHT_Delay(10);
    SHT_DATA_L();

    SHT_Delay(10);
    SHT_Delay(10);
}

/*******************************************************************************
 * 名    称： s_connectionreset
 * 功    能： 通信复位
 *                 _____________________________________________________         ________
 *           DATA:                                                      |_______|
 *                    _    _    _    _    _    _    _    _    _        ___     ___
 *           SCK : __| |__| |__| |__| |__| |__| |__| |__| |__| |______|   |___|   |______
 *
 * 入口参数： 无
 * 出口参数： 无
 * 作　　者： Roger-WY.
 * 创建日期： 2015-06-25
 * 修    改：
 * 修改日期：
 * 备    注：
 *******************************************************************************/
void s_connectionreset(void)
{
    unsigned char i;

    SHT_DATA_OUPUT(); // DATA设置为输出
    SHT_DATA_H();
    SHT_CLK_L();   // 初始状态
    /* 9个SCK 周期*/
    for(i=0;i<9;i++)
    {
        SHT_CLK_H();
        SHT_Delay(10);
        SHT_CLK_L();
        SHT_Delay(10);
    }
}

/*******************************************************************************
 * 名    称： s_softreset
 * 功    能： 软件复位
 * 入口参数： 无
 * 出口参数： 返回值为1表示SHT11未响应
 * 作　　者： Roger-WY.
 * 创建日期： 2015-06-25
 * 修    改：
 * 修改日期：
 * 备    注：
 *******************************************************************************/
char s_softreset(void)
{
    unsigned char error=0;

    s_connectionreset();                                // 通信复位
    error+=s_write_byte(SHTCMD_RESET);                         // 发送"复位"命令给SHT11
    return error;                                       // error=1表示SHT11未响应
}

/*******************************************************************************
 * 名    称： s_read_statusreg
 * 功    能： 读取状态寄存器和校验和
 * 入口参数： p_value      状态寄存器的值
 *            p_checksum   校验和
 * 出口参数： 返回值为1表示SHT11未响应
 * 作　　者： Roger-WY.
 * 创建日期： 2015-06-25
 * 修    改：
 * 修改日期：
 * 备    注：
 *******************************************************************************/
char s_read_statusreg(unsigned char *p_value, unsigned char *p_checksum)
{
    unsigned char error=0;

    s_transstart();                                     // 发送一个"启动传输"序列
    error=s_write_byte(SHTCMD_READ_STATUES);                   // 发送"读状态寄存器"命令
    *p_value=s_read_byte(ACK);                          // 读状态寄存器
    *p_checksum=s_read_byte(noACK);                     // 读校验和

    return error;                                       // error=1表示SHT11未响应
}

/*******************************************************************************
 * 名    称： s_write_statusreg
 * 功    能： 写状态寄存器
 * 入口参数： p_value      状态寄存器的值
 * 出口参数： 返回值为1表示SHT11未响应
 * 作　　者： Roger-WY.
 * 创建日期： 2015-06-25
 * 修    改：
 * 修改日期：
 * 备    注：
 *******************************************************************************/
char s_write_statusreg(unsigned char *p_value)
{
    unsigned char error=0;

    s_transstart();                                     // 发送一个"启动传输"序列
    error += s_write_byte(SHTCMD_WRITE_STATUS);         // 发送"写状态寄存器"命令
    error += s_write_byte(*p_value);                    // 写状态寄存器

    return error;                                       // error=1表示SHT11未响应
}

/*******************************************************************************
 * 名    称： s_measure
 * 功    能： 进行一次测量(相对湿度或温度)
 * 入口参数： p_value      测量值
 *            checksum     校验和
 *            mode         TEMP表示进行温度测量
 *                         HUMI表示进行相对湿度测量
 * 出口参数： 返回值为1表示SHT11未响应
 * 作　　者： Roger-WY.
 * 创建日期： 2015-06-25
 * 修    改：
 * 修改日期：
 * 备    注：
 *******************************************************************************/
char s_measure(unsigned char *p_value, unsigned char *p_checksum, unsigned char mode)
{
    OS_ERR err;
    uint16_t error = 0;
    uint16_t i;

    s_transstart();                         // 发送一个"启动传输"序列
    switch(mode)                            // 根据输入参数mode进行一次相应的测量
    {
        case TEMP: error += s_write_byte(SHTCMD_MEASURE_TEMP); break;
        case HUMI: error += s_write_byte(SHTCMD_MEASURE_HUMI); break;
        default  : break;
    }
    /***********************************************
    * 描述： 如果有错误，直接返回
    */
    if(error != 0) {
        return error;
    }

//    for (i=0;i<200;i++)  {
//        //此处应该要加延时
//        __NOP();__NOP();__NOP();__NOP();__NOP();
//        if(!GPIO_ReadInputDataBit(SHT_PORT,GPIO_Pin_SHT_DAT)) // 等待SHT11完成测量
//            break;
//    }
//    if(GPIO_ReadInputDataBit(SHT_PORT,GPIO_Pin_SHT_DAT)) {
//        error += 1;
//    }                                      // 测量错误

    for(i = 0; i < 30; i ++) {
       OSTimeDly(20,OS_OPT_TIME_DLY,&err);
       if(!GPIO_ReadInputDataBit(SHT_PORT,GPIO_Pin_SHT_DAT)) // 等待SHT11完成测量
           break;
    }
    if(GPIO_ReadInputDataBit(SHT_PORT,GPIO_Pin_SHT_DAT)) {
        error+=1;
    }
    /*
    * Note:在此数据读取两个字节，数据存储需考虑大小端格式存放问题
    */
    *(p_value+1) = s_read_byte(ACK);                         // 读第1个字节 (MSB)
    *(p_value)   = s_read_byte(ACK);                         // 读第2个字节 (LSB)
    *p_checksum  = s_read_byte(noACK);                       // 读校验和

    return error;
}

/*******************************************************************************
 * 名    称： calc_sth11
 * 功    能： 初始化滴答定时器，清空所有软定时器
 * 入口参数： p_humidity      SHT11采集到的相对湿度值(经过本函数的转换，该参数返回实际物理量的值)
 *            p_temperature   SHT11采集到的温度值(经过本函数的转换，该参数返回实际物理量的值)
 * 出口参数： 无
 * 作　　者： Roger-WY.
 * 创建日期： 2015-06-25
 * 修    改：
 * 修改日期：
 * 备    注：
 *******************************************************************************/
void calc_sth11(float *p_humidity ,float *p_temperature)
{
    const float C1=-4.0;                                     // 12位
    const float C2=+0.0405;                                  // 12 Bit
    const float C3=-0.0000028;                               // 12 Bit
    const float T1=+0.01;                                    // 14位 5V
    const float T2=+0.00008;                                 // 14位 5V

    float rh=*p_humidity;                                  // 相对湿度采集值 12位
    float t=*p_temperature;                                // 温度采集值 14位
    float rh_lin;                                          // 相对湿度的非线性补偿
    float rh_true;                                         // 相对湿度物理量值
    float t_C;                                             // 温度物理量值

    t_C=t*0.01 - 39.60;                                    // 计算温度物理量值
    rh_lin=C3*rh*rh + C2*rh + C1;                      // 计算相对湿度的非线性补偿
    rh_true=(t_C-25)*(T1+T2*rh)+rh_lin;                    // 计算相对湿度物理量值

    /* 若计算出来的相对湿度物理量值超范围则截断 */
    if(rh_true>100) rh_true=100;
    if(rh_true<0.1) rh_true=0.1;

    *p_temperature = t_C;                                  // 返回温度物理量值
    *p_humidity = rh_true;                                 // 返回相对湿度物理量值
}

/*******************************************************************************
 * 名    称： calc_dewpoint
 * 功    能： 计算露点
 * 入口参数： h      相对湿度物理量值   t      温度物理量值
 * 出口参数： 露点值
 * 作　　者： Roger-WY.
 * 创建日期： 2015-06-25
 * 修    改：
 * 修改日期：
 * 备    注：
 *******************************************************************************/
float calc_dewpoint(float h,float t)
{
    float logEx,dew_point;

    logEx=0.66077+7.5*t/(237.3+t)+(log10(h)-2);
    dew_point = (logEx - 0.66077)*237.3/(0.66077+7.5-logEx);
    return dew_point;
}

/*******************************************************************************
 * 名    称： Bsp_ShtMeasureOnce
 * 功    能： SHT11启动一次测量
 * 入口参数： SHT_DATA_TYPE *t_shtval  SHT测量的温湿度值
 * 出口参数： 1：测量成功；0测量失败
 * 作　　者： Roger-WY.
 * 创建日期： 2015-06-25
 * 修    改：
 * 修改日期：
 * 备    注：
 *******************************************************************************/
u8 Bsp_ShtMeasureOnce(SHT_DATA_TYPE *t_shtval)
{
    float dew_point;  // 结露点 (浮点)
    value humi_val,temp_val;
    unsigned char error,checksum;
    unsigned char j,k;

    int retrytime = 3;
    /***********************************************
    * 描述： 测量有误时，重试的次数
    */
    while(retrytime--) {
        humi_val.i = 0;
        temp_val.i = 0;
        error=0;
        error += s_measure((unsigned char*) &humi_val.i,&checksum,HUMI);  //measure humidity
        error += s_measure((unsigned char*) &temp_val.i,&checksum,TEMP);  //measure temperature
        s_read_statusreg(&j,&k);
        /***********************************************
        * 描述： 测量有误，重新连接测量
        */
        if(error!=0) {
           s_connectionreset();                   //in case of an error: connection SHTCMD_RESET
        }  else {
            humi_val.f=(float)humi_val.i;                   //converts integer to float
            temp_val.f=(float)temp_val.i;                   //converts integer to float
            calc_sth11(&humi_val.f,&temp_val.f);            //calculate humidity, temperature
            dew_point =calc_dewpoint(humi_val.f,temp_val.f); //calculate dew point

            //以下数值为真实值
            t_shtval->fDewpoint   = dew_point;
            t_shtval->fTemp       = temp_val.f;
            t_shtval->fHumi       = humi_val.f;
            //以下数值均为实际数值放大10倍后的数值
            t_shtval->DewPoint    = ((int)(dew_point*10));
            t_shtval->Temperature = ((int)(temp_val.f*10));
            t_shtval->Humidity    = ((int)(humi_val.f*10));
            return 1;
        }
    }
    return 0;
}





