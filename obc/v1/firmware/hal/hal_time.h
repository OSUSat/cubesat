#ifndef HAL_TIME_H
#define HAL_TIME_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void hal_time_init(void);
uint32_t hal_time_get_ms(void);
uint64_t hal_time_get_us(void);

#ifdef __cplusplus
}
#endif

#endif // HAL_TIME_H
