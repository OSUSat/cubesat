# Schematic & Layout Design Guidelines

## Document Control

- Version: 1
- Author: Ethan Eggert
- Approver: Nick Alves 
- Last Revised date: 07/5/2026
- Approval Status: Approved


## Overview

This document outlines general guidelines that must be followed when designing PCB schematics and layouts.

## Schematic Design

### Organization

- Use hierarchical sheets
    - Follow the block diagram/flowchart here. It lays out the logical sections for the system.
    - Each sheet should also have a title, version, and revision date
- Label nets clearly
    - Before moving to layout, make sure that all important nets are labelled
- Include text and images for critical notes like resistor selection, layout, etc.
- Include tolerances on passives like resistors
- Include breakdown voltage for capacitors
- Include saturation current for inductors
- Toggle on inter-sheet-references

### Power & Ground

- Include fuses, TVS diodes, reverse-polarity and current protection, filtering if necessary on external power inputs
- Decoupling capacitors must be included per component datasheets

### Signals

- Include pull-up/pull-down resistors on shared buses (I2C lines, reset/boot)
- For each key signal, include test pads/headers. At least:
    - CANBus TX/RX
    - UART TX/RX
    - RESET
    - SWD/JTAG
    - I2C
    - SPI
- Add level shifting for mixed-voltage domains (e.g., 3.3V MCU driving 5V sensors)

### Protections

- Use ESD protection on external connectors
- Consider current sense resistors on key rails/lines
    - Add ADCs to read and send data to MCU
- Use redundancy as necessary, as defined in the subsystem info document
- 0 ohm resistors/solder-bridges on key lines
- Utilize DNP (do-not-populate)
    - Especially use them in places that may be useful for the flight model but maybe not on the bench (e.g., slew rate control)

### Connectors

- When possible, use connectors with flight heritage or are part of a space-rated line

### Accessibility

- Add test points for the following:
    - Main system power rails
    - Key signals

### Inhibits

- Every subsystem design must include at least one inhibit mechanism that disables the hardware when inserted
    - Pin jumpers offer a good solution for most applications
    - Deployment switches are necessary for the EPS and may be used for other subsystems as well

### Readiness Review

- Ensure ERC is clean
    - Clear out errors AND warnings
- Peer & design review before moving to the next stage
- Simulate subcircuits (regulators, etc.) if time allows
- Review the schematic for all of the above near release

## PCB Layout

### Stackup

- Prefer 4-layer SGPS (signal-ground-power-signal) or SGGS (signal-ground-ground-signal) stackup
- Keep return paths short and continuous; never split the ground plane
- Match impedance for higher speed lines like USB, CAN, RF

### Component Placement

- First, place mechanically constrained components
- Then, group by function as necessary (power, RF, logic, analog)
- Place decoupling capacitors as close as possible to relevant ICs
- Place MOSFETs, diodes, and resistors in-line with direct current paths, using thick traces

### Routing

- Trace width vs. current rule of thumb
    - 1A continuous = 1mm trace with 1oz copper, 10°C temperature rise
- Use KiCad's calculator tools
- Avoid 90 degree corners on high-speed or high-current traces
- Route clocks and oscillators away from switching signals
- Use thick traces between power components
- Consider via ordering
    - E.g., MCU GND pin -> via, decoupling capacitor uses a direct trace on the signal layer to connect the MCU GND pin
- Ensure all RF signals are properly impedance matched
    - Prefer grounded coplanar waveguide or microstrip routing
    - Add stitching vias to ground in accordance with frequency and wavelength

### Thermal & Mechanical

- Add thermal vias beneath hot components
- Add thermal reliefs to pads and ground pads

### Fabrication

- Follow manufacturers' design constraints. You can find design rules online for popular board manufacturers that can be imported into KiCad.
- Populate component symbols with manufacturer library part numbers
- Never select a pure-tin finish (e.g., lead free HASL)
    - Tin whiskers run the risk of shorting in a vacuum
    - Select an alternative like leaded HASL or ENIG
- Check material outgassing properties against launch provider requirements for flight revisions

### Silkscreen

- Add polarity indicators for polarized components like capacitors
- Add polarity indicators for inductors
- Add diode symbol silkscreen next to diode footprints
- Mark switch positions and pinouts with silkscreen
- Label jumpers for inhibits
- Always add a version & revision number

### Manufacturing & Assembly

- Choose in-stock parts
- Minimize exotic footprints - try to stick to packages like 0402 and 0603
- Provide fiducials for pick & place
    - Must be asymmetric in different parts of the board to identify orientation. Prefer a triangle shape
- Ensure silkscreen is visible and not on pads
- Orientation dots for ICs
    - Consider making pin one longer for orientation purposes

### Bring-Up

- Always start with a visual inspection of component orientations
- Perform continuity checks on components like diodes and power rail shorts to ground
- Power using bench supply and watch for in-rush currents
    - Keep supply current low on the power supply
- Measure regulator outputs before connecting sensitive components like MCU
- Validate communications lines with logic analyzer/oscilloscope
- Incrementally enable loads
- After the design is stable, fill out an [ATP](./writing_atps.md) and put it in Git

### Documentation

- All schematics and layouts must be in version control
- Export assembly file
  - Contains orientations of all components and outlines, but no traces
  - Good for early identification of hard-to-catch mistakes
- Maintain BOM, revision histories, assembly/test procedures
- Print version number on silkscreen
- Create a Git release with production files
