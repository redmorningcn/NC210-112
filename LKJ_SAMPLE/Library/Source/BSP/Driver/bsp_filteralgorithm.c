/*******************************************************************************
 *   Filename:       bsp_filteralgorithm.c
 *   Revised:        All copyrights reserved to Roger.
 *   Date:           2015-05-11
 *   Revision:       v1.0
 *   Writer:	     Roger-WY.
 *
 *   Description:    滤波算法
 *
 *
 *   Notes:
 *
 *******************************************************************************/
#include "bsp_filteralgorithm.h"

//============================================================================//

/*******************************************************************************
* 名    称： App_ModbusQPost
* 功    能： 时间平滑平均滤波
* 入口参数： filter：缓存一段时间内的dat值   dat:当前测量值
* 出口参数： 经过平滑平均函数之后的dat值
* 作　 　者： 无名沈
* 创建日期： 2015-05-30
* 修    改：
* 修改日期：
* 备    注： 取一段时间内所有的dat值，计算时间段内所有数据的平均值
*            例如：StrDataFilter    TempFilter = {8,0,{0}}; 8代表取缓存数组的前多少个值进行平均
*******************************************************************************/
INT16U App_GetTimeMeanFiler(StrDataFilter *filter, INT16S dat)
{
    filter->Buf[filter->Idx]  = dat;

    if ( ++filter->Idx >= filter->Nbr ) {
        filter->Idx = 0;
    } else {

    }
    double  sum = 0.0;
    INT16U  nbr = 0;

    for ( int i = 0; i < filter->Nbr; i++ ) {
        if ( filter->Buf[i] > 0 ) {
            sum += filter->Buf[i];
            nbr++;
        }
    }
    if (nbr)
        sum /= nbr;
    else
        sum  = dat;

    return  (INT16U)sum;
}


/*******************************************************************************
* 名    称： App_BubbleAscendingArray
* 功    能： 冒泡法升序排列
* 入口参数： buf： 需要排序的数组   bufsize:数组的大小
* 出口参数： 排序之后的数组
* 作　 　者： 无名沈
* 创建日期： 2015-05-30
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
void App_BubbleAscendingArray(INT16U *needbuf,INT16U bufsize,INT16U *afterbuf)
{
    INT16U i, j;
    INT16U usTemp;

    for(i = 0; i < bufsize; i++) {
      afterbuf[i] = *(needbuf+i);
    }
    for(i = 0; i < bufsize - 1; i++) {
        for(j = i + 1; j < bufsize; j++) {
            if(afterbuf[i] > afterbuf[j]) {
                usTemp = afterbuf[i];
                afterbuf[i] = afterbuf[j];
                afterbuf[j] = usTemp;
            }
        }
    }
}

/*******************************************************************************
* 名    称： App_BubbleAscendingArray
* 功    能： 冒泡法升序排列
* 入口参数： buf： 需要排序的数组   bufsize:数组的大小
* 出口参数： 排序之后的数组
* 作　 　者： 无名沈
* 创建日期： 2015-05-30
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
INT16U App_GetFilterValue(INT16U *needbuf, INT16U *afterbuf, INT16U bufsize, INT16U needsize)
{
    INT16U  i, j;
    INT16U  usTemp;          
    u16     cutsize     = 0;
    INT32U  sum         = 0;  
    u16     cnt         = 0;
    
    /***********************************************
    * 描述： 过滤
    */
    if ( bufsize == 0 ) {
        return 0;
    }  
        
    if ( 0 == needsize ) {
        return 0;
    }

    /***********************************************
    * 描述： 不做删减
    */
    if ( bufsize == needsize ) {
        for(u16 i = cutsize; i < bufsize - cutsize;i++) {
            if ( needbuf[i] != 0 ) {
                sum += needbuf[i];
                cnt++;
            }
        }
        
        return (INT16U)(sum/cnt);            // 返回平均值
    }
    
    /***********************************************
    * 描述： 
    */
    for(i = 0; i < bufsize; i++) {
      afterbuf[i] = *(needbuf+i);
    }
    
    /***********************************************
    * 描述： 排序
    */
    for(i = 0; i < bufsize - 1; i++) {
        for(j = i + 1; j < bufsize; j++) {
            if(afterbuf[i] > afterbuf[j]) {
                usTemp = afterbuf[i];
                afterbuf[i] = afterbuf[j];
                afterbuf[j] = usTemp;
            }
        }
    }    
    
    /***********************************************
    * 描述： 获取删除数量
    */
    cutsize     = (bufsize - needsize) / 2;
    /***********************************************
    * 描述： 求和
    */
    for(u8 i = cutsize; i < bufsize - cutsize;i++) {
        sum += afterbuf[i];
        cnt++;
    }
    
    return (INT16U)(sum/(cnt));                     // 返回平均值
}

/******************************************************************************/
//                              end of file                                   //
/******************************************************************************/