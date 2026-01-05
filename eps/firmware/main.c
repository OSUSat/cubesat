#include "adc.h"
#include "app/command_handler.h"
#include "clock.h"
#include "dma.h"
#include "gpio.h"
#include "hal_time.h"
#include "hal_uart.h"
#include "i2c.h"
#include "iwdg.h"
#include "logging.h"
#include "mppt_controller.h"
#include "osusat/event_bus.h"
#include "osusat/slog.h"
#include "power_policies.h"
#include "rail_controller.h"
#include "redundancy_manager.h"
#include "services/power_profiles.h"
#include "stm32l4xx_hal.h"
#include "stm32l4xx_hal_gpio.h"
#include "stm32l4xx_hal_rcc.h"
#include "uart_events.h"
#include "usart.h"

// event bus
#define EVENT_QUEUE_SIZE 16
static osusat_event_t event_queue[EVENT_QUEUE_SIZE];

// applications
static command_handler_t command_handler;
static power_policies_t power_policies;

// services
static rail_controller_t rail_controller;
static power_profiles_t power_profiles_service;
static mppt_t mppt_controller_service;
static redundancy_manager_t redundancy_manager_service;
static uart_events_t usart1_events_service;
static uart_events_t usart3_events_service;

int main() {
    // initialize BSP HAL
    HAL_Init();
    bsp_clock_init();

    MX_DMA_Init();

    MX_GPIO_Init();
    MX_ADC2_Init();

    MX_I2C1_Init();
    MX_I2C2_Init();
    MX_I2C3_Init();
    MX_I2C4_Init();

    MX_USART1_UART_Init();
    MX_USART3_UART_Init();

    MX_IWDG_Init();

    // initialize event bus
    osusat_event_bus_init(event_queue, EVENT_QUEUE_SIZE);

    // initialize HAL
    hal_time_init();

    uart_config_t uart_config = {.baudrate = 115200};

    hal_uart_init(UART_PORT_1, &uart_config);
    hal_uart_init(UART_PORT_3, &uart_config);

    // initialize services
    uart_events_init(&usart1_events_service, UART_PORT_1);
    uart_events_init(&usart3_events_service, UART_PORT_3);

    logging_init(OSUSAT_SLOG_INFO, &usart1_events_service,
                 &usart3_events_service);
    rail_controller_init(&rail_controller);
    power_profiles_init(&power_profiles_service, &rail_controller);
    mppt_init(&mppt_controller_service);
    redundancy_manager_init(&redundancy_manager_service);

    // initialize applications
    command_handler_init(&command_handler);
    power_policies_init(&power_policies);

    LOG_INFO(EPS_COMPONENT_MAIN, "Initialization complete");

    while (1) {
        osusat_event_bus_process();
    }

    return 0;
}
