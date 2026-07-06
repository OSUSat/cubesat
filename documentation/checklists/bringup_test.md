# Bring-Up / Test Checklist
*One page. Pass/fail only. Full context: [Schematic & Layout Design Guidelines](../design_guidelines/schematic_layout_design.md), [Writing Acceptance Test Procedures](../design_guidelines/writing_atps.md)*

## Document Control

- Version: 1
- Author: Ethan Eggert
- Approver: 
- Last Revised date: 07/5/2026
- Approval Status: Unapproved

## Before Power

- [ ] ATP & testing procedure present
- [ ] Visual inspection: component orientation, no shorts/debris, correct rev on silkscreen
- [ ] Continuity checks: no unintended shorts to ground
- [ ] ESD precautions in place (strap, mat)
- [ ] RBF/inhibit state confirmed correct for this test

## Power-On

- [ ] Bench supply current-limited before first power-up
- [ ] In-rush current observed and within expected bounds
- [ ] Regulator outputs measured before connecting sensitive downstream components (MCU, sensors)

## Functional

- [ ] Every voltage rail measured against spec (no-load and rated-load)
- [ ] Current draw measured against power budget (quiescent, nominal, peak)
- [ ] Communication lines validated with logic analyzer/scope (UART, I2C, SPI as applicable)
- [ ] Loads enabled incrementally, not all at once
- [ ] Protection circuits verified (OCP trip point, RBF cutoff, deployment switch inhibit)

## Records

- [ ] Results logged (not just observed, written down)
- [ ] Any anomaly logged with disposition (rework / waiver / reject)
- [ ] If this is a flight/deliverable unit: formal ATP run and signed, not just informal bring-up

**Tested by:** _______________  **Date:** _______________  **Result:** ☐ Pass ☐ Pass with action items ☐ Fail
