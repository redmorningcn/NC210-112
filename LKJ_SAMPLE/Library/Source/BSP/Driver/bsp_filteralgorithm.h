#ifndef BSP_FILTERALGORITHM_H__
#define BSP_FILTERALGORITHM_H__
/*******************************************************************************
 *   Filename:       bsp_filteralgorithm.h
 *   Revised:        All copyrights reserved to Roger.
 *   Date:           2015-05-11
 *   Revision:       v1.0
 *   Writer:	     wumingshen.
 *
 *   Description:    滤波算法
 *
 *
 *   Notes:
 *
 *******************************************************************************/
#include "includes.h"

#define FILTER_BUF_LEN      30

typedef struct {
    INT16U  Nbr                ;        //
    INT16U  Idx                ;        // 计数器
    INT16S  Buf[FILTER_BUF_LEN];        // 缓存数据数组
} StrDataFilter;


INT16U  App_GetTimeMeanFiler        (StrDataFilter *filter, INT16S dat);

void    App_BubbleAscendingArray    (INT16U *needbuf,INT16U bufsize,INT16U *afterbuf);

INT16U  App_GetFilterValue          (INT16U *needbuf, INT16U *afterbuf, INT16U bufsize, INT16U needsize);

#endif
/******************************************************************************/
//                              end of file                                   //
/******************************************************************************/