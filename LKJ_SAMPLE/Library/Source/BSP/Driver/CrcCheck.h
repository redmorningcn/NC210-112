
#ifndef  _CrcCheck_h_
#define  _CrcCheck_h_

//----------------------------------------------------------------------------
// 名    称：   uint16_t GetCrc16Check(uint8_t *Buf, uint16_t Len)
// 功    能：   取CRC16校验和
// 入口参数：  
// 出口参数：   
//----------------------------------------------------------------------------
uint16_t GetCrc16Check(uint8_t *Buf, uint16_t Len);

unsigned short GetModBusCrc16(unsigned char *puchMsg,unsigned short  usDataLen);

unsigned short GetModBusCrc16Up(unsigned char *puchMsg,unsigned short  usDataLen);

//----------------------------------------------------------------------------
// 名    称：   uint8_t GetCheckSum(uint8_t  Buf[], uint32_t Len)
// 功    能:    取校验和
// 入口参数：   Buf：求数组   Len：数组长度
// 出口参数：   校验和
//----------------------------------------------------------------------------
uint8_t GetCheckSum(uint8_t  Buf[], uint32_t Len);

//----------------------------------------------------------------------------
// 名    称：   uint16_t GetCheckSum16(uint8_t  Buf[], uint32_t Len)
// 功    能:    取校验和
// 入口参数：   Buf：求数组   Len：数组长度
// 出口参数：   校验和
//----------------------------------------------------------------------------
uint16_t GetCheckSum16(uint8_t  Buf[], uint32_t Len);

extern uint16_t crc16(uint8_t *ptr, uint8_t len);

#endif


