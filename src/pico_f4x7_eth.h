/*********************************************************************
PicoTCP. Copyright (c) 2012 TASS Belgium NV. Some rights reserved.
See LICENSE and COPYING for usage.
Do not redistribute without a written permission by the Copyright
holders.

*********************************************************************/
#ifndef INCLUDE_PICO_F4X7_ETH_H
#define INCLUDE_PICO_F4X7_ETH_H
#include "pico_config.h"
#include "pico_device.h"
#include "stm32f4xx_hal_conf.h"

void ETH_IRQHandler(void);


void pico_eth_destroy(struct pico_device *loop);
struct pico_device *pico_eth_create(char *name);

#endif /* INCLUDE_PICO_F4X7_ETH_H */

