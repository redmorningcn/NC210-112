/*------------------------------------------------------------------------/
/  Universal string handler for user console interface
/-------------------------------------------------------------------------/
/
/  Copyright (C) 2011, ChaN, all right reserved.
/
/ * This software is a free software and there is NO WARRANTY.
/ * No restriction on use. You can use, modify and redistribute it for
/   personal, non-profit or commercial products UNDER YOUR RESPONSIBILITY.
/ * Redistributions of source code must retain the above copyright notice.
/ modified by www.armjishu.com
/-------------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "SZ_STM32F107VC_LIB.h"
#include "stm32f10x_gpio.h"
#include "RS485.h"
#include <stdio.h>
#include "DELAY.h"
#include "CrcCheck.h"
#include "string.h"
#ifdef PHOTOELECTRIC_VELOCITY_MEASUREMENT
#include <Speed_sensor.h>
#include "app_task_mater.h"
#else
#include <power_macro.h>
#endif

#define RCC_RS485_DIR      		RCC_APB2Periph_GPIOA
#define GPIO_RS485_DIR          GPIO_Pin_1
#define GPIO_RS485_DIR_PORT     GPIOA

#define SetRS485Rev()           GPIO_ResetBits(GPIO_RS485_DIR_PORT, GPIO_RS485_DIR)
#define SetRS485Snd()           GPIO_SetBits(GPIO_RS485_DIR_PORT, GPIO_RS485_DIR)

//思维公司和南车公司通讯协议
#define         FRAM_HERD0     		0x7c         	//报文头			
#define     	HOST_ID      		0x11				//上位机地址	  		   
#define   		DEVICE_ID0      	0x51				//本机地址0	
#define         FRAM_END0     		0x7e         	//报文尾
#define			HOST_CODE_FLG		0x0c	
#define			DEVICE_CODE_FLG		0x8c
#define			ID_NUM				0x01	
#define			CODE_TYPE			0x01	

#define			HDLC_CODE0			0x7d
#define			HDLC_CODE1			0x5c
#define			HDLC_CODE2			0x5e
#define			HDLC_CODE3			0x5d


#if _USE_XFUNC_OUT
#include <stdarg.h>
void (*xfunc_out)(unsigned char);	/* Pointer to the output stream */
static char *outptr;

#ifndef SZ_STM32_COM1_STR
#define SZ_STM32_COM1_STR                    "USART2"
#endif

extern const uint8_t STM32F10x_STR[];
/* Private functions ---------------------------------------------------------*/
/**-------------------------------------------------------
* @函数名 RS485_SET_RX_Mode
* @功能   设置RS485为接收模式
* @参数   无
* @返回值 无
***------------------------------------------------------*/
void RS485_SET_RX_Mode(void)
{
    /*设置为接收模式*/
    SetRS485Rev();
}

/**-------------------------------------------------------
* @函数名 RS485_SET_TX_Mode
* @功能   设置RS485为发送模式
* @参数   无
* @返回值 无
***------------------------------------------------------*/
void RS485_SET_TX_Mode(void)
{
    /*设置为发送模式*/
    SetRS485Snd();
}


/**-------------------------------------------------------
* @函数名 RS485_DIR_Config
* @功能   RS485方向设置IO口初始化
* @参数   无
* @返回值 无
***------------------------------------------------------*/
void RS485_DIR_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    /* Enable GPIOB, GPIOC and AFIO clock */
    RCC_APB2PeriphClockCmd(RCC_RS485_DIR , ENABLE);
    
    /* LEDs pins configuration */
    GPIO_InitStructure.GPIO_Pin = GPIO_RS485_DIR;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIO_RS485_DIR_PORT, &GPIO_InitStructure);
}


void xPrintf_Init(uint32_t BaudRate)
{
    USART_InitTypeDef USART_InitStructure;   
    
    /* USARTx configured as follow:
    - BaudRate = 115200 baud  
    - Word Length = 8 Bits
    - One Stop Bit
    - No parity
    - Hardware flow control disabled (RTS and CTS signals)
    - Receive and transmit enabled
    */
    USART_InitStructure.USART_BaudRate = BaudRate;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    
    __SZ_STM32_COMInit(COM2, &USART_InitStructure);
    RS485_DIR_Config();							  //RS485方向设置IO口初始化
    xdev_out(xUSART2_putchar);
    xdev_in(xUSART2_getchar);  
    
    SystemCoreClockUpdate();
}

unsigned char xUSART1_putchar(unsigned char ch)
{
	/* Place your implementation of fputc here */
	/* e.g. write a character to the USART */
	USART_SendData(SZ_STM32_COM1, (uint8_t) ch); /*发送一个字符函数*/ 
	
	/* Loop until the end of transmission */
	while (USART_GetFlagStatus(SZ_STM32_COM1, USART_FLAG_TC) == RESET)/*等待发送完成*/
	{
        
	}
	return ch;
}

unsigned char xUSART2_putchar(unsigned char ch)
{
	/* Place your implementation of fputc here */
	/* e.g. write a character to the USART */
	RS485_SET_TX_Mode();
	Delay(10);
	USART_SendData(SZ_STM32_COM2, (uint8_t) ch); /*发送一个字符函数*/ 
	
	/* Loop until the end of transmission */
	while (USART_GetFlagStatus(SZ_STM32_COM2, USART_FLAG_TC) == RESET)/*等待发送完成*/
	{
        
	}
	Delay(10);
	RS485_SET_RX_Mode();
	return ch;
}



//------------------------------------------------------------------------
// 名    称：   uint8_t SendCOM2(void *buf,uint32_t len)
// 功    能：   串口0查询发送
// 入口参数：   *buf   入口地址
//              len    读取长度
// 出口参数：   发送成功返回1，不正常返回0
//------------------------------------------------------------------------
uint8_t SendCOM2(void *buf,uint32_t len)
{
    uint8_t ch = 0;
	uint8_t *p;
    if( (!len) )
    {
        return 0;
    }   
    p = buf;
    for(;len > 0;len--)
    {
        //        U0THR = *p++;
        xUSART2_putchar(*p++);
        //		SysFastHoldTast();
		
        while (USART_GetFlagStatus(SZ_STM32_COM2, USART_FLAG_TC) == RESET);/*等待发送完成*/
		ch = (int)SZ_STM32_COM2->DR & 0xFF;
        
    	putchar(ch); //回显
        //        {SysFastHoldTast();}  
    }
    
    RS485_SET_RX_Mode();    
    return 1;    
}


/*----------------------------------------------*/
/* Put a character                              */
/*----------------------------------------------*/
void xputc (char c /* A character to be output */) 
{
	if (_CR_CRLF && c == '\n') xputc('\r');		/* CR -> CRLF */
    
	if (outptr) {
		*outptr++ = (unsigned char)c;
		return;
	}
    
	if (xfunc_out) xfunc_out((unsigned char)c);
}



/*----------------------------------------------*/
/* Put a null-terminated string                 */
/*----------------------------------------------*/

void xputs (					/* Put a string to the default device */
            const char* str				/* Pointer to the string */
                )
{
	while (*str)
		xputc(*str++);
}


void xfputs (					/* Put a string to the specified device */
             void(*func)(unsigned char),	/* Pointer to the output function */
             const char*	str				/* Pointer to the string */
                 )
{
	void (*pf)(unsigned char);
    
    
	pf = xfunc_out;		/* Save current output device */
	xfunc_out = func;	/* Switch output to specified device */
	while (*str)		/* Put the string */
		xputc(*str++);
	xfunc_out = pf;		/* Restore output device */
}



/*----------------------------------------------*/
/* Formatted string output                      */
/*----------------------------------------------*/
/*  xprintf("%d", 1234);			"1234"
xprintf("%6d,%3d%%", -200, 5);	"  -200,  5%"
xprintf("%-6u", 100);			"100   "
xprintf("%ld", 12345678L);		"12345678"
xprintf("%04x", 0xA3);			"00a3"
xprintf("%08LX", 0x123ABC);		"00123ABC"
xprintf("%016b", 0x550F);		"0101010100001111"
xprintf("%s", "String");		"String"
xprintf("%-4s", "abc");			"abc "
xprintf("%4s", "abc");			" abc"
xprintf("%c", 'a');				"a"
xprintf("%f", 10.0);            <xprintf lacks floating point support>
*/

static
void xvprintf (
               const char*	fmt,	/* Pointer to the format string */
               va_list arp			/* Pointer to arguments */
                   )
{
	unsigned int r, i, j, w, f;
	unsigned long v;
	char s[16], c, d, *p;
    
    
	for (;;) {
		c = *fmt++;					/* Get a char */
		if (!c) break;				/* End of format? */
		if (c != '%') {				/* Pass through it if not a % sequense */
			xputc(c); continue;
		}
		f = 0;
		c = *fmt++;					/* Get first char of the sequense */
		if (c == '0') {				/* Flag: '0' padded */
			f = 1; c = *fmt++;
		} else {
			if (c == '-') {			/* Flag: left justified */
				f = 2; c = *fmt++;
			}
		}
		for (w = 0; c >= '0' && c <= '9'; c = *fmt++)	/* Minimum width */
			w = w * 10 + c - '0';
		if (c == 'l' || c == 'L') {	/* Prefix: Size is long int */
			f |= 4; c = *fmt++;
		}
		if (!c) break;				/* End of format? */
		d = c;
		if (d >= 'a') d -= 0x20;
		switch (d) {				/* Type is... */
		case 'S' :					/* String */
			p = va_arg(arp, char*);
			for (j = 0; p[j]; j++) ;
			while (!(f & 2) && j++ < w) xputc(' ');
			xputs(p);
			while (j++ < w) xputc(' ');
			continue;
		case 'C' :					/* Character */
			xputc((char)va_arg(arp, int)); continue;
		case 'B' :					/* Binary */
			r = 2; break;
		case 'O' :					/* Octal */
			r = 8; break;
		case 'D' :					/* Signed decimal */
		case 'U' :					/* Unsigned decimal */
			r = 10; break;
		case 'X' :					/* Hexdecimal */
			r = 16; break;
		default:					/* Unknown type (passthrough) */
			xputc(c); continue;
		}
        
		/* Get an argument and put it in numeral */
		v = (f & 4) ? va_arg(arp, long) : ((d == 'D') ? (long)va_arg(arp, int) : (long)va_arg(arp, unsigned int));
		if (d == 'D' && (v & 0x80000000)) {
			v = 0 - v;
			f |= 8;
		}
		i = 0;
		do {
			d = (char)(v % r); v /= r;
			if (d > 9) d += (c == 'x') ? 0x27 : 0x07;
			s[i++] = d + '0';
		} while (v && i < sizeof(s));
		if (f & 8) s[i++] = '-';
		j = i; d = (f & 1) ? '0' : ' ';
		while (!(f & 2) && j++ < w) xputc(d);
		do xputc(s[--i]); while(i);
		while (j++ < w) xputc(' ');
	}
}


/*----------------------------------------------/
/  xprintf - Formatted string output
/----------------------------------------------*/
/*  xprintf("%d", 1234);            "1234"
xprintf("%6d,%3d%%", -200, 5);  "  -200,  5%"
xprintf("%-6u", 100);           "100   "
xprintf("%ld", 12345678L);      "12345678"
xprintf("%04x", 0xA3);          "00a3"
xprintf("%08LX", 0x123ABC);     "00123ABC"
xprintf("%016b", 0x550F);       "0101010100001111"
xprintf("%s", "String");        "String"
xprintf("%-4s", "abc");         "abc "
xprintf("%4s", "abc");          " abc"
xprintf("%c", 'a');             "a"
xprintf("%f", 10.0);            <xprintf lacks floating point support>
*/
void xprintf (			/* Put a formatted string to the default device */
              const char*	fmt,	/* Pointer to the format string */
              ...					/* Optional arguments */
                  )
{
	va_list arp;
    
    
	va_start(arp, fmt);
	xvprintf(fmt, arp);
	va_end(arp);
}


void xsprintf (			/* Put a formatted string to the memory */
               char* buff,			/* Pointer to the output buffer */
               const char*	fmt,	/* Pointer to the format string */
               ...					/* Optional arguments */
                   )
{
	va_list arp;
    
    
	outptr = buff;		/* Switch destination for memory */
    
	va_start(arp, fmt);
	xvprintf(fmt, arp);
	va_end(arp);
    
	*outptr = 0;		/* Terminate output string with a \0 */
	outptr = 0;			/* Switch destination for device */
}


void xfprintf (					/* Put a formatted string to the specified device */
               void(*func)(unsigned char),	/* Pointer to the output function */
               const char*	fmt,			/* Pointer to the format string */
               ...							/* Optional arguments */
                   )
{
	va_list arp;
	void (*pf)(unsigned char);
    
    
	pf = xfunc_out;		/* Save current output device */
	xfunc_out = func;	/* Switch output to specified device */
    
	va_start(arp, fmt);
	xvprintf(fmt, arp);
	va_end(arp);
    
	xfunc_out = pf;		/* Restore output device */
}



/*----------------------------------------------*/
/* Dump a line of binary dump                   */
/*----------------------------------------------*/

void put_dump (
               const void* buff,		/* Pointer to the array to be dumped */
               unsigned long addr,		/* Heading address value */
               int len,				/* Number of items to be dumped */
               int width				/* Size of the items (DF_CHAR, DF_SHORT, DF_LONG) */
                   )
{
	int i;
	const unsigned char *bp;
	const unsigned short *sp;
	const unsigned long *lp;
    
    
	xprintf("%08lX ", addr);		/* address */
    
	switch (width) {
	case DW_CHAR:
		bp = buff;
		for (i = 0; i < len; i++)		/* Hexdecimal dump */
			xprintf(" %02X", bp[i]);
		xputc(' ');
		for (i = 0; i < len; i++)		/* ASCII dump */
			xputc((bp[i] >= ' ' && bp[i] <= '~') ? bp[i] : '.');
		break;
	case DW_SHORT:
		sp = buff;
		do								/* Hexdecimal dump */
			xprintf(" %04X", *sp++);
		while (--len);
		break;
	case DW_LONG:
		lp = buff;
		do								/* Hexdecimal dump */
			xprintf(" %08LX", *lp++);
		while (--len);
		break;
	}
    
	xputc('\n');
}

#endif /* _USE_XFUNC_OUT */



#if _USE_XFUNC_IN
unsigned char (*xfunc_in)(void);	/* Pointer to the input stream */


unsigned char xUSART2_getchar(void)
{
    unsigned char key = 0;
    
    /* Waiting for user input */
    //  RS485_SET_RX_Mode();
    while (1)
    {
        if ( USART_GetFlagStatus(SZ_STM32_COM2, USART_FLAG_RXNE) != RESET)
        {
            key = (uint8_t)SZ_STM32_COM2->DR & 0xFF;
            break;
        }
    }
    return key;
}

unsigned char xUSART1_getchar(void)
{
    unsigned char key = 0;
    
    /* Waiting for user input */
    while (1)
    {
        if ( USART_GetFlagStatus(SZ_STM32_COM1, USART_FLAG_RXNE) != RESET)
        {
            key = (uint8_t)SZ_STM32_COM1->DR & 0xFF;
            break;
        }
    }
    return key;
}

/*----------------------------------------------*/
/* Get a line from the input                    */
/*----------------------------------------------*/

int xgets (		/* 0:End of stream, 1:A line arrived */
           char* buff,	/* Pointer to the buffer */
           int len		/* Buffer length */
               )
{
	int c, i;
    
    
	if (!xfunc_in) return 0;		/* No input function specified */
    
	i = 0;
	for (;;) {
		c = xfunc_in();				/* Get a char from the incoming stream */
		if (!c) return 0;			/* End of stream? */
		if (c == '\r') break;		/* End of line? */
		if (c == '\b' && i) {		/* Back space? */
			i--;
			if (_LINE_ECHO) xputc(c);
			continue;
		}
		if (c >= ' ' && i < len - 1) {	/* Visible chars */
			buff[i++] = c;
			if (_LINE_ECHO) xputc(c);
		}
	}
	buff[i] = 0;	/* Terminate with a \0 */
	if (_LINE_ECHO) xputc('\n');
	return 1;
}

/**
* @brief  Test to see if a key has been pressed on the HyperTerminal
* @param  key: The key pressed
* @retval 1: Correct
*         0: Error
*/
uint32_t xUSART2_ValidInput(void)
{
    
    if ( USART_GetFlagStatus(SZ_STM32_COM2, USART_FLAG_RXNE) != RESET)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

/**
* @brief  Test to see if a key has been pressed on the HyperTerminal
* @param  key: The key pressed
* @retval 1: Correct
*         0: Error
*/
uint32_t xUSART1_ValidInput(void)
{
    
    if ( USART_GetFlagStatus(SZ_STM32_COM1, USART_FLAG_RXNE) != RESET)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

uint32_t xgets_nowait (uint8_t * buffP, uint32_t length)
{
    static uint32_t bytes_read = 0;
    uint8_t c = 0;
    
    if(xUSART2_ValidInput())
    {
        c = xUSART2_getchar();
        if (c == '\r')
        {
            xUSART2_putchar('\n');
            xUSART2_putchar('\r');          
            buffP[bytes_read] = '\0';
            bytes_read = 0;
            
            return 1;
        }
        
        if (c == '\b') /* Backspace */
        {
            if (bytes_read > 0)
            {
                xUSART2_putchar('\b');
                xUSART2_putchar(' ');
                xUSART2_putchar('\b');
                bytes_read--;
            }
            return 0;          
        }
        //if (bytes_read >= (CMD_STRING_SIZE))
        if (bytes_read >= (length))
        {
            //printf("Command string size overflow\r\n");
            bytes_read = 0;
            return 0;
        }
        if (c >= 0x20 && c <= 0x7E)
        {
            buffP[bytes_read] = c;
            bytes_read++;
            xUSART2_putchar(c);
        }
    }
    
    return 0;
}


int xfgets (	/* 0:End of stream, 1:A line arrived */
            unsigned char (*func)(void),	/* Pointer to the input stream function */
            char* buff,	/* Pointer to the buffer */
            int len		/* Buffer length */
                )
{
	unsigned char (*pf)(void);
	int n;
    
    
	pf = xfunc_in;			/* Save current input device */
	xfunc_in = func;		/* Switch input to specified device */
	n = xgets(buff, len);	/* Get a line */
	xfunc_in = pf;			/* Restore input device */
    
	return n;
}


/*----------------------------------------------*/
/* Get a value of the string                    */
/*----------------------------------------------*/
/*	"123 -5   0x3ff 0b1111 0377  w "
^                           1st call returns 123 and next ptr
^                        2nd call returns -5 and next ptr
^                3rd call returns 1023 and next ptr
^         4th call returns 15 and next ptr
^    5th call returns 255 and next ptr
^ 6th call fails and returns 0
*/

int xatoi (			/* 0:Failed, 1:Successful */
           char **str,		/* Pointer to pointer to the string */
           long *res		/* Pointer to the valiable to store the value */
               )
{
	unsigned long val;
	unsigned char c, r, s = 0;
    
    
	*res = 0;
    
	while ((c = **str) == ' ') (*str)++;	/* Skip leading spaces */
    
	if (c == '-') {		/* negative? */
		s = 1;
		c = *(++(*str));
	}
    
	if (c == '0') {
		c = *(++(*str));
		switch (c) {
		case 'x':		/* hexdecimal */
			r = 16; c = *(++(*str));
			break;
		case 'b':		/* binary */
			r = 2; c = *(++(*str));
			break;
		default:
			if (c <= ' ') return 1;	/* single zero */
			if (c < '0' || c > '9') return 0;	/* invalid char */
			r = 8;		/* octal */
		}
	} else {
		if (c < '0' || c > '9') return 0;	/* EOL or invalid char */
		r = 10;			/* decimal */
	}
    
	val = 0;
	while (c > ' ') {
		if (c >= 'a') c -= 0x20;
		c -= '0';
		if (c >= 17) {
			c -= 7;
			if (c <= 9) return 0;	/* invalid char */
		}
		if (c >= r) return 0;		/* invalid char for current radix */
		val = val * r + c;
		c = *(++(*str));
	}
	if (s) val = 0 - val;			/* apply sign if needed */
    
	*res = val;
	return 1;
}

#endif /* _USE_XFUNC_IN */

/*----------------------------------------------------------------------------
名    称：   RecData
功    能：   接
入口参数：   无
出口参数：   无
---------------------------------------------------------------------------- */
uint8_t  RecData(uint8_t  *RecBuf,uint8_t  RecLen,uint8_t  *DataBuf,uint8_t  DataLen)
{
    uint16_t	i,j,k;
    uint8_t		Tmp;
    uint16_t	RecCRC16,Check16;
    uint8_t		HostCode,CodeType;
	
	uint16_t	RecDataLen;
	uint16_t	ReCopyTime;
	uint8_t		IDNum;
    
  	ReCopyTime = 0;
	for(j = 0;j < RecLen -1 - ReCopyTime ;j++ )											//去7d5c
	{
		if(	RecBuf[j] 	== HDLC_CODE0   					
           && 	((RecBuf[j+1]) == HDLC_CODE1)
               )
		{
			RecBuf[j] = FRAM_HERD0;														//7c
			for(k = j+1;k < RecLen-1; k++)
			{
				Tmp	 = RecBuf[k+1];
				
				RecBuf[k] = Tmp;
			}
			ReCopyTime++;
		}
		
		if(	RecBuf[j] 	== HDLC_CODE0   												//去7d5e
           && 	((RecBuf[j+1]) == HDLC_CODE2)
               )
		{
			RecBuf[j] = FRAM_END0;														//7e
			for(k = j+1;k < RecLen-1; k++)
			{
				Tmp	 = RecBuf[k+1];
				
				RecBuf[k] = Tmp;
			}
			ReCopyTime++;			
		}
		
		if(	RecBuf[j] 	== HDLC_CODE0   												//去7d5d
           && 	((RecBuf[j+1]) == HDLC_CODE3)
               )
		{
			RecBuf[j] = HDLC_CODE0;														//7d
			for(k = j+1;k < RecLen; k++)
			{
				Tmp	 = RecBuf[k+1];
				
				RecBuf[k] = Tmp;
			}
			ReCopyTime++;
		}					
	}
	
    for(i = 0; i < RecLen - 10;i++)												       //循环遍历
    {
		if(		((RecBuf[i+0]) == FRAM_HERD0)  				//报头
           &&	((RecBuf[i+1]) == HOST_ID) 
               &&	((RecBuf[i+2]) == DEVICE_ID0)  	 
                   )											
		{				
			RecDataLen  = RecBuf[i+3];
			RecDataLen  += RecBuf[i+4]*256;
			
			DataLen    =  RecDataLen;
			HostCode   =  RecBuf[i+5];
			IDNum	   =  RecBuf[i+6];
			CodeType   =  RecBuf[i+8];
			
			if(		(	HostCode 	!= HOST_CODE_FLG) 
               ||	(   IDNum       != ID_NUM   )
                   ||  (   CodeType       != CODE_TYPE   )
                       )
            {	
			 	i = i + 8;															//地址不对或帧尾错误，跳过此帧
			 	continue;
            }			
            
			RecCRC16 = ((uint16_t)RecBuf[i+5+RecDataLen]) *256 + RecBuf[i+6+RecDataLen];	//校验	
            
			Check16 = GetCrc16Check(&RecBuf[i+5],DataLen);	
			
			if(		RecCRC16  ==	Check16 		
               )
			{
                memcpy(DataBuf,&RecBuf[i+5],DataLen);				//数据拷贝
                
                return	1;
			}
		}
	}
   	
   	DataLen = 0;
    return	0;
}

#define		ENERGY_DATA_LEN	40
#define		ATHER_DATA_LEN	4
void	Sendrs485(uint8_t	*DataBuf,uint32_t	DataLen,uint8_t	CheckNum)
{
	uint8_t		SndBuf[100] = {0};
	uint8_t		SndBufTmp[100] = {0};
	uint16_t	Crc16;
	uint16_t	AddHeadNum;
	uint16_t	i;
	uint8_t		ByteStation;
    
	static	uint8_t		Test = 0x02;
    
	ByteStation = 0;
	SndBuf[ByteStation++] = FRAM_HERD0;
	SndBuf[ByteStation++] = DEVICE_ID0;
	SndBuf[ByteStation++] = HOST_ID;	 
	SndBuf[ByteStation++] = ENERGY_DATA_LEN+ATHER_DATA_LEN;	
	SndBuf[ByteStation++] = 0;	
	SndBuf[ByteStation++] = DEVICE_CODE_FLG;	
	SndBuf[ByteStation++] = ID_NUM;
	
	memcpy(&SndBuf[ByteStation],DataBuf,DataLen);
	
	SndBuf[ByteStation+DataLen+0] = Test;			//数据备用起始位置(电能量标志位)
	SndBuf[ByteStation+DataLen+1] = CheckNum;
	
	Crc16 = GetCrc16Check(&SndBuf[1+4],2+DataLen+2);//帧头（1）+帧长度前包字节（4）
	
    
	SndBuf[ByteStation+DataLen+3] = Crc16;
	SndBuf[ByteStation+DataLen+2] = Crc16>>8;
	
    //////////////////////////////////////////////////////////
	SndBufTmp[0] = SndBuf[0];						//准备数据发送
    
	AddHeadNum = 0;
	for(i = 1; i< ByteStation+DataLen+4;i++ )				//数据，补移位 FRAM_HERD0
	{
		
		if(SndBuf[i] == FRAM_HERD0)					//7c
		{
			SndBufTmp[i+AddHeadNum] = HDLC_CODE0;
			AddHeadNum++;
			SndBufTmp[i+AddHeadNum] = HDLC_CODE1;
		}
		else if(SndBuf[i] == FRAM_END0)				//7e
		{
			SndBufTmp[i+AddHeadNum] = HDLC_CODE0;
			AddHeadNum++;
			SndBufTmp[i+AddHeadNum] = HDLC_CODE2;
		}
		else if(SndBuf[i] == HDLC_CODE0)			//7d		
		{
			SndBufTmp[i+AddHeadNum] = HDLC_CODE0;
			AddHeadNum++;
			SndBufTmp[i+AddHeadNum] = HDLC_CODE3;
		}	
		else
		{
			SndBufTmp[i+AddHeadNum] = SndBuf[i];
		}
	}
    
	SndBufTmp[ByteStation+DataLen+4 +AddHeadNum] = FRAM_END0;
	
    RS485_SET_TX_Mode();	
    RS485_SET_TX_Mode();		
	SendCOM2(SndBufTmp,ByteStation+DataLen+4 +AddHeadNum + 1);  // 数据标号+1=数据长度
	RS485_SET_RX_Mode();			
}


#define		BUF_LEN		256

uint8_t	g_NoRs485ComFlg = 0;
/*----------------------------------------------------------------------------
名    称：   void  RecTAX2Info(void)
功    能：   接收TAX2信息。
入口参数：   无
出口参数：   无
---------------------------------------------------------------------------- */
void  RecRs485(void)
{
	static	uint8_t		RecBuf[BUF_LEN] = {0};								//接收缓冲区
	static	uint8_t		DataBuf[BUF_LEN/2] = {0};								//
	static	uint8_t		SendEnergyBuf[ENERGY_DATA_LEN] = {0};		
    
	static uint8_t	    LifeInfo = 0;
	uint8_t		        HostCodeFlg;		
	uint8_t		        CodeType;	
	uint8_t		        IDNum;	
	    
	uint32_t	        RecLen;		
	uint32_t	        DataLen;
	
	uint32_t            S_VOLTAGE;
	uint32_t            S_CURRENT;
	int32_t             S_FREQ;
	int32_t             S_FACTOR;
    
    
	if( GetFramRecEndFlg() == 1)								//有新的帧尾，进行数据处理
	{
		printf("\n\r  有新的帧尾，进行数据处理\n");	
		RecLen 	= ReadRs485Data(RecBuf);
		
		ClearFramRecEndFlg();									//清标志，可重新取数
		
		if(!RecData(RecBuf,RecLen,DataBuf,DataLen))	{			//接受数据
            printf("\r\n RS485数据接收失败!");
		} else {
			printf("\r\n RS485数据接收成功!");
            
			//////////////////////////////对数据解析
			HostCodeFlg = DataBuf[0];			   //源地址标识
			IDNum		= DataBuf[1];			   //目的地址标识
			LifeInfo 	= DataBuf[2];			   //动态识别码
			CodeType 	= DataBuf[3];			   //命令类型
			
			if(	HostCodeFlg == HOST_CODE_FLG 			
                &&	IDNum	== ID_NUM						
                &&	CodeType== CODE_TYPE ) {			
				printf("\r\n 请求数据发送!");
#if 0
				g_DipDisVal[0] =  AC.PPpower_NUM & 0xFF;			   //将电量数据转存到缓冲区
				g_DipDisVal[1] = (AC.PPpower_NUM >> 8) & 0xFF;	   //低字节在前，高字节在后
				g_DipDisVal[2] = (AC.PPpower_NUM >> 16) & 0xFF;
				g_DipDisVal[3] = (AC.PPpower_NUM >> 24) & 0xFF;
                
				g_DipDisVal[4] =  AC.NPpower_NUM & 0xFF;			   //将电量数据转存到缓冲区
				g_DipDisVal[5] = (AC.NPpower_NUM >> 8) & 0xFF;	   //低字节在前，高字节在后
				g_DipDisVal[6] = (AC.NPpower_NUM >> 16) & 0xFF;
				g_DipDisVal[7] = (AC.NPpower_NUM >> 24) & 0xFF;
                
				g_DipDisVal[8] =   AC.PQpower_NUM & 0xFF;			   //将电量数据转存到缓冲区
				g_DipDisVal[9] =  (AC.PQpower_NUM >> 8) & 0xFF;	   //低字节在前，高字节在后
				g_DipDisVal[10] = (AC.PQpower_NUM >> 16) & 0xFF;
				g_DipDisVal[11] = (AC.PQpower_NUM >> 24) & 0xFF;
                
				g_DipDisVal[12] =  AC.NQpower_NUM & 0xFF;			   //将电量数据转存到缓冲区
				g_DipDisVal[13] = (AC.NQpower_NUM >> 8) & 0xFF;	   //低字节在前，高字节在后
				g_DipDisVal[14] = (AC.NQpower_NUM >> 16) & 0xFF;
				g_DipDisVal[15] = (AC.NQpower_NUM >> 24) & 0xFF;
                
				S_VOLTAGE = AC.U_RMS * 1000;
				if(S_VOLTAGE < 200)
					S_VOLTAGE = 0;
				g_DipDisVal[16] =  S_VOLTAGE & 0xFF;			   //将电量数据转存到缓冲区
				g_DipDisVal[17] = (S_VOLTAGE >> 8) & 0xFF;	       //低字节在前，高字节在后
				g_DipDisVal[18] = (S_VOLTAGE >> 16) & 0xFF;
				g_DipDisVal[19] = (S_VOLTAGE >> 24) & 0xFF;
                
				S_CURRENT = AC.I_RMS * 1000;
				if(S_CURRENT < 100)
					S_CURRENT = 0;
				g_DipDisVal[20] =  S_CURRENT & 0xFF;			   //将电量数据转存到缓冲区
				g_DipDisVal[21] = (S_CURRENT >> 8) & 0xFF;	   //低字节在前，高字节在后
				g_DipDisVal[22] = (S_CURRENT >> 16) & 0xFF;
				g_DipDisVal[23] = (S_CURRENT >> 24) & 0xFF;
                
				S_FREQ = AC.Power_Freq * 1000;
				g_DipDisVal[24] =  S_FREQ & 0xFF;			//将频率数据转存到缓冲区
				g_DipDisVal[25] = (S_FREQ >> 8) & 0xFF;	   //低字节在前，高字节在后
				g_DipDisVal[26] = (S_FREQ >> 16) & 0xFF;
				g_DipDisVal[27] = (S_FREQ >> 24) & 0xFF;
                
				S_FACTOR = AC.Power_Factor * 1000;
				g_DipDisVal[28] =  S_FACTOR & 0xFF;			   //将电量数据转存到缓冲区
				g_DipDisVal[29] = (S_FACTOR >> 8) & 0xFF;	   //低字节在前，高字节在后
				g_DipDisVal[30] = (S_FACTOR >> 16) & 0xFF;
				g_DipDisVal[31] = (S_FACTOR >> 24) & 0xFF;
                
				g_DipDisVal[32] =  AC.ACTIVE_POWER & 0xFF;			   //将电量数据转存到缓冲区
				g_DipDisVal[33] = (AC.ACTIVE_POWER >> 8) & 0xFF;	   //低字节在前，高字节在后
				g_DipDisVal[34] = (AC.ACTIVE_POWER >> 16) & 0xFF;
				g_DipDisVal[35] = (AC.ACTIVE_POWER >> 24) & 0xFF;
                
				g_DipDisVal[36] =  AC.REACTIVE_POWER & 0xFF;			   //将电量数据转存到缓冲区
				g_DipDisVal[37] = (AC.REACTIVE_POWER >> 8) & 0xFF;	   //低字节在前，高字节在后
				g_DipDisVal[38] = (AC.REACTIVE_POWER >> 16) & 0xFF;
				g_DipDisVal[39] = (AC.REACTIVE_POWER >> 24) & 0xFF;
#endif
				
				memcpy(&SendEnergyBuf[0],(uint8_t *)&g_DipDisVal,sizeof(g_DipDisVal));
				
				Sendrs485(SendEnergyBuf,sizeof(SendEnergyBuf),LifeInfo);	//发送数据			
				
				g_NoRs485ComFlg = 0;								//置通讯标志	通讯成功 
			}
            
		}
        
	}
}



