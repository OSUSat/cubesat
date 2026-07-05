# Layout Review Checklist
*One page. Pass/fail only. Full context: [Schematic & Layout Design Guidelines](../design_guidelines/schematic_layout_design.md)*

## Document Control

- Version: 1
- Author: Ethan Eggert
- Approver: 
- Last Revised date: 07/5/2026
- Approval Status: Unapproved

## Stackup & Routing

- [ ] Stackup matches spec (4-layer SGPS/SGGS or approved alternative)
- [ ] Ground plane continuous, not split
- [ ] Impedance-matched for high-speed lines (USB, CAN, RF)
- [ ] Trace widths verified against current requirements (calculator used, not eyeballed)
- [ ] No 90° corners on high-speed/high-current traces
- [ ] Clocks/oscillators routed away from switching signals
- [ ] Via ordering considered on decoupling/GND paths

## Placement

- [ ] Mechanically constrained components placed first
- [ ] Functional grouping applied (power, RF, logic, analog)
- [ ] Decoupling caps as close as possible to their ICs

## RF (if applicable)

- [ ] Impedance-matched, coplanar waveguide/microstrip used
- [ ] Stitching vias present per frequency/wavelength

## Thermal & Mechanical

- [ ] Thermal vias under hot components
- [ ] Thermal reliefs on relevant pads

## Fabrication & Assembly

- [ ] Manufacturer design rules imported/followed
- [ ] Manufacturer part numbers attached to symbols
- [ ] In-stock parts used, exotic footprints avoided
- [ ] Fiducials present, asymmetric, orientation-correct

## Silkscreen

- [ ] Polarity marks on caps/inductors/diodes
- [ ] Orientation dots on ICs
- [ ] Version/revision number printed and bumped
- [ ] Silkscreen not overlapping pads

## Documentation

- [ ] Schematic + layout committed to version control
- [ ] Assembly file exported
- [ ] BOM and revision history updated

**Reviewed by:** _______________  **Date:** _______________  **Result:** ☐ Pass ☐ Pass with action items ☐ Fail
