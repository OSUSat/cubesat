# Preliminary Hardware Design Guidelines

## Document Control

- Version: 1
- Author: Ethan Eggert
- Approver: 
- Last Revised date: 07/5/2026
- Approval Status: Unapproved


## Overview

These guidelines outline the process and requirements for ALL OSUSat hardware design before CAD tools like KiCad are ever touched. All preliminary design information should primarily live in a `subsystem_info.md` document like [eps/documentation/subsystem_info.md](../../eps/documentation/subsystem_info.md), where system requirements, procedures, deliverables, and other important information about the design of the subsystem are listed.

## Subsystem Info Contents & Considerations

### Mission & System Context

- The subsystem's purpose and required interfaces (e.g., power, data, mechanical) should all be clearly documented and defined
    - Also list the requirements of each interface, like voltage requirements, current limits, connectors, pinouts, logic levels, and more
- Identify and document mission phases (e.g., integration, launch, deployment, nominal operation, fault)
    - Furthermore, identify the effect these phases have on the electrical state of the subsystem (current draw, power sequencing)
- Identify and document PCB failure modes, mitigations, and solutions
    - Emphasis on external connections

### Functional -> Concrete Electrical Requirements

- Once the system's functional requirements are defined, those requirements must be translated into measurable electrical requirements. List:
    - Supply voltage ranges & nominal voltage
    - Peak and average current draw
        - Consider how vias affect current draw
    - Worst-case power budgets
    - Sensor ranges, ADC requirements, signal speeds
    - Temperature, vibration, radiation, EMC, EMI, ESD

### Deliverables

- In most cases, the following deliverables should be present:
    - Failure mode identification documents
    - Block diagram
    - Schematic
    - Layout
    - Manufacturing files
        - Includes gerbers, assembly files, schematic, documentation
    - Firmware block diagram (if required)
    - Firmware implementation (if required)
    - Integration and testing documentation

### CubeSat Design Specification Compliance

- Evaluate functional, mechanical & electrical requirements against the [CubeSat Design Standard](../spec/CDS+REV14_1+2022-02-09.pdf)
    - **Envelope & mass** - confirm the subsystem's footprint and mass fit within the allocated U-slice of the bus mass budget (1U ≤ 2 kg, 3U ≤ 6 kg, 6U ≤ 12 kg, 12U ≤ 24 kg) [^1]
        - No components may protrude more than 6.5 mm normal to the rail plane on the constrained (yellow-shaded) faces
        - Deployables must be constrained by the CubeSat itself, not the dispenser
    - **Rails** - if the subsystem board contacts or mounts near the rail system, confirm compatibility with:
        - Minimum rail width of 8.5 mm, surface roughness < 1.6 µm, edges rounded to > 1 mm radius
        - At least 75% rail contact with the dispenser rails
        - Hard-anodized aluminum finish on any rail/standoff surfaces to prevent cold welding
    - **RBF (Remove Before Flight) pin** - the subsystem's power architecture must support a hard power cutoff via RBF pin
        - RBF pin shall cut all power to the satellite when inserted, and protrude no more than 6 mm from the rail surface when fully inserted
        - Determine whether the RBF is accessible via a dispenser access port, or must be removed prior to integration (affects power-on/boot sequence design)
    - **Deployment switches** - identify which rail-mounted deployment switches turn off this subsystem's power rails, and document the inhibit logic (typically 2+ independent inhibits between battery and any deployable/RF/actuation circuit)
    - **RF inhibit** - if the subsystem includes a radio, document the RF inhibit mechanism and timer that prevents transmission until the launch-provider-specified post-deployment delay has elapsed
    - **Real-time clock circuits** (if used) - must be isolated from the main power system, current-limited to < 10 mA, and operate below 320 kHz
    - **Center of gravity** - flag any subsystem placement decisions that could push the overall bus CG outside the allowed offset from geometric center (guideline: within 2 cm per axis)
    - **Materials & outgassing** - restrict material selection to low-outgassing per NASA-STD outgassing limits (TML < 1%, CVCM < 0.1%); avoid pure tin finishes (tin whisker risk)
        - Best surface finish choices are Leaded HASL or ENIG
    - **Testing philosophy** - identify which CDS-mandated environmental tests apply to this subsystem's design (random vibration, thermal vacuum bakeout); shock testing is typically not required for CubeSats per CDS Rev 14.1 §3.3.1.1 [^1]
    - Document any planned deviations from the CDS as a formal deviation request, including justification and risk assessment, for review with the Launch Provider/Mission Integrator

### Component Derating

- When selecting components (e.g., resistors, capacitors), follow derating guidelines outlined in the [NASA EEE Part Selection guidelines](../spec/EEE-INST-002_add1.pdf) [^2].
- Per the [NASA GSFC CubeSat Success Handbook](../spec/gsfc-hdbk-8007.pdf), if resources for radiation hardened parts aren't available, parts should be derated below EEE-INST-002 levels [^1], [^2].

### Part Feasibility Studies

- The system info document should also have a section for key part tradeoffs, documenting pros/cons of each part, as well as the general decision-making process and result for each part
- Some example tradeoffs:
    - MCU/Compute - compare reliability mechanisms, radiation tolerance, current draw, temperature range, peripherals
    - Communication interfaces - CAN, UART, I2C, SPI
    - Power architecture - noise tolerance, rail pinout/requirements, regulator efficiencies
    - Packaging - mechanical constraints, positioning within the bus
    - Radiation mitigation & redundancy - decide which parts need redundancy, and consider watchdogs, ECC memory
- Always select parts compliant with [NASA EEE (Electrical, Electronic, Electromechanical)](../spec/EEE-INST-002_add1.pdf) guidelines [^2]
    - This means matching temperature coefficients, failure rates, derating, etc.

## References

[^1]: The Cubesat Program, Cal Poly (2022). *CubeSat Design Specification Rev. 14.1*
[^2]: NASA (2003). EEE-INST-002: Instructions for EEE Parts Selection, Screening, Qualification,
and Derating 

