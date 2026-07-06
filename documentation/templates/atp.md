# Acceptance Test Procedure (ATP)

*Full guidance: [Writing Acceptance Test Procedures](../design_guidelines/writing_atps.md). Delete any test category below that doesn't apply*

## 1. Document Control Header


| Field | Value |
| :---- | :---- |
| Title | |
| Subsystem | |
| Document Number | |
| Revision | |
| Unit Under Test: Serial Number | |
| Unit Under Test: Board Revision | |
| Date of Fabrication | |
| Author | |
| Approver | |
| Date of Release | |


**Parent requirements documents referenced by this ATP:**


## 2. Scope & Applicable Documents

**Unit(s) and requirements this ATP verifies:**


**Applicable documents (requirements doc, ICD, schematic revision):**


| Document | Revision |
| :---- | :---- |
| | |


## 3. Test Equipment & Setup


| Equipment | Required Accuracy/Resolution |
| :---- | :---- |
| | |


**Fixtures, cables, harnesses, jigs:**


**Physical setup notes** (ESD-safe bench, grounding, current limiting):


## 4. Safety & Handling Notes

- [ ] ESD precautions confirmed (wrist strap, mat)
- [ ] Power sequencing precautions reviewed
- [ ] Lithium battery handling precautions reviewed (if applicable)
- [ ] RBF/inhibit switch state confirmed correct before test begins

**Additional notes:**


## 5. Test Procedure

*Copy the step table into each category that applies. Delete categories that don't apply to this unit.*

### 5.1 Visual Inspection


| Step | Action | Expected Result / Pass Criteria | Actual Result | Pass/Fail |
| :---- | :---- | :---- | :---- | :---- |
| | | | | ☐ |


### 5.2 Continuity & Isolation Checks


| Step | Action | Expected Result / Pass Criteria | Actual Result | Pass/Fail |
| :---- | :---- | :---- | :---- | :---- |
| | | | | ☐ |


### 5.3 Power-On Sequencing


| Step | Action | Expected Result / Pass Criteria | Actual Result | Pass/Fail |
| :---- | :---- | :---- | :---- | :---- |
| | | | | ☐ |


### 5.4 Voltage Rail Verification


| Step | Rail | Load Condition | Expected Value ± Tolerance | Actual Result | Pass/Fail |
| :---- | :---- | :---- | :---- | :---- | :---- |
| | | No load | | | ☐ |
| | | Rated load | | | ☐ |


### 5.5 Current Draw Verification


| Step | Condition | Expected Value ± Tolerance | Actual Result | Pass/Fail |
| :---- | :---- | :---- | :---- | :---- |
| | Quiescent | | | ☐ |
| | Nominal | | | ☐ |
| | Peak | | | ☐ |


### 5.6 Protection Circuit Verification


| Step | Action | Expected Result / Pass Criteria | Actual Result | Pass/Fail |
| :---- | :---- | :---- | :---- | :---- |
| | Overcurrent trip point | | | ☐ |
| | RBF/deployment switch inhibit cutoff | | | ☐ |
| | Load switch enable/disable | | | ☐ |


### 5.7 Functional / Interface Tests


| Step | Interface | Action | Expected Result / Pass Criteria | Actual Result | Pass/Fail |
| :---- | :---- | :---- | :---- | :---- | :---- |
| | CAN | | | | ☐ |
| | I2C | | | | ☐ |
| | SPI | | | | ☐ |
| | ADC channel | | | | ☐ |
| | GPIO | | | | ☐ |


### 5.8 Communication Bus Verification


| Step | Action | Expected Result / Pass Criteria | Actual Result | Pass/Fail |
| :---- | :---- | :---- | :---- | :---- |
| | | | | ☐ |


### 5.9 Thermal Spot-Check *(if applicable)*


| Step | Component | Condition | Expected Max Temp (derated) | Actual Result | Pass/Fail |
| :---- | :---- | :---- | :---- | :---- | :---- |
| | | | | | ☐ |


## 6. Anomaly / Discrepancy Log


| # | Description | Root Cause (if known) | Disposition | Approved By |
| :---- | :---- | :---- | :---- | :---- |
| | | | ☐ Rework & retest ☐ Waiver ☐ Reject | |


*No unit should be accepted with an undispositioned anomaly.*

## 7. Final Sign-Off


| Role | Signature | Date |
| :---- | :---- | :---- |
| Test Conductor | | |
| Witness (if required) | | |
| Subsystem Lead / Quality | | |

**Overall unit disposition:** ☐ ACCEPT ☐ ACCEPT WITH WAIVER ☐ REJECT

## 8. Traceability


| Requirement ID | Source Document | ATP Step(s) |
| :---- | :---- | :---- |
| | | |
