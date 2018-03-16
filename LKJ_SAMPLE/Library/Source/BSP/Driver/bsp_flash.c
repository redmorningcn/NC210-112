/*******************************************************************************
 *   Filename:       bsp_flash.c
 *   Revised:        $Date: 2014-04-23
 *   Revision:       $
 *	 Writer:		 Wuming Shen.
 *
 *   Description:    
 *
 *   Notes:        
 *					   
 *			
 *   All copyrights reserved to Wuming Shen.
 *
 *******************************************************************************/

#define BSP_FLASH_MODULE_EN 1
#if BSP_FLASH_MODULE_EN > 0
/*******************************************************************************
 * INCLUDES
 */
#include "bsp_flash.h"
#include  <stdio.h>
#include  <string.h>
#include  <ctype.h>
#include  <stdlib.h>
#include  <stdarg.h>

UnionFlashBuf   FlashBuf;
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
/***********************************************
* 描述：
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
 
/*******************************************************************************/


/*******************************************************************************
 * 名    称： FlashReadHalfWord()
 * 功    能： 读取指定地址的16位数据
 * 入口参数：
 * 出口参数： 无
 * 作　　 者： 无名沈
 * 创建日期： 2014-04-23
 * 修    改： 
 * 修改日期： 
 *******************************************************************************/
u16  FlashReadHalfWord(u32 addr)
{
	return *(vu16 *)addr;
}

/*******************************************************************************
 * 名    称： BSP_FlashRead()
 * 功    能： 外部调用
 * 入口参数：
 * 出口参数： 无
 * 作　　 者： 无名沈
 * 创建日期： 2014-04-23
 * 修    改： 
 * 修改日期： 
 *******************************************************************************/
void  BSP_FlashRead(u32 addr,u16 *pbuffer,u16 num)   	
{
	u16 i;
    
    FLASH_Unlock();                             // 解锁
	for(i = 0;i < num;i++) {
		*pbuffer++ = FlashReadHalfWord(addr);   //读取2个字节.
		addr += 2;                              //偏移2个字节.	
	}
	FLASH_Lock();             				    // 上锁
}

/*******************************************************************************
 * 名    称： FlashRead()
 * 功    能： 
 * 入口参数：
 * 出口参数： 无
 * 作　　 者： 无名沈
 * 创建日期： 2014-04-23
 * 修    改： 
 * 修改日期： 
 *******************************************************************************/
void  FlashRead(u32 addr,u16 *pbuffer,u16 num)   	
{
	u16 i;
    
	for(i = 0;i < num;i++) {
		*pbuffer++ = FlashReadHalfWord(addr);   //读取2个字节.
		addr += 2;                              //偏移2个字节.	
	}
}
/*******************************************************************************
 * 名    称： FlashWriteNoCheck()
 * 功    能： 
 * 入口参数：
 * 出口参数： 无
 * 作　　 者： 无名沈
 * 创建日期： 2014-04-23
 * 修    改： 
 * 修改日期： 
 *******************************************************************************/
u8  FlashWriteNoCheck(u32 addr,u16 *pbuffer,u16 num)
{
	u16 i;
    
	for(i = 0;i < num;i++) {
		//if ( FLASH_COMPLETE != FLASH_ProgramHalfWord (addr,pbuffer[i]) ) {
        //    return 0;
        //}
        FLASH_ProgramHalfWord (addr,pbuffer[i]);
		addr += 2;
	}
    return 1;
}

/*******************************************************************************
 * 名    称： FlashWrite()
 * 功    能： 带擦除的写
 * 入口参数：
 * 出口参数： 无
 * 作　　 者： 无名沈
 * 创建日期： 2014-04-23
 * 修    改： 
 * 修改日期： 
 *******************************************************************************/
void  BSP_FlashWrite(u32 addr,u16 *pbuffer,u16 num)
{
//	u16  num_sector;  						        // 在第几页
//	u16  offset_sector;  					        // 在一页的偏移量
//	u16  num_last;    						        // 剩下的半字
//	u16  i;
//	/***********************************************
//    * 描述：
//    */
//    num_sector      = (addr - STM32_FLASH_BASE) / STM_SECTOR_SIZE;	   			// 获取第几页
//	offset_sector   = ((addr - STM32_FLASH_BASE) % STM_SECTOR_SIZE) / 2;  	    // 获取偏移量，16个字节为单位
//	num_last        = STM_SECTOR_SIZE / 2 - offset_sector;                     	// 求出剩下的半字
//	
//    FLASH_Unlock();                                 // 解锁
//	/***********************************************
//    * 描述：
//    */
//    if(num <= num_last)  
//      num_last      = num;		                    // 可以一次写完
//    /***********************************************
//    * 描述：
//    */
//	for(;;)	{									        // 开始不停的写
//		FlashRead(num_sector * STM_SECTOR_SIZE + STM32_FLASH_BASE,FlashBuf.buf2,STM_SECTOR_SIZE / 2);  // 读出一页
//		for(i = 0;i < num_last;i++) {
//			if(FlashBuf.buf2[offset_sector + i] != 0xffff) break;	
//		}
//		if(i < num_last) {						        // 需要擦除
//			FLASH_ErasePage (num_sector * STM_SECTOR_SIZE + STM32_FLASH_BASE);  // 擦除
//			for(i = 0;i < num_last;i++) {
//				FlashBuf.buf2[offset_sector + i] = pbuffer[i];	                // 把要写入的数据填入数组
//			}
//			FlashWriteNoCheck(num_sector * STM_SECTOR_SIZE + STM32_FLASH_BASE,FlashBuf.buf2,STM_SECTOR_SIZE / 2);               //写整个扇区		
//		} else {								        // 不需要擦除
//			FlashWriteNoCheck(addr,pbuffer,num_last);	// 直接将要写入的数据写入flash
//		}
//        /***********************************************
//        * 描述：
//        */
//		if(num_last == num) { 
//            break;                                      // 写完后就跳出
//		} else {                                        // 没有写完继续
//			num_sector     += 1;    				    // 页加一
//			offset_sector   = 0;  				        // 偏移为0
//			pbuffer        += num_last;
//			addr           += num_last * 2;
//			num            -= num_last;    				// 调整要写入的数量
//            
//			if(num > (STM_SECTOR_SIZE / 2))  
//                num_last = STM_SECTOR_SIZE / 2;
//			else  
//              num_last = num;   			            // 调整num_last		
//		}		
//	}
//    
//	FLASH_Lock();             					        // 上锁
    
	u16  num_sector;                                    //在第几页
	u16  offset_sector;                                 //在一页的偏移量
	u16  num_last;                                      //剩下的半字
	u16  i;
	num_sector      =  (addr - STM32_FLASH_BASE) / STM_SECTOR_SIZE;	        //获取第几页
	offset_sector   =  ((addr - STM32_FLASH_BASE) % STM_SECTOR_SIZE) / 2;   //获取偏移量，16个字节为单位
	num_last        = 	STM_SECTOR_SIZE / 2 - offset_sector;                //求出剩下的半字
	FLASH_Unlock();                                     //解锁
	if(num <= num_last)  num_last = num;		        //可以一次写完
	
    for(;;)	{									        //开始不停的写
		FlashRead(num_sector * STM_SECTOR_SIZE + STM32_FLASH_BASE,FlashBuf.buf2,STM_SECTOR_SIZE / 2);  //读出一页
		for(i = 0;i < num_last;i++) {
			if(FlashBuf.buf2[offset_sector + i] != 0xffff) break;	
		}
		if(i < num_last) { 					//需要擦除
			FLASH_ErasePage (num_sector * STM_SECTOR_SIZE + STM32_FLASH_BASE);                      //擦除
			for(i = 0;i < num_last;i++) {
				FlashBuf.buf2[offset_sector + i] = pbuffer[i];	               //把要写入的数据填入数组
			}
			FlashWriteNoCheck(num_sector * STM_SECTOR_SIZE + STM32_FLASH_BASE,FlashBuf.buf2,STM_SECTOR_SIZE / 2);               //写整个扇区		
		} else {								    //不需要擦除
		
			FlashWriteNoCheck(addr,pbuffer,num_last);	//直接将要写入的数据写入flash
		}
		if(num_last == num)  
            break;                                  //写完后就跳出
		else  {                                     //没有写完继续
			num_sector += 1;    //页加一
			offset_sector = 0;  //偏移为0
			pbuffer += num_last;
			addr += num_last * 2;
			num -= num_last;    //调整要写入的数量
			if(num > (STM_SECTOR_SIZE / 2))  num_last = STM_SECTOR_SIZE / 2;
			else  num_last = num;   //调整num_last		
		}		
	}
    
	FLASH_Lock();             //上锁
}

/*******************************************************************************
* 名    称： BSP_FLASH_ReadPage
* 功    能： 读一页数据，起始地址在页首
* 入口参数： addr FLASH地址  pbuf ： 读取字节的数组 len : 数组的大小
* 出口参数： 0（操作有误），1（操作成功）
* 作　 　者： wumingshen
* 创建日期： 2015-10-20
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
uint8_t BSP_FLASH_ReadPage( uint32_t addr , uint8_t *pbuf, u16 len )
{
    uint16_t    size   = 0;

    if ( len > STM_SECTOR_SIZE )
      return 0;

    u16 *p      = (u16 *)pbuf;
    
    for ( ; size < len ; size += 2 ) {
		*p++     = *(vu16 *)addr;
		addr    += 2;	
	}
    
    return size;
}

/*******************************************************************************
* 名    称： BSP_FLASH_WritePage
* 功    能： 写一页数据，起始地址在页首
* 入口参数： addr:  FLASH地址  pbuf: 写入字节的数组 len: 数组的大小
* 出口参数： 0（操作有误），1（操作成功）
* 作　 　者： wumingshen
* 创建日期： 2015-10-20
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
uint8_t BSP_FLASH_WritePage( uint32_t addr , uint8_t *pbuf, u16 len )
{
    uint16_t    size   = 0;

    if ( len > STM_SECTOR_SIZE )
      return 0;

    u16 *p      = (u16 *)pbuf;
    s8  retrys;
    
    for ( ; size < len ; size += 2 ) {
        retrys  = 5;
        do {
            FLASH_ProgramHalfWord(addr,*(uint16_t *)p);
            if (*(uint16_t *)addr == *(uint16_t *)p) {
                break;
            } else if ( 1 == retrys ) {
                return size;
            } 
        } while ( --retrys );
        
		addr    += 2;
        p++;
	}
            
    return size;
}

/*******************************************************************************
* 名    称： BSP_FlashWriteBytes
* 功    能： 写多个字节
* 入口参数： addr： FLASH地址  pbuf ：写入字节的数组 len : 数组的大小
* 出口参数： 0（操作有误），1（操作成功）
* 作　 　者： wumingshen
* 创建日期： 2015-10-20
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
u16     BSP_FlashWriteBytes     (u32 addr, u8 *pbuf, u16 len)
{
    uint16_t      bytes   = 0;
    /***********************************************
    * 描述： 检测地址是否会溢出，如果溢出则直接返回
    */
    if ( (addr + len) > (FLASH_SIZE_EEP) )
        return 0;

    uint16_t    copyLen;                            // 要复制的长度
    uint32_t    pageAddr;
    uint16_t    size    = 0;                        // 一次写入长度
    u32         page    = addr / STM_SECTOR_SIZE;   // 写入地址所在页（相对）
                addr    += PARAM_START_ADDR;        // 转换成绝对地址
    
	FLASH_Unlock();             				    // 上锁
    /***********************************************
    * 描述： 非页首开始，读取之前数据进行填充
    */
    if ( ( addr % STM_SECTOR_SIZE ) != 0 ) {
        size        = STM_SECTOR_SIZE - ( addr % STM_SECTOR_SIZE );
        copyLen     = STM_SECTOR_SIZE - size;
        pageAddr    = PARAM_START_ADDR + page * STM_SECTOR_SIZE;
        // 读取当前页数据
        BSP_FLASH_ReadPage(pageAddr, FlashBuf.buf1, STM_SECTOR_SIZE);

        if ( size > len )
            size = len;
        // 复制属于当前页的数据到缓冲区中
        memcpy(&FlashBuf.buf1[copyLen], pbuf, size);
        // 整页探险当前页
		FLASH_ErasePage ( pageAddr );
        // 将缓冲区的数据写入当前页
        BSP_FLASH_WritePage( pageAddr , FlashBuf.buf1 , STM_SECTOR_SIZE );
        // 源数据地址、写入地址、写入字节数增加size，未写数据长度减少size
        addr    += size;
        pbuf    += size;
        bytes   += size;
        len     -= size;
    }

    /***********************************************
    * 描述： 页首开始，整页
    */
    size = len / STM_SECTOR_SIZE;
    while ( size > 0 ) {
        // 探险当前页
		FLASH_ErasePage (addr);
        // 写入当前数据页
        BSP_FLASH_WritePage( addr , (u8 *)pbuf, STM_SECTOR_SIZE );
        // 源数据地址、写入地址、写入字节数增加STM_SECTOR_SIZE，未写数据长度减少STM_SECTOR_SIZE
        addr    += STM_SECTOR_SIZE;
        pbuf    += STM_SECTOR_SIZE;
        bytes   += STM_SECTOR_SIZE;
        len     -= STM_SECTOR_SIZE;
        // 页数减1
        size    -= 1;
    }

    /***********************************************
    * 描述： 页首开始，不足一页
    */
    if ( len > 0 ) {
        size        = STM_SECTOR_SIZE - ( addr % STM_SECTOR_SIZE );
        copyLen     = STM_SECTOR_SIZE - size;
        pageAddr    = addr - copyLen;
        
        BSP_FLASH_ReadPage(pageAddr, FlashBuf.buf1, STM_SECTOR_SIZE);

        if ( size > len )
            size = len;
        // 复制属于当前页的数据到缓冲区中
        memcpy(&FlashBuf.buf1[copyLen],pbuf,size);

        // 整页探险当前页
		FLASH_ErasePage (pageAddr);
        // 将缓冲区的数据写入当前页
        BSP_FLASH_WritePage( pageAddr , (u8 *)FlashBuf.buf1 , STM_SECTOR_SIZE );
        bytes   += len;
    }

	FLASH_Lock();             				    // 上锁
    return(bytes);
}

/*******************************************************************************
* 名    称： BSP_FlashReadBytes
* 功    能： 读多个字节（必须是2的倍数）
* 入口参数： addr： FLASH地址  pbuf ：写入字节的数组 len : 数组的大小（2的倍数）
* 出口参数： 0（操作有误），1（操作成功）
* 作　 　者： wumingshen
* 创建日期： 2015-10-20
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
u16     BSP_FlashReadBytes      (u32 addr, u8 *pbuf, u16 len)
{ 
    addr         = (PARAM_START_ADDR + addr);
    
	FLASH_Unlock();             					        // 解锁

    u16 *p      = (u16 *)pbuf;
    u16 size    = 0;
    
    for ( ; size < len ; size += 2 ) {
		*p++     = *(vu16 *)addr;
		addr    += 2;	
	}
    
	FLASH_Lock();             					            // 上锁
    
    return len;
} 

/*******************************************************************************
 * 				end of file
 *******************************************************************************/ 
#endif