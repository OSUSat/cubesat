/**
 * @file mocks/bsp/iwdg.h
 * @brief Mocks the STM32CubeMX generated IWDG header.
 */

#ifndef MOCK_BSP_IWDG_H
#define MOCK_BSP_IWDG_H

#include "stm32l4xx_hal.h"
#include <stdint.h>

typedef struct {
    uint32_t Instance;
} IWDG_HandleTypeDef;

extern IWDG_HandleTypeDef hiwdg;
extern uint32_t mock_iwdg_refresh_count;

void MX_IWDG_Init(void);
HAL_StatusTypeDef HAL_IWDG_Refresh(IWDG_HandleTypeDef *hiwdg);

#endif // MOCK_BSP_IWDG_H
