/*******************************************************************************
 *   Filename:       bsp_pwm.c
 *   Revised:        All copyrights reserved to Wuming Shen.
 *   Date:           2014-07-05
 *   Revision:       v1.0
 *   Writer:	     Wuming Shen.
 *
 *   Description:
 *
 *   Notes:
 *					QQ:276193028
 *     				E-mail:shenchangwei945@163.com
 *
 *******************************************************************************/

/*******************************************************************************
 * INCLUDES
 */
#include <includes.h>
#include <bsp_pwm.h>

#define BSP_PWM_MODULE_EN 1
#if BSP_PWM_MODULE_EN > 0

/*******************************************************************************
 * 描述： 频率单位为HZ
 */
#define FANPWM_FREQ            10000
#define LEDPWM_FREQ            10000
#define BEEPPWM_FREQ           2000

#define FANPWM_DUTY_MAX        100
#define LEDPWM_DUTY_MAX        100
#define BEEPPWM_DUTY_MAX       100
/*******************************************************************************
 * MACROS
 */

/*******************************************************************************
 * TYPEDEFS
 */

/*******************************************************************************
 * LOCAL VARIABLES
 */

/*******************************************************************************
 * GLOBAL VARIABLES
 */
//StrPwm   FanPwm   = {
//    TIM5,
//    FANPWM_FREQ,        FANPWM_FREQ,        FANPWM_FREQ,
//    FANPWM_DUTY_MAX,    FANPWM_DUTY_MAX,    FANPWM_DUTY_MAX,
//    FANPWM_DUTY_MAX,    FANPWM_DUTY_MAX,    FANPWM_DUTY_MAX,
//    FANPWM_DUTY_MAX,    FANPWM_DUTY_MAX,    FANPWM_DUTY_MAX,
//    FANPWM_DUTY_MAX,    FANPWM_DUTY_MAX,    FANPWM_DUTY_MAX,
//    0,                  0,                  0,
//    0,                  2,                  0
//};

StrPwm   LedPwm   = {
    TIM1,
    LEDPWM_FREQ,        LEDPWM_FREQ,        LEDPWM_FREQ,
    LEDPWM_DUTY_MAX,    LEDPWM_DUTY_MAX,    LEDPWM_DUTY_MAX,
    LEDPWM_DUTY_MAX,    LEDPWM_DUTY_MAX,    LEDPWM_DUTY_MAX,
    LEDPWM_DUTY_MAX,    LEDPWM_DUTY_MAX,    LEDPWM_DUTY_MAX,
    LEDPWM_DUTY_MAX,    LEDPWM_DUTY_MAX,    LEDPWM_DUTY_MAX,
    0,                  0,                  0,
    1,                  2,                  3
};

StrPwm   BeepPwm   = {
    TIM2,
    BEEPPWM_FREQ,       BEEPPWM_FREQ,       BEEPPWM_FREQ,
    BEEPPWM_DUTY_MAX,   BEEPPWM_DUTY_MAX,   BEEPPWM_DUTY_MAX,
    BEEPPWM_DUTY_MAX,   BEEPPWM_DUTY_MAX,   BEEPPWM_DUTY_MAX,
    BEEPPWM_DUTY_MAX,   BEEPPWM_DUTY_MAX,   BEEPPWM_DUTY_MAX,
    BEEPPWM_DUTY_MAX,   BEEPPWM_DUTY_MAX,   BEEPPWM_DUTY_MAX,
    0,                  0,                  0,
    1,                  0,                  0
};
/*******************************************************************************
 * LOCAL FUNCTIONS
 */
void    FanPWM_GPIO_Configuration      (void);
void    FanPWM_TIM_Configuration       (void);
void    BeepPWM_TIM_Configuration      (void);
void    LedPWM_GPIO_Configuration      (void);
void    LedPWM_TIM_Configuration       (void);
void    BeepPWM_GPIO_Configuration     (void);
void    BeepPWM_TIM_Configuration      (void);

/*******************************************************************************
 * 名    称： FanPWM_Init
 * 功    能： 风机PWM初始化
 * 入口参数：
 * 出口参数： 无
 * 作　 　者： 无名沈
 * 创建日期： 2014-07-05
 * 修    改：
 * 修改日期：
 *******************************************************************************/
StrPwm *FanPWM_Init(void)
{
    //memcpy(&LedPwm,&FanPwm,sizeof(StrPwm));
    FanPWM_GPIO_Configuration();
    FanPWM_TIM_Configuration();

    //PWM_Start(&FanPwm);
#if ( PWM_OUT_NEGTIVE == DEF_ENABLED )
    PWM_SetProperty(&FanPwm, PWM_CH_FAN, FANPWM_FREQ, 100.0);
#else
    PWM_SetProperty(&FanPwm, PWM_CH_FAN, FANPWM_FREQ, 0.0);
#endif
    return &FanPwm;
}
/*******************************************************************************
 * 名    称： LedPWM_Init
 * 功    能： RGB三色LEDPWM
 * 入口参数：
 * 出口参数： 无
 * 作　 　者： 无名沈
 * 创建日期： 2014-07-05
 * 修    改：
 * 修改日期：
 *******************************************************************************/
StrPwm *LedPWM_Init(void)
{
    LedPWM_GPIO_Configuration();
    LedPWM_TIM_Configuration();

    PWM_Start(&LedPwm);
    PWM_SetProperty(&LedPwm, 0, LEDPWM_FREQ, 0);
    PWM_SetProperty(&LedPwm, 1, LEDPWM_FREQ, 0);
    PWM_SetProperty(&LedPwm, 2, LEDPWM_FREQ, 0);
    PWM_SetProperty(&LedPwm, 3, LEDPWM_FREQ, 0);

    return &LedPwm;
}

/*******************************************************************************
 * 名    称： BeepPWM_Init
 * 功    能： 蜂鸣器PWM初始化
 * 入口参数：
 * 出口参数： 无
 * 作　 　者： 无名沈
 * 创建日期： 2014-07-05
 * 修    改：
 * 修改日期：
 *******************************************************************************/
StrPwm *BeepPWM_Init(void)
{
    BeepPWM_GPIO_Configuration();
    BeepPWM_TIM_Configuration();
    PWM_Start(&BeepPwm);

    PWM_SetProperty(&BeepPwm, 1, BEEPPWM_FREQ, 0);

    return &BeepPwm;
}

/*******************************************************************************
 * 名    称： FanPWM_GPIO_Configuration
 * 功    能： 风机引脚配置
 * 入口参数：
 * 出口参数： 无
 * 作　 　者： 无名沈
 * 创建日期： 2014-07-05
 * 修    改：
 * 修改日期：
 *******************************************************************************/
void FanPWM_GPIO_Configuration(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    GPIO_InitStructure.GPIO_Pin     = GPIO_Pin_8;
    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_50MHz;
#if ( PWM_OUT_NEGTIVE == DEF_ENABLED )
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_AF_PP;     // 开漏输出
#else
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_AF_OD;     // 开漏输出
#endif
    GPIO_Init(GPIOA, &GPIO_InitStructure);
}

/*******************************************************************************
 * 名    称： LedPWM_GPIO_Configuration
 * 功    能： RGB三色LED引脚配置
 * 入口参数：
 * 出口参数： 无
 * 作　 　者： 无名沈
 * 创建日期： 2014-07-05
 * 修    改：
 * 修改日期：
 *******************************************************************************/
void LedPWM_GPIO_Configuration(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    GPIO_InitStructure.GPIO_Pin     = GPIO_Pin_9
                                    | GPIO_Pin_10
                                    | GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_AF_PP;     // 复用推挽输出
    GPIO_Init(GPIOA, &GPIO_InitStructure);
}

/*******************************************************************************
 * 名    称： BeepPWM_GPIO_Configuration
 * 功    能： 蜂鸣器引脚配置
 * 入口参数：
 * 出口参数： 无
 * 作　 　者： 无名沈
 * 创建日期： 2014-07-05
 * 修    改：
 * 修改日期：
 *******************************************************************************/
void BeepPWM_GPIO_Configuration(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    GPIO_InitStructure.GPIO_Pin     = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_AF_PP;     // 开漏输出
    GPIO_Init(GPIOA, &GPIO_InitStructure);
}

/*******************************************************************************
 * 名    称： FanPWM_TIM_Configuration
 * 功    能： 风机PWM定时器配置
 * 入口参数：
 * 出口参数： 无
 * 作　 　者： 无名沈
 * 创建日期： 2014-07-05
 * 修    改：
 * 修改日期：
 *******************************************************************************/
void FanPWM_TIM_Configuration (void)
{
    /***********************************************
    * 描述： 
    */
    //TIM_TimeBaseInitTypeDef   TIM_TimeBaseStructure;
    //TIM_OCInitTypeDef         TIM_OCInitStructure;
    //TIM_BDTRInitTypeDef       TIM_BDTRInitStructure;

    /***********************************************
    * 描述： 
    */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
    /***********************************************
    * 描述： 
    */
    FanPwm.freq[0]   = FanPwm.freq[1]   = FanPwm.freq[2]    = FanPwm.freq[3]    = FANPWM_FREQ;
    FanPwm.period[0] = FanPwm.period[1] = FanPwm.period[2]  = FanPwm.period[3]  = 1000000 / (uint32_t)FanPwm.freq[0]-1;
    FanPwm.TIMx      = TIM1;

    /***********************************************
    * 描述： 第三步，定时器基本配置
    */
    //TIM_TimeBaseStructure.TIM_Prescaler         = SYS_CLK-1;                     //对定时器时钟TIMxCLK设置预分频：不预分频，即为72MHz
    //TIM_TimeBaseStructure.TIM_Period            = FanPwm.period[0];         //当定时器从0计数到999，即为1000次，为一个定时周期  FanPwm.period[0]
    //TIM_TimeBaseStructure.TIM_ClockDivision     = TIM_CKD_DIV1;             //采样分频(不分频)
    //TIM_TimeBaseStructure.TIM_CounterMode       = TIM_CounterMode_Up;       //向上计数
    //TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;                        //重复寄存器，用于自动更新FanPwm占空比
    //TIM_TimeBaseInit(LedPwm.TIMx, &TIM_TimeBaseStructure);

    /***********************************************
    * 描述： 第四步LedPwm输出配置
    */
    //TIM_OCInitStructure.TIM_OCMode              = TIM_OCMode_PWM2;          //设置为Pwm1输出模式(100%输出高电平)
    //TIM_OCInitStructure.TIM_Pulse               = LedPwm.compare[0] ;       //设置占空比时间
    //TIM_OCInitStructure.TIM_OCPolarity          = TIM_OCPolarity_High;      //设置跳变值，当计数器计数到这个值时，电平发生跳变 FanPwm.compare[0]
    //TIM_OCInitStructure.TIM_OutputState         = TIM_OutputState_Enable;   //有效电平的极性，当定时器计数值小于CCR1_Val时为高电平
    /***********************************************
    * 描述： 下面几个参数是高级定时器才会用到，通用定时器不用配置
    */
    //TIM_OCInitStructure.TIM_OCNPolarity         = TIM_OCNPolarity_High;     //设置互补端输出极性
    //TIM_OCInitStructure.TIM_OutputNState        = TIM_OutputNState_Disable; //使能互补端输出
    //TIM_OCInitStructure.TIM_OCIdleState         = TIM_OCIdleState_Reset;    //死区后输出状态
    //TIM_OCInitStructure.TIM_OCNIdleState        = TIM_OCNIdleState_Reset;   //死区后互补端输出状态
    
    /***********************************************
    * 描述： 
    */
    //TIM_OC1Init(FanPwm.TIMx,&TIM_OCInitStructure);                          //按照指定参数初始化
}

/*******************************************************************************
 * 名    称： LedPWM_TIM_Configuration
 * 功    能： RGB三色LED-PWM定时器配置
 * 入口参数：
 * 出口参数： 无
 * 作　 　者： 无名沈
 * 创建日期： 2014-07-05
 * 修    改：
 * 修改日期：
 *******************************************************************************/
void LedPWM_TIM_Configuration (void)
{
    /***********************************************
    * 描述： 临时变量定义
    */
    //TIM_TimeBaseInitTypeDef TIM_BaseInitStructure;
    TIM_TimeBaseInitTypeDef   TIM_TimeBaseStructure;
    TIM_OCInitTypeDef         TIM_OCInitStructure;
    TIM_BDTRInitTypeDef       TIM_BDTRInitStructure;

    /***********************************************
    * 描述： 使能时钟
    */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
    //TIM1 使用内部时钟
    //TIM_InternalClockConfig(TIM1);
    //TIM1基本设置
    //设置预分频器分频系数71，即APB2=72M, TIM1_CLK=72/72=1MHz
    //TIM_Period（TIM1_ARR）=1000，计数器向上计数到1000后产生更新事件，计数值归零
    //向上计数模式
    //    TIM_RepetitionCounter(TIM1_RCR)   =0，每次向上溢出都产生更新事件
    //    TIM_BaseInitStructure.TIM_Period  = 50000;
    //    TIM_BaseInitStructure.TIM_Prescaler = 71;
    //    TIM_BaseInitStructure.TIM_ClockDivision = 0;
    //    TIM_BaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    //    TIM_BaseInitStructure.TIM_RepetitionCounter = 0;
    //    TIM_TimeBaseInit(TIM1, &TIM_BaseInitStructure);
    //    //清中断，以免一启用中断后立即产生中断
    //    TIM_ClearFlag(TIM1, TIM_FLAG_Update);
    //    //使能TIM1中断源
    //    TIM_ITConfig(TIM1, TIM_IT_Update, ENABLE);
    //    //TIM1总开关：开启
    //    TIM_Cmd(TIM1, ENABLE);TIMx->ARR
    /***********************************************
    * 描述： 初始化结构体
    */
    LedPwm.freq[0]   = LedPwm.freq[1]   = LedPwm.freq[2]    = LedPwm.freq[3]    = LEDPWM_FREQ;
    LedPwm.period[0] = LedPwm.period[1] = LedPwm.period[2]  = LedPwm.period[3]  = 1000000 / (uint32_t)LedPwm.freq[0]-1;
    LedPwm.TIMx      = TIM1;

    /***********************************************
    * 描述： 第三步，定时器基本配置
    */
    TIM_TimeBaseStructure.TIM_Prescaler         = BSP_CPU_ClkFreq_MHz-1;    //时钟预分频数
    TIM_TimeBaseStructure.TIM_Period            = LedPwm.period[0];         //自动重装载寄存器的值
    TIM_TimeBaseStructure.TIM_ClockDivision     = TIM_CKD_DIV1;             //采样分频(不分频)
    TIM_TimeBaseStructure.TIM_CounterMode       = TIM_CounterMode_Up;       //向上计数
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;                        //重复寄存器，用于自动更新FanPwm占空比
    TIM_TimeBaseInit(LedPwm.TIMx, &TIM_TimeBaseStructure);

    /***********************************************
    * 描述： 第四步FanPwm输出配置
    */
#if ( PWM_OUT_NEGTIVE == DEF_ENABLED )
    TIM_OCInitStructure.TIM_OCMode              = TIM_OCMode_PWM1;          //设置为Pwm2输出模式(0%输出高电平)
    TIM_OCInitStructure.TIM_Pulse               = LedPwm.compare[0] ;       //设置占空比时间
    TIM_OCInitStructure.TIM_OCPolarity          = TIM_OCPolarity_High;      //设置输出极性
#else
    TIM_OCInitStructure.TIM_OCMode              = TIM_OCMode_PWM2;          //设置为Pwm2输出模式(0%输出高电平)
    TIM_OCInitStructure.TIM_Pulse               = LedPwm.compare[0] ;       //设置占空比时间
    TIM_OCInitStructure.TIM_OCPolarity          = TIM_OCPolarity_High;      //设置输出极性
#endif
    TIM_OCInitStructure.TIM_OutputState         = TIM_OutputState_Enable;   //使能该通道输出

    /***********************************************
    * 描述： 下面几个参数是高级定时器才会用到，通用定时器不用配置
    */
    TIM_OCInitStructure.TIM_OCNPolarity         = TIM_OCNPolarity_High;     //设置互补端输出极性
    TIM_OCInitStructure.TIM_OutputNState        = TIM_OutputNState_Disable; //使能互补端输出
    TIM_OCInitStructure.TIM_OCIdleState         = TIM_OCIdleState_Reset;    //死区后输出状态
    TIM_OCInitStructure.TIM_OCNIdleState        = TIM_OCNIdleState_Reset;   //死区后互补端输出状态

    /***********************************************
    * 描述： 初始化比通道1输出
    */
    TIM_OC1Init(LedPwm.TIMx,&TIM_OCInitStructure);                          //按照指定参数初始化
   
    /***********************************************
    * 描述： 第四步LedPwm输出配置
    */
    TIM_OCInitStructure.TIM_OCMode              = TIM_OCMode_PWM1;          //设置为Pwm1输出模式(100%输出高电平)
    TIM_OCInitStructure.TIM_Pulse               = LedPwm.compare[0] ;       //设置占空比时间
    TIM_OCInitStructure.TIM_OCPolarity          = TIM_OCPolarity_High;      //设置输出极性
    TIM_OCInitStructure.TIM_OutputState         = TIM_OutputState_Enable;   //使能该通道输出

    /***********************************************
    * 描述： 下面几个参数是高级定时器才会用到，通用定时器不用配置
    */
    TIM_OCInitStructure.TIM_OCNPolarity         = TIM_OCNPolarity_High;     //设置互补端输出极性
    TIM_OCInitStructure.TIM_OutputNState        = TIM_OutputNState_Disable; //使能互补端输出
    TIM_OCInitStructure.TIM_OCIdleState         = TIM_OCIdleState_Reset;    //死区后输出状态
    TIM_OCInitStructure.TIM_OCNIdleState        = TIM_OCNIdleState_Reset;   //死区后互补端输出状态

    /***********************************************
    * 描述： 初始化通道2，3，4比较输出
    */
    TIM_OC2Init(LedPwm.TIMx,&TIM_OCInitStructure);
    TIM_OC3Init(LedPwm.TIMx,&TIM_OCInitStructure);
    TIM_OC4Init(LedPwm.TIMx,&TIM_OCInitStructure);

    /***********************************************
    * 描述： 第五步，死区和刹车功能配置，高级定时器才有的，通用定时器不用配置
    */
    TIM_BDTRInitStructure.TIM_OSSRState         = TIM_OSSRState_Disable;    //运行模式下输出选择
    TIM_BDTRInitStructure.TIM_OSSIState         = TIM_OSSIState_Disable;    //空闲模式下输出选择
    TIM_BDTRInitStructure.TIM_LOCKLevel         = TIM_LOCKLevel_OFF;        //锁定设置
    TIM_BDTRInitStructure.TIM_DeadTime          = 0x90;                     //死区时间设置
    TIM_BDTRInitStructure.TIM_Break             = TIM_Break_Disable;        //刹车功能使能
    TIM_BDTRInitStructure.TIM_BreakPolarity     = TIM_BreakPolarity_High;   //刹车输入极性
    TIM_BDTRInitStructure.TIM_AutomaticOutput   = TIM_AutomaticOutput_Enable;//自动输出使能
    TIM_BDTRConfig(LedPwm.TIMx,&TIM_BDTRInitStructure);

    /***********************************************
    * 描述： 第六步，使能端的打开
    */
    TIM_OC1PreloadConfig(LedPwm.TIMx, TIM_OCPreload_Enable);                //使能TIMx在CCR1上的预装载寄存器
    TIM_OC2PreloadConfig(LedPwm.TIMx, TIM_OCPreload_Enable);
    TIM_OC3PreloadConfig(LedPwm.TIMx, TIM_OCPreload_Enable);
    TIM_OC4PreloadConfig(LedPwm.TIMx, TIM_OCPreload_Enable);
    TIM_ARRPreloadConfig(LedPwm.TIMx, ENABLE);                              //使能TIMx在ARR上的预装载寄存器
    TIM_Cmd(LedPwm.TIMx,ENABLE);                                            //打开TIM1
    
    /***********************************************
    * 描述： 下面这句是高级定时器才有的，输出FanPwm必须打开
    */
    TIM_CtrlPWMOutputs(LedPwm.TIMx, ENABLE);                                //LedPwm输出使能，一定要记得打开
}

/*******************************************************************************
 * 名    称： FanPWM_TIM_Configuration
 * 功    能： 风机PWM定时器配置
 * 入口参数：
 * 出口参数： 无
 * 作　 　者： 无名沈
 * 创建日期： 2014-07-05
 * 修    改：
 * 修改日期：
 *******************************************************************************/
void BeepPWM_TIM_Configuration (void)
{
    /***********************************************
    * 描述： 
    */
    //TIM_TimeBaseInitTypeDef TIM_BaseInitStructure;
    TIM_TimeBaseInitTypeDef   TIM_TimeBaseStructure;
    TIM_OCInitTypeDef         TIM_OCInitStructure;
    //TIM_BDTRInitTypeDef       TIM_BDTRInitStructure;

    /***********************************************
    * 描述： 
    */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    /***********************************************
    * 描述： 
    */
    BeepPwm.freq[0]   = BeepPwm.freq[1]   = BeepPwm.freq[2]   = BeepPwm.freq[3]   = BEEPPWM_FREQ;
    BeepPwm.period[0] = BeepPwm.period[1] = BeepPwm.period[2] = BeepPwm.period[3] = 1000000 / (uint32_t)BeepPwm.freq[0] - 1;
    BeepPwm.TIMx      = TIM2;

    /***********************************************
    * 描述： 
    */
    TIM_TimeBaseStructure.TIM_Prescaler = BSP_CPU_ClkFreq_MHz-1;    //对定时器时钟TIMxCLK设置预分频：不预分频，即为72MHz
    TIM_TimeBaseStructure.TIM_Period = BeepPwm.period[0];           //当定时器从0计数到999，即为1000次，为一个定时周期  FanPwm.period[0]
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1 ;        //设置时钟分频系数：不分频
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;     //向上计数模式
    TIM_TimeBaseInit(BeepPwm.TIMx, &TIM_TimeBaseStructure);         //时基初始化

    /***********************************************
    * 描述： 
    */
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;	            //配置为PWM模式1
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;	//配置输出状态为使能输出
    TIM_OCInitStructure.TIM_Pulse = BeepPwm.compare[0];	            //设置跳变值，当计数器计数到这个值时，电平发生跳变 FanPwm.compare[0]
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;       //有效电平的极性，当定时器计数值小于CCR1_Val时为高电平

    /***********************************************
    * 描述： 
    */
    TIM_OC2Init(BeepPwm.TIMx, &TIM_OCInitStructure);	            //使能定时器的通道2
    TIM_OC2PreloadConfig(BeepPwm.TIMx, TIM_OCPreload_Enable);       //配置各通道的比较寄存器TIM_CCR预装载使能

    TIM_ARRPreloadConfig(BeepPwm.TIMx, ENABLE);			            // 使能TIM5重载寄存器ARR

    /***********************************************
    * 描述： 
    */
    TIM_Cmd(BeepPwm.TIMx, ENABLE);                                  //使能定时器5
}

/*******************************************************************************
 * 名    称： FanPWM_SetFreq
 * 功    能： 设置风机PWM输出频率
 * 入口参数： 频率
 * 出口参数： 无
 * 作　 　者： 无名沈
 * 创建日期： 2014-07-05
 * 修    改：
 * 修改日期：
 *******************************************************************************/
void PWM_SetFreq(StrPwm *pwm, uint8_t ch, float freq)
{
    if ( ch > 3 )
        return;

    if ( freq > 10000.0 )
      freq = 10000.0;
    else if ( freq < 1.0 )
      freq  = 1.0;

    pwm->freq[ch]    = freq;
    pwm->period[ch]  = 1000000 / (uint32_t)pwm->freq[ch] - 1;

    PWM_SetDuty(pwm, ch,pwm->duty[ch]);
    TIM_SetAutoreload(pwm->TIMx, pwm->period[ch]);
}

/*******************************************************************************
 * 名    称： FanPWM_SetDuty
 * 功    能： 设置风机PWM输出占空比
 * 入口参数：
 * 出口参数： 无
 * 作　 　者： 无名沈
 * 创建日期： 2014-07-05
 * 修    改：
 * 修改日期：
 *******************************************************************************/
void PWM_SetDuty(StrPwm *pwm, uint8_t ch, float duty)
{
    if ( ch > 3 )
        return;

    if ( duty > 100.0 )
      duty  = 100.0;
    else if ( duty < 0.0 )
      duty  = 0.0;

    pwm->duty[ch]    = duty;
    pwm->compare[ch] = (uint32_t)( pwm->duty[ch] * pwm->period[ch] / 100.0 );

    switch(ch) {
    case 0:
        TIM_SetCompare1(pwm->TIMx, pwm->compare[ch]);
        break;
    case 1:
        TIM_SetCompare2(pwm->TIMx, pwm->compare[ch]);
        break;
    case 2:
        TIM_SetCompare3(pwm->TIMx, pwm->compare[ch]);
        break;
    case 3:
        TIM_SetCompare4(pwm->TIMx, pwm->compare[ch]);
        break;
    default:break;
    }
}

/*******************************************************************************
 * 名    称： FanPWM_SetProperty
 * 功    能： 启动风机PWM输出
 * 入口参数：
 * 出口参数： 无
 * 作　 　者： 无名沈
 * 创建日期： 2014-07-05
 * 修    改：
 * 修改日期：
 *******************************************************************************/
void PWM_SetProperty(StrPwm *pwm, uint8_t ch, float freq, float duty)
{
  PWM_SetFreq(pwm, ch, freq);
  PWM_SetDuty(pwm, ch, duty);
}

/*******************************************************************************
 * 名    称： FanPWM_Start
 * 功    能： 启动风机PWM输出
 * 入口参数：
 * 出口参数： 无
 * 作　 　者： 无名沈
 * 创建日期： 2014-07-05
 * 修    改：
 * 修改日期：
 *******************************************************************************/
void PWM_Start(StrPwm *pwm)
{
    //第六步，使能端的打开
    for ( int i = 0; i < PWM_CH_NBR; i++ ) {
        switch (pwm->chEn[i]) {
        case 1:
            TIM_OC1PreloadConfig(pwm->TIMx, TIM_OCPreload_Enable);   //使能TIMx在CCR1上的预装载寄存器
            break;
        case 2:
            TIM_OC2PreloadConfig(pwm->TIMx, TIM_OCPreload_Enable);   //使能TIMx在CCR2上的预装载寄存器
            break;
        case 3:
            TIM_OC3PreloadConfig(pwm->TIMx, TIM_OCPreload_Enable);   //使能TIMx在CCR3上的预装载寄存器
            break;
        case 4:
            TIM_OC4PreloadConfig(pwm->TIMx, TIM_OCPreload_Enable);   //使能TIMx在CCR4上的预装载寄存器
            break;
        default:
            break;
        }
    }
    TIM_ARRPreloadConfig(pwm->TIMx, ENABLE);                 //使能TIMx在ARR上的预装载寄存器
    TIM_Cmd(pwm->TIMx,ENABLE);                               //打开TIM2
}

/*******************************************************************************
 * 名    称： FanPWM_Stop
 * 功    能： 停止风机PWM输出
 * 入口参数：
 * 出口参数： 无
 * 作　 　者： 无名沈
 * 创建日期： 2014-07-05
 * 修    改：
 * 修改日期：
 *******************************************************************************/
void PWM_Stop(StrPwm *pwm)
{
    //第六步，使能端的关闭
    for ( int i = 0; i < PWM_CH_NBR; i++ ) {
        switch (pwm->chEn[i]) {
        case 0:
            TIM_OC1PreloadConfig(pwm->TIMx, TIM_OCPreload_Disable);   //使能TIMx在CCR1上的预装载寄存器
            break;
        case 1:
            TIM_OC2PreloadConfig(pwm->TIMx, TIM_OCPreload_Disable);   //使能TIMx在CCR2上的预装载寄存器
            break;
        case 2:
            TIM_OC3PreloadConfig(pwm->TIMx, TIM_OCPreload_Enable);   //使能TIMx在CCR3上的预装载寄存器
            break;
        case 3:
            TIM_OC4PreloadConfig(pwm->TIMx, TIM_OCPreload_Disable);   //使能TIMx在CCR4上的预装载寄存器
            break;
        default:
            break;
        }
    }
    TIM_ARRPreloadConfig(pwm->TIMx, DISABLE);                     //使能TIMx在ARR上的预装载寄存器
    TIM_CtrlPWMOutputs(pwm->TIMx, DISABLE);                         //高级定时器Pwm输出使能，一定要记得打开

}

/*******************************************************************************
 * 名    称： LED_SetColor
 * 功    能： 设置LED颜色
 * 入口参数：
 * 出口参数： 无
 * 作　 　者： 无名沈
 * 创建日期： 2014-07-05
 * 修    改：
 * 修改日期：
 *******************************************************************************/
void LED_SetColor(INT32U color ) 
{
    INT08U red  = B2_INT32U(color);
    INT08U gre  = B1_INT32U(color);
    INT08U ble  = B0_INT32U(color);
    
//    PWM_SetDuty(&LedPwm, LED_CH_R, red / 2.55);
//    PWM_SetDuty(&LedPwm, LED_CH_G, gre / 2.55);
//    PWM_SetDuty(&LedPwm, LED_CH_B, ble / 2.55);
}
/*******************************************************************************
 * 				end of file
 *******************************************************************************/
#endif