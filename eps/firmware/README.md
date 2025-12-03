# eps-firmware

This directory contains the firmware for the EPS board.

# Setup & Building

## Compilation Dependencies

- arm-none-eabi toolchain

## Submodule Initialization

```
git submodule update --init --recursive
```

## Building

```
mkdir build
cd build
cmake -DCMAKE_TOOLCHAIN_FILE=../arm-none-eabi-toolchain.cmake ..
# or: cmake -DCMAKE_TOOLCHAIN_FILE=../arm-none-eabi-toolchain.cmake -DBUILD_HITL=ON ..
make
```

# Firmware Scope & Component Specifications

- For general firmware philosophy, please refer to [the HITL document](../../documentation/design_guidelines/hitl.md)

## Application Layer Modules

### Command Handler

- The command handler will be responsible for responding to commands over UART from other subsystems
- Interacts with the UART events interface service to listen for incoming packets
- Case handler that calls out to other services based on the requested action
- Interacts directly with OSUSat messaging packet structures passed from the UART listener service

### Power Policies

- The high-level decision maker for the EPS
- Rules/logic for system/satellite-wide power sequencing & transitions
- State machine managing:
    - Startup/boot
    - Safe mode
    - Nominal operation
    - Charging mode
    - Fault/recovery
    - Shutdown/inhibits
- Defines and transitions between EPS states based on system conditions like voltage, current, and other subsystem commands
- Controls which power rails come up and in what order to enable stable startup
- Stop battery charging/discharging if limits (voltage, temperature, current) are exceeded
- Respond to OBC commands and requests like:
    - Enabling power rails
    - Entering safe mode
    - Restarts/telemetry
- Detect and handle critical faults and transition to safe states
    - Many faults can be detected using teh `redundancy_manager` service
- Watches system health, pets watchdogs, and responds to OBC heartbeat requests

## Service Layer Modules

### Battery Management

- Uses HAL drivers to read sensors (voltage/current monitors, charger status)
- Provides interfaces to the application layer to:
    - Fetch battery voltage
    - Start charging
    - Stop charging
- Cooperates with MPPT and rail controller services to balance input and power draw
- Reads, collects, and packages voltage, current, temperature, and other sensors for use by the power policy application
- Estimate remaining battery capacity (state of charge)
- Enable/disable charging circuitry using HAL GPIO drivers
- Manages battery balancing circuitry
- Enforces protection thresholds by disabling rails if overcurrents, overvoltages, or temperature ratings are too high
- Also exposes general telemetry for the power policy application
- Notifies redundancy manager about sensor or charge failures

### MPPT Controller

- Configures MPPT chip parameters if necessary using HAL drivers
- Enables/disables MPPT IC, monitors status pins using GPIO HAL interface
- Reads and packages telemetry from the chip

### Rail Controller

- Abstraction for load switches and current monitors on each power rail
- Provides interfaces to enable/disable power rails and read their status
- Monitors for overcurrent events and automatically disables rails if necessary
- Notifies the redundancy manager of any rail faults

### Redundancy Manager

- Acts as a centralized fault detection and response service
- Aggregates health statuses from other services (battery, MPPT, rails)
- Maintains a system wide health status
- Can trigger a transition to safe mode via the power policies application if a critical fault is detected
- Manages redundant hardware components (switching between sensors upon failures)
- Individual services detect their own faults, but the redundancy manager determines the system level response
- Simplifies complex faults into higher-level faults to make decision making in the application clearer

### Telemetry

- Gathers and packages data from all other services
- Provides a unified interface for the application layer to access all system telemetry
- Formats data into packets for transmission (using OSUSat messaging)

### UART Events

- Manages communication with other subsystems via the UART bus
- Uses the messaging library to encode and decode packets
- Used by the command handler application to detect and process new commands
    - This service is polled in the application layer. The interface allows the user to check if packets are available, and to fetch them
- Provides an interface for other services to send and receive packets

### Watchdog

- Manages the hardware watchdog timer
- Provides an interface for the application layer to pet the watchdog
- If the watchdog is not petted within a certain time frame, it will trigger a system reset
- This is a critical safety feature to prevent the system from getting stuck in a bad state
