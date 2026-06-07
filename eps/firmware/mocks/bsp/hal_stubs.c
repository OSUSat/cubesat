#include "iwdg.h"
#include "stm32l4xx_hal.h"

IWDG_HandleTypeDef hiwdg;
uint32_t mock_iwdg_refresh_count = 0;

HAL_StatusTypeDef HAL_IWDG_Refresh(IWDG_HandleTypeDef *hiwdg_ptr) {
    (void)hiwdg_ptr;
    mock_iwdg_refresh_count++;
    return HAL_OK;
}

HAL_StatusTypeDef HAL_Init(void) { return HAL_OK; }
void MX_GPIO_Init(void) {}
void MX_ADC2_Init(void) {}
void MX_I2C1_Init(void) {}
void MX_I2C2_Init(void) {}
void MX_I2C3_Init(void) {}
void MX_I2C4_Init(void) {}
void MX_IWDG_Init(void) {}
void MX_DMA_Init(void) {}

void MX_USART1_UART_Init(void) {}
void MX_USART3_UART_Init(void) {}
