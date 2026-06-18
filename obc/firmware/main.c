#include "can_events.h"
#include "clock.h"
#include "dma.h"
#include "hal_can.h"
#include "hal_gpio.h"
#include "hal_i2c.h"
#include "hal_time.h"
#include "hal_uart.h"
#include "logging.h"
#include "obc_config.h"
#include "osusat/event_bus.h"
#include "peripherals.h"
#include "stm32h7xx_hal.h"

#define EVENT_QUEUE_SIZE 16
static osusat_event_t event_queue[EVENT_QUEUE_SIZE];
static can_events_t can1_events_service;
static can_events_t can2_events_service;

int main(void) {
#if defined(__arm__)
    SCB->VTOR = 0x08000000;
#endif

    MPU_Config();

    HAL_Init();

#if defined(__arm__)
    HAL_DBGMCU_EnableDBGSleepMode();
    HAL_DBGMCU_EnableDBGStopMode();
    HAL_DBGMCU_EnableDBGStandbyMode();
#endif

    bsp_clock_init();

    MX_GPIO_Init();
    hal_gpio_init();

    hal_gpio_write(WATCHDOG_SET0_PIN, HAL_GPIO_STATE_HIGH);
    hal_gpio_write(WATCHDOG_SET1_PIN, HAL_GPIO_STATE_HIGH);
    hal_gpio_write(WATCHDOG_SET2_PIN, HAL_GPIO_STATE_LOW);
    hal_gpio_write(WATCHDOG_WDI_PIN, HAL_GPIO_STATE_LOW);

    MX_FDCAN1_Init();
    MX_FDCAN2_Init();
    // MX_I2C2_Init();
    // MX_SDMMC1_SD_Init();
    // MX_SPI1_Init();
    // MX_SPI4_Init();
    // MX_UART7_Init();
    // MX_USART1_UART_Init();
    // MX_USART6_UART_Init();
    // MX_USB_OTG_HS_PCD_Init();

    osusat_event_bus_init(event_queue, EVENT_QUEUE_SIZE);

    hal_time_init();

    logging_init(OSUSAT_SLOG_INFO);

    // uart_config_t uart_config = {.baudrate = 115200};
    // hal_uart_init(UART_PORT_1, &uart_config);
    // hal_uart_init(UART_PORT_6, &uart_config);
    // hal_uart_init(UART_PORT_7, &uart_config);

    hal_can_config_t can_config = {.baudrate = 250000};
    hal_can_init(HAL_CAN_PORT_1, &can_config);
    hal_can_init(HAL_CAN_PORT_2, &can_config);

    can_events_init(&can1_events_service, HAL_CAN_PORT_1);
    can_events_init(&can2_events_service, HAL_CAN_PORT_2);

    LOG_INFO(OBC_COMPONENT_MAIN, "OBC initialization complete");

    /*
    hal_gpio_write(WATCHDOG_SET0_PIN, HAL_GPIO_STATE_LOW);
    hal_gpio_write(WATCHDOG_SET1_PIN, HAL_GPIO_STATE_LOW);
    hal_gpio_write(WATCHDOG_SET2_PIN, HAL_GPIO_STATE_HIGH);
    */

    // hal_gpio_toggle(WATCHDOG_WDI_PIN);

    // hal_i2c_init(I2C_BUS_2);

#if defined(__arm__)
    extern volatile uint8_t g_main_tick_flag;

    while (1) {
        if (g_main_tick_flag) {
            g_main_tick_flag = 0;
            osusat_event_bus_publish(EVENT_SYSTICK, NULL, 0);
            osusat_event_bus_process();

            static uint32_t last_blink = 0;
            uint32_t now = hal_time_get_ms();
            if (now - last_blink >= 500) {
                last_blink = now;

                hal_gpio_toggle(GREEN_LED_PIN);
                hal_gpio_write(RED_LED_PIN, HAL_GPIO_STATE_LOW);

                LOG_INFO(OBC_COMPONENT_MAIN, "OBC heartbeat, uptime: %lu ms",
                         (unsigned long)now);
            }

            hal_gpio_toggle(WATCHDOG_WDI_PIN);
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
