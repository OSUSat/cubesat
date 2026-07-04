#ifndef __CAN_H__
#define __CAN_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

extern CAN_HandleTypeDef hcan1;
extern CAN_HandleTypeDef hcan2;

void MX_CAN1_Init(void);
void MX_CAN2_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* __CAN_H__ */
