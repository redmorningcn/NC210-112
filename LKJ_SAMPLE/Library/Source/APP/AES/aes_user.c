/*******************************************************************************
 *   Filename:       aes_user.c
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

/*******************************************************************************
 * INCLUDES
 */
#include <includes.h>
#include <global.h>
#include <bsp_info.h>
#include <aes.h>

#define BSP_AES_MODULE_EN 1
#if BSP_AES_MODULE_EN > 0
/*******************************************************************************
 * CONSTANTS
 */
#define AES_BITS        128
#define AES_BYTES       16

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
* 描述：定义函数指针
*/

/*******************************************************************************
 * GLOBAL VARIABLES
 */
#if MB_AES_EN == DEF_ENABLED
static INT8U       mbszIvec[16] = {0X12, 0x34, 0x56, 0x78,0X87, 0x65, 0x56, 0x78,
                                 0x87, 0x65, 0x43, 0x21,0x12, 0x34, 0x43, 0x21};
static AES_KEY     mbaesKey;
static INT8U       mbuserIvec[16];
#endif

static AES_KEY  aesKey;                            		            // AES 扩展密钥
static INT8U 	userKey[AES_BYTES+1];//    = "snlion>2014+2^99" / "2014<snlion>2^99";		// 解密密钥 16字节
static INT8U 	userIvec[AES_BYTES+1];//   = "4307783638394734";		// 解密向量 16字节

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
 * 名    称： AES_GetUserKey
 * 功    能： 获取用户密码
 * 入口参数： 
 * 出口参数： 
 * 作　 　者： 无名沈
 * 创建日期： 2014-05-04
 * 修    改：
 * 修改日期：
 *******************************************************************************/
static void AES_GetUserKey( unsigned char *userKey ) 
{
  // "snlion>2014+2^99"
  *userKey++    = 's';//*szKey;
  *userKey++    = 'n';//*(szKey + 2);
  *userKey++    = 'l';//*(szKey + 3);
  *userKey++    = 'i';//*(szKey + 5);
  *userKey++    = 'o';//*(szKey + 6);
  *userKey++    = 'n';//*(szKey + 7);
  *userKey++    = '>';//*(szKey + 9);
  *userKey++    = '2';//*(szKey + 10);
  *userKey++    = '0';//*(szKey + 11);
  *userKey++    = '1';//*(szKey + 12);
  *userKey++    = '4';//*(szKey + 14);
  *userKey++    = '+';//*(szKey + 15);
  *userKey++    = '2';//*(szKey + 1);
  *userKey++    = '^';//*(szKey + 4);
  *userKey++    = '9';//*(szKey + 8);
  *userKey++    = '9';//*(szKey + 13);
  *userKey      = 0;
}

/*******************************************************************************
 * 名    称： AES_GetUserKey
 * 功    能： 获取用户密码
 * 入口参数： 
 * 出口参数： 
 * 作　 　者： 无名沈
 * 创建日期： 2014-05-04
 * 修    改：
 * 修改日期：
 *******************************************************************************/
static void AES_GetUserKeyForApp( unsigned char *userKey ) 
{
  // "2014<snlion>2^99"
  *userKey++    = '2';//*szKey;
  *userKey++    = '0';//*(szKey + 2);
  *userKey++    = '1';//*(szKey + 3);
  *userKey++    = '4';//*(szKey + 5);
  *userKey++    = '<';//*(szKey + 6);
  *userKey++    = 's';//*(szKey + 7);
  *userKey++    = 'n';//*(szKey + 9);
  *userKey++    = 'l';//*(szKey + 10);
  *userKey++    = 'i';//*(szKey + 11);
  *userKey++    = 'o';//*(szKey + 12);
  *userKey++    = 'n';//*(szKey + 14);
  *userKey++    = '>';//*(szKey + 15);
  *userKey++    = '2';//*(szKey + 1);
  *userKey++    = '^';//*(szKey + 4);
  *userKey++    = '9';//*(szKey + 8);
  *userKey++    = '9';//*(szKey + 13);
  *userKey      = 0;
}

/*******************************************************************************
 * 名    称： AES_CbcInitKey
 * 功    能： 
 * 入口参数： 
 * 出口参数： 
 * 作　 　者： 无名沈
 * 创建日期： 2014-05-04
 * 修    改：
 * 修改日期：
 *******************************************************************************/
void AES_CbcInitKey(void)
{
	/***********************************************
	* 描述： 设置密钥
	*/
    AES_GetUserKey((INT8U *)userKey);
    //memcpy(userKey,"snlion>2014+2^99",16);
    userKey[AES_BYTES] = 0;
    
 	memset(aesKey.rd_key, 0, sizeof(aesKey.rd_key));// 清零扩展密钥
    AES_SetDecryptKey( userKey, 				    // 密钥
                        AES_BITS, 		            // 128位方式，还有192和256位方式
                        &aesKey);			        // 扩展密钥
}

/*******************************************************************************
 * 名    称： AES_CbcInitKey
 * 功    能： 
 * 入口参数： 
 * 出口参数： 
 * 作　 　者： 无名沈
 * 创建日期： 2014-05-04
 * 修    改：
 * 修改日期：
 *******************************************************************************/
void AES_CbcInitKeyForApp(BOOL dec)
{
	/***********************************************
	* 描述： 设置密钥
	*/
    AES_GetUserKeyForApp((INT8U *)userKey);
    //memcpy(userKey,"2014<snlion>2^99",16);
    userKey[AES_BYTES] = 0;
    
 	memset(aesKey.rd_key, 0, sizeof(aesKey.rd_key));// 清零扩展密钥
    if( dec == AES_DECRYPT )
      AES_SetDecryptKey( userKey, 				    // 密钥
                        AES_BITS, 		            // 128位方式，还有192和256位方式
                        &aesKey);			        // 扩展密钥
    else
      AES_SetEncryptKey( userKey, 				    // 密钥
                        AES_BITS, 		            // 128位方式，还有192和256位方式
                        &aesKey);			        // 扩展密钥
}

/*******************************************************************************
 * 名    称： AES_CbcInitViec
 * 功    能： 
 * 入口参数： 
 * 出口参数： 
 * 作　 　者： 无名沈
 * 创建日期： 2014-05-04
 * 修    改：
 * 修改日期：
 *******************************************************************************/
void AES_CbcInitViec(void)
{	/***********************************************
	* 描述： 设置密钥
	*/
    BSP_GetChipInfo(0,ChipInfo);
    memcpy(userIvec,ChipInfo[0].cid, AES_BYTES); 
    userIvec[AES_BYTES]  = 0;
}

/*******************************************************************************
 * 名    称： BOOT_AesCbcDec
 * 功    能： AES_CBC解密bin文件
 * 入口参数： pIn:加密后的bin文件明文；pOut:解密后的写入FLASH的bin文件
 * 出口参数： 
 * 作　 　者： 无名沈
 * 创建日期： 2014-04-23
 * 修    改：
 * 修改日期：
 *******************************************************************************/	
void AES_CbcDecrypt(const unsigned char *pIn, unsigned char *pOut, long len, unsigned char dec)
{
	/***********************************************
	* 描述： 
	*/
    AES_CbcEnOrDecrypt((INT8U*)pIn,                 // 待解密缓冲区
                   (INT8U*)pOut,                    // 解密后的缓冲区
                    len,				            // 缓冲区长度
                   (AES_KEY*)&aesKey, 		        // 扩展密钥
                   (INT8U*)userIvec, 		        // 向量
                    dec);		                    // 加密
}

/*******************************************************************************
 * 名    称： MB_AESCbcDecrypt()
 * 功    能： 接收数据加密码
 * 入口参数： buf 输入接收缓冲区,len 输入要加密的缓冲区长度
 * 出口参数： 解密后的长度
 * 作　 　者： 无名沈
 * 创建日期： 2014-03-28
 * 修    改：
 * 修改日期：
 *******************************************************************************/
INT16U MB_AESCbcDecrypt(INT8U *buf, INT16U len)
{
#if MB_AES_EN == DEF_ENABLED    
    memset(mbaesKey.rd_key, 0, sizeof(mbaesKey.rd_key));// 清零扩展密钥
    AES_SetDecryptKey(  "scw->ll->zs->scw",         // 密钥
                        128, 		                // 128位方式，还有192和256位方式
                       &mbaesKey);			        // 扩展密钥
    /***********************************************
    * 描述： 设置密钥向量
    */
    memcpy((INT8U*)mbuserIvec,(INT8U*)&mbszIvec[0], 16);// 向量
    /***********************************************
    * 描述： 更改密钥向量（为增加安全性，可使向量自动与发送端进行同步）
    *
    if (++mbszIvec[6] > 10)
      mbszIvec[6]  = 0;
    *//***********************************************
    * 描述： 检查数据长度是否为16的整数倍
    */
    if ( (len & 0x0f) != 0  )
      return (DEF_FALSE);
    
    AES_CbcEnOrDecrypt((INT8U*)buf,                 // 待解密缓冲区
                       (INT8U*)buf,                 // 解密后的缓冲区
                        len,				        // 缓冲区长度
                       (AES_KEY*)&mbaesKey, 		// 扩展密钥
                       (INT8U*)mbuserIvec, 		    // 向量
                        AES_DECRYPT);		        // 加密
    /***********************************************
    * 描述： 检查最后一个字节是否为1，为1则表示填充位字节数为1
    */
    if ( buf[len-1] == 1) {                         // 如果最后字节为1
      len--;                                        // 有效数据数-1
    /***********************************************
    * 描述： 检查>1的填充字节数
    */
    } else if ( buf[len-1] < 16 && buf[len-1] > 1) {
      INT8U i   = buf[len-1];                       // 获取填充值
      INT8U *p  = &buf[len-i];                      // 指向开始填充的字节地址
      for (i-=1;i > 0;i--,p++) {
        if ( *p != *(p+1) )                         // 如果相邻两个字节的值不相等
          break;                                    // 说明不是填充值，直接跳出
      }
      if (i==0)                                     // 如果较完成，说明是填充字节
        len -= buf[len-1];                          // 获取有效数据帧字节数
    }
#endif
    return      len;                                // 重新赋值给接收计数器
}

/*******************************************************************************
 * 名    称： MB_AESCbcEncrypt()
 * 功    能： 发送数据加密码
 * 入口参数： buf 输入发送缓冲区,len 输入要加密的缓冲区长度
 * 出口参数： 加密后的长度
 * 作　 　者： 无名沈
 * 创建日期： 2014-03-28
 * 修    改：
 * 修改日期：
 *******************************************************************************/
INT16U MB_AESCbcEncrypt(INT8U *buf, INT16U len)
{
#if MB_AES_EN == DEF_ENABLED
    INT8U       temp;
    
    memset(mbaesKey.rd_key, 0, sizeof(mbaesKey.rd_key));// 清零扩展密钥
    AES_SetEncryptKey(  "scw->ll->zs->scw",         // 密钥
                        128, 		                // 128位方式，还有192和256位方式
                       &mbaesKey);			        // 扩展密钥
    /***********************************************
    * 描述： 设置密钥向量
    */
    memcpy((INT8U*)mbuserIvec,(INT8U*)&mbszIvec[0], 16);// 向量
    temp    = len & 0x0f;
    if ( temp != 0) {
      temp    = 16 - temp;
      memset((INT8U*)&buf[len],temp,temp);
      len = len + temp;
    }
    AES_CbcEnOrDecrypt((INT8U*)&buf[0],             // 待解密缓冲区
                       (INT8U*)&buf[0],             // 解密后的缓冲区
                        len,	                    // 缓冲区长度
                       (AES_KEY*)&mbaesKey, 		// 扩展密钥
                       (INT8U*)mbuserIvec, 		    // 向量
                        AES_ENCRYPT);		        // 加密
#endif
    return len;
}

/*******************************************************************************
 * 				end of file
 *******************************************************************************/ 
#endif