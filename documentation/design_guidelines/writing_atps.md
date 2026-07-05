# Writing Acceptance Test Procedures (ATPs)

## Document Control

- Version: 1
- Author: Ethan Eggert
- Approver: 
- Last Revised date: 07/5/2026
- Approval Status: Unapproved

## Overview

An Acceptance Test Procedure (ATP) is a formal document used to verify that a specific, serial-numbered unit (a PCB, assembly, or subsystem) meets its requirements before it is accepted into the next level of integration (e.g., subsystem board -> bus integration, or bus -> flight readiness).

This is distinct from the informal bring-up procedure described in [the hardware design guidelines](./schematic_layout_design.md). Bring-up happens once, on a prototype, to find and fix problems. An ATP happens on every unit you build, including flight units, spares, and engineering qualification models (EQMs), to prove that unit, specifically, is good.

## When to Write One

- After the schematic/layout is frozen and bring-up debugging is complete on at least one prototype
- Before the formal integration & testing process begins
- Typically one ATP per board/subsystem, plus a separate ATP at full bus/system level once subsystems are integrated
- The ATP should be baselined (reviewed & released as a controlled document) before it is run against real hardware, so that results are traceable against an unchanging procedure

## Document Structure

### 1. Document Control Header

- Title, subsystem name, document number, revision
- Unit under test: serial number, board revision, date of fabrication
- Author, approver, date of release
- Reference to the ATP's parent requirements documents (e.g., the subsystem's info doc, decision making docs, checklists)

### 2. Scope & Applicable Documents

- State which unit(s) and which requirements this ATP verifies
- List all documents this procedure derives from or must remain consistent with (requirements doc, interface control document, schematic revision)

### 3. Test Equipment & Setup

- List every piece of test equipment needed, with required accuracy/resolution (e.g., "multimeter, +-0.1% accuracy or better", "bench PSU, current-limited, 0-30V/0-3A")
- List fixtures, cables, harnesses, and any test-specific jigs
- Describe the physical setup (ESD-safe bench, grounding, current limiting on power supply, etc.)

### 4. Safety & Handling Notes

- ESD precautions (wrist strap, mat)
- Power sequencing precautions
- Any lithium battery handling precautions if battery is under test
- RBF/inhibit switch state required before test begins

### 5. Test Procedure

Structure each test as a numbered step with:

| Step | Action | Expected Result / Pass Criteria | Actual Result | Pass/Fail |
|---|---|---|---|---|
| 1 | [Action taken, e.g., "Apply 3.3V to J1 pin 2, measure current draw"] | [Expected value + tolerance, e.g., "< 50 mA"] | [blank, filled during test] | ☐ |

Common test categories to include, in rough execution order:

1. **Visual inspection** - solder joints, component orientation, no shorts/debris, silkscreen/rev number correct
2. **Continuity & isolation checks** - verify no shorts to ground/power on unpowered board, verify isolation between isolated domains
3. **Power-on sequencing** - apply power per the documented sequence, confirm in-rush current and rail rise times are within expected bounds
4. **Voltage rail verification** - measure every regulated rail against its spec (nominal ± tolerance) under no-load and rated-load conditions
5. **Current draw verification** - measure quiescent, nominal, and peak current draw against the power budget from the kickoff doc
6. **Protection circuit verification** - verify overcurrent protection trip points, confirm RBF/deployment-switch inhibit logic actually cuts power, verify any load switch enable/disable behavior
7. **Functional/interface tests** - exercise every external interface (CAN, I2C, SPI, ADC channels, GPIOs) against expected behavior, using known-good test equipment or a golden reference unit
8. **Communication bus verification** - confirm the unit responds correctly on the spacecraft bus/event system per its defined protocol
9. **Thermal spot-check** (if applicable at this level) - confirm no component exceeds derated thermal limits during rated-load operation

### 6. Anomaly / Discrepancy Log

- Any step that fails or produces an unexpected result gets logged here with:
    - Description of the anomaly
    - Root cause (if known)
    - Disposition: rework and retest, use-as-is with waiver, or reject
    - Sign-off on the disposition
- No unit should be "accepted" with an open, undispositioned anomaly

### 7. Final Sign-Off

- Test conductor signature/date
- Witness signature/date (if required by program policy)
- Subsystem lead / quality signature/date
- Overall unit disposition: ACCEPT / ACCEPT WITH WAIVER / REJECT

## Traceability

Each test step should trace back to a specific requirement (from the subsystem's kickoff doc, interface spec, or CDS-compliance checklist) so that acceptance test coverage can be checked against requirements during a design review. A simple traceability table (requirement ID -> ATP step number) at the end of the document is often sufficient.

## Version Control & Records

- The blank ATP is a controlled, versioned document like any schematic or firmware source file
- Each executed run produces an "as-run" copy — the filled-in procedure with actual results and signatures — which is a permanent record, not to be edited after the fact
- As-run ATPs are typically retained as part of the unit's data pack, referenced during CDR and flight readiness reviews
