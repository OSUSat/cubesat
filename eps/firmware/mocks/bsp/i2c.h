#ifndef MOCK_BSP_I2C_H
#define MOCK_BSP_I2C_H

#include <stdint.h>

typedef struct {
    uint32_t Timing;
    uint32_t OwnAddress1;
    uint32_t AddressingMode;
    uint32_t DualAddressMode;
    uint32_t OwnAddress2;
    uint32_t OwnAddress2Masks;
    uint32_t GeneralCallMode;
    uint32_t NoStretchMode;
} I2C_InitTypeDef;

typedef struct {
    void *Instance;
    I2C_InitTypeDef Init;
    uint32_t ErrorCode;
} I2C_HandleTypeDef;

void MX_I2C1_Init(void);
void MX_I2C2_Init(void);
void MX_I2C3_Init(void);
void MX_I2C4_Init(void);

#endif // MOCK_BSP_I2C_H
