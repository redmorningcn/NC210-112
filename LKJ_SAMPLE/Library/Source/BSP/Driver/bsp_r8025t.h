/*******************************************************************************
 *   Filename:      bsp_r8025t.h
 *   Revised:       $Date: 2015-01-26
 *   Revision:      $V1.0
 *   Writer:        wumingshen.
 *
 *   Description:   RX8025实时时钟芯片驱动模块头文件。
 *
 *   Notes:
 *
 *
 *   All copyrights reserved to wumingshen.
 *
 *******************************************************************************/
#ifndef	__BSP_RX8025T_H__
#define	__BSP_RX8025T_H__


/*******************************************************************************
 * INCLUDES
 */
#include  <global.h>
#include  <includes.h>
#include  <time.h>


/***********************************************
* 描述： RX8025x使用的IO，配置为开漏输出
*/
#define RX8025x_SCL_RCC             RCC_APB2Periph_GPIOB
#define RX8025x_SCL_PROT            GPIOB
#define RX8025x_SCL_PIN             GPIO_Pin_8

#define RX8025x_SDA_RCC             RCC_APB2Periph_GPIOB
#define RX8025x_SDA_PROT            GPIOB
#define RX8025x_SDA_PIN             GPIO_Pin_9

/*******************************************************************************
* 描述： RX8025x使用的端口设置(直接操作寄存器)，直接修改该处端口
*/
#define RX8025_SCL_H()              ( GPIOB->BSRR = RX8025x_SCL_PIN )
#define RX8025_SCL_L()              ( GPIOB->BRR  = RX8025x_SCL_PIN )

#define RX8025_SDA_H()              ( GPIOB->BSRR = RX8025x_SDA_PIN )
#define RX8025_SDA_L()              ( GPIOB->BRR  = RX8025x_SDA_PIN )

#define RX8025_SCL_S()              ( GPIOB->IDR  & RX8025x_SCL_PIN )
#define RX8025_SDA_S()              ( GPIOB->IDR  & RX8025x_SDA_PIN )

//============================================================================//

/***********************************************
* 描述： RX8025x使用的端口设置，直接修改该处端口
*/
#define RX8025x_SET_SCL(sta)    { if(sta) {RX8025_SCL_H(); Delay_Nus(DLY_CLK_HIGH);} else { RX8025_SCL_L(); Delay_Nus(DLY_CLK_SDA); } }
#define RX8025x_SET_SDA(sta)    { if(sta) {RX8025_SDA_H(); Delay_Nus(DLY_CLK_LOW); } else { RX8025_SDA_L(); Delay_Nus(DLY_CLK_LOW); } }
#define RX8025x_GET_SCL(sta)    { if( RX8025_SCL_S() ) sta |= 1; else sta &= ~1; }
#define RX8025x_GET_SDA(sta)    { if( RX8025_SDA_S() ) sta |= 1; else sta &= ~1; }

#define SET_RX8025x_SDA_INPUT()   { GPIO_InitTypeDef  GPIO_InitStructure;\
                                    GPIO_InitStructure.GPIO_Pin =  RX8025x_SDA_PIN;\
                                    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;\
                                    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;\
                                    GPIO_Init(RX8025x_SDA_PROT, &GPIO_InitStructure);  }

#define SET_RX8025x_SDA_OUTPUT()  { GPIO_InitTypeDef  GPIO_InitStructure;\
                                    GPIO_InitStructure.GPIO_Pin =  RX8025x_SDA_PIN;\
                                    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;\
                                    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;\
                                    GPIO_Init(RX8025x_SDA_PROT, &GPIO_InitStructure);  }

#define SET_RX8025x_SCL_INPUT()   { GPIO_InitTypeDef  GPIO_InitStructure;\
                                    GPIO_InitStructure.GPIO_Pin =  RX8025x_SCL_PIN;\
                                    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;\
                                    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;\
                                    GPIO_Init(RX8025x_SCL_PROT, &GPIO_InitStructure);  }

#define SET_RX8025x_SCL_OUTPUT()  { GPIO_InitTypeDef  GPIO_InitStructure;\
                                    GPIO_InitStructure.GPIO_Pin =  RX8025x_SCL_PIN;\
                                    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;\
                                    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;\
                                    GPIO_Init(RX8025x_SCL_PROT, &GPIO_InitStructure);  }



uint8_t      BSP_RX8025T_Init            (void);
void         BSP_RX8025T_Main            (void);

uint8_t      BSP_RX8025T_GetTime         (struct tm *t_tm);    //数据从高位到低位//
uint8_t      BSP_RX8025T_SetDate         (struct tm t_tm);

#endif	/* BSP_RX8025T_H */
/*******************************************************************************
 * 				end of file
 *******************************************************************************/