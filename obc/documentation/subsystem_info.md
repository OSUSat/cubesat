# Subsystem Info

# OBC Subsystem

## Document Control

- Version: 1
- Author: Ethan Eggert
- Approver: 
- Last Revised date: 07/5/2026
- Approval Status: Unapproved

# Background and context

The on-board computer (OBC) subsystem handles main system control, managing the state of the CubeSat, receiving commands from the ground station, and executing known procedures. An MCU on the OBC receives and transmits commands from the other subsystems over a bidirectional communication protocol (CANBus). When in range of ground stations, it can receive commands to execute functions on the satellite. When out of range, it maintains a set of known procedures to execute mission functionality. The OBC also maintains an internal state machine for handling happy-path states and flowing through desired functionality, and for handling errors or latchups and correcting them accordingly.

# OBC Responsibilities

1. Performs health checks on boot
    1. Timeout/retry functionality to allow boards of different varieties time to arm themselves
    2. Easy configuration
2. Runs a mission-controller state machine that handles transitions and actions between IDLE -> ARMED -> RECORD -> DOWNLINK, etc.
    1. Handles alarm states and performs corrective actions accordingly
    2. Accepts commands from the ground station that can override the state machine
3. Triggers sensor recording and collection by sending commands to other subsystem MCUs
    1. Handles telemetry, like battery levels and temperature, from the EPS
    2. Saves log data to external storage
    3. Implements ACK/NACK and other handshake behavior to ensure robust communication between subsystems
4. Controls the payload module by sending commands to the payload board processor to trigger camera actions and data collection
5. Uses the UHF communications subsystem to transmit data
    1. Handles data compression
    2. Maintains a queue for downlink so no messages are dropped or unprocessed
    3. Uses the OSUSat packet standard for ground station communication
6. Responds to resets from an external watchdog that handles latchup events
7. Maintains a software watchdog system that can handle alarm states on other boards or systems
8. Handles transitions between normal operations and safe mode, where some modules are shut down to conserve power
9. Manages data storage
    1. Tags images with additional information like location and timestamp
    2. Maintains extensive logging
10. Exposes a "maintenance mode" accessible over radio by the ground station
    1. Enables remote debugging from the ground (functionality TBD)
    2. Starts a watchdog that requires petting from the ground station at a configurable interval; if not petted within the time limit, the system boots to safe mode automatically and attempts recovery

# OBC Architecture

- Lower power MCU with enough peripherals to allow for multi-protocol and multi-device communication
    - May also carry a high-power co-processor for compute intensive tasks
- Communicates with other subsystems over the inter-board-connector (IBC)
- Has an inhibit that prevents system operation in a down state or when not integrated
- When the inhibit is removed, the device boots up and follows a series of steps:
    1. Wait for specified health probe timeouts for each subsystem
    2. Send health-check queries to other subsystems, retrying up to the max retry count on failure
    3. If a system fails a health check, boot directly to safe mode and enter an alarm state
        1. In safe mode, continuously query and try to cycle failing subsystems until the system is in a healthy state
    4. Once in a healthy state, arm and start the state machine that performs system functionality
    5. Repeatedly transition between IDLE -> ARMED -> RECORD -> DOWNLINK, etc.
    6. Override normal system functionality when a command arrives from the ground station
    7. Continuously monitor for latchups or errors

# Scope and deliverables


| Deliverable | Description |
| :---- | :---- |
| OBC Hardware | The hardware design and manufacturing of the OBC board. Should contain a suitable MCU, IBC, non-volatile storage, hardware watchdogs, inhibits, and other safety features. |
| Health-check functionality | Software functionality to configure, boot, and run health checks on OBC startup |
| Safe mode implementation | Software functionality that allows the OBC to transition to a safe mode where some system features are disabled and monitored for health over time |
| Barebones state machine | Basic software functionality to run through the system state machine; doesn't need to execute command functionality yet |
| Ground station overrides | Software functionality that allows state machine overrides via ground station commands |
| Command implementations | Software functionality that allows the state machine to actually execute actions on other system boards |
| Test harness and software tests (throughout project) | A testing framework that allows OBC testing to occur without the other actual subsystems, by mocking subsystems in software so the OBC can run in a dummy mode as if they were present |
| Software dependency injection framework (throughout project) | A DI framework that allows rapid creation and swapping of components, so that if a communications or payload board's architecture or pinout changes, swappable components can be loaded at boot time |

# Resources

- https://github.com/oresat/oresat-c3-hardware

# Next steps (v1r1)

- [X] Design OBC block diagram
- [X] Select hardware components
- [X] Design and manufacture hardware
- [X] Scope software and create state machine flowchart
- [ ] Create testable framework without hardware while it's being designed
- [X] Implement and test software functionality
- [X] Test on real hardware
- [X] Integrate with system

# OBC Messaging Standard

- Packet based
- Messages are framed consistently, with headers, checksums, and ACK/NACK support
- Backwards compatibility is important, to avoid breaking old commands
- Shared functionality for parsing and serializing between subsystem codebases


| Field | Size (bytes) | Notes |
| :---- | :---- | :---- |
| Start Byte | 1 | Tentatively 0x7E. Radio protocols like AX.25 use 0x7E as a start byte. |
| Version # | 1 | Messaging standard version number |
| Destination | 1 | Target destination enum. 0x01 for EPS, 0x02 for payload, etc. |
| Source | 1 | Same enum, corresponds to who sent the packet |
| Message Type | 1 | Enum for message type. 0x01 is command, 0x02 telemetry, etc. |
| Command ID | 1 | Enum for command ID. Subsystem-specific command tables can differ; a common commands table should be maintained with a few commands every subsystem uses, and a specified offset at which subsystem-specific commands begin. |
| Sequence | 1 | Packet sequence number, used for multi-packet payloads |
| Last Chunk Flag | 1 | Indicates whether this packet marks the end of the packet sequence |
| Length | 1 | Packet payload length in bytes |
| Payload | N | Variable length payload |
| CRC | 2 | Cyclic redundancy check for error correction (CRC-16-CCITT) |


## Message Handshake

- Every command must be ACKed with a new packet using the ACK message type
- If an error or invalid state occurs, send back a new packet with the NACK/ERR message type, with the error message/state in the payload
- OBC maintains retry logic to retry messages until a successful ACK occurs or it reaches a specified threshold, at which point it enters safe mode

## Command Table Code Generation

- Command table and system configuration live in a YAML document
- A simple Python autogeneration script parses the YAML configuration and generates C header files and Python files containing the command table, accessible programmatically
- This way, commands are known at build time
