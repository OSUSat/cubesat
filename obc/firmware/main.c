#include "clock.h"
#include "dma.h"
#include "hal_can.h"
#include "hal_gpio.h"
#include "hal_i2c.h"
#include "hal_time.h"
#include "hal_uart.h"
#include "obc_config.h"
#include "osusat/event_bus.h"
#include "peripherals.h"
#include "stm32h7xx_hal.h"

#define EVENT_QUEUE_SIZE 16
static osusat_event_t event_queue[EVENT_QUEUE_SIZE];

int main(void) {
    MPU_Config();
    HAL_Init();

    bsp_clock_init();

    MX_GPIO_Init();
    MX_FDCAN1_Init();
    MX_FDCAN2_Init();
    MX_I2C2_Init();
    MX_SDMMC1_SD_Init();
    MX_SPI1_Init();
    MX_SPI4_Init();
    MX_UART7_Init();
    MX_USART1_UART_Init();
    MX_USART6_UART_Init();
    MX_USB_OTG_HS_PCD_Init();

    hal_gpio_init();

    hal_gpio_write(0, HAL_GPIO_STATE_HIGH);

    osusat_event_bus_init(event_queue, EVENT_QUEUE_SIZE);

    hal_time_init();

    uart_config_t uart_config = {.baudrate = 115200};
    hal_uart_init(UART_PORT_1, &uart_config);
    hal_uart_init(UART_PORT_6, &uart_config);
    hal_uart_init(UART_PORT_7, &uart_config);

    hal_can_config_t can_config = {.baudrate = 250000};
    hal_can_init(HAL_CAN_PORT_1, &can_config);
    hal_can_init(HAL_CAN_PORT_2, &can_config);

    hal_i2c_init(I2C_BUS_2);

#if defined(__arm__)
    extern volatile uint8_t g_main_tick_flag;

    while (1) {
        if (g_main_tick_flag) {
            g_main_tick_flag = 0;
            osusat_event_bus_publish(EVENT_SYSTICK, NULL, 0);
            osusat_event_bus_process();
        } else {
            __WFI();
        }
    }
#else
    while (1) {
        osusat_event_bus_process();
    }
#endif

    return 0;
}
