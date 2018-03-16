/*******************************************************************************
*   Filename:       bsp_time.C
*   Revised:        $Date: 2014-05-27$
*   Revision:       $V1.0
*	Writer:		    wumingshen.
*
*   Description:
*   本文件实现基于RTC的日期功能，提供年月日的读写。（基于ANSI-C的time.h）
*
*   RTC中保存的时间格式，是UNIX时间戳格式的。即一个32bit的TIME_t变量（实为u32）
*
*   ANSI-C的标准库中，提供了两种表示时间的数据  型：
*   time_t:    UNIX时间戳（从1970-1-1起到某时间经过的秒数）
*   typedef unsigned int time_t;
*
*   struct tm: Calendar格式（年月日形式）
*   tm结构如下：
*   struct tm {
*   int tm_sec;   // 秒   seconds after the minute, 0 to 60
*                 (0 - 60 allows for the occasional leap second)
*   int tm_min;   // 分   minutes after the hour, 0 to 59
*   int tm_hour;  // 时   hours since midnight, 0 to 23
*   int tm_mday;  // 日   day of the month, 1 to 31
*   int tm_mon;   // 月   months since January, 0 to 11
*   int tm_year;  // 年   years since 1900
*   int tm_wday;  // 星期 days since Sunday, 0 to 6
*   int tm_yday;  // 从元旦起的天数 days since January 1, 0 to 365
*   int tm_isdst; // 夏令时         Daylight Savings Time flag
*   ...
* }
*  其中wday，yday可以自动产生，软件直接读取
* ***注意***：
*  tm_year:在time.h库中定义为1900年起的年份，即2008年应表示为2008-1900=108
*  这种表示方法对用户来说不是十分友好，与现实有较大差异。
*  所以在本文件中，屏蔽了这种差异。
*  即外部调用本文件的函数时，tm结构体类型的日期，tm_year即为2008
*  注意：若要调用系统库time.c中的函数，需要自行将tm_year-=1900
*
* 成员函数说明：
* struct tm TIME_ConvUnixToCalendar(time_t t);
* 输入一个Unix时间戳（TIME_t），返回Calendar格式日期
* time_t TIME_ConvCalendarToUnix(struct tm t);
* 输入一个Calendar格式日期，返回Unix时间戳（TIME_t）
* time_t TIME_GetUnixTime(void);
* 从RTC取当前时间的Unix时间戳值
* struct tm TIME_GetCalendarTime(void);
* 从RTC取当前时间的日历时间
* void TIME_SetUnixTime(time_t);
* 输入UNIX时间戳格式时间，设置为当前RTC时间
* void TIME_SetCalendarTime(struct tm t);
* 输入Calendar格式时间，设置为当前RTC时间
*
* 外部调用实例：
* 定义一个Calendar格式的日期变量：
* struct tm now;
* now.tm_year = 2015;
* now.tm_mon  = 05;
* now.tm_mday = 20;
* now.tm_hour = 20;
* now.tm_min  = 12;
* now.tm_sec  = 30;
*
* 获取当前日期时间：
* tm_now = TIME_GetCalendarTime();
* 然后可以直接读tm_now.tm_wday获取星期数
*
* 设置时间：
* Step1. tm_now.xxx = xxxxxxxxx;
* Step2. TIME_SetCalendarTime(tm_now);
*
* 计算两个时间的差
* struct tm t1,t2;
* t1_t = TIME_ConvCalendarToUnix(t1);
* t2_t = TIME_ConvCalendarToUnix(t2);
* dt = t1_t - t2_t;
* dt就是两个时间差的秒数
* dt_tm = mktime(dt); //注意dt的年份匹配，ansi库中函数为相对年份，注意超限
* 另可以参考相关资料，调用ansi-c库的格式化输出等功能，ctime，strftime等
*
* Notes:
*
* All copyrights reserved to wumingshen.
*
*******************************************************************************/

/*******************************************************************************
 * INCLUDES
 */
#include <bsp_time.h>
#include "bsp_rtc.h"
#include "bsp_r8025t.h"

/*******************************************************************************
 * GLOBAL VARIABLES
 */
struct  tm   tm_now;



//============================================================================//

/*******************************************************************************
 * 名    称： TIME_ConvUnixToCalendar
 * 功    能： 转换UNIX时间戳为日历时间
 * 入口参数： time_t t 当前日历时间
 * 出口参数： UniX时间
 * 作　 　者： 无名沈.
 * 创建日期： 2015-06-25
 * 修    改：
 * 修改日期：
 * 备    注：
 *******************************************************************************/
struct tm TIME_ConvUnixToCalendar(time_t t)
{
  struct tm *t_tm;
  t_tm = localtime(&t);
  t_tm->tm_year += 1900; //localtime转换结果的tm_year是相对值，需要转成绝对值
  return *t_tm;
}

/*******************************************************************************
 * 名    称： TIME_ConvCalendarToUnix
 * 功    能： 转换日历时间为UNIX时间戳
 * 入口参数： 当前UNIX时间
 * 出口参数： 当前日历时间
 * 作　 　者： 无名沈.
 * 创建日期： 2015-06-25
 * 修    改：
 * 修改日期：
 * 备    注：
 *******************************************************************************/
time_t TIME_ConvCalendarToUnix(struct tm t)
{
  t.tm_year -= 1900;  //外部tm结构体存储的年份为2008格式
                      //而time.h中定义的年份格式为1900年开始的年份
                      //所以，在日期转换时要考虑到这个因素。
  return mktime(&t);
}

/*******************************************************************************
 * 名    称： TIME_GetUnixTime
 * 功    能： 从RTC取当前时间的Unix时间戳值
 * 入口参数： 无
 * 出口参数： time_t t
 * 作　 　者： 无名沈.
 * 创建日期： 2015-06-25
 * 修    改：
 * 修改日期：
 * 备    注：
 *******************************************************************************/
time_t TIME_GetUnixTime(void)
{
  return (time_t)RTC_GetCounter();
}

/*******************************************************************************
 * 名    称： is_leap_year
 * 功    能： 判断是否是闰年函数
 * 入口参数： 现在的年份
 * 出口参数： 该年份是不是闰年.1,是.0,不是
 * 作　 　者： 无名沈.
 * 创建日期： 2015-06-25
 * 修    改：
 * 修改日期：
 * 备    注：
 *******************************************************************************/
int is_leap_year( int year )
{
    int    iRet = 0;

    if( 0 == ( year % 400 ) ) {
        iRet = 1;
    } else if( ( 0 == ( year % 4 ) ) && ( 0 != ( year % 100 ) ) ) {
        iRet = 1;
    }
    return( iRet );
}

/*******************************************************************************
 * 名    称： RTC_Get_Week
 * 功    能： 获得现在是星期几
 * 入口参数： 输入公历日期得到星期(只允许1901-2099年)
 * 出口参数： tm_wday 星期号(0-6)
 * 作　 　者： 无名沈.
 * 创建日期： 2015-06-25
 * 修    改：
 * 修改日期：
 * 备    注： 0表示周日，1-6表示周一到周六
 *******************************************************************************/
int RTC_Get_Week( int year,int month,int day )
{

    int iWeek, iConst, iYear;
    int aiRate[13] = { 0, 0, 3, 3, 6, 1, 4, 6, 2, 5, 0, 3, 5 };

    iYear = year % 400;

    if(is_leap_year( year ) && ( month < 3 ) ) {
        iConst = 5;
    } else {
        iConst = 6;
    }
    iWeek = ( iYear + iYear / 4 - iYear / 100 + aiRate[month] + day + iConst ) % 7;

    return( iWeek );
}

/*******************************************************************************
 * 名    称： TIME_SetUnixTime
 * 功    能： 将给定的Unix时间戳写入RTC
 * 入口参数： time_t t 时间结构体
 * 出口参数： 无
 * 作　 　者： 无名沈.
 * 创建日期： 2015-06-25
 * 修    改：
 * 修改日期：
 * 备    注：
 *******************************************************************************/
void TIME_SetUnixTime(time_t t)
{
  PWR_BackupAccessCmd(ENABLE);
  RTC_WaitForLastTask();
  RTC_SetCounter((u32)t);
  RTC_WaitForLastTask();
  PWR_BackupAccessCmd(DISABLE);
}

/*******************************************************************************
 * 名    称： BSP_TIME_Init
 * 功    能： 获取实时时间初始化
 * 入口参数： 无
 * 出口参数： 无
 * 作　 　者： 无名沈.
 * 创建日期： 2015-06-25
 * 修    改：
 * 修改日期：
 * 备    注： 初始化RTC
 *******************************************************************************/
void BSP_TIME_Init(void)
{
    /***********************************************
    * 描述： 初始化内部RTC
    */
    BSP_RTC_Init();
    /***********************************************
    * 描述： 读取内部RTC时间
    */
    tm_now  = TIME_GetCalendarTime();

//    //--------------------------------------------//
//    /***********************************************
//    * 描述： 初始化外部RTC
//    */
//    BSP_RX8025T_Init();
//    /***********************************************
//    * 描述： 读取外部RTC时间
//    */
//    BSP_RX8025T_GetTime(&tm_now);


}

/*******************************************************************************
 * 名    称： TIME_GetCalendarTime
 * 功    能： 从RTC取当前时间的日历时间（struct tm）
 * 入口参数： 无
 * 出口参数： 当前实时时间
 * 作　 　者： 无名沈.
 * 创建日期： 2015-06-25
 * 修    改：
 * 修改日期：
 * 备    注：
 *******************************************************************************/
struct tm TIME_GetCalendarTime(void)
{
    struct tm t_tm;
    /***********************************************
    * 描述： 读取内部RTC实时时间
    */
    time_t    t_t;
    t_t   = (time_t)RTC_GetCounter();
    t_tm  = TIME_ConvUnixToCalendar(t_t);
    t_tm.tm_wday = RTC_Get_Week(t_tm.tm_year,t_tm.tm_mon,t_tm.tm_mday);

//    //--------------------------------------------//
//    /***********************************************
//    * 描述： 读取外部RTC实时时间
//    */
//    BSP_RX8025T_GetTime(&t_tm);

    return t_tm;
}

/*******************************************************************************
 * 名    称： TIME_SetCalendarTime
 * 功    能： 将给定的Calendar格式时间转换成UNIX时间戳写入RTC
 * 入口参数： 无
 * 出口参数： 无
 * 作　 　者： 无名沈.
 * 创建日期： 2015-06-25
 * 修    改：
 * 修改日期：
 * 备    注：
 *******************************************************************************/
void TIME_SetCalendarTime(struct tm t)
{
    /***********************************************
    * 描述： 修改内部RTC实时时间
    */
    TIME_SetUnixTime(TIME_ConvCalendarToUnix(t));

//    //--------------------------------------------//
//    /***********************************************
//    * 描述： 修改外部RTC实时时间
//    */
//    BSP_RX8025T_SetDate(t);


}


/*******************************************************************************
 *              end of file                                                    *
 *******************************************************************************/