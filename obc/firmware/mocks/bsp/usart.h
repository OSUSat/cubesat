/**
 * @file mocks/bsp/usart.h
 * @brief Mocks the STM32CubeMX generated USART header.
 */

#ifndef MOCK_BSP_USART_H
#define MOCK_BSP_USART_H

#include <stdint.h>

typedef struct {
    uint32_t Instance;
} UART_HandleTypeDef;

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart3;

void MX_USART1_UART_Init(void);
void MX_USART3_UART_Init(void);

#endif // MOCK_BSP_USART_H
