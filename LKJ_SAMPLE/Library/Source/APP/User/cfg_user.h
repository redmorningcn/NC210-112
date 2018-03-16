
#ifndef __CFG_USE_H
#define __CFG_USE_H



#define LKJ_MODEL

#ifdef LKJ_MODEL
#define	DISABLE_UART3_4_FOR_MODBUS
#define 	PHOTOELECTRIC_VELOCITY_MEASUREMENT
#define 	NON_ENERGY_CALC
#endif

#define SAMPLE_BOARD

#ifdef SAMPLE_BOARD
#define DISABLE_OSAL_TASK
#define DISABLE_MATER_TASK
#define DISABLE_DTU_TASK
#endif

#endif


