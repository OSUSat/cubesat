#include "can_events.h"
#include "clock.h"
#include "dma.h"
#include "hal_can.h"
#include "hal_gpio.h"
#include "hal_i2c.h"
#include "hal_time.h"
#include "hal_uart.h"
#include "logging.h"
#include "sensors.h"
#include "obc_config.h"
#include "osusat/event_bus.h"
#include "peripherals.h"
#include "stm32h7xx_hal.h"
#include "hal_fram.h"

uint8_t debug_fram_buf[1024];
uint8_t debug_telemetry_buf[1024];

#define EVENT_QUEUE_SIZE 16
static osusat_event_t event_queue[EVENT_QUEUE_SIZE];
static can_events_t can1_events_service;
static can_events_t can2_events_service;

#if defined(__arm__) && defined(DEEP_SLEEP_ENABLED) && (DEEP_SLEEP_ENABLED == 1)
static void check_deep_sleep_status(void) {
    __HAL_RCC_PWR_CLK_ENABLE();
    HAL_PWR_EnableBkUpAccess();

    // Disable write protection
    RTC->WPR = 0xCA;
    RTC->WPR = 0x53;

    uint32_t magic = RTC->BKP1R;
    if (magic == 0x534C505F) {
        uint32_t cycles = RTC->BKP0R;
        if (cycles > 0) {
            RTC->BKP0R = cycles - 1;
            
            // Enable LSI
            RCC_OscInitTypeDef RCC_OscInitStruct = {0};
            RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI;
            RCC_OscInitStruct.LSIState = RCC_LSI_ON;
            if (HAL_RCC_OscConfig(&RCC_OscInitStruct) == HAL_OK) {
                // Enable RTC clock source (LSI)
                __HAL_RCC_RTC_CONFIG(RCC_RTCCLKSOURCE_LSI);
                __HAL_RCC_RTC_ENABLE();

                // Disable Wakeup Timer
                RTC->CR &= ~RTC_CR_WUTE;

                // Wait for wakeup timer write flag to be set
                while ((RTC->ISR & RTC_ISR_WUTWF) == 0);

                // Set wakeup clock source to ck_spre (1 Hz clock)
                RTC->CR &= ~RTC_CR_WUCKSEL;
                RTC->CR |= 4U; // 4: ck_spre (1Hz)

                // Set counter value (10 - 1 = 9)
                RTC->WUTR = 9;

                // Clear Wakeup Flag
                RTC->ISR &= ~RTC_ISR_WUTF;

                // Enable Wakeup Timer and interrupt
                RTC->CR |= RTC_CR_WUTIE | RTC_CR_WUTE;

                // Clear PWR wakeup flags
                __HAL_PWR_CLEAR_WAKEUPFLAG(PWR_FLAG_WKUP1 | PWR_FLAG_WKUP2 | PWR_FLAG_WKUP3 | PWR_FLAG_WKUP4 | PWR_FLAG_WKUP5 | PWR_FLAG_WKUP6);

                // Enter Standby Mode
                HAL_PWR_EnterSTANDBYMode();
            }
        } else {
            RTC->BKP1R = 0;
        }
    }
    
    // Enable write protection
    RTC->WPR = 0xFF;
}

static void enter_deep_sleep(void) {
    LOG_INFO(OBC_COMPONENT_MAIN, "Entering deep sleep standby mode for %d seconds...", DEEP_SLEEP_SLEEP_DURATION_S);
    HAL_Delay(100);

    __HAL_RCC_PWR_CLK_ENABLE();
    HAL_PWR_EnableBkUpAccess();

    // Enable LSI
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI;
    RCC_OscInitStruct.LSIState = RCC_LSI_ON;
    HAL_RCC_OscConfig(&RCC_OscInitStruct);

    // Enable RTC clock source (LSI)
    __HAL_RCC_RTC_CONFIG(RCC_RTCCLKSOURCE_LSI);
    __HAL_RCC_RTC_ENABLE();

    // Disable write protection
    RTC->WPR = 0xCA;
    RTC->WPR = 0x53;

    // Disable Wakeup Timer
    RTC->CR &= ~RTC_CR_WUTE;

    // Wait for wakeup timer write flag to be set
    while ((RTC->ISR & RTC_ISR_WUTWF) == 0);

    // Set wakeup clock source to ck_spre (1 Hz clock)
    RTC->CR &= ~RTC_CR_WUCKSEL;
    RTC->CR |= 4U; // 4: ck_spre (1Hz)

    // Set counter value (duration - 1)
    RTC->WUTR = DEEP_SLEEP_SLEEP_DURATION_S - 1;

    // Clear Wakeup Flag
    RTC->ISR &= ~RTC_ISR_WUTF;

    // Enable Wakeup Timer and interrupt
    RTC->CR |= RTC_CR_WUTIE | RTC_CR_WUTE;

    // Enable write protection
    RTC->WPR = 0xFF;

    // Clear PWR wakeup flags
    __HAL_PWR_CLEAR_WAKEUPFLAG(PWR_FLAG_WKUP1 | PWR_FLAG_WKUP2 | PWR_FLAG_WKUP3 | PWR_FLAG_WKUP4 | PWR_FLAG_WKUP5 | PWR_FLAG_WKUP6);

    // Enter Standby Mode
    HAL_PWR_EnterSTANDBYMode();

    // Fallback in case Standby failed
    HAL_NVIC_SystemReset();
}
#endif

int main(void) {
#if defined(__arm__)
    SCB->VTOR = 0x08000000;
#endif

    MPU_Config();

    HAL_Init();

#if defined(__arm__) && defined(DEEP_SLEEP_ENABLED) && (DEEP_SLEEP_ENABLED == 1)
    check_deep_sleep_status();
#endif

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
    MX_I2C2_Init();
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

    hal_i2c_init(I2C_BUS_2);
    sensors_init();

    // Debug FRAM reads removed to prevent startup I2C conflicts

#if defined(__arm__)
    extern volatile uint8_t g_main_tick_flag;

    while (1) {
        if (g_main_tick_flag) {
            g_main_tick_flag = 0;
            osusat_event_bus_publish(EVENT_SYSTICK, NULL, 0);
            osusat_event_bus_process();

            static uint32_t last_blink = 0;
            uint32_t now = hal_time_get_ms();
            if (now - last_blink >= 2000) {
                last_blink = now;
                hal_gpio_write(GREEN_LED_PIN, HAL_GPIO_STATE_HIGH);
                hal_gpio_write(RED_LED_PIN, HAL_GPIO_STATE_LOW);
                LOG_INFO(OBC_COMPONENT_MAIN, "OBC heartbeat, uptime: %lu ms",
                         (unsigned long)now);
            } else if (now - last_blink >= 20) {
                hal_gpio_write(GREEN_LED_PIN, HAL_GPIO_STATE_LOW);
            }

            hal_gpio_toggle(WATCHDOG_WDI_PIN);

#if defined(__arm__) && defined(DEEP_SLEEP_ENABLED) && (DEEP_SLEEP_ENABLED == 1)
            if (now >= DEEP_SLEEP_WAKE_DURATION_S * 1000) {
                enter_deep_sleep();
            }
#endif
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
