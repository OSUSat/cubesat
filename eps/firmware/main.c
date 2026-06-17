#include "adc.h"
#include "app/command_handler.h"
#include "battery_management.h"
#include "can_events.h"
#include "clock.h"
#include "dma.h"
#include "gpio.h"
#include "hal_can.h"
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
#include "telemetry.h"
#include "uart_events.h"
#include "usart.h"
#include "watchdog.h"

// event bus
#define EVENT_QUEUE_SIZE 16
static osusat_event_t event_queue[EVENT_QUEUE_SIZE];

// applications
static command_handler_t command_handler;
static power_policies_t power_policies;

// services
static battery_management_t battery_manager_service;
static rail_controller_t rail_controller;
static power_profiles_t power_profiles_service;
static mppt_t mppt_controller_service;
static redundancy_manager_t redundancy_manager_service;
static uart_events_t usart1_events_service;
static uart_events_t usart3_events_service;
static can_events_t can1_events_service;
static can_events_t can2_events_service;
static watchdog_t watchdog;
static telemetry_t telemetry;

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

    hal_can_config_t can_config = {.baudrate = 250000};
    hal_can_init(HAL_CAN_PORT_1, &can_config);
    hal_can_init(HAL_CAN_PORT_2, &can_config);

    can_events_init(&can1_events_service, HAL_CAN_PORT_1);
    can_events_init(&can2_events_service, HAL_CAN_PORT_2);

    logging_init(OSUSAT_SLOG_INFO, &can1_events_service, &can2_events_service);
    battery_init(&battery_manager_service);
    rail_controller_init(&rail_controller);
    power_profiles_init(&power_profiles_service, &rail_controller);
    mppt_init(&mppt_controller_service);
    redundancy_manager_init(&redundancy_manager_service);
    watchdog_init(&watchdog);
    telemetry_init(&telemetry);

    telemetry.battery_manager = &battery_manager_service;
    telemetry.mppt_controller = &mppt_controller_service;
    telemetry.rail_controller = &rail_controller;
    telemetry.redundancy_manager = &redundancy_manager_service;
    telemetry.usart1_events = &usart1_events_service;
    telemetry.usart3_events = &usart3_events_service;
    telemetry.can1_events = &can1_events_service;
    telemetry.can2_events = &can2_events_service;

    // initialize applications
    command_handler_init(&command_handler);
    power_policies_init(&power_policies);

    LOG_INFO(EPS_COMPONENT_MAIN, "Initialization complete");

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
