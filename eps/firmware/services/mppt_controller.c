/**
 * @file mppt_controller.c
 * @brief MPPT Controller Service Implementation
 */

#include "mppt_controller.h"
#include "eps_config.h"
#include "events.h"
#include "hal_gpio.h"
#include "hal_i2c.h"
#include "osusat/event_bus.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define MPPT_CONTROLLER_UPDATE_INTEVAL_TICKS 10
#define TELEMETRY_INTERVAL_CYCLES 600

#define MPPT_ENABLE_PIN 26
#define MPPT_PGOOD_PIN 27

#define MPPT_I2C_ADDR 0x24
#define REG_STATUS 0x00

#define STATUS_PGOOD_MASK 0x01
#define STATUS_OVERTEMP_MASK 0x02
#define STATUS_UNDERVOLT_MASK 0x04
#define STATUS_FAULT_MASK 0x08

static mppt_t *g_mppt = NULL;

/**
 * @brief System Tick Handler.
 * Called automatically by the Event Bus.
 *
 * @param e   The event (SYSTICK).
 * @param ctx The context pointer (points to mppt_t).
 */
static void mppt_handle_tick(const osusat_event_t *e, void *ctx);

/**
 * @brief Enable Channel Request Handler.
 * Called when the application wants a channel enabled.
 *
 * @param e   The event (SYSTICK).
 * @param ctx The context pointer (points to mppt_t).
 */
static void mppt_handle_request(const osusat_event_t *e, void *ctx);

/**
 * @brief Internal update logic (reads sensors).
 */
static void mppt_perform_update(mppt_t *mppt);

/**
 * @brief Callback for PGOOD GPIO interrupt.
 */
static void mppt_handle_pgood_interrupt(uint8_t pin, void *ctx);

void mppt_init(mppt_t *mppt) {
    if (mppt == NULL) {
        return;
    }

    memset(mppt, 0, sizeof(mppt_t));

    static mppt_channel_t mppt_channels[NUM_MPPT_CHANNELS];
    memset(mppt_channels, 0, sizeof(mppt_channels));

    mppt->channels = mppt_channels;
    mppt->num_channels = NUM_MPPT_CHANNELS;
    mppt->initialized = true;

    g_mppt = mppt;

    // configure control and telemetry pins
    hal_gpio_set_mode(MPPT_ENABLE_PIN, HAL_GPIO_MODE_OUTPUT);
    hal_gpio_write(MPPT_ENABLE_PIN, HAL_GPIO_STATE_LOW);

    hal_gpio_set_mode(MPPT_PGOOD_PIN, HAL_GPIO_MODE_IT_RISING_FALLING);
    hal_gpio_register_callback(MPPT_PGOOD_PIN, mppt_handle_pgood_interrupt,
                               mppt);

    osusat_event_bus_subscribe(EVENT_SYSTICK, mppt_handle_tick, mppt);
    osusat_event_bus_subscribe(APP_EVENT_REQUEST_MPPT_ENABLE_CHANNEL,
                               mppt_handle_request, mppt);
    osusat_event_bus_subscribe(APP_EVENT_REQUEST_MPPT_DISABLE_CHANNEL,
                               mppt_handle_request, mppt);
}

static void mppt_handle_request(const osusat_event_t *e, void *ctx) {
    if (e->payload_len != 1) {
        return;
    }

    if (e->id == APP_EVENT_REQUEST_MPPT_ENABLE_CHANNEL) {
        mppt_enable(e->payload[0]);
    } else {
        mppt_disable(e->payload[0]);
    }

    // manually perform update to process new change
    mppt_perform_update((mppt_t *)ctx);
}

void mppt_enable(uint8_t ch) {
    if (ch >= NUM_MPPT_CHANNELS || g_mppt == NULL) {
        return;
    }

    g_mppt->channels[ch].enabled = true;
    g_mppt->channels[ch].status = MPPT_STATUS_OK;

    hal_gpio_write(MPPT_ENABLE_PIN, HAL_GPIO_STATE_HIGH);

    gpio_state_t pgood_state = hal_gpio_read(MPPT_PGOOD_PIN);
    g_mppt->channels[ch].pgood = (pgood_state == HAL_GPIO_STATE_HIGH);
}

void mppt_disable(uint8_t ch) {
    if (ch >= NUM_MPPT_CHANNELS || g_mppt == NULL) {
        return;
    }

    g_mppt->channels[ch].enabled = false;
    g_mppt->channels[ch].status = MPPT_STATUS_DISABLED;
    g_mppt->channels[ch].pgood = false;

    hal_gpio_write(MPPT_ENABLE_PIN, HAL_GPIO_STATE_LOW);
}

static void mppt_handle_tick(const osusat_event_t *e, void *ctx) {
    (void)e;

    mppt_t *mppt = (mppt_t *)ctx;

    if (mppt == NULL || !mppt->initialized) {
        return;
    }

    mppt->tick_counter++;

    if (mppt->tick_counter >= MPPT_CONTROLLER_UPDATE_INTEVAL_TICKS) {
        mppt->tick_counter = 0;
        mppt_perform_update(mppt);
    }
}

static void mppt_perform_update(mppt_t *mppt) {
    if (mppt == NULL || !mppt->initialized) {
        return;
    }

    for (size_t channel = 0; channel < mppt->num_channels; channel++) {
        mppt_channel_t *ch = &mppt->channels[channel];
        if (!ch->enabled) {
            continue;
        }

        uint8_t buf[9] = {0};
        i2c_error_t err = hal_i2c_mem_read(I2C_BUS_2, MPPT_I2C_ADDR, REG_STATUS,
                                           buf, 9, NULL, NULL, NULL);

        if (err == I2C_HAL_ERR_NONE) {
            uint8_t status_reg = buf[0];
            uint16_t raw_vin = ((uint16_t)buf[1] << 8) | buf[2];
            uint16_t raw_iin = ((uint16_t)buf[3] << 8) | buf[4];
            uint16_t raw_vout = ((uint16_t)buf[5] << 8) | buf[6];
            uint16_t raw_iout = ((uint16_t)buf[7] << 8) | buf[8];

            ch->pgood = (status_reg & STATUS_PGOOD_MASK) != 0;

            if ((status_reg & STATUS_OVERTEMP_MASK) != 0) {
                ch->status = MPPT_STATUS_OVERTEMP;
            } else if ((status_reg & STATUS_UNDERVOLT_MASK) != 0) {
                ch->status = MPPT_STATUS_UNDERVOLT;
            } else if ((status_reg & STATUS_FAULT_MASK) != 0) {
                ch->status = MPPT_STATUS_FAULT;
            } else {
                ch->status = MPPT_STATUS_OK;
            }

            ch->input_voltage = (float)raw_vin / 1000.0f;
            ch->input_current = (float)raw_iin / 1000.0f;
            ch->output_voltage = (float)raw_vout / 1000.0f;
            ch->output_current = (float)raw_iout / 1000.0f;
            ch->power = ch->output_voltage * ch->output_current;

            if (ch->status != MPPT_STATUS_OK) {
                osusat_event_bus_publish(MPPT_EVENT_FAULT_DETECTED, ch,
                                         sizeof(mppt_channel_t));
            }
        } else {
            ch->status = MPPT_STATUS_FAULT;
            osusat_event_bus_publish(MPPT_EVENT_FAULT_DETECTED, ch,
                                     sizeof(mppt_channel_t));
        }
    }

    mppt->telemetry_tick_counter++;

    if (mppt->telemetry_tick_counter >= TELEMETRY_INTERVAL_CYCLES) {
        mppt->telemetry_tick_counter = 0;

        for (size_t channel = 0; channel < mppt->num_channels; channel++) {
            if (!mppt->channels[channel].enabled) {
                continue;
            }

            osusat_event_bus_publish(MPPT_EVENT_TELEMETRY,
                                     &mppt->channels[channel],
                                     sizeof(mppt_channel_t));
        }
    }
}

static void mppt_handle_pgood_interrupt(uint8_t pin, void *ctx) {
    (void)pin;
    mppt_t *mppt = (mppt_t *)ctx;
    if (mppt == NULL || !mppt->initialized) {
        return;
    }

    gpio_state_t state = hal_gpio_read(MPPT_PGOOD_PIN);
    bool pgood_val = (state == HAL_GPIO_STATE_HIGH);

    for (size_t ch = 0; ch < mppt->num_channels; ch++) {
        if (mppt->channels[ch].enabled) {
            mppt->channels[ch].pgood = pgood_val;
            osusat_event_bus_publish(MPPT_EVENT_PGOOD_CHANGED, &pgood_val,
                                     sizeof(bool));
        }
    }
}
