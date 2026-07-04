/**
 * @file telemetry.c
 * @brief Telemetry aggregation service implementation.
 */

#include "telemetry.h"
#include "events.h"
#include "hal_time.h"
#include "logging.h"
#include "osusat/event_bus.h"
#include "hal_adc.h"
#include <string.h>

#define TELEMETRY_UPDATE_INTERVAL_TICKS 600

/**
 * @brief System Tick Handler.
 * Called automatically by the Event Bus.
 *
 * @param e   The event (SYSTICK).
 * @param ctx The context pointer (points to mppt_t).
 */
static void telemetry_handle_tick(const osusat_event_t *e, void *ctx);

void telemetry_init(telemetry_t *telemetry) {
    if (telemetry == NULL) {
        return;
    }

    osusat_event_bus_subscribe(EVENT_SYSTICK, telemetry_handle_tick, telemetry);

    memset(telemetry, 0, sizeof(telemetry_t));

    telemetry->initialized = true;
}

void telemetry_update(telemetry_t *telemetry) {
    if (telemetry == NULL) {
        return;
    }

    // copy battery status if valid
    if (telemetry->battery_manager != NULL) {
        telemetry->telemetry.battery =
            telemetry->battery_manager->battery_status;
    }

    // copy mppt channels if valid
    if (telemetry->mppt_controller != NULL &&
        telemetry->mppt_controller->channels != NULL) {

        size_t count = telemetry->mppt_controller->num_channels;

        if (count > NUM_MPPT_CHANNELS) {
            count = NUM_MPPT_CHANNELS;
        }

        for (size_t i = 0; i < count; i++) {
            telemetry->telemetry.mppt_channels[i] =
                telemetry->mppt_controller->channels[i];
        }
    }

    // copy rail statuses if valid
    if (telemetry->rail_controller != NULL) {
        for (size_t i = 0; i < NUM_POWER_RAILS; i++) {
            telemetry->telemetry.rails[i] =
                telemetry->rail_controller->rails[i];
        }
    }

    // copy redundancy metrics if valid
    if (telemetry->redundancy_manager != NULL) {
        telemetry->telemetry.redundancy.health =
            telemetry->redundancy_manager->health;
        telemetry->telemetry.redundancy.total_faults_since_boot =
            telemetry->redundancy_manager->total_fault_count;

        uint32_t active_faults = 0;

        for (size_t i = 0; i < REDUNDANCY_MANAGER_MAX_FAULTS; i++) {
            if (telemetry->redundancy_manager->faults[i].active) {
                active_faults++;
            }
        }

        telemetry->telemetry.redundancy.active_fault_count = active_faults;

        uint32_t degraded_mask = 0;

        for (size_t i = 0; i < COMPONENT_COUNT; i++) {
            if (!telemetry->redundancy_manager->component_status[i]) {
                degraded_mask |= (1U << i);
            }
        }

        telemetry->telemetry.redundancy.degraded_components = degraded_mask;
        telemetry->telemetry.redundancy.timestamp_ms = hal_time_get_ms();
    }

    // copy uart stats if valid
    if (telemetry->usart1_events != NULL) {
        telemetry->telemetry.uart1.rx_byte_count =
            telemetry->usart1_events->rx_byte_count;
        telemetry->telemetry.uart1.rx_packet_count =
            telemetry->usart1_events->rx_packet_count;
        telemetry->telemetry.uart1.rx_crc_error_count =
            telemetry->usart1_events->rx_crc_error_count;
        telemetry->telemetry.uart1.initialized =
            telemetry->usart1_events->initialized;
    }

    if (telemetry->usart3_events != NULL) {
        telemetry->telemetry.uart3.rx_byte_count =
            telemetry->usart3_events->rx_byte_count;
        telemetry->telemetry.uart3.rx_packet_count =
            telemetry->usart3_events->rx_packet_count;
        telemetry->telemetry.uart3.rx_crc_error_count =
            telemetry->usart3_events->rx_crc_error_count;
        telemetry->telemetry.uart3.initialized =
            telemetry->usart3_events->initialized;
    }

    // copy can stats if valid
    if (telemetry->can1_events != NULL) {
        telemetry->telemetry.can1.rx_byte_count =
            telemetry->can1_events->rx_byte_count;
        telemetry->telemetry.can1.rx_packet_count =
            telemetry->can1_events->rx_packet_count;
        telemetry->telemetry.can1.rx_crc_error_count =
            telemetry->can1_events->rx_crc_error_count;
        telemetry->telemetry.can1.initialized =
            telemetry->can1_events->initialized;
    }

    if (telemetry->can2_events != NULL) {
        telemetry->telemetry.can2.rx_byte_count =
            telemetry->can2_events->rx_byte_count;
        telemetry->telemetry.can2.rx_packet_count =
            telemetry->can2_events->rx_packet_count;
        telemetry->telemetry.can2.rx_crc_error_count =
            telemetry->can2_events->rx_crc_error_count;
        telemetry->telemetry.can2.initialized =
            telemetry->can2_events->initialized;
    }

    // Read 5V regulator voltage (ADC_CHANNEL_8)
    uint16_t raw_5v = hal_adc_read(ADC_CHANNEL_8);
    telemetry->telemetry.v_reg_5v = (raw_5v / 4095.0f) * 6.6f;

    // Read 3.3V internal reference / regulator voltage (ADC_CHANNEL_9)
    uint16_t raw_3v3 = hal_adc_read(ADC_CHANNEL_9);
    telemetry->telemetry.v_reg_3v3 = (raw_3v3 / 4095.0f) * 4.0f;
}

eps_telemetry_t telemetry_get_all(telemetry_t *telemetry) {
    eps_telemetry_t empty_tlm = {0};

    if (telemetry == NULL) {
        return empty_tlm;
    }

    return telemetry->telemetry;
}

static void telemetry_handle_tick(const osusat_event_t *e, void *ctx) {
    (void)e;

    telemetry_t *telemetry = (telemetry_t *)ctx;

    if (telemetry == NULL || !telemetry->initialized) {
        return;
    }

    telemetry->tick_counter++;

    if (telemetry->tick_counter >= TELEMETRY_UPDATE_INTERVAL_TICKS) {
        telemetry->tick_counter = 0;
        telemetry_update(telemetry);

        LOG_INFO(EPS_COMPONENT_ADC,
                 "Telemetry snapshot: Battery V: %.2fV, I: %.2fA, Health: %d",
                 telemetry->telemetry.battery.voltage,
                 telemetry->telemetry.battery.current,
                 telemetry->telemetry.redundancy.health);

        LOG_INFO(EPS_COMPONENT_ADC,
                 "Rails 0-3 I: OBC=%.2fA, Radio=%.2fA, GPS=%.2fA, Pld1=%.2fA",
                 telemetry->telemetry.rails[RAIL_OBC].current,
                 telemetry->telemetry.rails[RAIL_RADIO].current,
                 telemetry->telemetry.rails[RAIL_GPS].current,
                 telemetry->telemetry.rails[RAIL_PAYLOAD_1].current);

        LOG_INFO(EPS_COMPONENT_ADC,
                 "Rails 4-7 I: Pld2=%.2fA, 5V=%.2fA, 3V3=%.2fA, Aux=%.2fA",
                 telemetry->telemetry.rails[RAIL_PAYLOAD_2].current,
                 telemetry->telemetry.rails[RAIL_5V_BUS].current,
                 telemetry->telemetry.rails[RAIL_3V3_BUS].current,
                 telemetry->telemetry.rails[RAIL_AUX].current);

        LOG_INFO(EPS_COMPONENT_ADC,
                 "Regs/MPPT V: 5V=%.2fV, 3V3=%.2fV, MPPT=%.2fV",
                 telemetry->telemetry.v_reg_5v,
                 telemetry->telemetry.v_reg_3v3,
                 telemetry->telemetry.mppt_channels[0].input_voltage);

        osusat_event_bus_publish(APP_EVENT_REQUEST_LOGGING_FLUSH_LOGS, NULL, 0);
    }
};
