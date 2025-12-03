/**
 * @file test_harness.h
 * @brief Hardware-in-the-loop test harness for EPS firmware
 */

#ifndef TEST_HARNESS_H
#define TEST_HARNESS_H

#include <stdint.h>

/**
 * @brief Run the main HITL test harness loop
 *
 * This function provides an interactive menu for testing various
 * firmware scenarios with mocked hardware. It never returns.
 */
void test_harness_run(void);

/**
 * @brief Simulate battery discharge scenario
 */
void simulate_battery_discharge_scenario(void);

/**
 * @brief Simulate solar panel activity throughout a day/night cycle
 */
void simulate_solar_panel_activity(void);

/**
 * @brief Simulate external interrupt from sensor
 */
void simulate_external_interrupt(void);

/**
 * @brief Simulate incoming UART commands
 */
void simulate_uart_commands(void);

/**
 * @brief Simulate critical power scenario
 */
void simulate_critical_power_scenario(void);

/**
 * @brief Simulate normal operations
 */
void simulate_normal_operations(void);

/**
 * @brief Run interactive test menu
 */
void run_interactive_menu(void);

/**
 * @brief Execute one iteration of background firmware tasks
 */
void run_background_tasks(void);

#endif // TEST_HARNESS_H
