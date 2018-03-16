/*******************************************************************************
 *   Filename:       bsp_pid.c
 *   Revised:        $Date: 2009-01-31	20:15 (Fri) $
 *   Revision:       $
 *	 Writer:		 WMing Shen.
 *
 *   Description:    PID算法
 *
 *   Notes:
 *
 *
 *   All copyrights reserved to WMing Shen.
 *
 *******************************************************************************/

/*******************************************************************************
 * INCLUDES
 */
#include <includes.h>
#include <global.h>
#include <bsp_pid.h>

#define BSP_PID_MODULE_EN 1
#if BSP_PID_MODULE_EN > 0
/*******************************************************************************
 * CONSTANTS
 */

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
StrPid          *pid;

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

/*******************************************************************************/

/*******************************************************************************
 * 名    称： PID_Init
 * 功    能： PID_Init DESCRIPTION This function initializes the pointers in the
 *            _PID structure to the process
 *            variable and the setpoint. *Pv and *Sv are integer pointers.
 * 入口参数：
 * 出口参数： 无
 * 作　　者： 无名沈
 * 创建日期： 2014-08-23
 * 修    改：
 * 修改日期：
 *******************************************************************************/
void PID_Init(StrPid *pid, int pv, int sv)
{
	pid->Pv 		= pv;
	pid->Sv 		= sv;
    pid->Kp         = 1;                            // 比例系数
    pid->Ki         = pid->Kp * pid->T / pid->Ti;   // 积分系数
    pid->Kd         = pid->Kp * pid->Td / pid->T;   // 微分系数
}

/*******************************************************************************
 * 名    称：
 * 功    能： PID_Tune DESCRIPTION Sets the proportional gain (p_gain),
Integral gain (i_gain),
derivitive gain (d_gain), and the dead band (dead_band) of a pid control structure _PID.
//设定PID参数 －－－－ P,I,D,死区
 * 入口参数：
 * 出口参数： 无
 * 作　　者： 无名沈
 * 创建日期： 2014-08-23
 * 修    改：
 * 修改日期：
 *******************************************************************************/
void PID_Tune(StrPid        *pid,
              float         p_gain,
              float         i_gain,
              float         d_gain,
              float         Integral,
              CPU_INT16S    dead_band,
              CPU_INT16S    MaxPv,
              CPU_INT16S    MaxOut,
              CPU_INT16S    PidEnBand)
{
	pid->Kp 		= p_gain;
	pid->Ki 		= i_gain;
	pid->Kd 		= d_gain;
	pid->DeadBand 	= dead_band;
	pid->Integral	= Integral;
    pid->MaxPv      = MaxPv;
    pid->MaxOut     = MaxOut;
    pid->PidEnBand    = PidEnBand;
	pid->LastErr	= 0;
}
/*******************************************************************************
 * 名    称：
 * 功    能： PID_SetInteral DESCRIPTION Set a new value for the Integral term of the pid equation.
//This is useful for setting the initial output of the pid controller at start up.
//设定输出初始值
 * 入口参数：
 * 出口参数： 无
 * 作　　者： 无名沈
 * 创建日期： 2014-08-23
 * 修    改：
 * 修改日期：
 *******************************************************************************/
void PID_SetInteral(StrPid *pid,float new_Integral)
{
	pid->Integral	= new_Integral;
	pid->LastErr = 0;
}

/*******************************************************************************
 * 名    称：
 * 功    能： PID_Bumpless DESCRIPTION Bumpless transfer algorithim.
//When suddenly changing setpoints, or when restarting the PID equation after an extended pause,
//the derivative of the equation can cause a bump in the
//controller output. This function will help smooth out that bump.
//The process value in *Pv should be the updated just before this function is used.
//PID_Bumpless 实现无扰切换
//当突然改变设定值时，或重新启动后，将引起扰动输出。这个函数将能实现平顺扰动，在调用该函数之前需要先更新PV值

 * 入口参数：
 * 出口参数： 无
 * 作　　者： 无名沈
 * 创建日期： 2014-08-23
 * 修    改：
 * 修改日期：
 *******************************************************************************/
void PID_Bumpless(StrPid *pid)
{
	pid->LastErr = (pid->Sv)-(pid->Pv);  //设定值与反馈值偏差
}
/*******************************************************************************
 * 名    称：
 * 功    能： //PID_Calc DESCRIPTION Performs PID calculations for the _PID structure *a.
//This function uses the positional form of the pid equation, and incorporates an Integral windup prevention algorithim.
//Rectangular integration is used, so this function must be repeated on a consistent time basis for accurate control.
//RETURN VALUE The new output value for the pid loop. USAGE #include "control.h"
//本函数使用位置式PID计算方式，并且采取了积分饱和限制运算 PID计算
Kp                          比例系数
Ki = Kp*T/Ti                积分系数
Kd = Kp*Td/T                微分系数
 * 入口参数：
 * 出口参数： -100% ~ 100%
 * 作　　者： 无名沈
 * 创建日期： 2014-08-23
 * 修    改：
 * 修改日期：
 *******************************************************************************/
float PID_Calc(StrPid *pid)
{
	float 	pterm;                                      // 比例项
    float   dterm;                                      // 微分项
    float   result;                                     // 输出结果
    float   err;                                        // 偏差取反

    /*************************************************
    * 描述： 计算当前偏差
    */
	pid->Err    = pid->Sv - pid->Pv;                    // 计算当前偏差
    err         = 0.0 - pid->Err;                       // 偏差取反

    if ( 0 == pid->Ki )
        pid->Integral   = 0.0;

    /*************************************************
    * 描述： PID调节区域
    */
	if ( abs(pid->Err) > pid->DeadBand ) {                  // 判断是否大于死区
        /*************************************************
        * 描述： 比例项
        */
		pterm   = pid->Kp * pid->Err;                       // 比例项 (0~100)
        /*************************************************
        * 描述： 积分项
        */
        switch ( pid->OutPolarity ) {
        /*************************************************
        * 描述： 正极性
        */
        case PID_POLARITY_POSITIVE:
            if ( pterm >= abs(pid->LoLimit) ) {             // 如果比例项 > 100
                pid->Integral = 0.0;                        // 则积分项为0
            } else {
                pid->Integral += pid->Ki * pid->Err;        // 积分项
            }
            break;
        /*************************************************
        * 描述： 负极性
        */
        case PID_POLARITY_NEGATIVE:
            if ( pterm <= abs(pid->HiLimit)) {              // 如果比例项 > 100
                pid->Integral = 0.0;                        // 则积分项为0
            } else {
                pid->Integral += pid->Ki * pid->Err;        // 积分项
            }
            break;
        /*************************************************
        * 描述： 双极性
        */
        case PID_POLARITY_UNIPOLAR:
            if ( fabs(pterm) >= abs(pid->LoLimit) ) {       // 如果比例项 > 100
                pid->Integral = 0.0;                        // 则积分项为0
            } else {
                pid->Integral += pid->Ki * pid->Err;        // 积分项
            }
            break;
        }

        /*************************************************
        * 描述： 微分项
        */
		dterm   = pid->Kd * ( pid->Err - pid->LastErr );    // 微分项
        /*************************************************
        * 描述： 输出结果
        *        y = Kp [ e(t) + T/Ti|(e(t)*dt) + TD/T*de(t)/dt]
        */
		result 	= pterm                                     // 比例调节项
                + pid->Integral                             // 积分调节项
                + dterm;                                    // 微分调节项
    /*************************************************
    * 描述： 死区范围，不动作
    */
	} else {

	}

    /*************************************************
    * 描述： 满幅输出
    */
    if ( err < pid->LoLimit ) {                         // 如果低于PID启用下限值
        pid->LastErr    = pid->Err;                             // 保存上次偏差
        return pid->MaxOut;                             // 则以最大值输出
    } else if ( err > pid->HiLimit ) {                  // 如果高于PID关闭上限值
        if ( pid->LastErr > pid->Err ) {
            pid->Integral = 0.0;                            // 则积分项为0
            pid->LastErr    = pid->Err;                             // 保存上次偏差
            return pid->MinOut;                             // 则以最小值输出
        } else {
        }
    }

    /*************************************************
    * 描述： 保存当前偏差
    */
	pid->LastErr    = pid->Err;                             // 保存上次偏差

    /*************************************************
    * 描述： 计算输出值
    */
    switch ( pid->OutPolarity ) {
    /*************************************************
    * 描述： 正极性
    */
    case PID_POLARITY_POSITIVE:
        result  = result / (float)abs(pid->LoLimit) * (float)pid->MaxOut;
        break;
    /*************************************************
    * 描述： 负极性
    */
    case PID_POLARITY_NEGATIVE:
        result = result / (float)abs(pid->HiLimit) * (float)pid->MinOut;
        result  = 0.0 - abs(result);
        break;
    /*************************************************
    * 描述： 双极性
    */
    case PID_POLARITY_UNIPOLAR:
        break;
    }

    /*************************************************
    * 描述： 输出过滤
    */
    if ( result > pid->MaxOut ) { 				            // 如果计算结果大于100
        result = pid->MaxOut;                               // 则等于100
    } else if ( result < pid->MinOut ) { 				    // 如果计算结果大于100
        result = pid->MinOut;                               // 则等于100
    }
    /*************************************************
    * 描述： 返回
    */
	return (result);                                        // 输出PID值(0-100%)
}

/*******************************************************************************
 * 名    称：
 * 功    能：
 * 入口参数：
 * 出口参数： 无
 * 作　　者： 无名沈
 * 创建日期： 2014-08-23
 * 修    改：
 * 修改日期：
 *******************************************************************************/
void BSP_PID_Main(void) {
	float 	result;
	int 	count	= 0;
    StrPid  HeatPid;
			pid 	= &HeatPid;

	printf("The values of Process point, Set point, P gain, I gain, D gain \n");
	printf("Enter the values of Process point\n");

    PID_Init(&HeatPid, 10.0, 50.0);             // 设定PV,SP值
//    PID_Tune(&HeatPid,                          // 调节对象
//             HeatPid.Kp,                        // 比例系数
//             HeatPid.Ki,                        // 积分系数
//             HeatPid.Kd,                        // 微分系数
//             HeatPid.DeadBand,                  // 死区
//             HeatPid.Integral);                 // 积分
    PID_SetInteral(&HeatPid, 0.0);              // 初始化PID输出值
    PID_Bumpless(&HeatPid);                     // 获取初始化误差

	while(count++<=1000) {
		result          = PID_Calc(&HeatPid);
        HeatPid.Pv     += result * 10.0 / 100.0;

		printf("%d: %f\n",  count, result);
		printf("\n%f%f%f%f",(float)HeatPid.Pv, (float)HeatPid.Sv, (float)HeatPid.Ki, (float)HeatPid.Kd);
	}
}




//////////////////////////////////////////////////////////

struct _pid{
	float Sv;				    //定义设定值
	float Pv;			        //定义实际值
	float Err;					//定义偏差值
	float LastErr;				//定义上一个偏差值
	float Kp,Ki,Kd;				//定义比例、积分、微分系数
	float Integral;				//定义积分值
    float DeadBand;
	float CtrlVal;  		    //定义输出（控制执行器的变量）
	float Max;
	float Min;
}pid2;

void PID_Init2(){
	printf("PID_init begin \n");
	pid2.Sv         = 0.0;
	pid2.Pv         = 0.0;
	pid2.Err        = 0.0;
	pid2.LastErr    = 0.0;
	pid2.CtrlVal    = 0.0;
	pid2.Integral   = 0.0;
	pid2.Kp         = 0.2;
	pid2.Ki         = 0.1;          //注意，和上几次相比，这里加大了积分环节的值
	pid2.Kd         = 0.2;
    pid2.DeadBand   = 120;
	pid2.Max        = 120;
	pid2.Min        = 0;
	printf("PID_init end \n");
}

float PID_Realize(float Sv){
	int             IntegralEn;
	pid2.Sv	        = Sv;
	pid2.Err	    = pid2.Sv - pid2.Pv;

	if(pid2.Pv > pid2.Max) {                            //灰色底色表示抗积分饱和的实现
		if(fabs(pid2.Err) > 100) {                      //蓝色标注为积分分离过程
			IntegralEn=0;
		} else {
			IntegralEn=1;
			if(pid2.Err<0) {
				pid2.Integral	+= pid2.Err;
			}
		}
	} else if (pid2.Pv < pid2.Max) {
		if(fabs(pid2.Err) > 100) {                      //积分分离过程
			IntegralEn=0;
		} else {
			IntegralEn=1;
			if(pid2.Err > 0) {
				pid2.Integral	+= pid2.Err;
			}
		}
	} else {
		if(fabs(pid2.Err) > 100) {                      //积分分离过程
			IntegralEn=0;
		} else {
			IntegralEn=1;
			pid2.Integral	+= pid2.Err;
		}
	}

	pid2.CtrlVal    =	pid2.Kp * pid2.Err				    +
						pid2.Ki * pid2.Integral * IntegralEn+
						pid2.Kd *(pid2.Err	  - pid2.LastErr);

	pid2.LastErr	= pid2.Err;
	pid2.Pv	        = pid2.CtrlVal * 1.0;

	return pid2.Pv;
}

int main2(){
    PID_Init2();
    int count=0;

    while(count<1000)
    {
        float Pv	= PID_Realize(100.0);
        printf("%f\n",Pv);
        count++;
    }
    return 0;
}
//////////////////////////////////////////////////////////
//PID控制算法的C语言实现一 PID算法原理
//	最近两天在考虑一般控制算法的C语言实现问题，发现网络上尚没有一套完整的比较体系的
//	讲解。   于是总结了几天，整理一套思路分享给大家。   在工业应用中PID及其衍生算法
//	是应用最广泛的算法之一，是当之无愧的万能算法，   如果能够熟练掌握PID算法的设计
//	与实现过程，对于一般的研发人员来讲，应该是足够应对一般研发问题了，而难能可贵的是，
//	在我所接触的控制算法当中，PID控制算法又是最简单，最能体现反馈思想的控制算法，可
//	谓经典中的经典。经典的未必是复杂的，经典的东西常常是简单的，而且是最简单的，想想
//	牛顿的力学三大定律吧，想想爱因斯坦的质能方程吧，何等的简单！简单的不是原始的，简
//	单的也不是落后的，简单到了美的程度。先看看PID算法的一般形式：
//
//	PID的流程简单到了不能再简单的程度，通过误差信号控制被控量，而控制器本身就是比例、
//	积分、微分三个环节的加和。这里我们规定（在t时刻）：
//	1.输入量为rin(t);
//	2.输出量为rout(t);
//	3.偏差量为Err(t)=rin(t)-rout(t);
//	pid的控制规律为
//
//	理解一下这个公式，主要从下面几个问题着手，为了便于理解，把控制环境具体一下：
//	1.规定这个流程是用来为直流电机调速的;
//	2.输入量rin(t)为电机转速预定值;
//	3.输出量rout(t)为电机转速实际值;
//	4.执行器为直流电机;
//	5.传感器为光电码盘，假设码盘为10线;
//	6.直流电机采用PWM调速转速用单位 转/min 表示;
//	不难看出以下结论：
//	1.输入量rin（t）为电机转速预定值（转/min）;
//	2. 输出量rout(t)为电机转速实际值（转/min）;
//	3.偏差量为预定值和实际值之差（转/min）;
//	那么以下几个问题需要弄清楚：
//	1.通过PID环节之后的U(t)是什么值呢？
//	2.控制执行器（直流电机）转动转速应该为电压值（也就是PWM占空比）。
//	3.那么U(t)与PWM之间存在怎样的联系呢？
//	http://blog.21ic.com/user1/3407/archives/2006/33541.html这篇文章上给出了一种方
//	法，即，每个电压对应一个转速，电压和转速之间呈现线性关系。但是我考虑这种方法的
//	前提是吧直流电机的特性理解为线性了，而实际情况下，直流电机的特性绝对不是线性的，
//	或者说在局部上是趋于线性的，这就是为什么说PID调速有个范围的问题。具体看一下
//	http://articles.e-works.net.cn/component/article90249.htm这篇文章就可以了解了。
//	所以在正式进行调速设计之前，需要现有开环系统，测试电机和转速之间的特性曲线（或者
//	查阅电机的资料说明），然后再进行闭环参数整定。这篇先写到这，下一篇说明连续系统的
//	离散化问题。并根据离散化后的特点讲述位置型PID和增量型PID的用法和C语言实现过程。
//
////////////////////////////////////////////////////////////
//PID控制算法的C语言实现二 PID算法的离散化
//	上一节中，我论述了PID算法的基本形式，并对其控制过程的实现有了一个简要的说明，通
//	过上一节的总结，基本已经可以明白PID控制的过程。这一节中先继续上一节内容补充说明
//	一下。
//	1.说明一下反馈控制的原理，通过上一节的框图不难看出，PID控制其实是对偏差的控制过程;
//	2.如果偏差为0,则比例环节不起作用，只有存在偏差时，比例环节才起作用。
//	3.积分环节主要是用来消除静差，所谓静差，就是系统稳定后输出值和设定值之间的差值，
//	 积分环节实际上就是偏差累计的过程，把累计的误差加到原有系统上以抵消系统造成的静差。
//	4.而微分信号则反应了偏差信号的变化规律，或者说是变化趋势，根据偏差信息的变化趋势
//	 来进行调节，从而增加了系统的快速性。
//	好了，关于PID的基本说明就补充到这里，下面将对PID连续系统离散化，从而方便在处理器
//	上实现。下面把连续状态的公式再贴一下：
//
//	假设采样间隔为T，则在第KT时刻：
//	Err(K)=rin(K)-rout(K);
//	积分环节用加和的形式表示，即Err(K)+Err(K+1)+……;
//	微分环节用斜率的形式表示，即[Err(K)-Err(K-1)]/T;
//	从而形成如下PID离散表示形式：
//	则u(K)可表示成为：
//	至于说Kp、Ki、Kd三个参数的具体表达式，我想可以轻松的推出了，这里节省时间，不再
//	详细表示了。	其实到这里为止，PID的基本离散表示形式已经出来了。目前的这种表述
//	形式属于位置型PID，另外一种表述方式为增量式PID，由U上述表达式可以轻易得到：
//	那么：
//	这就是离散化PID的增量式表示方式，由公式可以看出，增量式的表达结果只和最近三次的
//	偏差差有关，这样就大大提高了系统的稳定性。需要注意的是最终的输出结果应该为
//	u(K)+增量调节值;
//	PID的离散化过程基本思路就是这样，下面是将离散化的公式转换成为C语言，从而实现微
//	控制器的控制作用。
//	PID控制算法的C语言实现二 PID算法的离散化   上一节中，我论述了PID算法的基本形式，
//	并对其控制过程的实现有了一个简要的说明，通过上一节的总结，基本已经可以明白PID
//	控制的过程。这一节中先继续上一节内容补充说明一下。
//	1.说明一下反馈控制的原理，通过上一节的框图不难看出，PID控制其实是对偏差的控制过程;
//	2.如果偏差为0,则比例环节不起作用，只有存在偏差时，比例环节才起作用。
//	3.积分环节主要是用来消除静差，所谓静差，就是系统稳定后输出值和设定值之间的差值，
//	  积分环节实际上就是偏差累计的过程，把累计的误差加到原有系统上以抵消系统造成的静差。
//	4.而微分信号则反应了偏差信号的变化规律，或者说是变化趋势，根据偏差信息的变化趋
//	  势来进行调节，从而增加了系统的快速性。
//	好了，关于PID的基本说明就补充到这里，下面将对PID连续系统离散化，从而方便在处理
//	器上实现。下面把连续状态的公式再贴一下：
//
//	假设采样间隔为T，则在第KT时刻：
//	Err(K)=rin(K)-rout(K);
//	积分环节用加和的形式表示，即Err(K)+Err(K+1)+……;
//	微分环节用斜率的形式表示，即[Err(K)-Err(K-1)]/T;
//	从而形成如下PID离散表示形式：
//	则u(K)可表示成为：
//	至于说Kp、Ki、Kd三个参数的具体表达式，我想可以轻松的推出了，这里节省时间，不再
//	详细表示了。其实到这里为止，PID的基本离散表示形式已经出来了。目前的这种表述形式
//	属于位置型PID，另外一种表述方式为增量式PID，由U上述表达式可以轻易得到：
//	那么：
//	这就是离散化PID的增量式表示方式，由公式可以看出，增量式的表达结果只和最近三次的
//	偏差差有关，这样就大大提高了系统的稳定性。需要注意的是最终的输出结果应该为
//	u(K)+增量调节值;
//	PID的离散化过程基本思路就是这样，下面是将离散化的公式转换成为C语言，从而实现
//	微控制	器的控制作用。
//
////////////////////////////////////////////////////////////
//PID控制算法的C语言实现三 位置型PID的C语言实现
//   上一节中已经抽象出了位置性PID和增量型PID的数学表达式，这一节，重点讲解C语言代码
//   的实现过程，算法的C语言实现过程具有一般性，通过PID算法的C语言实现，可以以此类推，
//   设计其它算法的C语言实现。
//   第一步：定义PID变量结构体，代码如下：
//struct _pid{
//    float Sv;				//定义设定值
//    float Pv;			//定义实际值
//    float Err;					//定义偏差值
//    float LastErr;				//定义上一个偏差值
//    float Kp,Ki,Kd;				//定义比例、积分、微分系数
//    float Result;				//定义电压值（控制执行器的变量）
//    float Integral;				//定义积分值
//}pid;
//	控制算法中所需要用到的参数在一个结构体中统一定义，方便后面的使用。
//	第二部：初始化变量，代码如下：
//void PID_init(){
//    printf("PID_init begin \n");
//    pid.Sv=0.0;
//    pid.Pv=0.0;
//    pid.Err=0.0;
//    pid.LastErr=0.0;
//    pid.Result=0.0;
//    pid.Integral=0.0;
//    pid.Kp=0.2;
//    pid.Ki=0.015;
//    pid.Kd=0.2;
//    printf("PID_init end \n");
//}
//统一初始化变量，尤其是Kp,Ki,Kd三个参数，调试过程当中，对于要求的控制效果，可以通过
//调节这三个量直接进行调节。
//第三步：编写控制算法，代码如下：
//float PID_Realize(float Sv){
//    pid.Sv=Sv;
//    pid.Err=pid.Sv-pid.Pv;
//    pid.Integral+=pid.Err;
//    pid.Result		=	pid.Kp * pid.Err					+
//						pid.Ki * pid.Integral				+
//						pid.Kd *(pid.Err	  - pid.LastErr);
//    pid.LastErr=pid.Err;
//    pid.Pv=pid.Result*1.0;
//    return pid.Pv;
//}
//注意：这里用了最基本的算法实现形式，没有考虑死区问题，没有设定上下限，只是对公式的
//一种直接的实现，后面的介绍当中还会逐渐的对此改进。
//   到此为止，PID的基本实现部分就初步完成了。下面是测试代码：
//int main(){
//    printf("System begin \n");
//    PID_init();
//    int count=0;
//    while(count<1000)
//    {
//        float Sv=PID_Realize(200.0);
//        printf("%f\n",Sv);
//        count++;
//    }
//return 0;
//}
//
//PID控制算法的C语言实现四 增量型PID的C语言实现
//   上一节中介绍了最简单的位置型PID的实现手段，这一节主要讲解增量式PID的实现方法，
//   位置型和增量型PID的数学公式请参见我的系列文《PID控制算法的C语言实现二》中的讲解。
//   实现过程仍然是分为定义变量、初始化变量、实现控制算法函数、算法测试四个部分，
//   详细分类请参加《PID控制算法的C语言实现三》中的讲解，这里直接给出代码了。
//
///*
// * PID.c
// *
// *  Created on: 2011-11-7
// *      Author: wang
// */
//#include<stdio.h>
//#include<stdlib.h>
//
//struct _pid{
//    float Sv;            	//定义设定值
//    float Pv;        	//定义实际值
//    float Err;                	//定义偏差值
//    float Err_next;            	//定义上一个偏差值
//    float LastErr;            	//定义最上前的偏差值
//    float Kp,Ki,Kd;            	//定义比例、积分、微分系数
//}pid;
//
//void PID_Init(){
//    pid.Sv=0.0;
//    pid.Pv=0.0;
//    pid.Err=0.0;
//    pid.LastErr=0.0;
//    pid.Err_next=0.0;
//    pid.Kp=0.2;
//    pid.Ki=0.015;
//    pid.Kd=0.2;
//}
//
//float PID_Realize(float Sv){
//    pid.Sv			= Sv;
//    pid.Err					= pid.Sv-pid.Pv;
//    float incrementSpeed	=
//						pid.Kp * (pid.Err	  - pid.Err_next) + \
//						pid.Ki * (pid.Err) 					+ \
//						pid.Kd * (pid.Err 	  - 2*pid.Err_next + pid.LastErr);
//    pid.Pv		   += incrementSpeed;
//    pid.LastErr			= pid.Err_next;
//    pid.Err_next 			= pid.Err;
//
//    return pid.Pv;
//}
//
//int main(){
//    PID_Init();
//    int count=0;
//
//    while(count<1000)
//    {
//        float Sv	= PID_Realize(200.0);
//        printf("%f\n",Sv);
//        count++;
//    }
//    return 0;
//}
//
//PID控制算法的C语言实现五 积分分离的PID控制算法C语言实现
//    通过三、四两篇文章，基本上已经弄清楚了PID控制算法的最常规的表达方法。
//	在普通PID控制中，引入积分环节的目的，主要是为了消除静差，提高控制精度。
//	但是在启动、结束或大幅度增减设定时，短时间内系统输出有很大的偏差，
//	会造成PID运算的积分积累，导致控制量超过执行机构可能允许的最大动作范围对应极限
//	控制量，	从而引起较大的超调，甚至是震荡，这是绝对不允许的。为了克服这一问题，
//	引入了积分分离的概念，其基本思路是 当被控量与设定值偏差较大时，取消积分作用;
//	当被控量接近给定值时，引入积分控制，以消除静差，提高精度。其具体实现代码如下：
//    pid.Kp=0.2;
//    pid.Ki=0.04;
//    pid.Kd=0.2;  //初始化过程
//
//	if(abs(pid.Err)>200) {
//		IntegralEn=0;
//    }else{
//		IntegralEn=1;
//		pid.Integral+=pid.Err;
//    }
//    pid.Result		= 	pid.Kp * pid.Err					+
//						pid.Ki * pid.Integral * IntegralEn		+
//						pid.Kd *(pid.Err	  - pid.LastErr);    //算法具体实现过程
//
//	其它部分的代码参见《PID控制算法的C语言实现三》中的讲解，不再赘述。同样采集1000
//	个量，会发现，系统道道199所有的时间是原来时间的1/2,系统的快速性得到了提高。
//
//PID控制算法的C语言实现六 抗积分饱和的PID控制算法C语言实现
//	所谓的积分饱和现象是指如果系统存在一个方向的偏差，PID控制器的输出由于积分作用的
//	不断累加而加大，从而导致执行机构达到极限位置，若控制器输出U(k)继续增大，执行
//	器开度不可能再增大，此时计算机输出控制量超出了正常运行范围而进入饱和区。一旦系统
//	出现反向偏差，u(k)逐渐从饱和区退出。进入饱和区越深则退出饱和区时间越长。在这段时
//	间里，执行机构仍然停留在极限位置而不随偏差反向而立即做出相应的改变，这时系统就像
//	失控一样，造成控制性能恶化，这种现象称为积分饱和现象或积分失控现象。防止积分饱和
//	的方法之一就是抗积分饱和法，该方法的思路是在计算u(k)时，首先判断上一时刻的控制量
//	u(k-1)是否已经超出了极限范围：
//	如果u(k-1)>Max，则只累加负偏差;
//	如果u(k-1)<Min，则只累加正偏差。
//	从而避免控制量长时间停留在饱和区。直接贴出代码，不懂的看看前面几节的介绍。
//struct _pid{
//	float Sv;				//定义设定值
//	float Pv;			//定义实际值
//	float Err;					//定义偏差值
//	float LastErr;				//定义上一个偏差值
//	float Kp,Ki,Kd;				//定义比例、积分、微分系数
//	float Result;				//定义电压值（控制执行器的变量）
//	float Integral;				//定义积分值
//	float Max;
//	float Min;
//}pid;
//
//void PID_init(){
//	printf("PID_init begin \n");
//	pid.Sv=0.0;
//	pid.Pv=0.0;
//	pid.Err=0.0;
//	pid.LastErr=0.0;
//	pid.Result=0.0;
//	pid.Integral=0.0;
//	pid.Kp=0.2;
//	pid.Ki=0.1;       //注意，和上几次相比，这里加大了积分环节的值
//	pid.Kd=0.2;
//	pid.Max=400;
//	pid.Min=-200;
//	printf("PID_init end \n");
//}
//
//float PID_Realize(float Sv){
//	int IntegralEn;
//	pid.Sv	= Sv;
//	pid.Err			= pid.Sv - pid.Pv;
//
//	if(pid.Pv > pid.Max) {         //灰色底色表示抗积分饱和的实现
//		if(abs(pid.Err) > 200) {                   //蓝色标注为积分分离过程
//			IntegralEn=0;
//		} else {
//			IntegralEn=1;
//			if(pid.Err<0) {
//				pid.Integral	+= pid.Err;
//			}
//		}
//	} else if (pid.Pv < pid.Max) {
//		if(abs(pid.Err) > 200) {                   //积分分离过程
//			IntegralEn=0;
//		} else {
//			IntegralEn=1;
//			if(pid.Err > 0) {
//				pid.Integral	+= pid.Err;
//			}
//		}
//	} else {
//		if(abs(pid.Err) > 200) {                   //积分分离过程
//			IntegralEn=0;
//		} else {
//			IntegralEn=1;
//			pid.Integral	+= pid.Err;
//		}
//	}
//
//	pid.Result		=	pid.Kp * pid.Err					+
//						pid.Ki * pid.Integral * IntegralEn		+
//						pid.Kd *(pid.Err	  - pid.LastErr);
//
//	pid.LastErr	= pid.Err;
//	pid.Pv	= pid.Result * 1.0;
//
//	return pid.Pv;
//}
//最终的测试程序运算结果如下，可以明显的看出系统的稳定时间相对前几次来讲缩短了不少。
//
//PID控制算法的C语言实现七 梯形积分的PID控制算法C语言实现
//   先看一下梯形算法的积分环节公式
//
//   作为PID控制律的积分项，其作用是消除余差，为了尽量减小余差，应提高积分项运算精度，
//   为此可以将矩形积分改为梯形积分，具体实现的语句为：
//	pid.Result		=	pid.Kp * pid.Err					+
//						pid.Ki * pid.Integral * IntegralEn/2		+
//						pid.Kd *(pid.Err 	  - pid.LastErr);  //梯形积分
//	其它函数请参见本系列教程六中的介绍
//	最后运算的稳定数据为：199.999890，较教程六中的199.9999390而言，精度进一步提高。
//
//
//PID控制算法的C语言实现八 变积分的PID控制算法C语言实现
//   变积分PID可以看成是积分分离的PID算法的更一般的形式。在普通的PID控制算法中，由于
//   积分系数ki是常数，   所以在整个控制过程中，积分增量是不变的。但是，系统对于积分
//   项的要求是，系统偏差大时，积分作用应该减弱甚至是全无，   而在偏差小时，则应该加
//   强。积分系数取大了会产生超调，甚至积分饱和，取小了又不能短时间内消除静差。因此，
//   根据系统的偏差大小改变积分速度是有必要的。变积分PID的基本思想是设法改变积分项的
//   累加速度，使其与偏差大小相对应：偏差越大，积分越慢; 偏差越小，积分越快。
//   这里给积分系数前加上一个比例值IntegralEn：
//   当abs(Err)<180时，IntegralEn=1;
//   当180<abs(Err)<200时，IntegralEn=（200-abs(Err)）/20;
//   当abs(Err)>200时，IntegralEn=0;
//   最终的比例环节的比例系数值为ki*IntegralEn;
//   具体PID实现代码如下：
//    pid.Kp=0.4;
//    pid.Ki=0.2;    //增加了积分系数
//    pid.Kd=0.2;
//
//   float PID_Realize(float Sv){
//    float IntegralEn;
//    pid.Sv=Sv;
//    pid.Err=pid.Sv-pid.Pv;
//
//    if(abs(pid.Err)>200)                    //变积分过程
//    {
//    IntegralEn=0.0;
//    }else if(abs(pid.Err)<180){
//    IntegralEn=1.0;
//    pid.Integral+=pid.Err;
//    }else{
//    IntegralEn=(200-abs(pid.Err))/20;
//    pid.Integral+=pid.Err;
//    }
//    pid.Result		=	pid.Kp * pid.Err				+
//						pid.Ki * pid.Integral * IntegralEn	+
//						pid.Kd * (pid.Err - pid.LastErr);
//
//    pid.LastErr=pid.Err;
//    pid.Pv=pid.Result*1.0;
//    return pid.Pv;
//}
//最终结果可以看出，系统的稳定速度非常快（测试程序参见本系列教程3）：
//
//
//
//    pid.Result		=	pid.Kp * pid.Err					+
//						pid.Ki * pid.Integral				+
//						pid.Kd *(pid.Err	  - pid.LastErr);
//    float incrementSpeed	=
//						pid.Kp * (pid.Err	  - pid.Err_next) + \
//						pid.Ki * (pid.Err) 					+ \
//						pid.Kd * (pid.Err 	  - 2*pid.Err_next + pid.LastErr);
//    pid.Result		= 	pid.Kp * pid.Err					+
//						pid.Ki * pid.Integral * IntegralEn		+
//						pid.Kd *(pid.Err	  - pid.LastErr);    //算法具体实现过程
//	pid.Result		=	pid.Kp * pid.Err					+
//						pid.Ki * pid.Integral * IntegralEn		+
//						pid.Kd *(pid.Err	  - pid.LastErr);
//	pid.Result		=	pid.Kp * pid.Err					+
//						pid.Ki * pid.Integral * IntegralEn/2		+
//						pid.Kd *(pid.Err 	  - pid.LastErr);  //梯形积分
//    pid.Result		=	pid.Kp * pid.Err					+
//						pid.Ki * pid.Integral * IntegralEn		+
//						pid.Kd * (pid.Err 	  - pid.LastErr);
/*******************************************************************************
 * 				end of file
 *******************************************************************************/
#endif