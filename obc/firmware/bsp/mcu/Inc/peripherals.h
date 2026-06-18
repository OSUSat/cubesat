#ifndef PERIPHERALS_H
#define PERIPHERALS_H

#include "main.h"

extern FDCAN_HandleTypeDef hfdcan1;
extern FDCAN_HandleTypeDef hfdcan2;
extern I2C_HandleTypeDef hi2c2;
extern SD_HandleTypeDef hsd1;
extern SPI_HandleTypeDef hspi1;
extern SPI_HandleTypeDef hspi4;
extern UART_HandleTypeDef huart7;
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart6;
extern PCD_HandleTypeDef hpcd_USB_OTG_HS;

void MPU_Config(void);
void MX_GPIO_Init(void);
void MX_FDCAN1_Init(void);
void MX_FDCAN2_Init(void);
void MX_I2C2_Init(void);
void MX_SDMMC1_SD_Init(void);
void MX_SPI1_Init(void);
void MX_SPI4_Init(void);
void MX_UART7_Init(void);
void MX_USART1_UART_Init(void);
void MX_USART6_UART_Init(void);
void MX_USB_OTG_HS_PCD_Init(void);

#endif // PERIPHERALS_H
