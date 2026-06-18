#ifndef MOCK_BSP_I2C_H
#define MOCK_BSP_I2C_H

#include <stdint.h>

typedef struct {
    uint32_t Instance;
} I2C_HandleTypeDef;

void MX_I2C1_Init(void);
void MX_I2C2_Init(void);
void MX_I2C3_Init(void);
void MX_I2C4_Init(void);

#endif // MOCK_BSP_I2C_H
