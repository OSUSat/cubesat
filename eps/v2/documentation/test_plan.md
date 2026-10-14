# EPS Test Plan

## Document Control

- Version: 1 (draft)
- Author: Griffin Ziemer
- Approver: 
- Last Revised date: 10/09/2026
- Approval Status: Unapproved
- Tracking issue: #101

## Overview

This plan defines how EPS functionality is verified throughout v2 development, from host unit tests that run on every pull request up to the acceptance test procedure (ATP) run on each built board. Each test traces back to a requirement in the [EPS subsystem info doc](../../documentation/subsystem_info.md).

v2 changes the MCU and other controllers, so v1's board-specific code (pin mapping, HAL drivers, ADC scaling) will be redone. The service layer (rail control, battery management, MPPT, power policies) talks to hardware only through the HAL and the event bus, so its logic and these L1 tests should carry over. On v2 the tests need new pin and rail values in `eps_config.h` and HAL mocks for the new MCU. Until v2 firmware exists, L1 tests are written against `eps/v1/firmware/`.

## Test Levels

| Level | Name | Runs on | When | Owner of pass/fail |
| :---- | :---- | :---- | :---- | :---- |
| L1 | Unit | Host (`-DTARGET_ARCH=HOST -DBUILD_HITL=ON`), HAL + event bus mocked | Every push/PR via CI (`.github/workflows/build_and_test.sh` runs `ctest`) | CI |
| L2 | Simulation | Host, `hitl/test_harness.c` scenarios against mocked HAL | Every PR via CI once scenarios run non-interactively (see Gaps) | CI |
| L3 | Hardware-in-the-loop | Real EPS MCU on a Raspberry Pi rig per [HITL guidelines](../../../documentation/design_guidelines/hitl.md) | Nightly / before merging firmware that touches the HAL | Pi rig |
| L4 | Acceptance | Each built v2 board on the bench, per [ATP template](../../../documentation/templates/atp.md) | Once per unit, after bring-up | Test conductor + witness |

## Requirements

IDs are assigned here for traceability. R1–R7 map to the numbered "EPS Responsibilities" list; B1–B5 map to the boot flow.

| ID | Requirement |
| :---- | :---- |
| R1 | Take power from up to 6 solar faces |
| R2 | Charge batteries from solar via MPPT, or USB-C passthrough for bench work |
| R3 | Expose 3.3 V and 5 V rails, with subsystem-specific branches |
| R4 | Charge up to 4 18650 cells in 2 independent packs |
| R5 | Every rail has a software-controllable load switch |
| R6 | Every rail has fast hardware-level current monitoring and load switch control |
| R7 | Hardware protections: watchdog, fuses, TVS/ESD, current monitoring |
| B1 | Inhibit jumper / deployment switch render the satellite electrically inert |
| B2 | After deployment, EPS boots with all software rails off except OBC |
| B3 | EPS self-check passes before rails are latched on |
| B4 | EPS enables software rails on OBC command after verifying system health |
| B5 | In nominal operation, EPS responds to faults by triggering load switches |

## Test Cases

Status key: **Exists** (ctest target in `eps/v1/firmware/tests/`), **To write**, **Blocked** (firmware or hardware not implemented yet; reason given).

### Rail control and rail faults (R3, R5, R6, B5)

These tests answer the question on #101 about which automated tests cover rail faults.

| ID | Level | Test | Pass criteria | Status |
| :---- | :---- | :---- | :---- | :---- |
| RC-01 | L1 | Init drives every rail off | Starting with all load-switch pins high (re-init), init leaves all rails `RAIL_STATUS_DISABLED` and every load-switch pin low | Exists (`test_rail_controller`) |
| RC-02 | L1 | Enable/disable request events drive the load switch | `APP_EVENT_REQUEST_RAIL_CONTROLLER_ENABLE_RAIL` sets GPIO `8 + rail` high and status `OK`; disable sets it low and `DISABLED` | Exists (`test_rail_controller`) |
| RC-03 | L1 | Out-of-range rail ID and bad payload length are ignored | No rail enabled, every load-switch pin stays low | Exists (`test_rail_controller`) |
| RC-04 | L1 | Overcurrent on an enabled rail | ADC reading above `current_limit` → load-switch pin driven low, status `OVERCURRENT`, then `RAIL_CONTROLLER_EVENT_OVERCURRENT_DETECTED` published with a `rail_t` snapshot; other rails untouched | Exists (`test_rail_controller`) |
| RC-05 | L1 | Fault reported once per trip | Holding overcurrent for several update cycles publishes exactly one event; re-enabling into the fault trips and reports again | Exists (`test_rail_controller`) |
| RC-06 | L1 | Current below the limit is not a fault | ADC reading below `current_limit` → status stays `OK`, no event | Exists (`test_rail_controller`) |
| RC-07 | L1 | Undervoltage / overvoltage on an enabled rail | Corresponding event and status | Blocked: rail voltage is hard-coded to nominal, no voltage sense |
| RC-08 | L1 | Disabled rails are never faulted | Overcurrent ADC reading on a disabled rail publishes nothing | Exists (`test_rail_controller`) |
| RC-09 | L1 | Rail telemetry published on interval | One `RAIL_CONTROLLER_EVENT_TELEMETRY` per rail every `TELEMETRY_INTERVAL_CYCLES` ticks | Exists (`test_rail_controller`) |
| RC-10 | L1 | Power profile switches set the right rails | Nominal / safe profiles enable and disable the expected rail sets | Exists (`test_power_profiles`) |
| RC-11 | L3 | Hardware overcurrent trip | Pi drives the current-monitor comparator output; EXTI fires and rail is shut off within the hardware response time | Blocked: needs v2 board + Pi rig |
| RC-12 | L4 | Load switch and current-limit trip points per rail | Each rail switches on/off by command; trips at its rated limit ± tolerance | Blocked: needs v2 board |
| RC-13 | L1 | Fault survives a full event queue | With the queue full, the rail still shuts off; the fault event is delivered exactly once on a later update | Exists (`test_rail_controller`) |

### Battery management (R2, R4, B3)

| ID | Level | Test | Pass criteria | Status |
| :---- | :---- | :---- | :---- | :---- |
| BM-01 | L1 | Init publishes self-check result | `BATTERY_EVENT_SELF_CHECK_PASSED` on healthy init | Exists (`test_battery_management`) |
| BM-02 | L1 | Critical low voltage | Voltage below `CRITICAL_BATTERY_VOLTAGE_THRESHOLD` → `BATTERY_EVENT_CRITICAL_LOW` | Exists (`test_battery_management`) |
| BM-03 | L1 | Self-check failure | Out-of-range voltage/temperature at init → `BATTERY_EVENT_SELF_CHECK_FAILED` with a specific failure code | Blocked: `battery_run_diagnostics()` always returns true |
| BM-04 | L1 | Fully charged | Voltage at full charge → `BATTERY_EVENT_FULLY_CHARGED` | Blocked: event is never published |
| BM-05 | L1 | Charge enable/disable and protect mode | `battery_charge_control` / `battery_protect_mode` publish `BATTERY_EVENT_CHARGING_CHANGE` and set protection flag | To write |
| BM-06 | L4 | Independent pack charging | Each pack charges with the other absent or disabled (see #160) | Blocked: needs v2 board |

### Solar / MPPT (R1, R2)

| ID | Level | Test | Pass criteria | Status |
| :---- | :---- | :---- | :---- | :---- |
| MP-01 | L1 | Enable/disable channel requests | Request events reach `mppt_enable` / `mppt_disable`; out-of-range channel ignored | To write (hardware action is still a stub) |
| MP-02 | L1 | Fault on a channel | Channel fault → `MPPT_EVENT_FAULT_DETECTED` with channel ID in payload | Blocked: event is never published |
| MP-03 | L1 | Telemetry for every channel | One `MPPT_EVENT_TELEMETRY` per enabled channel per interval | To write (only channel 0 is populated today) |
| MP-04 | L4 | MPPT input from each solar face connector | Each face connector delivers charge current under a bench supply | Blocked: needs v2 board |

### Power policies (B4, B5)

| ID | Level | Test | Pass criteria | Status |
| :---- | :---- | :---- | :---- | :---- |
| PP-01 | L1 | Critical battery → safe mode | `BATTERY_EVENT_CRITICAL_LOW` → `APP_EVENT_REQUEST_POWER_PROFILE_SAFE` | Exists (`test_power_policies`) |
| PP-02 | L1 | Fully charged → nominal | `BATTERY_EVENT_FULLY_CHARGED` → `APP_EVENT_REQUEST_POWER_PROFILE_NOMINAL` | Exists (`test_power_policies`); end to end still blocked by BM-04, nothing publishes the event |
| PP-03 | L1 | MPPT fault disables that channel | `MPPT_EVENT_FAULT_DETECTED` with channel N → `APP_EVENT_REQUEST_MPPT_DISABLE_CHANNEL` with N; empty payload ignored | Exists (`test_power_policies`) |
| PP-04 | L1 | Faulted MPPT channel is re-enabled after a backoff | Re-enable request published after the backoff period | Blocked: re-enable not implemented |
| PP-05 | L2 | Critical power scenario end to end | `simulate_critical_power_scenario` ends in safe profile with non-essential rails off | Blocked: harness is interactive only |

### Fault handling and redundancy (R7, B5)

| ID | Level | Test | Pass criteria | Status |
| :---- | :---- | :---- | :---- | :---- |
| RD-01 | L1 | Rail fault is recorded by the redundancy manager | Rail overcurrent event → fault logged with `FAULT_SOURCE_RAIL`, health degraded | Blocked: redundancy manager does not subscribe to rail/MPPT/UART faults |
| RD-02 | L1 | Health recovers when faults clear | `REDUNDANCY_EVENT_HEALTH_RECOVERED` published | To write |
| WD-01 | L1 | Watchdog init, pet, disabled | Existing behavior | Exists (`test_watchdog`) |
| WD-02 | L3 | Watchdog resets a hung MCU | Pi halts firmware; board resets within the watchdog window | Blocked: needs Pi rig |

### Communication (B4)

| ID | Level | Test | Pass criteria | Status |
| :---- | :---- | :---- | :---- | :---- |
| CM-01 | L1 | CAN events send/receive | Existing behavior | Exists (`test_can_events`) |
| CM-02 | L1 | UART events deliver packets to subscribers | Received packet is published on the event bus | To write |
| CM-03 | L1 | OBC rail-enable command | Command packet → rail enabled only when system health is OK | Blocked: command handler is a stub; needs OBC messaging ICD (#167) |
| TL-01 | L1 | Telemetry aggregation | Existing behavior | Exists (`test_telemetry`) |
| LG-01 | L1 | Logging and flash backup | Existing behavior | Exists (`test_logging`) |

### Boot and inhibits (B1, B2)

| ID | Level | Test | Pass criteria | Status |
| :---- | :---- | :---- | :---- | :---- |
| BT-01 | L1 | Boot state | After all services init, only the OBC rail is enabled | To write |
| BT-02 | L4 | Inhibit jumper / deployment switch | With jumper present or switch actuated, no rail or MCU power on any output | Blocked: needs v2 board |

## Gaps Found While Writing This Plan

These are firmware issues the tests above expose. Items not marked fixed should each get their own issue.

1. **Fixed:** Rail fault did not turn off the load switch. `rail_controller_handle_update()` set `enabled = false` on a fault but never called `hal_gpio_write(..., LOW)`, so the hardware rail stayed on. The fault path now drives the load switch low before publishing the fault event (RC-04).
2. **Rail voltage is never measured.** Voltage is set to the configured nominal value, so undervoltage/overvoltage faults can never trigger (RC-07).
3. **`BATTERY_EVENT_FULLY_CHARGED` and `MPPT_EVENT_FAULT_DETECTED` are never published.** Power policies subscribes to both, so the return-to-nominal and MPPT fault paths are dead code.
4. **Nothing subscribes to rail fault events.** The redundancy manager's subscriptions are TODOs, so a rail fault is not reflected in system health.
5. **HITL scenarios are interactive only.** `hitl/test_harness.c` runs from a menu, so CI cannot run the L2 scenarios. They need a non-interactive entry point that exits non-zero on failure.
6. **Fixed:** Rail telemetry used the wrong event ID. `rail_controller_handle_tick()` published the bare enum code `RAIL_CONTROLLER_TELEMETRY` (0x15) instead of an ID built with `OSUSAT_BUILD_EVENT_ID(RAIL_CONTROLLER_SERVICE_ID, ...)`, so subscribers couldn't match it and it could collide with other services' codes. Added `RAIL_CONTROLLER_EVENT_TELEMETRY` and publish with it (RC-09).
7. **Fixed:** Rail fault payload didn't match its documentation. Fault events sent the rail index as a `size_t` (8 bytes on host, 4 on ARM) while the header documents a `rail_t` snapshot. They now publish the `rail_t` snapshot taken after shutoff (RC-04).
8. **Fixed:** Fault events could be dropped silently. The publish return value was ignored, and the event queue holds 16 entries; since a faulted rail is disabled and skipped by later updates, a dropped fault was lost for good. Faults are now kept pending per rail and retried on each update until published (RC-13).

## Out of Scope for Now

- L3 Pi rig hardware and scripts: wait for v2 boards.
- The v2 ATP document: written after v2 bring-up, per the [ATP guidelines](../../../documentation/design_guidelines/writing_atps.md).

## Traceability

| Requirement | Test cases |
| :---- | :---- |
| R1 | MP-01, MP-03, MP-04 |
| R2 | BM-04, MP-01–MP-04 |
| R3 | RC-02, RC-10, RC-12 |
| R4 | BM-01–BM-06 |
| R5 | RC-01–RC-03, RC-10, RC-12 |
| R6 | RC-04–RC-09, RC-11–RC-13 |
| R7 | RC-11, WD-01, WD-02, RD-01, RD-02 |
| B1 | BT-02 |
| B2 | BT-01 |
| B3 | BM-01, BM-03 |
| B4 | PP-01, PP-02, CM-02, CM-03 |
| B5 | RC-04, PP-03–PP-05, RD-01 |
