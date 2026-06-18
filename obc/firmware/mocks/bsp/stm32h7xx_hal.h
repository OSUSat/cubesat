#ifndef MOCK_BSP_HAL_H
#define MOCK_BSP_HAL_H

#include <stdint.h>

typedef enum {
    HAL_OK = 0x00U,
    HAL_ERROR = 0x01U,
    HAL_BUSY = 0x02U,
    HAL_TIMEOUT = 0x03U
} HAL_StatusTypeDef;

HAL_StatusTypeDef HAL_Init(void);

#endif // MOCK_BSP_HAL_H
