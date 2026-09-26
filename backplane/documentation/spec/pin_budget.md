# Backplane Pin Budget & Connector Count Worksheet

## Document Control

- Version: 1
- Author: Ethan Eggert
- Approver:
- Last Revised Date: August 1, 2026
- Approval Status: Unapproved

## Purpose

Determines the backplane bus pin allocation, used to inform the backplane pinout ICD

## Basis

Revised from 2025's Inter-Board Connector (IBC) design, which used a 40-pin (2x20) connector and worked out reasonably well in practice. This year's revisions account for: standardizing on CAN as the shared data bus (replacing UART as the primary protocol), adding GND reference pins, reserving pins for the electronic slot-ID scheme defined in the Backplane Keying Scheme doc, and adding a point-to-point SPI bus for Payload<->OBC data transfer.

## Shared Bus Pin Inventory (every board's connector carries this common set)


| Category | Signals | Pin Count | Notes |
| :---- | :---- | :---- | :---- |
| Power rails | RAIL_1_VOUT through RAIL_8_VOUT, 2 pins each | 16 | Matches EPS's existing 8-channel architecture; 2 pins/rail, also present in 2025's design. Current rating per rail is TBD pending EPS's finalized worst-case power budget |
| CAN bus (redundant) | CAN_MAIN_H, CAN_MAIN_L, CAN_AUX_H, CAN_AUX_L | 4 | Renamed from last year's `+/-` convention to standard CAN_H/CAN_L |
| EPS/OBC control & status | EPS_PGOOD, EPS_OBC_RESET, OBC_EPS_RESET, OBC_HEARTBEAT | 4 | Unchanged from last year |
| Ground | Distributed GND | 7 | Replaced some reserved pins with GND |
| Slot-ID (keying scheme, Layer 4) | SLOT_ID0, SLOT_ID1, SLOT_ID2 | 3 | New for this design; supports up to 8 unique slot IDs, per the Backplane Keying Scheme doc |
| Payload<->OBC SPI (point-to-point) | SPI_SCLK, SPI_MOSI, SPI_MISO | 3 | Higher-speed link for image transfer. Not part of the shared communication |
| Spares | Reserved for future revisions | 3 | See rationale in the Backplane Pinout ICD |
| **Total** | | **40** | Matches last year's connector size (2x20 / 40-position), but using a different connector standard |


**Removed from 2025 design:** dedicated UART (SYSTEM_MAIN_UART_RX/TX, SYSTEM_AUX_UART_RX/TX, 4 pins).


## Open Items

- [ ] Confirm per-rail worst-case current; verify 2 pins/rail is sufficient or whether any rail needs additional paralleled pins
- [ ] Confirm slot-ID encoding scheme (grounded pattern vs. resistor-divider vs. other) per the open item in the Backplane Keying Scheme doc

## References

- [Backplane Subsystem Info doc](../subsystem_info.md)
- [Backplane Connector Trade Study](../bom/tradeoffs/backplane_connector.md)
- [Backplane Keying Scheme](../spec/keying_standard.md)
- [Prior-year Inter-Board Connector (IBC) design](../../../eps/v1/hardware/inter_board_connector.kicad_sch)
