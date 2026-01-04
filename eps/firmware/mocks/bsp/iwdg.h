/**
 * @file mocks/bsp/iwdg.h
 * @brief Mocks the STM32CubeMX generated IWDG header.
 */

#ifndef MOCK_BSP_IWDG_H
#define MOCK_BSP_IWDG_H

#include <stdint.h>

typedef struct {
    uint32_t Instance;
} IWDG_HandleTypeDef;

extern IWDG_HandleTypeDef hiwdg;

void MX_IWDG_Init(void);

#endif // MOCK_BSP_IWDG_H
