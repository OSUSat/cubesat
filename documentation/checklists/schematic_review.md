# Schematic Review Checklist
*One page. Pass/fail only. Full context: [Schematic & Layout Design Guidelines](../design_guidelines/schematic_layout_design.md)*

## Document Control

- Version: 1
- Author: Ethan Eggert
- Approver: 
- Last Revised date: 07/5/2026
- Approval Status: Unapproved

## Organization

- [ ] Hierarchical sheets used, each with title/version/revision date
- [ ] All nets labeled
- [ ] Critical notes present where needed
- [ ] Tolerances on passives, max voltage on caps, saturation current on inductors
- [ ] Inter-sheet references enabled
- [ ] IC Datasheets present in `documentation/datasheets/` folder

## Power & Ground

- [ ] Fuses/TVS/reverse-polarity/filtering on external power inputs
- [ ] Decoupling capacitors present per datasheet on every IC

## Signals

- [ ] Pull-up/pull-down on shared buses (I2C, reset, boot)
- [ ] Test points/headers on UART TX/RX, RESET, SWD/JTAG, I2C, SPI
- [ ] Level shifting present on mixed-voltage domains

## Protection

- [ ] ESD protection on external connectors
- [ ] Current sense resistors + ADC on key rails/lines
- [ ] Redundancy applied where the subsystem info doc requires it
- [ ] DNP/solder-bridge options used where appropriate

## Derating

- [ ] Every new part's operating point checked against team derating policy (not just worst-case survival)

## Readiness

- [ ] ERC clean: zero errors, zero unresolved warnings
- [ ] Peer review completed by someone other than the author
- [ ] Sub-circuits simulated if time allows

**Reviewed by:** _______________  **Date:** _______________  **Result:** ☐ Pass ☐ Pass with action items ☐ Fail
