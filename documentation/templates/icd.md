# Interface Control Document (ICD)

*Full guidance: [Writing Interface Control Documents](../design_guidelines/writing_atps.md). Delete any section that doesn't apply*

## 1. Document Control

| Field | Value |
| :---- | :---- |
| ICD Title | |
| ICD Number | |
| Revision | |
| Primary Maintainer (subsystem) | |
| Consuming Subsystems (must approve changes) | |
| Status | ☐ Draft ☐ Under Review ☐ Approved |


**Approvals:**

| Subsystem | Approver | Date |
| :---- | :---- | :---- |
| | | |
| | | |

## 2. Scope

**This ICD governs the interface between:**


**Explicitly out of scope:**


## 3. Physical Interface

**Connector type / part number:**

**Pin count:**

**Pinout table:**

| Pin | Signal Name | Direction | Voltage/Current Rating | Notes |
| :---- | :---- | :---- | :---- | :---- |
| | | | | |

**Mechanical notes (mounting, keying, orientation):**


## 4. Electrical Interface

| Parameter | Value | Notes |
| :---- | :---- | :---- |
| Logic high / low levels | | |
| Absolute max voltage | | |
| Nominal voltage | | |
| Current rating (per pin / per rail) | | |
| Rise/fall time | | (if applicable) |
| Setup/hold time | | (if applicable) |


**Protection responsibility** (which side owns ESD, reverse-polarity, overcurrent protection):


## 5. Protocol / Data Interface

**Packet/message format reference:**
*(link to the shared messaging standard, or define inline if this interface is unique)*

**Byte order / framing:**


**Command/response table** *(or reference to shared command table)*:

| Command ID | Name | Direction | Payload | Notes |
| :---- | :---- | :---- | :---- | :---- |
| | | | | |


**Error handling / retry behavior:**


**Protocol version in use:**


## 6. Sequencing & Timing

**Power-up/power-down order (if it matters):**


**Required handshaking (who initializes first, who waits):**


## 7. Change Control

**Process to propose a change:**


**Revision history:**

| Version | Date | Author | Summary of Change | Approved By |
| :---- | :---- | :---- | :---- | :---- |
| | | | | |


## 8. Traceability

**Requirements this ICD satisfies** (reference kickoff doc section(s)):


**Subsystems whose kickoff docs reference this ICD:**
