<!--
Pull Request Template
Applies to both hardware (schematic/layout) and firmware changes.
Fill out the Summary and Type of Change sections always.
Only fill out the checklist section(s) that apply and delete the ones that don't.
See the design guideline docs in /documentation for full context on any checklist item.
-->

## Summary

**What changed and why:**


**Related issue/ticket (if any):**


## Type of Change

- [ ] Hardware - schematic
- [ ] Hardware - layout
- [ ] Firmware - new service/module
- [ ] Firmware - refactor of existing service/module
- [ ] Documentation only
- [ ] Other:

**Affected subsystem(s):**

## Requirements Traceability

- [ ] This change is covered by an existing requirement in the subsystem's information doc
- [ ] This change requires a **new or updated** requirement in the information doc (link/describe below)
- [ ] This change requires an update to the CDS-compliance checklist (see [the Preliminary Hardware Design Guidelines](../documentation/design_guidelines/preliminary_hardware_design.md))

---

## Hardware Checklist
*(delete this section if the PR is firmware/docs only)*

Reference: [Preliminary Hardware Design Guidelines](../documentation/design_guidelines/preliminary_hardware_design.md), [Schematic & Layout Design Guidelines](../documentation/design_guidelines/schematic_layout_design.md)

**Schematic**
- [ ] All nets labeled, no floating/unlabeled nets
- [ ] Decoupling capacitors placed per datasheet for every new/changed IC
- [ ] Pull-up/pull-down resistors present on shared buses (I2C, reset, boot)
- [ ] Test points/headers added for key signals (UART, RESET, SWD/JTAG, I2C, SPI) if newly introduced
- [ ] Protection added where applicable (fuses, TVS, reverse-polarity, ESD) on any new external interface
- [ ] Components derated per team policy (voltage/current/power margin applied, not just worst-case survival)
- [ ] ERC is clean: no errors, no unresolved warnings
- [ ] Peer schematic review completed by someone other than the author

**Layout**
- [ ] Stackup/impedance requirements unaffected, or explicitly re-verified if changed
- [ ] Return paths remain short/continuous; ground plane not split
- [ ] Decoupling caps placed close to their ICs
- [ ] Trace widths verified against current requirements (via KiCad calculator or equivalent)
- [ ] Silkscreen updated: polarity marks, orientation dots, version/revision number bumped
- [ ] Assembly file exported and spot-checked for obvious placement errors
- [ ] DFM check: in-stock parts used, no exotic footprints introduced without discussion

**CDS / Mission Compliance** *(only if this change affects envelope, mass, power architecture, RBF, deployment switches, or RF)*
- [ ] Reviewed against CDS-compliance checklist in [Preliminary Hardware Design Guidelines](../documentation/design_guidelines/preliminary_hardware_design.md)
- [ ] No new CDS deviation introduced, or deviation documented and routed for approval

---

## Firmware Checklist
*(delete this section if the PR is hardware/docs only)*

Reference: [Refactoring Polling-Based Modules to be Event Driven](../documentation/design_guidelines/refactoring_polling_services.md), [Keeping the HAL Policy Free](../documentation/design_guidelines/keeping_the_hal_policy_free.md)

- [ ] New/changed service interfaces classified into Question / Command / Loop per [Refactoring Polling-Based Modules to be Event Driven](../documentation/design_guidelines/refactoring_polling_services.md), with Questions removed in favor of events
- [ ] Unique event UID chosen and confirmed non-colliding with existing services
- [ ] Service does not poll for state and subscribes to events or system tick as appropriate
- [ ] HAL layer changes do not publish to the event bus directly (callback-based reporting only, per [Keeping the HAL Policy Free](../documentation/design_guidelines/keeping_the_hal_policy_free.md))
- [ ] No new blocking or long-running calls introduced inside ISR-context or tick-handler code paths
- [ ] Watchdog/fault-handling behavior considered for this service (does a stall or fault get detected upstream?)
- [ ] Unit tested and/or tested on hardware; results summarized below

**Test summary:**


---

## ATP / Bring-Up Impact

- [ ] This change affects a board/unit that already has a released ATP: ATP updated or flagged for update
- [ ] This change was verified via bring-up/test procedure (attach results or link)
- [ ] No impact to existing ATP

## Deviations

List any deviation from the design guideline docs introduced by this PR, with justification. Leave blank if none.

| Guideline doc | Deviation | Justification | Approved by |
|---|---|---|---|
| | | | |


## Reviewer Sign-off

- [ ] I reviewed this PR against the relevant checklist section(s) above, not just the diff
- [ ] Any checked-but-not-actually-verified items were flagged and discussed with the author
- [ ] No open anomalies/deviations without an approver
