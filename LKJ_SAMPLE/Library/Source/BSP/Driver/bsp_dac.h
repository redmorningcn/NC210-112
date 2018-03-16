
#include "global.h"
#ifndef __BSP_DAC_H
#define __BSP_DAC_H


void BSP_dac_init(void);
void bsp_dac_set_dual_channel_voltage(uint16 ch1_vol, uint16 ch2_vol);

#endif
