/******************** (C) COPYRIGHT 2008 STMicroelectronics ********************
* File Name          : i2c_clk.h
* Author             : ZHOUWEI
* Version            : V3.5.0
* Date               : 01/02/2014
* Description        : Header for i2c_clk.c module
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/
/* Define to prevent recursive inclusion ------------------------------------ */
#ifndef __I2C_CLK_H
#define __I2C_CLK_H


extern void I2C_GPIO_Config(void);
extern void I2C_delay(void);
extern void I2C_Start(void);
extern void I2C_Stop(void);
extern void I2C_Ack(void);
extern void I2C_NoAck(void);
extern uint8_t I2C_WaitAck(void);
extern void I2C_SendByte(uint8_t SendByte);
extern uint8_t I2C_ReadByte(void);
extern void Single_Write(uint8_t SlaveAddress,uint8_t REG_Address,uint8_t REG_data);
//extern uint8_t Write(uint8_t SlaveAddress,uint8_t REG_Address);
extern uint8_t Single_Read(uint8_t SlaveAddress,uint8_t REG_Address);

/*******************************************************************************
* Function Name  : I2C_GPIO_Config
* Description    : Configration Simulation IIC GPIO
* Input          : None 
* Output         : None
* Return         : None
****************************************************************************** */
extern void I2C_GPIO_Config(void);

/*******************************************************************************
* Function Name  : I2C_delay
* Description    : 延时程序，i决定延时时间
* Input          : None
* Output         : None
* Return         : None
****************************************************************************** */
extern void I2C_delay(void);

/*******************************************************************************
* Function Name  : void delay100us(void)
* Description    : 延时600us
* Input          : None
* Output         : None
* Return         : None
****************************************************************************** */
extern void delay100us(void);

/*******************************************************************************
* Function Name  : I2C_Start
* Description    : 启动I2C： SCL为1时，SDA从1到0跳变
* Input          : None
* Output         : None
* Return         : None
****************************************************************************** */
extern void I2C_Start(void);

/*******************************************************************************
* Function Name  : I2C_Stop
* Description    : 停止I2C： SCL为1时，SDA从0到1跳变
* Input          : None
* Output         : None
* Return         : None
****************************************************************************** */
extern void I2C_Stop(void);

/*******************************************************************************
* Function Name  : I2C_Ack
* Description    : 主设备返回应答信号
* Input          : None
* Output         : None
* Return         : None
****************************************************************************** */
extern void I2C_Ack(void);
 
/*******************************************************************************
* Function Name  : I2C_NoAck
* Description    : 主设备返回最后一次应答信号
* Input          : None
* Output         : None
* Return         : None
****************************************************************************** */
extern void I2C_NoAck(void);

/*******************************************************************************
* Function Name  : I2C_WaitAck
* Description    : 从设备返回应答信号
* Input          : None
* Output         : None
* Return         : 应答信号，FALSE = 1无应答；TURE = 0有应答
****************************************************************************** */
extern uint8_t I2C_WaitAck(void);

/*******************************************************************************
* Function Name  : I2C_SendByte
* Description    : 发送一个字节到接收设备
* Input          : SendByte：需要发送的数据
* Output         : None
* Return         : None
****************************************************************************** */
extern void I2C_SendByte(uint8_t SendByte);

/*******************************************************************************
* Function Name  : I2C_ReadByte
* Description    : 通过I2C读取1个字节的数据
* Input          : None
* Output         : None
* Return         : 读取到的数据 
****************************************************************************** */
extern uint8_t I2C_ReadByte(void);
          
/*******************************************************************************
* Function Name  : Single_Write(uint8_t SlaveAddress,uint8_t REG_Address,uint8_t REG_data)
* Description    : 发送一个字节数据到指定的从设备的寄存器中
* Input          : SlaveAddress为从设备地址，REG_Address为寄存器地址，REG_data为要写的数据
* Output         : None
* Return         : None
****************************************************************************** */
extern void Single_Write(uint8_t SlaveAddress,uint8_t REG_Address,uint8_t REG_data);


/*******************************************************************************
* Function Name  : Single_Read(uint8_t SlaveAddress,uint8_t REG_Address)
* Description    : 从从设备读取1个字节的数据
* Input          : SlaveAddress为从设备地址，REG_Address为寄存器地址
* Output         : None
* Return         : None
****************************************************************************** */
extern uint8_t Single_Read(uint8_t SlaveAddress,uint8_t REG_Address);


#endif
/* end of i2c_clk.h */


