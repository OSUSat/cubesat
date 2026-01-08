/**
 * @file hal_gpio.c
 * @brief Implementation of the GPIO HAL library
 */

#include "hal_gpio.h"
#include "eps_config.h"
#include "hal_gpio_types.h"
#include "stm32l496xx.h"
#include "stm32l4xx_hal.h"
#include "stm32l4xx_hal_cortex.h"
#include "stm32l4xx_hal_gpio.h"
#include <stdint.h>

typedef struct {
    bool initialized; /**< Init flag */
} gpio_hal_state_t;

typedef struct {
    gpio_pin_t pin;     /**< HAL GPIO pin descriptor */
    GPIO_TypeDef *port; /**< STM32 HAL GPIO port */
    uint16_t pin_mask;  /**< Pin bitmask */
} gpio_pin_context_t;

typedef struct {
    IRQn_Type irq;    /**< Associated IRQ handle */
    uint8_t refcount; /**< Number of users for this IRQ */
} exti_irq_state_t;

static gpio_hal_state_t state;
static gpio_pin_context_t pins[NUM_GPIO_PINS];
static exti_irq_state_t exti_irqs[] = {
    {EXTI0_IRQn, 0}, {EXTI1_IRQn, 0},   {EXTI2_IRQn, 0},     {EXTI3_IRQn, 0},
    {EXTI4_IRQn, 0}, {EXTI9_5_IRQn, 0}, {EXTI15_10_IRQn, 0},
};

static GPIO_PinState hal_gpio_state_to_stm32_state(gpio_state_t state) {
    if (state == HAL_GPIO_STATE_HIGH) {
        return GPIO_PIN_SET;
    } else {
        return GPIO_PIN_RESET;
    }
}

static gpio_state_t hal_stm32_state_to_gpio_state(GPIO_PinState state) {
    if (state == GPIO_PIN_SET) {
        return HAL_GPIO_STATE_HIGH;
    } else {
        return HAL_GPIO_STATE_LOW;
    }
}

static GPIO_TypeDef *hal_port_id_to_port(uint8_t port_id) {
    switch (port_id) {
    case 0:
        return GPIOA;
    case 1:
        return GPIOB;
    case 2:
        return GPIOC;
    case 3:
        return GPIOD;
    case 4:
        return GPIOE;
    case 5:
        return GPIOF;
    case 6:
        return GPIOG;
    case 7:
        return GPIOH;
    case 8:
        return GPIOI;
    default:
        return NULL;
    }
}

static IRQn_Type hal_gpio_pin_to_irq(uint16_t pin_mask) {
    switch (pin_mask) {
    case GPIO_PIN_0:
        return EXTI0_IRQn;
    case GPIO_PIN_1:
        return EXTI1_IRQn;
    case GPIO_PIN_2:
        return EXTI2_IRQn;
    case GPIO_PIN_3:
        return EXTI3_IRQn;
    case GPIO_PIN_4:
        return EXTI4_IRQn;

    case GPIO_PIN_5:
    case GPIO_PIN_6:
    case GPIO_PIN_7:
    case GPIO_PIN_8:
    case GPIO_PIN_9:
        return EXTI9_5_IRQn;

    case GPIO_PIN_10:
    case GPIO_PIN_11:
    case GPIO_PIN_12:
    case GPIO_PIN_13:
    case GPIO_PIN_14:
    case GPIO_PIN_15:
        return EXTI15_10_IRQn;

    default:
        return (IRQn_Type)0; // error
    }
}

static uint16_t hal_gpio_pull_to_stm32_pull(gpio_pull_t pull) {
    if (pull == HAL_GPIO_PULL_UP) {
        return GPIO_PULLUP;
    } else if (pull == HAL_GPIO_PULL_DOWN) {
        return HAL_GPIO_PULL_DOWN;
    } else {
        return GPIO_NOPULL;
    }
}

static void exti_irq_acquire(uint16_t pin_mask) {
    IRQn_Type irq = hal_gpio_pin_to_irq(pin_mask);

    size_t exti_irqs_len = sizeof(exti_irqs) / sizeof(exti_irq_state_t);

    for (size_t i = 0; i < exti_irqs_len; i++) {
        if (exti_irqs[i].irq == irq) {
            if (exti_irqs[i].refcount++ == 0) {
                HAL_NVIC_SetPriority(irq, 5, 0);
                HAL_NVIC_EnableIRQ(irq);
            }
            break;
        }
    }
}

static void exti_irq_release(uint16_t pin_mask) {
    IRQn_Type irq = hal_gpio_pin_to_irq(pin_mask);

    size_t exti_irqs_len = sizeof(exti_irqs) / sizeof(exti_irq_state_t);

    for (size_t i = 0; i < exti_irqs_len; i++) {
        if (exti_irqs[i].irq == irq) {
            if (--exti_irqs[i].refcount == 0) {
                HAL_NVIC_DisableIRQ(irq);
            }
            break;
        }
    }
}

void hal_gpio_init() {
    for (int i = 0; i < NUM_GPIO_PINS; i++) {
        pins[i].port = hal_port_id_to_port(gpio_board_config[i].port);
        pins[i].pin_mask = (1U << gpio_board_config[i].pin);
        pins[i].pin.pull = gpio_board_config[i].pull;

        hal_gpio_set_mode(i, gpio_board_config[i].default_mode);
    }

    state.initialized = true;
}

void hal_gpio_set_mode(uint8_t pin, gpio_mode_t mode) {
    if (pin >= NUM_GPIO_PINS || !state.initialized) {
        return;
    }

    gpio_pin_context_t *ctx = &pins[pin];

    if (ctx->pin.irq_enabled) {
        // release IRQ before changing pin modes to clean up
        exti_irq_release(ctx->pin_mask);
    }

    GPIO_InitTypeDef init = {0};

    ctx->pin.mode = mode;

    init.Pin = ctx->pin_mask;
    init.Pull = GPIO_NOPULL;
    init.Speed = GPIO_SPEED_FREQ_LOW;

    bool enable_irq = false;

    switch (mode) {
    case HAL_GPIO_MODE_OUTPUT:
        init.Mode = GPIO_MODE_OUTPUT_PP;

        break;
    case HAL_GPIO_MODE_INPUT:
        init.Mode = GPIO_MODE_INPUT;

        break;
    case HAL_GPIO_MODE_IT_RISING:
        init.Mode = GPIO_MODE_IT_RISING;
        init.Pull = hal_gpio_pull_to_stm32_pull(ctx->pin.pull);
        enable_irq = true;

        break;
    case HAL_GPIO_MODE_IT_FALLING:
        init.Mode = GPIO_MODE_IT_FALLING;
        init.Pull = hal_gpio_pull_to_stm32_pull(ctx->pin.pull);
        enable_irq = true;

        break;
    case HAL_GPIO_MODE_IT_RISING_FALLING:
        init.Mode = GPIO_MODE_IT_RISING_FALLING;
        init.Pull = hal_gpio_pull_to_stm32_pull(ctx->pin.pull);
        enable_irq = true;

        break;

    default:
        return;
    }

    if (enable_irq) {
        ctx->pin.irq_enabled = true;
        exti_irq_acquire(ctx->pin_mask);
    }

    HAL_GPIO_Init(ctx->port, &init);
}

void hal_gpio_write(uint8_t pin, gpio_state_t state) {
    if (pin >= NUM_GPIO_PINS) {
        return;
    }

    gpio_pin_context_t *ctx = &pins[pin];

    HAL_GPIO_WritePin(ctx->port, ctx->pin_mask,
                      hal_gpio_state_to_stm32_state(state));
}

void hal_gpio_toggle(uint8_t pin) {
    if (pin >= NUM_GPIO_PINS) {
        return;
    }

    gpio_pin_context_t *ctx = &pins[pin];

    HAL_GPIO_TogglePin(ctx->port, ctx->pin_mask);
}

gpio_state_t hal_gpio_read(uint8_t pin) {
    if (pin >= NUM_GPIO_PINS) {
        return HAL_GPIO_STATE_UNKNOWN;
    }

    gpio_pin_context_t *ctx = &pins[pin];

    GPIO_PinState state = HAL_GPIO_ReadPin(ctx->port, ctx->pin_mask);

    return hal_stm32_state_to_gpio_state(state);
}

void hal_gpio_register_callback(uint8_t pin, gpio_callback_t callback,
                                void *ctx) {
    if (pin >= NUM_GPIO_PINS || !state.initialized) {
        return;
    }

    gpio_pin_context_t *pin_ctx = &pins[pin];

    pin_ctx->pin.cb = callback;
    pin_ctx->pin.ctx = ctx;
}

void hal_gpio_exti_dispatch(uint16_t pin_mask) {
    for (int i = 0; i < NUM_GPIO_PINS; i++) {
        if (pins[i].pin_mask == pin_mask && pins[i].pin.cb) {
            pins[i].pin.cb(i, pins[i].pin.ctx);
        }
    }
}

void HAL_GPIO_EXTI_Callback(uint16_t pin) { hal_gpio_exti_dispatch(pin); }
