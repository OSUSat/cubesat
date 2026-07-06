#include "iwdg.h"
#include "stm32h7xx_hal.h"

IWDG_HandleTypeDef hiwdg;
uint32_t mock_iwdg_refresh_count = 0;

FDCAN_HandleTypeDef hfdcan1;
FDCAN_HandleTypeDef hfdcan2;
I2C_HandleTypeDef hi2c2;
SD_HandleTypeDef hsd1;
SPI_HandleTypeDef hspi1;
SPI_HandleTypeDef hspi4;
UART_HandleTypeDef huart7;
UART_HandleTypeDef huart1;
UART_HandleTypeDef huart6;
PCD_HandleTypeDef hpcd_USB_OTG_HS;

HAL_StatusTypeDef HAL_IWDG_Refresh(IWDG_HandleTypeDef *hiwdg_ptr) {
    (void)hiwdg_ptr;
    mock_iwdg_refresh_count++;
    return HAL_OK;
}

HAL_StatusTypeDef HAL_Init(void) { return HAL_OK; }

void MPU_Config(void) {}
void MX_GPIO_Init(void) {}
void MX_FDCAN1_Init(void) {}
void MX_FDCAN2_Init(void) {}
void MX_I2C2_Init(void) {}
void MX_SDMMC1_SD_Init(void) {}
void MX_SPI1_Init(void) {}
void MX_SPI4_Init(void) {}
void MX_UART7_Init(void) {}
void MX_USART1_UART_Init(void) {}
void MX_USART6_UART_Init(void) {}
void MX_USB_OTG_HS_PCD_Init(void) {}
