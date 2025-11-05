# Hardware-In-The-Loop Testing (HITL)

# Overview

Hardware in-the-loop testing (HITL) is a technique used for testing embedded firmware by running real hardware and firmware components against simulated system components. For example, the firmware could be run on the actual microcontroller with some hardware components, but most of the system is simulated.

# Philosophy
- Testable, version-controlled, automatable, while maintaining realism to the actual hardware.

## Key Principles
- Isolate Logic/Function From Hardware: abstract hardware access behind interfaces (e.g., a load_switch interface with methods like enable and disable with multiple implementations, both in simulation and hardware).
- Test Early: firmware development can happen before hardware is readily available. When using simulated components, it’s possible to skeleton the code without access to real hardware.
- Automate Testing: flashing, test runs, data collection, and power cycling should all be scriptable and implementable by CI/CD.
- Determinism: keep hardware and firmware configurations similar to ensure reproducibility.

# Our Test Harness Architecture

- RPI-based test controller
- MCU or board directly wired to Pi GPIO and power pins
- The Pi is responsible for automatically:
   - Power cycling the hardware
   - Flashing firmware (SWD or USB)
   - Collecting and logging serial outputs
   - Analyzing results
- Optionally, the Pi may also:
    - Connect GPIO and/or I2C, UART, SPI pins to hardware to stimulate signals
        - Test with the real hardware layer enabled in firmware, but hardware signals are emulated by the Pi

# Firmware Architecture

- Tiered design:
    1. Application layer
        a. State machines, main loops, broad mission logic
        b. Glues services and hardware calls together
    2. Service layer
        a. Core system functionality, independent of hardware access
        b. State machines, PID, communication protocols, safety checks
    3. HAL (hardware abstraction layer)/drivers
        a. Each hardware peripheral gets a separate driver module
        b. Provide real & mock implementations

