/*******************************************************************************
 *   Filename:       app_lib.c
 *   Revised:        All copyrights reserved to Wuming Shen.
 *   Date:           2014-07-05
 *   Revision:       v1.0
 *   Writer:	     Wuming Shen.
 *
 *   Description:    双击选中 wdt 按 Ctrl + H, 钩选 Match the case, Replace with
 *                   输入您要的名字，点击 Replace All
 *                   双击选中 Wdt 按 Ctrl + H, 钩选 Match the case, Replace with
 *                   输入您要的名字，点击 Replace All
 *                   双击选中 WDT 按 Ctrl + H, 钩选 Match the case, Replace with
 *                   输入您要的名字，点击 Replace All
 *                   在 app_cfg.h 中指定任务的优先级 和 任务堆栈大小
 *
 *   Notes:
 *					QQ:276193028
 *     				E-mail:shenchangwei945@163.com
 *
 *******************************************************************************/

/*******************************************************************************
 * INCLUDES
 */
#define  SNL_APP_SOURCE
#include <app.h>
#include <CrcCheck.h>
#include <bsp_usart.h>

#define BSP_COMM_PROTOCOL_MODULE_EN 1
#if BSP_COMM_PROTOCOL_MODULE_EN > 0

#ifdef VSC_INCLUDE_SOURCE_FILE_NAMES
const  CPU_CHAR  *app_comm_protocol__c = "$Id: $";
#endif

/*******************************************************************************
 * CONSTANTS
 */
#define         FRAM_HERD0     		0x10         	//报文头
#define         FRAM_HERD1     		0x28       		//报文头				
#define     	HOST_ID      		10				//上位机地址	  		   
#define   		DEVICE_ID0      	15				//本机地址0	
#define   		DEVICE_ID1      	15				//本机地址1	
	   
#define			RS485_CHK_SUM		0x02			//b0001：CRC方式；b0010：累加和方式； b0011;累加和二进制补码方式 
#define			RS485_CHK_CRC		0x01			//b0001：CRC方式；b0010：累加和方式； b0011;累加和二进制补码方式 
#define			RS485_CHK_RESUM		0x03			//b0001：CRC方式；b0010：累加和方式； b0011;累加和二进制补码方式 

#define         FRAM_END0     		0x10         	//报文尾
#define         FRAM_END1     		0x2C       		//报文尾	


#define		FRAM_GAP			0x7e

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
 
unsigned char   l_recslaveaddr = 0;

/***********************************************
* 描述： 
*/
unsigned char GetRecSlaveAddr(void)
{
	return	l_recslaveaddr;
}

static uint8	RecvFrameNbr = 0;
/***********************************************
* 描述： 
*/
uint8	GetRecvFrameNbr(void)
{
	return 	RecvFrameNbr;
}


/***********************************************
* 描述： 
*/
void	SetRecvFrameNbr(void)
{
	RecvFrameNbr++;		
}

/*******************************************************************************
* 名    称： GetCheckSumNR
* 功    能： 
* 入口参数： 无
* 出口参数： 无
* 作　 　者： wumingshen.
* 创建日期： 2015-12-07
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
uint8 GetCheckSumNR(uint8  Buf[], uint32 Len)
{
    int32  i;
    int8   CheckSum = 0;
    
    for(i = 0;i < Len; i++) {
        CheckSum += Buf[i];                         //求和
    }
    
    if((unsigned char)CheckSum >= 0x80) {
    	CheckSum = ~CheckSum + 1 +0x80;             //取反+1
    } else {
    	CheckSum = CheckSum;                       	//取反+1
    }
    return  CheckSum;
}

/*******************************************************************************
* 名    称： CSNC_GetData
* 功    能： 
* 入口参数： 无
* 出口参数： 无
* 作　 　者： wumingshen.
* 创建日期： 2015-12-07
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
unsigned char  CSNC_GetData(	INT08U *RecBuf,
								INT16U	RecLen,
								INT08U *DataBuf,
								INT16U *InfoLen)
{
    unsigned short		i,j,k;
    unsigned char		SumCheck8;
    unsigned char		Tmp;
    unsigned short		RecCRC16,Check16;
		
	unsigned char 		DstAddr;	
	unsigned short		DataLen;  
    unsigned char     *pDat         = RecBuf;
    
    unsigned short    MaxDataLen    = RecLen - 12;
    
    *InfoLen            = 0;
    //StrCommRecvPack *Rd = (StrCommRecvPack *)DataBuf;
    DataBuf = DataBuf;
    /***********************************************
    * 描述： 识别帧头
    */
    for(i = 0; i < MaxDataLen; i++)	{		        //循环遍历
		if(	( (RecBuf[i + 0] ) == FRAM_HERD0 ) && 
            ( (RecBuf[i + 1] ) == FRAM_HERD1 ) ) {
            pDat    = &RecBuf[i];
            goto next;
        }
    }
    return 0;
next:
    /***********************************************
    * 描述： 接收数据去掉帧头识别字0X10的重复部分
    */
	for(j = 0;j < RecLen ;j++ )	{				    //去重复通讯首字节
        if( ( pDat[j]   == FRAM_HERD0 ) &&
            ( pDat[j+1] == FRAM_HERD0 ) ) {
            for(k = j; k < RecLen; k++) {
               Tmp	        = pDat[k+1];
               pDat[k]      = Tmp;
            }
        }
	}
    pDat    += 6;
    DataLen  = *pDat;
    // 长度超范围
    if ( DataLen > MaxDataLen )
        return 0;
    
    pDat    += DataLen + 4;
    /***********************************************
    * 描述： 检查帧尾
    */
    if( ( pDat[0] 	!= FRAM_END0)	||
        ( pDat[1] 	!= FRAM_END1) ) {
        return 0;
    }
    
    pDat            = &RecBuf[i];
    l_recslaveaddr 	= pDat[3];
    DstAddr         = l_recslaveaddr;
    RecvFrameNbr   	= pDat[4];
    *InfoLen 		= DataLen; 

    SumCheck8 		= GetCheckSumNR((unsigned char *)&pDat[2],5);	//索引区校验

    RecCRC16 		= ((unsigned short)pDat[8+DataLen])
                    * 256 + pDat[9+DataLen];	        //校验	
               
    switch(pDat[5]>>4) {							    //校验和算法判断
    case	RS485_CHK_SUM:
        break;
    case	RS485_CHK_CRC:
       Check16 = GetCrc16Check(&pDat[2],6+DataLen);;
        break;
    case	RS485_CHK_RESUM:
        break;
    }
    
    if(	(SumCheck8 == 	pDat[7] ) && 
        (RecCRC16  ==	Check16 ) ) {
        /***********************************************
        * 描述： 获取帧头
        */
        //memcpy( (INT08U *)&Rd.Head, (INT08U *)&pDat[0], 8 );

        /***********************************************
        * 描述： 获取数据
        */       
        //memcpy( (INT08U *)DataBuf,  (INT08U *)&pDat[8], DataLen);					//数据拷贝
        DataBuf     = (INT08U *)&pDat[8];
        return		DstAddr;//SourceAddr;
    }
               
    return	0;
}

static uint8	SendFrameNbr = 0;

/***********************************************
* 描述： 
*/
uint8	GetSendFrameNbr(void)
{
	return SendFrameNbr;		
}

/***********************************************
* 描述： 
*/
void	SetSendFrameNbr(void)
{
	SendFrameNbr++;		
}

/*******************************************************************************
* 名    称： CSNC_SendData
* 功    能： 
* 入口参数： 无
* 出口参数： 无
* 作  　者： wumingshen.
* 创建日期： 2015-12-07
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
void	CSNC_SendData(	MODBUS_CH *pch,
                        INT08U  SourceAddr,
						INT08U  DistAddr,
						INT08U *DataBuf,
						INT16U  DataLen)
{
	unsigned char	SndBuf[250] = {0};
	unsigned char	SndBufTmp[250] = {0};
	unsigned char	SumCheck8;
	unsigned short	Crc16;
	unsigned short	AddHeadNum;
	unsigned short	i;
	unsigned char	ByteStation;	
    
	ByteStation = 0;
	SndBuf[ByteStation++] = FRAM_HERD0;
	SndBuf[ByteStation++] = FRAM_HERD1;
	SndBuf[ByteStation++] = SourceAddr;	 
	SndBuf[ByteStation++] = DistAddr;				    // 地址
	SndBuf[ByteStation++] = SendFrameNbr;               // SendFrameNbr++;
	SndBuf[ByteStation++] = RS485_CHK_CRC<<4;	
	SndBuf[ByteStation++] = DataLen;
	
	SumCheck8 = GetCheckSumNR(&SndBuf[2],5);		   	// 索引区校验

	SndBuf[ByteStation++] = SumCheck8;

	memcpy(&SndBuf[8],DataBuf,DataLen);

	Crc16 = GetCrc16Check(&SndBuf[2],6+DataLen);

	SndBuf[2+6+DataLen+1] = Crc16;
	SndBuf[2+6+DataLen]   = Crc16>>8;

	SndBufTmp[0] = SndBuf[0];						    // 准备数据发送
	SndBufTmp[1] = SndBuf[1];
	
	AddHeadNum = 0;
	for(i = 2; i< 2+6+DataLen + 2;i++ )	{				// 数据，补移位 FRAM_HERD0
	
		SndBufTmp[i+AddHeadNum] = SndBuf[i];
		
		if(SndBuf[i] == FRAM_HERD0) {
			SndBufTmp[i+AddHeadNum+1] = FRAM_HERD0;
			AddHeadNum++;
		}
	}

	SndBufTmp[2+6+DataLen + 2 +AddHeadNum] = FRAM_END0;
	SndBufTmp[2+6+DataLen + 2 +AddHeadNum+1] = FRAM_END1;      
    
    CPU_INT16U nbr_bytes= (2+6+DataLen + 2 +AddHeadNum +2);
    if ( nbr_bytes < MB_DATA_NBR_REGS * 2 ) {  
        NMB_Tx(pch,
               (CPU_INT08U  *)SndBufTmp,
               (CPU_INT16U   )nbr_bytes);
    }
}



/*******************************************************************************
* 名    称： nmb_build_frame
* 功    能： 
* 入口参数send_buf   组帧成功后存储位置
				 send_data 数据传入指针
* 返回值： 构建帧长度
* 作  　者： fourth.peng.
* 创建日期： 2017-4-28
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
uint16 nmb_build_frame(uint8 *send_buf, uint8 *send_data, uint16 len)
{
	uint8 i = 0;
	uint16 send_cnt = 0;
	uint8 check_sum = 0;
	
	for (i = 0; i < len; i++)
		{
		check_sum += send_data[i];
		}

	i = 0;


	while ( len-- )
	{
		if ( FRAM_GAP == send_data[i] )
		{
			send_buf[send_cnt++] = 0x7d;
			send_buf[send_cnt++] = 0x55;
		}
		else if ( 0x7d == send_data[i] )
		{
			send_buf[send_cnt++] = 0x7d;
			send_buf[send_cnt++] = 0x56;			
		}
		else
		{
			send_buf[send_cnt++] = send_data[i];
		}
		i++;
	}
	send_buf[send_cnt++] = check_sum;
	send_buf[send_cnt++] = 0x7e;
	return send_cnt;
}
/*******************************************************************************
* 名    称： nmb_frame_translate
* 功    能： 还原0x7e,0x7d
* 入口参数 rx_data   传入数据
            data 转译完后数据
            len		传入数据长度
* 返回值： 解析完数据长度
* 作  　者： fourth.peng.
* 创建日期： 2017-4-28
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
uint16 nmb_frame_translate(uint8 *rx_data, uint8 *data, uint8 len)
{
	uint8   checksum =0;
	uint8   i = 0;
	uint16  decode_cnt =0;
	for(i = 0; i < len; i++)
	{
		if(0x7d == rx_data[i])
		{
			if(0x55 == rx_data[i+1])
			{
			data[decode_cnt++] = 0x7e;
			}
			else if(0x56 == rx_data[i+1])
			{
			data[decode_cnt++] = 0x7d;
			}
			else
			{
			//错误帧
				return 0;
			}
			i += 1;
		}
		else
		{
			data[decode_cnt++] = rx_data[i];
		}
	}
	return decode_cnt;
}



/*******************************************************************************
* 名    称： nmb_send_data
* 功    能： 
* 入口参数
* 出口参数： 无
* 作  　者： fourth.peng.
* 创建日期： 2017-4-27
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
void nmb_send_data(MODBUS_CH *pch, uint8 *send_data, uint16 len)
{
	uint8 send_buf[256] ={0};
	uint16 send_len = 0;
	send_len = nmb_build_frame(send_buf, send_data, len);
       NMB_Tx(pch, send_buf, send_len);
}

/*******************************************************************************
 * 				                    end of file                                *
 *******************************************************************************/
#endif
