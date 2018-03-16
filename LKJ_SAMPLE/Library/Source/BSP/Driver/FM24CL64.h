#ifndef		__FM24CL16_h__
#define		__FM24CL16_h__

extern uint8_t WriteFM24CL64(uint16_t  Addr, uint8_t *DataBuf, uint16_t DataBufLen);

extern uint8_t ReadFM24CL64(uint16_t  Addr, uint8_t *DataBuf, uint16_t DataBufLen);

extern void TO_Arayy(uint8_t *buf , uint32_t NUM);

#endif
