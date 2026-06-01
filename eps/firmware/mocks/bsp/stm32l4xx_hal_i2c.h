#ifndef MOCK_BSP_HAL_I2C_H
#define MOCK_BSP_HAL_I2C_H

#include "i2c.h"
#include "stm32l4xx_hal.h"

#define HAL_I2C_ERROR_NONE      0x00000000U
#define HAL_I2C_ERROR_BERR      0x00000001U
#define HAL_I2C_ERROR_ARLO      0x00000002U
#define HAL_I2C_ERROR_AF        0x00000004U
#define HAL_I2C_ERROR_OVR       0x00000008U
#define HAL_I2C_ERROR_DMA       0x00000010U
#define HAL_I2C_ERROR_TIMEOUT   0x00000020U

#define I2C1                    ((void*)1)
#define I2C_MEMADD_SIZE_8BIT    1

#define I2C_ADDRESSINGMODE_7BIT 0
#define I2C_DUALADDRESS_DISABLE 0
#define I2C_OA2_NOMASK          0
#define I2C_GENERALCALL_DISABLE 0
#define I2C_NOSTRETCH_DISABLE   0

HAL_StatusTypeDef HAL_I2C_Init(I2C_HandleTypeDef *hi2c);
HAL_StatusTypeDef HAL_I2C_Master_Receive_IT(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size);
HAL_StatusTypeDef HAL_I2C_Master_Transmit_IT(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size);
HAL_StatusTypeDef HAL_I2C_Mem_Read_IT(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size);
HAL_StatusTypeDef HAL_I2C_Mem_Write_IT(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size);
void HAL_I2C_EV_IRQHandler(I2C_HandleTypeDef *hi2c);

#endif // MOCK_BSP_HAL_I2C_H
