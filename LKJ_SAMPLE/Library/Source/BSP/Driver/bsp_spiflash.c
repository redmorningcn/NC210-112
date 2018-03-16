/*******************************************************************************
*   Filename:      bsp_externalflash.c
*   Revised:       $Date: 2015-03-05
*   Revision:      $
*   Writer:        snlion.
*
*   Description:   外接Flash存储器驱动
*
*   Notes:         驱动SPI接口的Flash ROM闪存芯片，使用STM32内部硬件SPI
*                  AT45DB161E硬件连接：
*                   ---------------------------------
*                  | PA4-SPI1-NSS  ：AT45DB161D-CS  |
*                  | PA5-SPI1-SCK  : AT45DB161D-CLK |
*                  | PA6-SPI1-MISO : AT45DB161D-SO  |
*                  | PA7-SPI1-MOSI : AT45DB161D-SI  |
*                  | PC5           : AT45DB161D-WP  |
*                  | PC4           : AT45DB161D-RST |
*                   ---------------------------------
*                  除特殊扇区（扇区0）外，1扇区 = 32个块， 1块 = 8页 1页 = 512/528个字节（默认528个）
*
*   All copyrights reserved to snlion.
*
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "bsp_spiflash.h"

unionflashstorage FlashStorageBuf;
u8  FlashStauteByte;

/******************************************************************************/
//  测试使用
//    u32 i;
//    static u8 Tx1_Buffer[512]={0};
//    static u8 Rx1_Buffer[512]={0};
//    for(i=0;i<512;i++)
//    {
//      Tx1_Buffer[i] = i&0x00FF;
//    }
//    SPI_FLASH_BufferWrite("first",0,5);       //任意地址写成功
//    SPI_FLASH_BufferWrite("secon",1,5);       //任意地址写成功
//    SPI_FLASH_ArraryRead(Rx1_Buffer,0,20);    //任意地址读成功
//    SPI_FLASH_Ram1Write(Tx1_Buffer,0,512);    //buff1已写成功！
//    SPI_FLASH_Ram1Read(Rx1_Buffer,0, 512);    //buff1已读成功!
//    SPI_FLASH_PageWrite(Tx1_Buffer,0,512);    //PAGE写成功！
//    SPI_FLASH_ArraryRead(Rx1_Buffer,0,512);   //连接读成功
/******************************************************************************/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

//============================================================================//
//
//请先在bsp_flashrom.h文件中配置实用硬件SPI还是软件SPI
//
//============================================================================//

/*******************************************************************************
* 名    称： BSP_FlashGPIO_Config
* 功    能： 外部Flash芯片控制引脚初始化
* 入口参数： 无
* 出口参数： 无
* 作　　者： Snlion-WY
* 创建日期： 2015-04-14
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
static void BSP_FlashGPIO_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    /*!< Configure SPI_FLASH_SPI pins: SCK */
    RCC_APB2PeriphClockCmd(BSP_FLASH_SPI_SCK_GPIO_RCC, ENABLE);
    GPIO_InitStructure.GPIO_Pin   = BSP_FLASH_SPI_SCK_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_Init(BSP_FLASH_SPI_SCK_GPIO_PORT, &GPIO_InitStructure);

    /*!< Configure SPI_FLASH_SPI pins: MISO */
    RCC_APB2PeriphClockCmd(BSP_FLASH_SPI_MISO_GPIO_RCC, ENABLE);
    GPIO_InitStructure.GPIO_Pin = BSP_FLASH_SPI_MISO_PIN;
    GPIO_Init(BSP_FLASH_SPI_MISO_GPIO_PORT, &GPIO_InitStructure);

    /*!< Configure SPI_FLASH_SPI pins: MOSI */
    RCC_APB2PeriphClockCmd(BSP_FLASH_SPI_MOSI_GPIO_RCC, ENABLE);
    GPIO_InitStructure.GPIO_Pin = BSP_FLASH_SPI_MOSI_PIN;
    GPIO_Init(BSP_FLASH_SPI_MOSI_GPIO_PORT, &GPIO_InitStructure);

    /*!< Configure SPI_FLASH_SPI_CS_PIN pin: SPI_FLASH Card CS pin */
    RCC_APB2PeriphClockCmd(BSP_FLASH_CS_GPIO_RCC, ENABLE);
    GPIO_InitStructure.GPIO_Pin  = BSP_FLASH_CS_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(BSP_FLASH_CS_GPIO_PORT, &GPIO_InitStructure);

    /*!< Configure SPI_FLASH_SPI pins:WP */
    RCC_APB2PeriphClockCmd(BSP_FLASH_WP_GPIO_RCC, ENABLE);
    GPIO_InitStructure.GPIO_Pin   = BSP_FLASH_WP_PIN;
    GPIO_Init(BSP_FLASH_WP_GPIO_PORT, &GPIO_InitStructure);
    /*!< Configure SPI_FLASH_SPI pins: RST */
    RCC_APB2PeriphClockCmd(BSP_FLASH_RST_GPIO_RCC, ENABLE);
    GPIO_InitStructure.GPIO_Pin   = BSP_FLASH_RST_PIN;
    GPIO_Init(BSP_FLASH_RST_GPIO_PORT, &GPIO_InitStructure);

    /* 复位失能 */
    BSP_FLASH_RST_HIGH();
    /* 片选失能 */
    BSP_FLASH_CS_HIGH();
    /* 写保护失能*/
    BSP_FLASH_WP_HIGH();
}

#if (BSP_USE_SPI_MODE == 1)
/*******************************************************************************
* 名    称： BSP_SPIxMode_Config
* 功    能： 配置硬件SPI
* 入口参数： 无
* 出口参数： 无
* 作　　者： Snlion-WY
* 创建日期： 2015-04-14
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
static void BSP_SPIxMode_Config(void)
{
    SPI_InitTypeDef  SPI_InitStructure;
    RCC_APB2PeriphClockCmd(BSP_FLASH_SPIx_RCC, ENABLE);
    /* SPIx configuration */
    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;    //双工模式
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;                         //SPI主模式
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;                     //8bit数据
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;                           //CLK空闲时为高
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;                          //第二个边沿采样
    SPI_InitStructure.SPI_NSS  = SPI_NSS_Soft;                            //片选用软件控制
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_32;   //SPI频率
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;                    //高位在前
    SPI_InitStructure.SPI_CRCPolynomial = 7;                              //crc7，stm32spi带硬件ecc
    SPI_Init(BSP_FLASH_SPIx, &SPI_InitStructure);

    /* Enable SPIx  */
    SPI_Cmd(BSP_FLASH_SPIx, ENABLE);
}
#endif


//============================================================================//
//下面使用条件编译选择使用模拟SPI还是硬件SPI
//============================================================================//
#if (BSP_USE_SPI_MODE == 1)

/**
* @brief  Sends a byte through the SPI interface and return the byte
*   received from the SPI bus.
* @param byte : byte to send.
* @retval : The value of the received byte.
*/
uint8_t SPI_FLASH_SendByte(uint8_t byte)
{
    /* Loop while DR register in not emplty */
    while (SPI_I2S_GetFlagStatus(BSP_FLASH_SPIx, SPI_I2S_FLAG_TXE) == RESET);
    /* Send byte through the SPI1 peripheral */
    SPI_I2S_SendData(BSP_FLASH_SPIx, byte);
    /* Wait to receive a byte */
    while (SPI_I2S_GetFlagStatus(BSP_FLASH_SPIx, SPI_I2S_FLAG_RXNE) == RESET);
    /* Return the byte read from the SPI bus */
    return SPI_I2S_ReceiveData(BSP_FLASH_SPIx);
}

/**
* @brief  Reads a byte from the SPI Flash.
*   This function must be used only if the Start_Read_Sequence
*   function has been previously called.
* @param  None
* @retval : Byte Read from the SPI Flash.
*/
uint8_t SPI_FLASH_ReadByte(void)
{
    return (SPI_FLASH_SendByte(Dummy_Byte));
}


/**
* @brief  Sends a Half Word through the SPI interface and return the
*         Half Word received from the SPI bus.
* @param HalfWord : Half Word to send.
* @retval : The value of the received Half Word.
*/
uint16_t SPI_FLASH_SendHalfWord(uint16_t HalfWord)
{
    /* Loop while DR register in not emplty */
    while (SPI_I2S_GetFlagStatus(BSP_FLASH_SPIx, SPI_I2S_FLAG_TXE) == RESET);
    /* Send Half Word through the SPI1 peripheral */
    SPI_I2S_SendData(BSP_FLASH_SPIx, HalfWord);
    /* Wait to receive a Half Word */
    while (SPI_I2S_GetFlagStatus(BSP_FLASH_SPIx, SPI_I2S_FLAG_RXNE) == RESET);
    /* Return the Half Word read from the SPI bus */
    return SPI_I2S_ReceiveData(BSP_FLASH_SPIx);
}
#else  //使用模拟SPI读写Flash

void SPI_FLASH_SendByte(uint8_t byte)
{
    for(u8 i = 0; i < 8; i++) {
        if((byte << i) & 0x80) {
            BSP_FLASH_MOSI_HIGH();
        } else {
            BSP_FLASH_MOSI_LOW();
        }
        BSP_FLASH_SCK_LOW();
        __NOP();
        __NOP();
        __NOP();
        BSP_FLASH_SCK_HIGH();
    }
}

u8  SPI_FLASH_ReadByte(void)
{
    u8 rByte = 0;
    for(u8 i = 0; i < 8; i++) {
        BSP_FLASH_SCK_LOW();
        __NOP();
        __NOP();
        __NOP();
        BSP_FLASH_SCK_HIGH();
        rByte <<= 1;
        if(BSP_FLASH_MISO_READ() == 1) {
            rByte = rByte + 1;
        }
    }
    return rByte;
}
#endif

/*******************************************************************************
* 名    称： BSP_FLASH_ReadID
* 功    能： 读取芯片本身ID
* 入口参数： 无
* 出口参数： 芯片ID
* 作　　者： Snlion-WY
* 创建日期： 2015-04-14
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
u32 BSP_FLASH_ReadID(void)
{
    static uint32_t Temp = 0;
    uint32_t TempVal[4] = {0};
    BSP_FLASH_CS_LOW();
  	SPI_FLASH_SendByte(FLASH_IDREAD);
    for(u8 i = 0;i < 4;i++) {
        TempVal[i] = SPI_FLASH_ReadByte();
    }
    BSP_FLASH_CS_HIGH();
    Temp = (TempVal[0] << 24) | (TempVal[1] << 16) | (TempVal[2]<<8) | TempVal[3];
    return Temp;
}

/*******************************************************************************
* Function Name  : SPI_FLASH_Init
* Description    : Initializes the peripherals used by the SPI FLASH driver.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SPI_FLASH_Init(void)
{
    /***********************************************
    * 描述： 初始化Flash芯片控制引脚
    */
    BSP_FlashGPIO_Config();

#if (BSP_USE_SPI_MODE == 1)
    /***********************************************
    * 描述： 初始化硬件SPI
    */
    BSP_SPIxMode_Config();
#endif

    /***********************************************
    * 描述： 通过读取自身ID来判断是否正常工作
    */
    if(BSP_FLASH_ReadID() == AT45DB161E_FLASH_ID) {
        SPI_FLASH_DisableSecProtect(); //去除保护
        FlashStauteByte = SPI_FLASH_GetStatueByte();
        if(FlashStauteByte&0x01){
            //该芯片为512 bytes per page.
            asm("nop");
        } else {
            asm("nop");
            //该芯片为528 bytes per page.
            SPI_FLASH_SetAt45DB161_512BytesOnePage(); //将528版本变成512版本
        }
        do {
            FlashStauteByte = SPI_FLASH_GetStatueByte();
        }while((FlashStauteByte&0x80) != 0x80); //等待芯片ready.
    } else {
       asm("nop");
    }
}

//void SPI_FLASH_BlockErase(u32 BlockAddr) //modified
//{
////  /* Sector Erase */
////  /* Select the FLASH: Chip Select low */
////  BSP_FLASH_CS_LOW();
////
////  /* Send Sector Erase instruction */
////  SPI_FLASH_SendByte(BLE);
////
////  /* Send SectorAddr high nibble address byte */
////  SPI_FLASH_SendByte((BlockAddr & 0xFF0000) >> 16);
////  /* Send SectorAddr medium nibble address byte */
////  SPI_FLASH_SendByte((BlockAddr & 0xFF00) >> 8);
////  /* Send SectorAddr low nibble address byte */
////  SPI_FLASH_SendByte(BlockAddr & 0xFF);
////  /* Deselect the FLASH: Chip Select high */
////  BSP_FLASH_CS_HIGH();
////
////  /* Wait the end of Flash writing */
////  SPI_FLASH_WaitForWriteEnd();
//}
/*******************************************************************************
* Function Name  : SPI_FLASH_SectorErase
* Description    : Erases the specified FLASH sector.
* Input          : SectorAddr: address of the sector to erase.
* Output         : None
* Return         : None
*******************************************************************************/
void SPI_FLASH_SectorErase(u32 SectorAddr) //modified
{
//#ifdef _USING_M25P64_
//  /* Send write enable instruction */
//  SPI_FLASH_WriteEnable();
//
//  /* Sector Erase */
//  /* Select the FLASH: Chip Select low */
//  BSP_FLASH_CS_LOW();
//  /* Send Sector Erase instruction */
//  SPI_FLASH_SendByte(SE);
//
//  /* Send SectorAddr high nibble address byte */
//  SPI_FLASH_SendByte((SectorAddr & 0xFF0000) >> 16);
//  /* Send SectorAddr medium nibble address byte */
//  SPI_FLASH_SendByte((SectorAddr & 0xFF00) >> 8);
//  /* Send SectorAddr low nibble address byte */
//  SPI_FLASH_SendByte(SectorAddr & 0xFF);
//
//  if(SectorAddr > 0x0f)
//    return;
//  else
//    SectorAddr <<= 18;
//  /* Send SectorAddr high nibble address byte */
//  SPI_FLASH_SendByte((SectorAddr & 0xFF0000) >> 16);
//  /* Send SectorAddr medium nibble address byte */
//  SPI_FLASH_SendByte((SectorAddr & 0xFF00) >> 8);
//  /* Send SectorAddr low nibble address byte */
//  SPI_FLASH_SendByte(SectorAddr & 0xFF);
//  /* Deselect the FLASH: Chip Select high */
//  BSP_FLASH_CS_HIGH();
//
//  /* Wait the end of Flash writing */
//  SPI_FLASH_WaitForWriteEnd();
//#endif
}

/*******************************************************************************
* Function Name  : SPI_FLASH_BlockErase(wisure add)
* Description    : Erases the specified FLASH block.
* Input          : BlockAddr: address of the block to erase.(4K/block)
* Output         : None
* Return         : None
*******************************************************************************/
void SPI_FLASH_BlockErase(u32 BlockAddr) //modified
{
//  /* Sector Erase */
//  /* Select the FLASH: Chip Select low */
//  BSP_FLASH_CS_LOW();
//
//  /* Send Sector Erase instruction */
//  SPI_FLASH_SendByte(BLE);
//
//  /* Send SectorAddr high nibble address byte */
//  SPI_FLASH_SendByte((BlockAddr & 0xFF0000) >> 16);
//  /* Send SectorAddr medium nibble address byte */
//  SPI_FLASH_SendByte((BlockAddr & 0xFF00) >> 8);
//  /* Send SectorAddr low nibble address byte */
//  SPI_FLASH_SendByte(BlockAddr & 0xFF);
//  /* Deselect the FLASH: Chip Select high */
//  BSP_FLASH_CS_HIGH();
//
//  /* Wait the end of Flash writing */
//  SPI_FLASH_WaitForWriteEnd();
}

void SPI_FLASH_PageErase(u32 Addr) //modified
{
/* Bulk Erase */
  /* Select the FLASH: Chip Select low */
  BSP_FLASH_CS_LOW();

  /* Send Bulk Erase instruction  */
  SPI_FLASH_SendByte(0x81);

  SPI_FLASH_SendByte((Addr & 0xFF0000) >> 16);
  SPI_FLASH_SendByte((Addr & 0x00FF00) >> 8);
  SPI_FLASH_SendByte(Addr & 0x0000FF);

  /* Deselect the FLASH: Chip Select high */
  BSP_FLASH_CS_HIGH();

  /* Wait the end of Flash writing */
  SPI_FLASH_WaitForWriteEnd();
}
/*******************************************************************************
* Function Name  : SPI_FLASH_BulkErase
* Description    : Erases the entire FLASH.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SPI_FLASH_BulkErase(void)  // modified
{
  /* Bulk Erase */
  /* Select the FLASH: Chip Select low */
  BSP_FLASH_CS_LOW();

  /* Send Bulk Erase instruction  */
  SPI_FLASH_SendByte(0xC7);
  SPI_FLASH_SendByte(0x94);
  SPI_FLASH_SendByte(0x80);
  SPI_FLASH_SendByte(0x9A);

  /* Deselect the FLASH: Chip Select high */
  BSP_FLASH_CS_HIGH();

  /* Wait the end of Flash writing */
  SPI_FLASH_WaitForWriteEnd();
}
void SPI_FLASH_EnableSecProtect(void)
{
  BSP_FLASH_CS_LOW();
  /* Send "Write to Buff " instruction */
  SPI_FLASH_SendByte(0x3D);
  SPI_FLASH_SendByte(0x2A);
  SPI_FLASH_SendByte(0x7F);
  SPI_FLASH_SendByte(0xA9);
  /* Deselect the FLASH: Chip Select high */
  BSP_FLASH_CS_HIGH();
}
void SPI_FLASH_DisableSecProtect(void)
{
  BSP_FLASH_CS_LOW();
  /* Send "Write to Buff " instruction */
  SPI_FLASH_SendByte(0x3D);
  SPI_FLASH_SendByte(0x2A);
  SPI_FLASH_SendByte(0x7F);
  SPI_FLASH_SendByte(0x9A);
  /* Deselect the FLASH: Chip Select high */
  BSP_FLASH_CS_HIGH();
}
/*******************************************************************************
* Function Name  : SPI_FLASH_PageWrite
* Description    : Writes more than one byte to the FLASH with a single WRITE
*                  cycle(Page WRITE sequence). The number of byte can't exceed
*                  the FLASH page size.
* Input          : - pBuffer : pointer to the buffer  containing the data to be
*                    written to the FLASH.
*                  - WriteAddr : FLASH's internal address to write to.
*                  - NumByteToWrite : number of bytes to write to the FLASH,
*                    must be equal or less than "BSP_FLASH_PageSize" value.
* Output         : None
* Return         : None
*******************************************************************************/
void SPI_FLASH_PageWrite(u8* pBuffer, u32 WriteAddr, u16 NumByteToWrite)// modified
{
  if(NumByteToWrite > BSP_FLASH_PageSize)
    return;

  //----------------写PAGE---------------------------------------
  BSP_FLASH_CS_LOW();

  SPI_FLASH_SendByte(0x82);

  /* Send WriteAddr high nibble address byte to write to */
  SPI_FLASH_SendByte((WriteAddr & 0xFF0000) >> 16);
  /* Send WriteAddr medium nibble address byte to write to */
  SPI_FLASH_SendByte((WriteAddr & 0x00FF00) >> 8);
  /* Send WriteAddr low nibble address byte to write to */
  SPI_FLASH_SendByte(WriteAddr & 0x0000FF);

  while (NumByteToWrite--) /* while there is data to be read */
  {
    /* Read a byte from the FLASH */
    SPI_FLASH_SendByte(*pBuffer);
    /* Point to the next location where the byte read will be saved */
    pBuffer++;
  }
  BSP_FLASH_CS_HIGH();
  /* Wait the end of Flash writing */
  SPI_FLASH_WaitForWriteEnd();
}

/*******************************************************************************
* Function Name  : SPI_FLASH_BufferWrite
* Description    : Writes block of data to the FLASH. In this function, the
*                  number of WRITE cycles are reduced, using Page WRITE sequence.
* Input          : - pBuffer : pointer to the buffer  containing the data to be
*                    written to the FLASH.
*                  - WriteAddr : FLASH's internal address to write to.
*                  - NumByteToWrite : number of bytes to write to the FLASH.
* Output         : None
* Return         : None
*******************************************************************************/
void SPI_FLASH_BufferWrite(u8* pBuffer, u32 WriteAddr, u16 NumByteToWrite)
{
    u16 NumOfPage = 0, NumOfSingle = 0, Addr = 0, count = 0, temp = 0;

    Addr = WriteAddr % BSP_FLASH_PageSize;
    count = BSP_FLASH_PageSize - Addr; //本页剩余空间
    NumOfPage =  NumByteToWrite / BSP_FLASH_PageSize;
    NumOfSingle = NumByteToWrite % BSP_FLASH_PageSize;

    if (Addr == 0) {/* WriteAddr is BSP_FLASH_PageSize aligned  */
        if (NumOfPage == 0){ /* NumByteToWrite < BSP_FLASH_PageSize 不满一页直接写*/
            SPI_FLASH_PageWrite(pBuffer, WriteAddr, NumByteToWrite);
        } else {/* NumByteToWrite > BSP_FLASH_PageSize */

            while (NumOfPage--) {//先写整PAGE
                SPI_FLASH_PageWrite(pBuffer, WriteAddr, BSP_FLASH_PageSize);
                WriteAddr +=  BSP_FLASH_PageSize;
                pBuffer += BSP_FLASH_PageSize;
            }
            SPI_FLASH_PageWrite(pBuffer, WriteAddr, NumOfSingle); //写不足一页PAGE
        }
    } else {/* WriteAddr is not BSP_FLASH_PageSize aligned  */
        if (NumOfPage == 0) {/* NumByteToWrite < BSP_FLASH_PageSize */
            if (NumOfSingle > count) {/* (NumByteToWrite + WriteAddr) > BSP_FLASH_PageSize 不足一页需要跨页情况*/

                temp = NumOfSingle - count;

                SPI_FLASH_PageWrite(pBuffer, WriteAddr, count);
                WriteAddr +=  count;
                pBuffer += count;

                SPI_FLASH_PageWrite(pBuffer, WriteAddr, temp);
            } else {
                SPI_FLASH_PageWrite(pBuffer, WriteAddr, NumByteToWrite);
            }
        } else {/* NumByteToWrite > BSP_FLASH_PageSize */

            NumByteToWrite -= count;
            NumOfPage =  NumByteToWrite / BSP_FLASH_PageSize;
            NumOfSingle = NumByteToWrite % BSP_FLASH_PageSize;

            SPI_FLASH_PageWrite(pBuffer, WriteAddr, count);
            WriteAddr +=  count;
            pBuffer += count;

            while (NumOfPage--) {
                SPI_FLASH_PageWrite(pBuffer, WriteAddr, BSP_FLASH_PageSize);
                WriteAddr +=  BSP_FLASH_PageSize;
                pBuffer += BSP_FLASH_PageSize;
            }
            if (NumOfSingle != 0) {
                SPI_FLASH_PageWrite(pBuffer, WriteAddr, NumOfSingle);
            }
        }
    }
}
//
void SPI_FLASH_Ram1Write(u8* pBuffer, u32 WriteAddr, u16 NumByteToWrite)
{
  /* Select the FLASH: Chip Select low */
  BSP_FLASH_CS_LOW();

  /* Send "Read from Memory " instruction */
  SPI_FLASH_SendByte(0x84);

  /* Send ReadAddr high nibble address byte to read from */
  SPI_FLASH_SendByte((WriteAddr & 0xFF0000) >> 16);
  /* Send ReadAddr medium nibble address byte to read from */
  SPI_FLASH_SendByte((WriteAddr& 0x00FF00) >> 8);
  /* Send ReadAddr low nibble address byte to read from */
  SPI_FLASH_SendByte(WriteAddr & 0x0000FF);

  while (NumByteToWrite--) {/* while there is data to be read */
    /* Read a byte from the FLASH */
    SPI_FLASH_SendByte(*pBuffer);
    /* Point to the next location where the byte read will be saved */
    pBuffer++;
  }

  /* Deselect the FLASH: Chip Select high */
  BSP_FLASH_CS_HIGH();
}
/*******************************************************************************
* Function Name  : SPI_FLASH_Ram1Read
* Description    : 读FLASH的BUFF1
* Input          : - pBuffer : pointer to the buffer that receives the data read
*                    from the FLASH.
*                  - ReadAddr : FLASH's internal address to read from.
*                  - NumByteToRead : number of bytes to read from the FLASH.
* Output         : None
* Return         : None
*******************************************************************************/
void SPI_FLASH_Ram1Read(u8* pBuffer, u32 ReadAddr, u16 NumByteToRead)
{
  /* Select the FLASH: Chip Select low */
  BSP_FLASH_CS_LOW();

  /* Send "Read from Memory " instruction */
  SPI_FLASH_SendByte(0xD4);

  /* Send ReadAddr high nibble address byte to read from */
  SPI_FLASH_SendByte((ReadAddr & 0xFF0000) >> 16);
  /* Send ReadAddr medium nibble address byte to read from */
  SPI_FLASH_SendByte((ReadAddr& 0x00FF00) >> 8);
  /* Send ReadAddr low nibble address byte to read from */
  SPI_FLASH_SendByte(ReadAddr & 0x0000FF);


  //发送1个DUMMY字节
  SPI_FLASH_SendByte(Dummy_Byte);


  while (NumByteToRead--) {/* while there is data to be read */

    /* Read a byte from the FLASH */
    *pBuffer = SPI_FLASH_SendByte(Dummy_Byte);
    /* Point to the next location where the byte read will be saved */
    pBuffer++;
  }

  /* Deselect the FLASH: Chip Select high */
  BSP_FLASH_CS_HIGH();
}
/*******************************************************************************
* Function Name  : SPI_FLASH_ArraryRead
* Description    : Reads a block of data from the FLASH.
* Input          : - pBuffer : pointer to the buffer that receives the data read
*                    from the FLASH.
*                  - ReadAddr : FLASH's internal address to read from.
*                  - NumByteToRead : number of bytes to read from the FLASH.
* Output         : None
* Return         : None
*******************************************************************************/
void SPI_FLASH_ArraryRead(u8* pBuffer, u32 ReadAddr, u16 NumByteToRead)
{
  /* Select the FLASH: Chip Select low */
  BSP_FLASH_CS_LOW();

  /* Send "Read from Memory " instruction */
  SPI_FLASH_SendByte(0xE8);

  /* Send ReadAddr high nibble address byte to read from */
  SPI_FLASH_SendByte((ReadAddr & 0xFF0000) >> 16);
  /* Send ReadAddr medium nibble address byte to read from */
  SPI_FLASH_SendByte((ReadAddr& 0xFF00) >> 8);
  /* Send ReadAddr low nibble address byte to read from */
  SPI_FLASH_SendByte(ReadAddr & 0xFF);

  SPI_FLASH_SendByte(Dummy_Byte);
  SPI_FLASH_SendByte(Dummy_Byte);
  SPI_FLASH_SendByte(Dummy_Byte);
  SPI_FLASH_SendByte(Dummy_Byte);
//
  while (NumByteToRead--) {/* while there is data to be read */

    /* Read a byte from the FLASH */
    *pBuffer = SPI_FLASH_SendByte(Dummy_Byte);
    /* Point to the next location where the byte read will be saved */
    pBuffer++;
  }

  /* Deselect the FLASH: Chip Select high */
  BSP_FLASH_CS_HIGH();
}
/*******************************************************************************
* Function Name  : SPI_FLASH_GetProtectStatue
* Description    : Get Flash protect statue
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
FlashPretectStatue SPI_FLASH_GetProtectStatue(u8 sectorNo)//wisure added
{
  static u8 SectorProtectStatue[16],i;
  FlashPretectStatue protectStatue;

  BSP_FLASH_CS_LOW(); //片选
  SPI_FLASH_SendByte(0x32);
  SPI_FLASH_SendByte(Dummy_Byte); //填充数据
  SPI_FLASH_SendByte(Dummy_Byte);
  SPI_FLASH_SendByte(Dummy_Byte);

  for(i=0;i<16;i++)
  {
    SectorProtectStatue[i] = SPI_FLASH_ReadByte();
  }

  if(SectorProtectStatue[sectorNo] == 0xFF) {

    protectStatue = SECTOR_PROTECT;
  }
  if(SectorProtectStatue[sectorNo] == 0x00) {

    protectStatue = SECTOR_UNPROTECT;
  }
  /* Deselect the FLASH: Chip Select high */
  BSP_FLASH_CS_HIGH();
  return protectStatue;
}
/*******************************************************************************
* Function Name  : SPI_FLASH_ProtectSet
* Description    : Flash protect set
* Input          : setValue
*                           1:enable chip protect
*                           0:disable chip protect
* Output         : operate result
*                           0:OK
*                           1:ERROR
* Return         : None
*******************************************************************************/
void SPI_FLASH_ProtectSet(u8 setValue)//wisure added
{
  SPI_FLASH_WriteEnable();

  /* Select the FLASH: Chip Select low */
  BSP_FLASH_CS_LOW();
  /* Send "Read Status Register" instruction */
  SPI_FLASH_SendByte(0x01);
  if(setValue == 1) {

    /* Global Protect */
    SPI_FLASH_SendByte(0x7f);
  } else if(setValue == 0) {
    /* Global Unprotect */
    SPI_FLASH_SendByte(0x00);
  }
}


/*******************************************************************************
* Function Name  : SPI_FLASH_StartReadSequence
* Description    : Initiates a read data byte (READ) sequence from the Flash.
*                  This is done by driving the /CS line low to select the device,
*                  then the READ instruction is transmitted followed by 3 bytes
*                  address. This function exit and keep the /CS line low, so the
*                  Flash still being selected. With this technique the whole
*                  content of the Flash is read with a single READ instruction.
* Input          : - ReadAddr : FLASH's internal address to read from.
* Output         : None
* Return         : None
*******************************************************************************/
void SPI_FLASH_StartReadSequence(u32 ReadAddr)
{
  /* Select the FLASH: Chip Select low */
  BSP_FLASH_CS_LOW();

  /* Send "Read from Memory " instruction */
  SPI_FLASH_SendByte(FLASH_CLREAD);

  /* Send the 24-bit address of the address to read from -----------------------*/
  /* Send ReadAddr high nibble address byte */
  SPI_FLASH_SendByte((ReadAddr & 0xFF0000) >> 16);
  /* Send ReadAddr medium nibble address byte */
  SPI_FLASH_SendByte((ReadAddr& 0xFF00) >> 8);
  /* Send ReadAddr low nibble address byte */
  SPI_FLASH_SendByte(ReadAddr & 0xFF);
}



/*******************************************************************************
* Function Name  : SPI_FLASH_WriteEnable
* Description    : Enables the write access to the FLASH.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SPI_FLASH_WriteEnable(void)// modified
{
  /* Select the FLASH: Chip Select low */
//  BSP_FLASH_CS_LOW();
//
//  /* Send "Write Enable" instruction */
//  SPI_FLASH_SendByte(WREN);
//
//  /* Deselect the FLASH: Chip Select high */
//  BSP_FLASH_CS_HIGH();
}

/*******************************************************************************
* Function Name  : SPI_FLASH_WaitForWriteEnd
* Description    : Polls the status of the Write In Progress (WIP) flag in the
*                  FLASH's status  register  and  loop  until write  opertaion
*                  has completed.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SPI_FLASH_WaitForWriteEnd(void)
{
  u8 flashStatue = 0;

  /* Select the FLASH: Chip Select low */
  BSP_FLASH_CS_LOW();

  /* Send "Read Status Register" instruction */
  SPI_FLASH_SendByte(FLASH_STATUSREAD);

  /* Loop as long as the memory is busy with a write cycle */
  do
  {
    /* Send a dummy byte to generate the clock needed by the FLASH
    and put the value of the status register in FLASH_Status variable */
    flashStatue = SPI_FLASH_SendByte(Dummy_Byte);
  }
  while ((flashStatue & 0x80) == 0); /* Write in progress */

  /* Deselect the FLASH: Chip Select high */
  BSP_FLASH_CS_HIGH();
}
/*******************************************************************************
* Function Name  : SPI_FLASH_GetStatueBYTE
* Description    :
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
u8 SPI_FLASH_GetStatueByte(void)
{
  u8 flashStatue = 0;

  /* Select the FLASH: Chip Select low */
  BSP_FLASH_CS_LOW();

  /* Send "Read Status Register" instruction */
  SPI_FLASH_SendByte(FLASH_STATUSREAD);

  flashStatue = SPI_FLASH_ReadByte();
  /* Deselect the FLASH: Chip Select high */
  BSP_FLASH_CS_HIGH();

  return flashStatue;
}
//注意：AT45DB161有528bytes per page版本和512bytes per page版本，购买时注意区别
//如果购买的是528bytes版本，则通过下面函数将528版本变成512版本，该过程一旦执行，
//则该不可逆回528版本，且只需调用一次即可!!!
void SPI_FLASH_SetAt45DB161_512BytesOnePage(void)
{
  u8 flashStatue = 0;
  /* Select the FLASH: Chip Select low */
  BSP_FLASH_CS_LOW();

  /* Send "Read Status Register" instruction */
  SPI_FLASH_SendByte(0x3D);
  SPI_FLASH_SendByte(0x2A);
  SPI_FLASH_SendByte(0x80);
  SPI_FLASH_SendByte(0xA6);

  /* Deselect the FLASH: Chip Select high */
  BSP_FLASH_CS_HIGH();

  do
  {
    flashStatue = SPI_FLASH_GetStatueByte();
  }while((flashStatue&0x80) != 0x80);
}
/******************* (C) COPYRIGHT 2008 STMicroelectronics *****END OF FILE****/
