# Project kickoff

# EPS Subsystem

Date Aug 24, 2025

# Background and context

The EPS subsystem is responsible for battery management and power delivery throughout the system, as well as offering a high level of protection against over/under voltage and current events. The EPS also needs to ensure a stable level of charge in the batteries when possible, along with allowing easily toggleable power rails via software commands to a microcontroller. Due to the critical nature of the EPS, redundancy, robustness, and reliability are key design considerations to ensure the system maintains a consistent power level.

# EPS Responsibilities

1. Takes electrical input from up to 6 solar faces  
2. Charges the batteries using solar input or a USB-C passthrough for testing  
3. Exposes general 3.3v and 5v power rails  
   1. May expose subsystem-specific power rails as branches of the general rails for more granular control  
4. Charges up to 4 18650 battery cells  
5. Each power rail should have a software-controllable load switch that can be toggled via software commands  
6. Maintain a high level of reliability using hardware protections like watchdog timers, fuses, TVS and ESD protections, and robust current monitoring

# Scope and deliverables

| Deliverable | Description |
| :---- | :---- |
| Solar face cable connection design | Design for the physical connections from solar face \-\> EPS card |
| EPS block diagram | Block diagram for the EPS hardware |
| EPS hardware schematic | Schematic for the EPS PCB |
| EPS PCB layout and manufacturing | Layout and manufacturing for the EPS PCB |
| EPS firmware design and block diagram | Preliminary design for the EPS control firmware |
| EPS firmware implementation | Implementation of the EPS firmware to control power rails and overall system power delivery |
| EPS hardware & firmware testing (throughout project) | Continuous testing plan and harness for testing EPS functionality |

# Resources

- [https://github.com/rgw3d/1KCubeSat\_Hardware/tree/master/eps\_board](https://github.com/rgw3d/1KCubeSat_Hardware/tree/master/eps_board)   
- [https://github.com/rgw3d/1KCubeSat\_Software/blob/main/eps/src/main.rs](https://github.com/rgw3d/1KCubeSat_Software/blob/main/eps/src/main.rs)   
- [https://github.com/oresat/oresat-batteries](https://github.com/oresat/oresat-batteries)   
- [https://codeberg.org/buildacubesat-project/bac-hardware](https://codeberg.org/buildacubesat-project/bac-hardware) 

# Next steps

- [ ] Design EPS hardware and schematics

- [ ] Design EPS firmware controller

- [ ] Implement EPS firmware functionality

- [ ] Test framework and planning

- [ ] Integration

# EPS boot flow

# Pre-Integration

1. EPS inhibit jumper is present, rendering the satellite electrically inert  
2. OPS inhibit jumper is present, preventing OBC functionality even if power is enabled

# Deployment Activation

1. EPS jumper is removed  
   1. Enables solar input and battery connection  
   2. EPS begins internal power sequence but keeps rails turned off  
2. EPS self-check  
   1. Internal regulators stabilize VBUS  
   2. Watchdogs and current monitors confirm statuses  
   3. EPS MCU comes online  
      1. Software activated rails are off  
3. OBC inhibit jumper is removed  
   1. The OBC is allowed to draw power from the general \+3.3v rail  
   2. Only the core rails are enabled at this time  
      1. Software rails are still turned off  
4. OBC procedurally enables software rails  
   1. OBC after performing general checks and booting will continuously send commands to the EPS to enable software power rails  
   2. EPS verifies current health before rails are latched on  
5. Nominal operations  
   1. EPS continuously manages power generation, battery charging, and power distribution  
   2. EPS handles fault events by triggering load switches  
      1. OBC should pick up on these events inadvertently given a failure to respond to heartbeat packets
