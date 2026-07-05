# Subsystem Info

# EPS Subsystem

Date July 3, 2026

# Background and context

The EPS subsystem is responsible for battery management and power delivery throughout the system, as well as offering a high level of protection against over/under voltage and current events. The EPS also needs to ensure a stable level of charge in the batteries when possible, along with allowing easily toggleable power rails via software commands to and from a microcontroller. Due to the critical nature of the EPS, redundancy, robustness, and reliability are key design considerations to ensure the system maintains a consistent power state.

# EPS Responsibilities

1. Takes electrical power from up to 6 solar faces  
2. Charges the batteries using solar input + MPPT or a USB-C passthrough for bench work  
3. Exposes general 3.3v and 5v power rails  
   1. Expose subsystem and circuit-specific power rails as branches of the system rails for more granular control  
4. Charges up to 4 18650 battery cells configured in 2 independent packs  
5. Each power rail should have a software-controllable load switch that can be toggled via software commands
6. Each power rail must have fast-response hardware-level current monitoring & load switch control
7. Maintain a high level of reliability using hardware protections like watchdog timers, fuses, TVS and ESD protections, and robust current monitoring

# Scope and deliverables

| Deliverable | Description |
| :---- | :---- |
| Solar face cable connection design | Design for the physical connections from solar face \-\> EPS board |
| EPS block diagram | Block diagram for the EPS hardware |
| EPS hardware schematic | Schematic for the EPS PCB |
| EPS PCB layout and manufacturing | Layout and manufacturing for the EPS PCB |
| EPS firmware design and block diagram | Preliminary design for the EPS control firmware |
| EPS firmware implementation | Implementation of the EPS firmware to control power rails and overall system power delivery |
| EPS hardware & firmware testing (throughout project) | Continuous testing plan and harness for testing EPS functionality |

# Resources

- [https://www.cubesat.org/cubesatinfo](https://www.cubesat.org/cubesatinfo)
- [https://github.com/rgw3d/1KCubeSat\_Hardware/tree/master/eps\_board](https://github.com/rgw3d/1KCubeSat_Hardware/tree/master/eps_board)
- [https://github.com/rgw3d/1KCubeSat\_Software/blob/main/eps/src/main.rs](https://github.com/rgw3d/1KCubeSat_Software/blob/main/eps/src/main.rs)
- [https://github.com/oresat/oresat-batteries](https://github.com/oresat/oresat-batteries)
- [https://codeberg.org/buildacubesat-project/bac-hardware](https://codeberg.org/buildacubesat-project/bac-hardware)

# Next steps (v1r1)

- [X] Design EPS hardware and schematics

- [X] Design EPS firmware controller

- [X] Implement EPS firmware functionality

- [X] Test framework and planning

- [X] Integration

# Next steps (v2r1)

- [ ] Account for all v1r1 bugs and justify all proposed changes

- [ ] Design EPS v2 schematic

- [ ] Design EPS v2 PCB layout

- [ ] Implement EPS firmware functionality

- [ ] Integration

- [ ] Continuous testing (ongoing)

# EPS boot flow

# Pre-Integration

1. EPS inhibit jumper is present, rendering the satellite electrically inert
2. EPS deployment switch is actuated, rendering the satellite electrically inert [^1]
3. OPS inhibit jumper is present, preventing OBC power rail from flowing to downstream loads even if power is enabled

# Deployment Activation

1. EPS jumper is removed & deployment switch is deactivated
   1. Enables solar input and battery connection
   2. EPS begins internal power sequence but keeps rails turned off
2. EPS self-check
   1. Internal regulators stabilize VBUS
   3. EPS MCU comes online
      1. Software activated rails are off, besides OBC
3. OBC inhibit jumper is removed
   1. The OBC is allowed to draw power from OBC power rail
   2. Only the core rails are enabled at this time
      1. Non-required software rails are still turned off
4. OBC procedurally enables software rails
   1. OBC after performing general checks and booting will continuously send commands to the EPS to enable software power rails
   2. EPS verifies system health before rails are latched on
5. Nominal operations
   1. EPS continuously manages power generation, battery charging, and power distribution  
   2. EPS handles fault events by triggering load switches  
      1. OBC should pick up on these events inadvertently given a failure to respond to heartbeat packets

# References

[^1]: The Cubesat Program, Cal Poly (2022). *CubeSat Design Specification Rev. 14.1*, section 2.3.2
