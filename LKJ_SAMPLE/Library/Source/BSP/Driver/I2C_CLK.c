/******************** (C) COPYRIGHT 2008 STMicroelectronics ********************
* File Name          : i2c_clk.c
* Author             : ZHOUWEI
* Version            : V3.5.0
* Date               : 01/07/2014
* Description        : This file provides a set of functions needed to manage the
*                      communication between I2C peripheral and I2C DS3231SN.
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "I2C_CLK.h"
#include "DS3231.h"
#include <stdio.h>


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/


#define RCC_GPIO_I2C1      RCC_APB2Periph_GPIOE
#define SCL                GPIO_Pin_7
#define SDA                GPIO_Pin_8
#define GPIO_I2C1_PORT     GPIOE

#define FALSE              1
#define TRUE               0
#define Delay_Factor       5                                        		//延时因子

#define SCL_SET            GPIO_SetBits(GPIO_I2C1_PORT, SCL)				//SCL=1
#define SCL_RESET          GPIO_ResetBits(GPIO_I2C1_PORT, SCL)			    //SCL=0
   
#define SDA_SET            GPIO_SetBits(GPIO_I2C1_PORT, SDA)				//SDA=1
#define SDA_RESET          GPIO_ResetBits(GPIO_I2C1_PORT, SDA)			    //SDA=0
                            
#define SDA_read()         GPIO_ReadInputDataBit(GPIO_I2C1_PORT, SDA)	    //读取SDA状态


/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
/*******************************************************************************
* Function Name  : I2C_GPIO_Config
* Description    : Configration Simulation IIC GPIO
* Input          : None 
* Output         : None
* Return         : None
****************************************************************************** */
void I2C_GPIO_Config(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure; 			     //定义结构体
  
  RCC_APB2PeriphClockCmd(RCC_GPIO_I2C1 , ENABLE);	     //使能GPIOB时钟

  GPIO_InitStructure.GPIO_Pin =  SCL;				 
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;	     //设置SCL口速度为2MHz
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;       //设置SCL口为开漏极输出
  GPIO_Init(GPIO_I2C1_PORT, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin =  SDA;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;	     //设置SDA口速度为2MHz
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;	     //设置SDA口为开漏极输出
  GPIO_Init(GPIO_I2C1_PORT, &GPIO_InitStructure);
}

/*******************************************************************************
* Function Name  : I2C_delay
* Description    : 延时程序，i决定延时时间
* Input          : None
* Output         : None
* Return         : None
****************************************************************************** */
void I2C_delay(void)
{
	uint8_t i = Delay_Factor; 
	while(i) 
	{ 
		i--; 
	}  
}

/*******************************************************************************
* Function Name  : SCL_L
* Description    : 设置SCL为低电平，并产生延时
* Input          : None
* Output         : None
* Return         : None
****************************************************************************** */
void SCL_L(void)
{
  SCL_RESET;
  I2C_delay();  
}

/*******************************************************************************
* Function Name  : SCL_H
* Description    : 设置SCL为高电平，并产生延时
* Input          : None
* Output         : None
* Return         : None
****************************************************************************** */
void SCL_H(void)
{
  SCL_SET;
  I2C_delay();  
}

/*******************************************************************************
* Function Name  : SDA_L
* Description    : 设置SDA为低电平，并产生延时
* Input          : None
* Output         : None
* Return         : None
****************************************************************************** */
void SDA_L(void)
{
  SDA_RESET;
  I2C_delay();  
}

/*******************************************************************************
* Function Name  : SDA_H
* Description    : 设置SDA为高电平，并产生延时
* Input          : None
* Output         : None
* Return         : None
****************************************************************************** */
void SDA_H(void)
{
  SDA_SET;
  I2C_delay();  
}

/*******************************************************************************
* Function Name  : I2C_Start
* Description    : 启动I2C： SCL为1时，SDA从1到0跳变
* Input          : None
* Output         : None
* Return         : None
****************************************************************************** */
void I2C_Start(void)
{
	SCL_L();	                  //SCL = 0
	SDA_H();					  //SDA = 1
	SCL_H();					  //SCL = 1
	SDA_L();					  //SDA = 0
	SCL_L();					  //SCL = 0
}
/*******************************************************************************
* Function Name  : I2C_Stop
* Description    : 停止I2C： SCL为1时，SDA从0到1跳变
* Input          : None
* Output         : None
* Return         : None
****************************************************************************** */
void I2C_Stop(void)
{
	SCL_L();					  //SCL = 0
	SDA_L();					  //SDA = 0
	SCL_H();					  //SCL = 1
	SDA_H();					  //SDA = 1
	SCL_L();					  //SCL = 0
} 
/*******************************************************************************
* Function Name  : I2C_Ack
* Description    : 主设备返回应答信号
* Input          : None
* Output         : None
* Return         : None
****************************************************************************** */
void I2C_Ack(void)
{	
	SCL_L();					  //SCL = 0
	SDA_L();					  //SDA = 0    
	SCL_H();					  //SCL = 1
	SCL_L();					  //SCL = 0
	SDA_L();					  //SDA = 0
}   
/*******************************************************************************
* Function Name  : I2C_NoAck
* Description    : 主设备返回最后一次应答信号
* Input          : None
* Output         : None
* Return         : None
****************************************************************************** */
void I2C_NoAck(void)
{	
	SCL_L();				      //SCL = 0
	SDA_H();					  //SDA = 1 
	SCL_H();					  //SCL = 1   
	SCL_L();					  //SCL = 0
	SDA_L();					  //SDA = 0
} 
/*******************************************************************************
* Function Name  : I2C_WaitAck
* Description    : 从设备返回应答信号
* Input          : None
* Output         : None
* Return         : 应答信号，FALSE = 1无应答；TURE = 0有应答
****************************************************************************** */
uint8_t I2C_WaitAck(void) 	                      //返回为:=0有ACK,=1无ACK
{
	SCL_L();				                      //SCL = 0
	SDA_H();					                  //SDA = 1
	SCL_H();					                  //SCL = 1
	
	SCL_L();					                  //SCL = 0

	if(SDA_read())			                      //读SDA状态，总线是否被占用？
	{
    	SCL_L();				                  //SCL = 0
		I2C_delay();
//   	return FALSE;			                  //返回FALSE = 1，无应答     ?????   未弄明白
    	return TRUE;
	}
	else
	{
		SCL_L();					              //SCL = 0
		I2C_delay();
		return TRUE;			                  //返回TRUE = 0，有应答
	}
}
/*******************************************************************************
* Function Name  : I2C_SendByte
* Description    : 发送一个字节到接收设备
* Input          : SendByte：需要发送的数据
* Output         : None
* Return         : None
****************************************************************************** */
void I2C_SendByte(uint8_t SendByte) //数据从高位到低位
{
	uint8_t i = 8;
	
	while(i--)
	{
		SCL_L();				 	             //SCL = 0
		if(SendByte&0x80)		 	             //先发高位
		{                         
	    	SDA_H();				 	         //SDA = 1
		}  
		else 
		{
	  		SDA_L();  			 	            //SDA = 0
		}
		SendByte<<=1;				            //左移1位
		I2C_delay();
		SCL_H();					            //SCL = 1
		I2C_delay();
	}
	SCL_L();					 	            //SCL = 0
}  
/*******************************************************************************
* Function Name  : I2C_ReadByte
* Description    : 通过I2C读取1个字节的数据
* Input          : None
* Output         : None
* Return         : 读取到的数据 
****************************************************************************** */
uint8_t I2C_ReadByte(void) 		 	       		//数据从低位到高位
{ 
	uint8_t i = 8;
	uint8_t ReceiveByte = 0;
	
	SDA_H();				    		        //SDA = 1
	while(i--)
	{
		ReceiveByte<<=1;      		            //先接收低位 
		SCL_L();			    		        //SCL = 0
		SCL_H();			   		 	        //SCL = 1
		if(SDA_read())	    		            //读SDA状态，是否为1，为1则赋值；为0，则忽略
		{
			ReceiveByte|=0x01;  	  	        //给最低位赋值
		}                         
	}
	SCL_L();				    		        //SCL = 0，释放总线
	return ReceiveByte;	    		       		//返回数据
} 
          
/*******************************************************************************
* Function Name  : Single_Write(uint8_t SlaveAddress,uint8_t REG_Address,uint8_t REG_data)
* Description    : 发送一个字节数据到指定的从设备的寄存器中
* Input          : SlaveAddress为从设备地址，REG_Address为寄存器地址，REG_data为要写的数据
* Output         : None
* Return         : None
****************************************************************************** */

void Single_Write(uint8_t SlaveAddress,uint8_t REG_Address,uint8_t REG_data)
{
	I2C_Start();				           //启动I2C
	I2C_SendByte(SlaveAddress);            //写从设备地址(DS3231写模式为0xD0) 
	while(I2C_WaitAck());			  	   //等待从设备应答
	I2C_SendByte(REG_Address );            //写从设备寄存器地址      
	while(I2C_WaitAck());				   //等待从设备应答
	I2C_SendByte(REG_data);		           //写数据到寄存器
	while(I2C_WaitAck());				   //等待从设备应答
	I2C_Stop(); 				           //停止I2C
}


/*******************************************************************************
* Function Name  : Single_Read(uint8_t SlaveAddress,uint8_t REG_Address)
* Description    : 从从设备读取1个字节的数据
* Input          : SlaveAddress为从设备地址，REG_Address为寄存器地址
* Output         : None
* Return         : None
****************************************************************************** */
uint8_t Single_Read(uint8_t SlaveAddress,uint8_t REG_Address)
{   
	uint8_t REG_data;     	
	I2C_Start();				        	//启动I2C
	I2C_SendByte(SlaveAddress); 	        //写从设备地址(DS3231写模式为0xD0) 
	while(I2C_WaitAck());				    //等待从设备应答
	I2C_SendByte(REG_Address);              //写从设备寄存器地址      
	while(I2C_WaitAck());				    //等待从设备应答
	I2C_Start();					        //启动I2C
	I2C_SendByte(SlaveAddress+1);	        //写从设备地址(DS3231写模式为0xD1=0xD0+1)
	while(I2C_WaitAck());				    //等待从设备应答
	
	REG_data= I2C_ReadByte();		        //主机接收数据
	I2C_NoAck();					        //等待主机应答
	I2C_Stop();						        //停止I2C
	return REG_data;				        //返回读取到的数据
}
 
/* END OF I2C_CLK.C */



