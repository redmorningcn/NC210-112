/*
*********************************************************************************************************
*                                     MICIRUM BOARD SUPPORT PACKAGE
*
*                            (c) Copyright 2007-2008; Micrium, Inc.; Weston, FL
*
*                   All rights reserved.  Protected by international copyright laws.
*                   Knowledge of the source code may not be used to write a similar
*                   product.  This file may only be used in accordance with a license
*                   and should not be redistributed in any way.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                        BOARD SUPPORT PACKAGE
*
*                                     ST Microelectronics STM32
*                                              on the
*
*                                     Micrium uC-Eval-STM32F107
*                                        Evaluation Board
*
* Filename      : bsp_int.c
* Version       : V1.00
* Programmer(s) : EHS
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/

#define  BSP_INT_MODULE
#include <bsp.h>


/*
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*/

#define  BSP_INT_SRC_NBR                                 68

#define  CPU_INT_SRC_POS_MAX                  ((((CPU_REG_NVIC_NVIC + 1) & 0x1F) * 32) + 16)

#define  CPU_BIT_BAND_SRAM_REG_LO                 0x20000000
#define  CPU_BIT_BAND_SRAM_REG_HI                 0x200FFFFF
#define  CPU_BIT_BAND_SRAM_BASE                   0x22000000


#define  CPU_BIT_BAND_PERIPH_REG_LO               0x40000000
#define  CPU_BIT_BAND_PERIPH_REG_HI               0x400FFFFF
#define  CPU_BIT_BAND_PERIPH_BASE                 0x42000000

/*
*********************************************************************************************************
*                                           LOCAL CONSTANTS
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                          LOCAL DATA TYPES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                            LOCAL TABLES
*********************************************************************************************************
*/

static  CPU_FNCT_VOID  BSP_IntVectTbl[BSP_INT_SRC_NBR];


/*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static  void  BSP_IntHandler     (CPU_DATA  int_id);
static  void  BSP_IntHandlerDummy(void);


/*
*********************************************************************************************************
*                                     LOCAL CONFIGURATION ERRORS
*********************************************************************************************************
*/



/*$PAGE*/
/*
*********************************************************************************************************
*                                           CPU_IntSrcEn()
*
* Description : Enable an interrupt source.
*
* Argument(s) : pos     Position of interrupt vector in interrupt table (see 'CPU_IntSrcDis()').
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
* Note(s)     : (1) See 'CPU_IntSrcDis()  Note #1'.
*
*               (2) See 'CPU_IntSrcDis()  Note #2'.
*
*               (3) See 'CPU_IntSrcDis()  Note #3'.
*********************************************************************************************************
*/

void  CPU_IntSrcEn (CPU_INT08U  pos)
{
    CPU_INT08U  group;
    CPU_INT08U  nbr;
    CPU_INT08U  pos_max;
    CPU_SR_ALLOC();


    switch (pos) {
        case CPU_INT_STK_PTR:                                   /* ---------------- INVALID OR RESERVED --------------- */
        case CPU_INT_RSVD_07:
        case CPU_INT_RSVD_08:
        case CPU_INT_RSVD_09:
        case CPU_INT_RSVD_10:
        case CPU_INT_RSVD_13:
             break;


                                                                /* ----------------- SYSTEM EXCEPTIONS ---------------- */
        case CPU_INT_RESET:                                     /* Reset (see Note #2).                                 */
        case CPU_INT_NMI:                                       /* Non-maskable interrupt (see Note #2).                */
        case CPU_INT_HFAULT:                                    /* Hard fault (see Note #2).                            */
        case CPU_INT_SVCALL:                                    /* SVCall (see Note #2).                                */
        case CPU_INT_DBGMON:                                    /* Debug monitor (see Note #2).                         */
        case CPU_INT_PENDSV:                                    /* PendSV (see Note #2).                                */
             break;

        case CPU_INT_MEM:                                       /* Memory management.                                   */
             CPU_REG_NVIC_SHCSR |= CPU_REG_NVIC_SHCSR_MEMFAULTENA;
             break;

        case CPU_INT_BUSFAULT:                                  /* Bus fault.                                           */
             CPU_REG_NVIC_SHCSR |= CPU_REG_NVIC_SHCSR_BUSFAULTENA;
             break;

        case CPU_INT_USAGEFAULT:                                /* Usage fault.                                         */
             CPU_REG_NVIC_SHCSR |= CPU_REG_NVIC_SHCSR_USGFAULTENA;
             break;

        case CPU_INT_SYSTICK:                                   /* SysTick.                                             */
             CPU_REG_NVIC_ST_CTRL |= CPU_REG_NVIC_ST_CTRL_ENABLE;
             break;


                                                                /* ---------------- EXTERNAL INTERRUPT ---------------- */
        default:
            pos_max = CPU_INT_SRC_POS_MAX;
            if (pos < pos_max) {                                /* See Note #3.                                         */
                 group = (pos - 16) / 32;
                 nbr   = (pos - 16) % 32;

                 CPU_REG_NVIC_SETEN(group) = DEF_BIT(nbr);
             }
             break;
    }
}

/*
*********************************************************************************************************
*                                              BSP_IntClr()
*
* Description : Clear interrupt.
*
* Argument(s) : int_id      Interrupt to clear.
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
* Note(s)     : (1) An interrupt does not need to be cleared within the interrupt controller.
*********************************************************************************************************
*/

void  BSP_IntClr (CPU_DATA  int_id)
{

}


/*
*********************************************************************************************************
*                                              BSP_IntDis()
*
* Description : Disable interrupt.
*
* Argument(s) : int_id      Interrupt to disable.
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  BSP_IntDis (CPU_DATA  int_id)
{
    if (int_id < BSP_INT_SRC_NBR) {
//        CPU_IntSrcDis(int_id + 16);
    }
}


/*
*********************************************************************************************************
*                                           BSP_IntDisAll()
*
* Description : Disable ALL interrupts.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  BSP_IntDisAll (void)
{
//    CPU_IntDis();
}


/*
*********************************************************************************************************
*                                               BSP_IntEn()
*
* Description : Enable interrupt.
*
* Argument(s) : int_id      Interrupt to enable.
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  BSP_IntEn (CPU_DATA  int_id)
{
    if (int_id < BSP_INT_SRC_NBR) {
        CPU_IntSrcEn(int_id + 16);
    }
}


/*
*********************************************************************************************************
*                                            BSP_IntVectSet()
*
* Description : Assign ISR handler.
*
* Argument(s) : int_id      Interrupt for which vector will be set.
*
*               isr         Handler to assign
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  BSP_IntVectSet (CPU_DATA       int_id,
                      CPU_FNCT_VOID  isr)
{


    if (int_id < BSP_INT_SRC_NBR) {
        BSP_IntVectTbl[int_id] = isr;
    }
}


/*
*********************************************************************************************************
*                                            BSP_IntPrioSet()
*
* Description : Assign ISR priority.
*
* Argument(s) : int_id      Interrupt for which vector will be set.
*
*               prio        Priority to assign
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  BSP_IntPrioSet (CPU_DATA    int_id,
                      CPU_INT08U  prio)
{


    if (int_id < BSP_INT_SRC_NBR) {
        CPU_IntSrcPrioSet(int_id + 16, prio);
    }
}


/*
*********************************************************************************************************
*********************************************************************************************************
*                                           INTERNAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                              BSP_IntInit()
*
* Description : Initialize interrupts:
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : BSP_Init().
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  BSP_IntInit (void)
{
    CPU_DATA  int_id;


    for (int_id = 0; int_id < BSP_INT_SRC_NBR; int_id++) {
        BSP_IntVectSet(int_id, BSP_IntHandlerDummy);
    }
}


/*
*********************************************************************************************************
*                                        BSP_IntHandler####()
*
* Description : Handle an interrupt.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : This is an ISR.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  BSP_IntHandlerWWDG          (void)  { BSP_IntHandler(BSP_INT_ID_WWDG);            }
void  BSP_IntHandlerPVD           (void)  { BSP_IntHandler(BSP_INT_ID_PVD);             }
void  BSP_IntHandlerTAMPER        (void)  { BSP_IntHandler(BSP_INT_ID_TAMPER);          }
void  BSP_IntHandlerRTC           (void)  { BSP_IntHandler(BSP_INT_ID_RTC);             }
void  BSP_IntHandlerFLASH         (void)  { BSP_IntHandler(BSP_INT_ID_FLASH);           }
void  BSP_IntHandlerRCC           (void)  { BSP_IntHandler(BSP_INT_ID_RCC);             }
void  BSP_IntHandlerEXTI0         (void)  { BSP_IntHandler(BSP_INT_ID_EXTI0);           }
void  BSP_IntHandlerEXTI1         (void)  { BSP_IntHandler(BSP_INT_ID_EXTI1);           }
void  BSP_IntHandlerEXTI2         (void)  { BSP_IntHandler(BSP_INT_ID_EXTI2);           }
void  BSP_IntHandlerEXTI3         (void)  { BSP_IntHandler(BSP_INT_ID_EXTI3);           }
void  BSP_IntHandlerEXTI4         (void)  { BSP_IntHandler(BSP_INT_ID_EXTI4);           }
void  BSP_IntHandlerDMA1_CH1      (void)  { BSP_IntHandler(BSP_INT_ID_DMA1_CH1);        }
void  BSP_IntHandlerDMA1_CH2      (void)  { BSP_IntHandler(BSP_INT_ID_DMA1_CH2);        }
void  BSP_IntHandlerDMA1_CH3      (void)  { BSP_IntHandler(BSP_INT_ID_DMA1_CH3);        }
void  BSP_IntHandlerDMA1_CH4      (void)  { BSP_IntHandler(BSP_INT_ID_DMA1_CH4);        }
void  BSP_IntHandlerDMA1_CH5      (void)  { BSP_IntHandler(BSP_INT_ID_DMA1_CH5);        }
void  BSP_IntHandlerDMA1_CH6      (void)  { BSP_IntHandler(BSP_INT_ID_DMA1_CH6);        }
void  BSP_IntHandlerDMA1_CH7      (void)  { BSP_IntHandler(BSP_INT_ID_DMA1_CH7);        }
void  BSP_IntHandlerADC1_2        (void)  { BSP_IntHandler(BSP_INT_ID_ADC1_2);          }
void  BSP_IntHandlerCAN1_TX       (void)  { BSP_IntHandler(BSP_INT_ID_CAN1_TX);         }
void  BSP_IntHandlerCAN1_RX0      (void)  { BSP_IntHandler(BSP_INT_ID_CAN1_RX0);        }
void  BSP_IntHandlerCAN1_RX1      (void)  { BSP_IntHandler(BSP_INT_ID_CAN1_RX1);        }
void  BSP_IntHandlerCAN1_SCE      (void)  { BSP_IntHandler(BSP_INT_ID_CAN1_SCE);        }
void  BSP_IntHandlerEXTI9_5       (void)  { BSP_IntHandler(BSP_INT_ID_EXTI9_5);         }
void  BSP_IntHandlerTIM1_BRK      (void)  { BSP_IntHandler(BSP_INT_ID_TIM1_BRK);        }
void  BSP_IntHandlerTIM1_UP       (void)  { BSP_IntHandler(BSP_INT_ID_TIM1_UP);         }
void  BSP_IntHandlerTIM1_TRG_COM  (void)  { BSP_IntHandler(BSP_INT_ID_TIM1_TRG_COM);    }
void  BSP_IntHandlerTIM1_CC       (void)  { BSP_IntHandler(BSP_INT_ID_TIM1_CC);         }
void  BSP_IntHandlerTIM2          (void)  { BSP_IntHandler(BSP_INT_ID_TIM2);            }
void  BSP_IntHandlerTIM3          (void)  { BSP_IntHandler(BSP_INT_ID_TIM3);            }
void  BSP_IntHandlerTIM4          (void)  { BSP_IntHandler(BSP_INT_ID_TIM4);            }
void  BSP_IntHandlerI2C1_EV       (void)  { BSP_IntHandler(BSP_INT_ID_I2C1_EV);         }
void  BSP_IntHandlerI2C1_ER       (void)  { BSP_IntHandler(BSP_INT_ID_I2C1_ER);         }
void  BSP_IntHandlerI2C2_EV       (void)  { BSP_IntHandler(BSP_INT_ID_I2C2_EV);         }
void  BSP_IntHandlerI2C2_ER       (void)  { BSP_IntHandler(BSP_INT_ID_I2C2_ER);         }
void  BSP_IntHandlerSPI1          (void)  { BSP_IntHandler(BSP_INT_ID_SPI1);            }
void  BSP_IntHandlerSPI2          (void)  { BSP_IntHandler(BSP_INT_ID_SPI2);            }
void  BSP_IntHandlerUSART1        (void)  { BSP_IntHandler(BSP_INT_ID_USART1);          }
void  BSP_IntHandlerUSART2        (void)  { BSP_IntHandler(BSP_INT_ID_USART2);          }
void  BSP_IntHandlerUSART3        (void)  { BSP_IntHandler(BSP_INT_ID_USART3);          }
void  BSP_IntHandlerEXTI15_10     (void)  { BSP_IntHandler(BSP_INT_ID_EXTI15_10);       }
void  BSP_IntHandlerRTCAlarm      (void)  { BSP_IntHandler(BSP_INT_ID_RTC_ALARM);       }
void  BSP_IntHandlerUSBWakeUp     (void)  { BSP_IntHandler(BSP_INT_ID_OTG_FS_WKUP);     }
void BSP_IntHandlerTIM8_UP			(void){ BSP_IntHandler(BSP_INT_ID_TIM8_UP);     }
void BSP_IntHandlerTIM8_CC				(void){ BSP_IntHandler(BSP_INT_ID_TIM_CC);     }
void  BSP_IntHandlerTIM5          (void)  { BSP_IntHandler(BSP_INT_ID_TIM5);            }
void  BSP_IntHandlerSPI3          (void)  { BSP_IntHandler(BSP_INT_ID_SPI3);            }
void  BSP_IntHandlerUSART4        (void)  { BSP_IntHandler(BSP_INT_ID_USART4);          }
void  BSP_IntHandlerUSART5        (void)  { BSP_IntHandler(BSP_INT_ID_USART5);          }
void  BSP_IntHandlerTIM6          (void)  { BSP_IntHandler(BSP_INT_ID_TIM6);            }
void  BSP_IntHandlerTIM7          (void)  { BSP_IntHandler(BSP_INT_ID_TIM7);            }
void  BSP_IntHandlerDMA2_CH1      (void)  { BSP_IntHandler(BSP_INT_ID_DMA2_CH1);        }
void  BSP_IntHandlerDMA2_CH2      (void)  { BSP_IntHandler(BSP_INT_ID_DMA2_CH2);        }
void  BSP_IntHandlerDMA2_CH3      (void)  { BSP_IntHandler(BSP_INT_ID_DMA2_CH3);        }
void  BSP_IntHandlerDMA2_CH4      (void)  { BSP_IntHandler(BSP_INT_ID_DMA2_CH4);        }
void  BSP_IntHandlerDMA2_CH5      (void)  { BSP_IntHandler(BSP_INT_ID_DMA2_CH5);        }
void  BSP_IntHandlerETH           (void)  { BSP_IntHandler(BSP_INT_ID_ETH);             }
void  BSP_IntHandlerETHWakeup     (void)  { BSP_IntHandler(BSP_INT_ID_ETH_WKUP);        }
void  BSP_IntHandlerCAN2_TX       (void)  { BSP_IntHandler(BSP_INT_ID_CAN2_TX);         }
void  BSP_IntHandlerCAN2_RX0      (void)  { BSP_IntHandler(BSP_INT_ID_CAN2_RX0);        }
void  BSP_IntHandlerCAN2_RX1      (void)  { BSP_IntHandler(BSP_INT_ID_CAN2_RX1);        }
void  BSP_IntHandlerCAN2_SCE      (void)  { BSP_IntHandler(BSP_INT_ID_CAN2_SCE);        }
void  BSP_IntHandlerOTG           (void)  { BSP_IntHandler(BSP_INT_ID_OTG_FS);          }


/*
*********************************************************************************************************
*********************************************************************************************************
*                                           LOCAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                          BSP_IntHandler()
*
* Description : Central interrupt handler.
*
* Argument(s) : int_id          Interrupt that will be handled.
*
* Return(s)   : none.
*
* Caller(s)   : ISR handlers.
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  BSP_IntHandler (CPU_DATA  int_id)
{
    CPU_FNCT_VOID  isr;

    if (int_id < BSP_INT_SRC_NBR) {
        isr = BSP_IntVectTbl[int_id];
        if (isr != (CPU_FNCT_VOID)0) {
            isr();
        }
    }

}


/*
*********************************************************************************************************
*                                        BSP_IntHandlerDummy()
*
* Description : Dummy interrupt handler.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : BSP_IntHandler().
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  BSP_IntHandlerDummy (void)
{

}
