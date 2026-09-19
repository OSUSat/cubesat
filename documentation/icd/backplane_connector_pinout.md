# Interface Control Document: Backplane Connector Pinout

*Full guidance: [Writing Interface Control Documents](../design_guidelines/writing_atps.md). Delete any section that doesn't apply*

## 1. Document Control


| Field | Value |
| :---- | :---- |
| ICD Title | Backplane Connector Pinout |
| ICD Number | #1 |
| Revision | 1 |
| Primary Maintainer (subsystem) | Backplane |
| Consuming Subsystems (must approve changes) | EPS, OBC, Payload, ADCS, Comms |
| Status | X Draft ☐ Under Review ☐ Approved |


**Approvals:**

| Subsystem | Approver | Date |
| :---- | :---- | :---- |
| | | |


# 2. Scope

**This ICD governs the interface between:** every subsystem board and the shared backplane


**Explicitly out of scope:** 

- Inter-subsystem RF routing (should be handled using a coax harness)
- Debug connections
- Keying information (already found in the backplane docs)

**Point-to-point exception**: pins 33-35 (SPI_SCLK/MOSI/MISO) are not part of the shared bus like every other signal on this connector. They are only routed point-to-point between the Payload and OBC; the corresponding pins are unconnected at every other slot.


## 3. Physical Interface

**Connector type / part number:** Samtec SFM/SFH family, THT, 1.27mm pitch

**Pin count:** 40 (2x20)

**Pinout table:**


| Pin | Signal Name | Direction | Voltage/Current Rating | Notes |
| :---- | :---- | :---- | :---- | :---- |
| 1 | GND | - | - | |
| 2 | GND | - | - | |
| 3 | RAIL_1_VOUT | Out (from EPS) | TBD | Paired with pin 4 |
| 4 | RAIL_1_VOUT | Out (from EPS) | TBD | Paired with pin 3 |
| 5 | RAIL_2_VOUT | Out (from EPS) | TBD | Paired with pin 6 |
| 6 | RAIL_2_VOUT | Out (from EPS) | TBD | Paired with pin 5 |
| 7 | RAIL_3_VOUT | Out (from EPS) | TBD | Paired with pin 8 |
| 8 | RAIL_3_VOUT | Out (from EPS) | TBD | Paired with pin 7 |
| 9 | RAIL_4_VOUT | Out (from EPS) | TBD | Paired with pin 10 |
| 10 | RAIL_4_VOUT | Out (from EPS) | TBD | Paired with pin 9 |
| 11 | RAIL_5_VOUT | Out (from EPS) | TBD | Paired with pin 12 |
| 12 | RAIL_5_VOUT | Out (from EPS) | TBD | Paired with pin 11 |
| 13 | RAIL_6_VOUT | Out (from EPS) | TBD | Paired with pin 14 |
| 14 | RAIL_6_VOUT | Out (from EPS) | TBD | Paired with pin 13 |
| 15 | RAIL_7_VOUT | Out (from EPS) | TBD | Paired with pin 16 |
| 16 | RAIL_7_VOUT | Out (from EPS) | TBD | Paired with pin 15 |
| 17 | RAIL_8_VOUT | Out (from EPS) | TBD | Paired with pin 18 |
| 18 | RAIL_8_VOUT | Out (from EPS) | TBD | Paired with pin 17 |
| 19 | GND | - | - | |
| 20 | GND | - | - | |
| 21 | EPS_PGOOD | Out (from EPS) | 3.3v Logic level | |
| 22 | GND | - | - | |
| 23 | CAN_MAIN_H | Bidirectional | CAN logic levels | |
| 24 | CAN_MAIN_H | Bidirectional | CAN logic levels | - |
| 25 | CAN_MAIN_L| Bidirectional | CAN logic levels | |
| 26 | CAN_MAIN_L | Bidirectional | CAN logic levels | - |
| 27 | OBC_HEARTBEAT | Out (from OBC) | 3.3v Logic level | |
| 28 | EPS_OBC_RESET | Out (from EPS) | 3.3v Logic level|  |
| 29 | SLOT_ID0 | In (per board) | TBD | Keying scheme Layer 4; encoding TBD |
| 30 | OBC_EPS_RESET | Out (from OBC) | 3.3v Logic level |  |
| 31 | SLOT_ID1 | In (per board) | TBD | Keying scheme Layer 4; encoding TBD |
| 32 | SLOT_ID2 | In (per board) | TBD | Keying scheme Layer 4; encoding TBD |
| 33 | SPI_SCLK | Out (from OBC) | Logic level, TBD | Point-to-point, Payload<->OBC only. Not present on other slots. |
| 34 | SPI_MOSI | Out (from OBC) | Logic level, TBD | Point-to-point, Payload<->OBC only. Not present on other slots. |
| 35 | SPI_MISO | Out (from Payload) | Logic level, TBD | Point-to-point, Payload<->OBC only. Not present on other slots. |
| 36 | CAN_AUX_H | Bidirectional | CAN logic levels |  |
| 37 | SPARE | - | - | Reserved for future revision |
| 38 | CAN_AUX_L | Bidirectional | CAN logic levels |  |
| 39 | GND | - | - | |
| 40 | GND | - | - | |


**Mechanical notes (mounting, keying, orientation):** See Backplane Keying Scheme doc for guide pin placement and connector orientation strategy (Layers 1-2)


## 4. Electrical Interface


| Parameter | Value | Notes |
| :---- | :---- | :---- |
| Logic high / low levels | 3.3v | |
| Absolute max voltage | TBD | |
| Nominal voltage | TBD | |
| Current rating (per pin / per rail) | TBD | |
| Rise/fall time | TBD | (if applicable) |
| Setup/hold time | TBD | (if applicable) |


**Protection responsibility** (which side owns ESD, reverse-polarity, overcurrent protection): Each board is responsible for its own protections at its own connector interface, per the Schematic & Layout Design Guidelines. The backplane itself does not provide protection beyond its physical/electrical pass-through role.


## 5. Protocol / Data Interface

**Packet/message format reference:**

- OSUSat messaging standard (CAN-based)

**Byte order / framing:**

- Per OSUSat messaging standard

**Command/response table:**

- Per OSUSat messaging standard

**Error handling / retry behavior:**

- Per OSUSat messaging standard

**Protocol version in use:**

- v1

## 6. Sequencing & Timing

**Power-up/power-down order (if it matters):** not relevant


**Required handshaking (who initializes first, who waits):** not relevant


## 7. Change Control

**Process to propose a change:**

Submit a design review request (per the Design Review Request template) referencing this ICD's current revision. Requires a sign-off from every subsystem listed in Section 1 before a new revision is finalized.


**Revision history:**


| Version | Date | Author | Summary of Change | Approved By |
| :---- | :---- | :---- | :---- | :---- |
| 1 | 08/01/2026 | Ethan Eggert | Draft initial ICD | |
| 1 | 08/01/2026 | Ethan Eggert | Swap CANBus pins to be adajacent to make differential routing easier | |
| 1 | 09/01/2026 | Ethan Eggert | Cluster main CANBus pins to make routing straight, remove SPAREs, and move AUX CAN line | |


## 8. Traceability

**Requirements this ICD satisfies**:

- [Backplane Subsystem Info Doc](../../backplane/documentation/subsystem_info.md)

**Subsystems whose info docs reference this ICD:** All
