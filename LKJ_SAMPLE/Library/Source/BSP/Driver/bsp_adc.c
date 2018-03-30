/*******************************************************************************
 *   Filename:       bsp_adc.c
 *   Revised:        All copyrights reserved to Roger.
 *   Date:           2014-08-11
 *   Revision:       v1.0
 *   Writer:	     wumingshen.
 *
 *   Description:    ADC模数转换模块（使用DMA传输数据）
 *
 *
 *   Notes:
 *
 *   All copyrights reserved to wumingshen
 *******************************************************************************/
#include "bsp_adc.h"
#include "bsp.h"
#include "app_ctrl.h"
#include "..\tasks\task_comm.h"

__IO uint16_t ADC_Value[Channel_Times][Channel_Number];
__IO uint16_t ADC_AverageValue[Channel_Number];


/*****************************************************************************************************/
/* EXTERN VARIABLES*/
extern OS_Q                DET_RxQ;


//============================================================================//
void detect_vol_dma_isr_handler(void);


#ifndef SAMPLE_BOARD

/*******************************************************************************
 * 名    称： ADCx_GPIO_Config
 * 功    能： 初始化ADC使用的引脚
 * 入口参数： 无
 * 出口参数： 无
 * 作　 　者： 无名沈.
 * 创建日期： 2015-06-25
 * 修    改记录：
 2017-5-3	fourth		ADC采样引脚改为PB0\PC0~PC5
 *******************************************************************************/
static void ADCx_GPIO_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/* Enable DMA clock */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
#ifdef SAMPLE_BOARD

	/* Enable ADC1 and GPIOC clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 |RCC_APB2Periph_GPIOC, ENABLE);
	GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_AIN;
	GPIO_InitStructure.GPIO_Pin     = GPIO_Pin_0
                                    | GPIO_Pin_1
                                    | GPIO_Pin_2
                                    | GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_AIN;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
#else

	/* Enable ADC1 and GPIOB GPIOC clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOC, ENABLE);
	GPIO_InitStructure.GPIO_Pin     = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_AIN;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin     = GPIO_Pin_0
								| GPIO_Pin_1
								| GPIO_Pin_2
								| GPIO_Pin_3
								| GPIO_Pin_4
								| GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_AIN;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
#endif
	
}

/*******************************************************************************
 * 名    称： ADCx_Mode_Config
 * 功    能： 配置ADCx的工作模式
 * 入口参数： 无
 * 出口参数： 无
 * 作　 　者： 无名沈.
 * 创建日期： 2015-06-25
 * 修    改：
 * 修改日期：
 * 备    注：
 *******************************************************************************/
static void  ADCx_Mode_Config(void)
{
    DMA_InitTypeDef DMA_InitStructure;
    ADC_InitTypeDef ADC_InitStructure;

    /* DMA channel1 configuration */
    //给DMA配置通道1
    DMA_DeInit(DMA1_Channel1);

    //设置DMA源：内存地址或者串口数据寄存器地址
    DMA_InitStructure.DMA_PeripheralBaseAddr = ADC1_DR_Address;	    //ADC地址
    //内存地址（要传输的变量的指针）
    DMA_InitStructure.DMA_MemoryBaseAddr = (u32)&ADC_Value;         //内存地址
    //方向：单向传输
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    //设置传输时缓冲区的长度（1就是一个Half-word16位）
    DMA_InitStructure.DMA_BufferSize = Channel_Times * Channel_Number;
    //外设地址固定
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;//外设地址固定
    //内存地址递增
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;        //内存地址固定
    //DMA在访问每次操作 数据长度
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;//半字（16位）
    DMA_InitStructure.DMA_MemoryDataSize     = DMA_MemoryDataSize_HalfWord;
    //设置DMA的传输方式：循环传输模数
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;		//循环传输模式
    //DMA通道x的优先等级:高
    DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
    //DMA通道x禁止内存到内存的传输
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    //初始化DMA
    DMA_Init(DMA1_Channel1, &DMA_InitStructure);

    /* Enable DMA channel1 */
    //使能DMA
    DMA_Cmd(DMA1_Channel1, ENABLE);
	DMA_ITConfig(DMA1_Channel1, DMA_IT_TC, ENABLE); //使能DMA传输完成中断 
	
    /* ADC1 configuration */
    ADC_DeInit(ADC1); //将外设 ADC1 的全部寄存器重设为缺省值

    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;	    //独立模式ADC模式
    ADC_InitStructure.ADC_ScanConvMode = ENABLE ; 	        //开启扫描模式，扫描模式用于多通道采集
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;	    //开启连续转换模式，即不停地进行ADC转换
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;	//不使用外部触发转换
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right; 	//采集数据右对齐
    ADC_InitStructure.ADC_NbrOfChannel = Channel_Number;	//要转换的通道数目Channel_Number
    ADC_Init(ADC1, &ADC_InitStructure);

    //----------------------------------------------------------------------
    //ADC的转换时间与ADC的时钟和采样周期相关，下面就是配置ADC转换时间的函数
    //ADC采样时间计算公式：T = 采样周期+12.5个周期

    /*配置ADC时钟，为PCLK2的6分频，即12MHz*/
    //ADC时钟频率越高，转换速度越快，但ADC时钟有上限值（不超过14MHZ）
    RCC_ADCCLKConfig(RCC_PCLK2_Div6);
    /*配置ADC1的通道11为55.	5个采样周期，序列为1 */
    //RANK值是指在多通道扫描模式时，本通道的扫描顺序
#ifdef SAMPLE_BOARD
    ADC_RegularChannelConfig(ADC1, ADC_Channel_11, 1, ADC_SampleTime_239Cycles5);
    ADC_RegularChannelConfig(ADC1, ADC_Channel_12, 2, ADC_SampleTime_239Cycles5);
    ADC_RegularChannelConfig(ADC1, ADC_Channel_13, 3, ADC_SampleTime_239Cycles5);
    ADC_RegularChannelConfig(ADC1, ADC_Channel_14, 4, ADC_SampleTime_239Cycles5);
#else
    ADC_RegularChannelConfig(ADC1, ADC_Channel_11, 1, ADC_SampleTime_239Cycles5);
    ADC_RegularChannelConfig(ADC1, ADC_Channel_12, 2, ADC_SampleTime_239Cycles5);
    ADC_RegularChannelConfig(ADC1, ADC_Channel_13, 3, ADC_SampleTime_239Cycles5);
    ADC_RegularChannelConfig(ADC1, ADC_Channel_14, 4, ADC_SampleTime_239Cycles5);
    ADC_RegularChannelConfig(ADC1, ADC_Channel_15, 5, ADC_SampleTime_239Cycles5);
    ADC_RegularChannelConfig(ADC1, ADC_Channel_10, 6, ADC_SampleTime_239Cycles5);
    ADC_RegularChannelConfig(ADC1, ADC_Channel_8, 	7, ADC_SampleTime_239Cycles5);
#endif
    //----------------------------------------------------------------------

    /* Enable ADC1 DMA */
    //使能ADC1 的DMA
    ADC_DMACmd(ADC1, ENABLE);

    /* Enable ADC1 */
    //使能ADC
    ADC_Cmd(ADC1, ENABLE);

    //----------------------------------------------------------------------
    //在开始ADC转换之前，需要启动ADC的自校准
    /*复位校准寄存器 */
    ADC_ResetCalibration(ADC1);
    /*等待校准寄存器复位完成 */
    while(ADC_GetResetCalibrationStatus(ADC1));

    /* ADC校准 */
    ADC_StartCalibration(ADC1);
    /* 等待校准完成*/
    while(ADC_GetCalibrationStatus(ADC1));
    BSP_IntVectSet(BSP_INT_ID_DMA1_CH1, detect_vol_dma_isr_handler);
    BSP_IntEn(BSP_INT_ID_DMA1_CH1);
    /* 由于没有采用外部触发，所以使用软件触发ADC转换 */
	ADC_SoftwareStartConvCmd(ADC1, DISABLE);
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}


/*******************************************************************************
 * 名    称： ADC1Convert_Begin
 * 功    能： 开始ADC1的采集和转换
 * 入口参数： 无
 * 出口参数： 无
 * 作　 　者： 无名沈.
 * 创建日期： 2015-06-25
 * 修    改：
 * 修改日期：
 * 备    注：
 *******************************************************************************/
void ADC1Convert_Begin(void)
{
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
//	ADC_Cmd(ADC1,ENABLE);
}


/*******************************************************************************
 * 名    称： ADC1Convert_Stop
 * 功    能： 停止ADC1的采集和转换
 * 入口参数： 无
 * 出口参数： 无
 * 作　 　者： 无名沈.
 * 创建日期： 2015-06-25
 * 修    改：
 * 修改日期：
 * 备    注：
 *******************************************************************************/
void ADC1Convert_Stop(void)
{
    ADC_SoftwareStartConvCmd(ADC1, DISABLE);
//	ADC_Cmd(ADC1,DISABLE);

}


/*******************************************************************************
 * 名    称： Get_AD_AverageValue
 * 功    能： 获取AD采集的平均值
 * 入口参数： 无
 * 出口参数： 无
 * 作　 　者： 无名沈.
 * 创建日期： 2015-06-25
 * 修    改：
 * 修改日期：
 * 备    注：
 *******************************************************************************/
void Get_AD_AverageValue(void)
{
    uint8_t count;
    uint8_t i;
    uint32_t sum = 0;

    for(i=0;i<Channel_Number;i++) {
        for(count=0;count<Channel_Times;count++) {
            sum += ADC_Value[count][i];
        }
        ADC_AverageValue[i] = sum/Channel_Times;
        sum = 0;
    }
}
/*******************************************************************************
 * 名    称： Get_ADC_Value
 * 功    能： 
 * 入口参数： ch ADC采样通道
 * 出口参数： ADC采样点电压值，单位为mV
 * 作　　者： fourth.
 * 创建日期： 2017-5-3
 * 修    改：
 * 修改日期：
 *******************************************************************************/
uint16 Get_ADC_Value(uint8 ch)
{
	return ADC_AverageValue[ch]*3300/4095;
}
/*******************************************************************************
 * 名    称： detect_vol_dma_isr_handler
 * 功    能：DMA通道1中断函数
 * 入口参数： 无
 * 出口参数： 无
 * 作    者：	 fourth.peng
 * 创建日期： 2017-05-6
 * 修    改记录:
 *******************************************************************************/
void detect_vol_dma_isr_handler(void)
{
	OS_ERR err;
	static ST_QUEUETCB mailbox = {0};
	static uint16_t adc_value[Channel_Times][Channel_Number] = {0};
	
	if(DMA_GetITStatus(DMA1_IT_TC1) != RESET) 
	{
	memcpy((void *)&adc_value, (void *)&ADC_Value, sizeof(adc_value));
	mailbox.event = DETECT_TASK_VOL_ADC_UPDATE;
	mailbox.queue_data.pdata = &adc_value;
	mailbox.queue_data.len = sizeof(adc_value);
//	OSQPost(&DET_RxQ, &mailbox, sizeof(mailbox), OS_OPT_POST_FIFO, &err);
	
	DMA_ClearITPendingBit(DMA1_IT_TC1);
#ifdef SAMPLE_BOARD
//	ADC1Convert_Stop();
#endif
	}

}
#endif
void ADC_Mode_Config(void)
{
	ADC_InitTypeDef ADC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC|RCC_APB2Periph_ADC1,ENABLE);//使能ADC1时钟通道
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);//设置分频因子6 72M/6=12M，最大不能超过14M
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;//模拟输入
	GPIO_Init(GPIOC,&GPIO_InitStructure);//初始化
	
	ADC_DeInit(ADC1);//复位ADC1，将外设ADC1的全部寄存机设置为缺省值
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;//ADC独立模式
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;//单通道模式
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;//单次转换
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;//转换由软件启动
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;//ADC数据右对齐
	ADC_InitStructure.ADC_NbrOfChannel = 1;//顺序进行转换的通道数
	
	ADC_Init(ADC1,&ADC_InitStructure);//根据指定的参数初始化ADC
	ADC_Cmd(ADC1,ENABLE);//使能ADC1
	ADC_ResetCalibration(ADC1);//开启复位校准
	while(ADC_GetResetCalibrationStatus(ADC1));//等待复位结束
	ADC_StartCalibration(ADC1);//开启AD校准
	while(ADC_GetCalibrationStatus(ADC1));//等待校准结束
}


/*******************************************************************************
* Description  : 读取ADC值，数据转换过程采用硬等待。（需调整）
* Author       : 2018/1/24 星期三, by redmorningcn
*******************************************************************************/
uint16_t Get_ADC(uint8_t ch)//ch为通道号
{
    //设置指定ADC的规则组通道，设置它们的转换顺序和采样时间
    uint32  tmp32;
    
	ADC_RegularChannelConfig(ADC1,ch,1,ADC_SampleTime_7Cycles5);//通道1，采样周期为7.5周期+12.5
	ADC_SoftwareStartConvCmd(ADC1,ENABLE);//使能指定的ADC1的软件转换功能
    /*******************************************************************************
    * Description  : 等待ADC转换
    * Author       : 2018/3/29 星期四, by redmorningcn
    *******************************************************************************/
	while(!ADC_GetFlagStatus(ADC1,ADC_FLAG_EOC));   //等待转换结束
    
    tmp32 = ADC_GetConversionValue(ADC1);           //取转换值
    
    tmp32 = (tmp32 * 3300)/4096;                    //返回测量电压值，单位mV
    
	return  tmp32;
}

/*******************************************************************************
 * 名    称： Bsp_ADC_Init
 * 功    能： ADC初始化
 * 入口参数： 无
 * 出口参数： 无
 * 作　 　者： 无名沈.
 * 创建日期： 2015-06-25
 * 修    改：
 * 修改日期：
 * 备    注：
 *******************************************************************************/
void Bsp_ADC_Init(void)
{
//	ADCx_GPIO_Config();
//	ADCx_Mode_Config();
	ADC_Mode_Config();
}

/*******************************************************************************
 *              end of file                                                    *
 *******************************************************************************/
