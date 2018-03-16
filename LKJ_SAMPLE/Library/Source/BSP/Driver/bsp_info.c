/*******************************************************************************
 *   Filename:       bsp_info.c
 *   Revised:        $Date: 2015-08-24	 $
 *   Revision:       $V1.0
 *	 Writer:		 wumingshen.
 *
 *   Description:    读取芯片唯一UID模块驱动
 *
 *   Notes:          每个STM32芯片在出厂时都具有一个全球唯一的ID，该ID为96位，其地址分布如下：
 *                   Start Address : 0x1FFF F7E8
 *                   Address offset: 0×00 U_ID(00-31)
 *                   Address offset: 0×04 U_ID(32-63)
 *                   Address offset: 0×08 U_ID(64-96)
 *                   该ID的应用：
 *
 *                   ●用来作为序列号(例如USB字符序列号或者其他的终端应用)
 *                   ●用来作为密码，在编写闪存时，将此唯一标识与软件加解密算法结合使用，提高代码在闪存存储器内的安全性。
 *                   ●用来激活带安全机制的自举过程
 *
 *
 *   All copyrights reserved to wumingshen.
 *
 *******************************************************************************/

#define BSP_INFO_EN 1
#if BSP_INFO_EN > 0

/*******************************************************************************
 * INCLUDES
 */
#include <includes.h>
#include <bsp_info.h>
#include <app.h>


/*******************************************************************************
 * 描述： 芯片UID存储数组
 */
StrChipInfo ChipInfo[3];

//============================================================================//

/*******************************************************************************
* 名    称： BSP_GetChipID
* 功    能： 获取芯片的UID
* 入口参数： 无
* 出口参数： 无
* 作　 　者： 无名沈
* 创建日期： 2014-08-18
* 修    改：
* 修改日期：
* 备    注： 96位的ID是stm32唯一身份标识，可以以8bit、16bit、32bit读取提供了大端和小端两种表示方法。
*            ID号存放在闪存存储器模块的系统存储区域，可以通过JTAG/SWD或者CPU读取。
*            它所包含的芯片识别信息在出厂时编写，用户固件或者外部设备可以读取电子签名，
*            用以自动匹配不同配置的STM32F10xxx微控制器。
*            产品唯一的身份标识的作用：
*          ● 用来作为序列号(例如USB字符序列号或者其他的终端应用)；
*          ● 用来作为密码，在编写闪存时，将此唯一标识与软件加解密算法结合使用，提高代码在闪存存储器内的安全性；
*          ● 用来激活带安全机制的自举过程；
*            96位的产品唯一身份标识所提供的参考号码对任意一个STM32微控制器，
*            在任何情况下都是唯一的。用户在何种情况下，都不能修改这个身份标识。
*            按照用户不同的用法，可以以字节(8位)为单位读取，
*            也可以以半字(16位)或者全字(32位)读取。嘿嘿，要注意大端小端模式哟~~~
*******************************************************************************/
uint8_t BSP_GetChipID( uint8_t devType, UnionChipId *pId )
{
    switch ( devType ) {
        case 0:
            pId->buf3[0] = *(vu32 *)(0X1FFFF7F0);            // 高字节
            pId->buf3[1] = *(vu32 *)(0X1FFFF7EC);            //
            pId->buf3[2] = *(vu32 *)(0X1FFFF7E8);            // 低字节
        return TRUE;
        break;
        case 1:
        case 2:
        break;
    }
    return FALSE;
}


/*******************************************************************************
* 名    称： BSP_GetChipInfo
* 功    能： 获取芯片相关信息
* 入口参数： 无
* 出口参数： 无
* 作　 　者： 无名沈
* 创建日期： 2014-08-18
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
uint8_t BSP_GetChipInfo(uint8_t devType, StrChipInfo *pInfo)
{
    //INT16U STM32_FLASH_SIZE;

    pInfo->cid[CID_BUF_LEN-1] = 0;
    pInfo->flash[FLASH_BUF_LEN-1] = 0;
    pInfo->date[DATE_BUF_LEN-1] = 0;
    pInfo->time[TIME_BUF_LEN-1] = 0;
    pInfo->version[VERSION_BUF_LEN-1] = 0;

    if ( !BSP_GetChipID(devType, &pInfo->id) ){
      return FALSE;
    }
    //STM32_FLASH_SIZE= *(INT16U*)(0x1FFFF7E0);           //闪存容量寄存器
//    usprintf(pInfo->cid,        "%08X%08X%08X",         pInfo->id.buf3[0],
//                                                        pInfo->id.buf3[1],
//                                                        pInfo->id.buf3[2]);
//    usprintf(pInfo->flash,     "%dK",                   STM32_FLASH_SIZE);
//    usprintf(pInfo->date,      ""__DATE__"");
//    usprintf(pInfo->time,      ""__TIME__"");
//    usprintf(pInfo->version,   "V%d.%d.%d",             __STM32F10X_STDPERIPH_VERSION_MAIN,              // 输出使用固件库版本号
//                                                        __STM32F10X_STDPERIPH_VERSION_SUB1,
//                                                        __STM32F10X_STDPERIPH_VERSION_SUB2);
    pInfo->cid[CID_BUF_LEN-1] = 0;
    pInfo->flash[FLASH_BUF_LEN-1] = 0;
    pInfo->date[DATE_BUF_LEN-1] = 0;
    pInfo->time[TIME_BUF_LEN-1] = 0;
    pInfo->version[VERSION_BUF_LEN-1] = 0;

    return TRUE;
}
/*******************************************************************************
 * 				end of file
 *******************************************************************************/
#endif