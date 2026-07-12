# Backplane Connector Trade Study

## Document Control

- Version: 1
- Author: Ethan Eggert
- Approver:
- Last Revised Date: 07/05/2026
- Approval Status: Unapproved

## Purpose

This document records the connector standards considered for the primary backplane connector, the evaluation criteria used, and the rationale for the final selection. It supports the connector-selection deliverable in the Backplane Subsystem Info doc and should be referenced from the eventual backplane ICD.

## Decision Criteria


| Criterion | Why it matters |
| :---- | :---- |
| Contact type | Pin-and-socket vs. card-edge/gold-finger: affects vibration robustness |
| Flight heritage | Reduces risk relative to an unproven connector; ideally on a currently-flying CubeSat |
| Mounting technology | Through-hole (THT) vs. surface-mount (SMT): THT joints are mechanically anchored through the board. THT parts are better suited for high-vibration environments than SMT joints. Also easier to assemble and rework |
| Pin density / pitch | Determines how many connector instances are needed per board, per the pin-budget sizing methodology in the Backplane Subsystem doc |
| Cost & availability | Team budget and procurement lead time |
| Mechanical retention | Locking/latching options vs. relying entirely on external retention hardware |


## Candidates Considered

### 1. Card-edge / gold-finger connector (e.g., desktop PCIe-style slot)

The original starting point. This connector family is friction-fit and was designed for benchtop/industrial use, not launch vibration. A NASA review of the similar CompactPCI card-edge family found it can experience a "contact spreading" effect (intermittent loss of contact pressure) under vibration. No THT option applicable: this connector type mates directly to board copper fingers, not through-hole pins. **Rejected**

### 3. Harwin Gecko-SL

Lightweight (~1g), screw-locking mechanism directly addresses vibration retention at the connector level, flight heritage. Primarily an SMT-mounted product line. Locking mechanism is attractive, but the lack of no specific flight-heritage citation for this exact use case weighed against it. **Rejected for this decision, worth revisiting for secondary/debug connectors.**

### 4. Samtec ERM8/ERF8 (2mm pitch)

Considered for higher current-per-pin needs on power-heavy rails. Larger footprint than a 1.27mm-pitch part, and not confirmed to have the same flight-heritage track record for a full backplane role. **Rejected as the primary standard; may be revisited for a dedicated high-current power connector if the single-connector-standard approach can't meet a specific rail's current budget.**

### 5. Samtec SFM/SFH series (as used on OreSat's backplane)

1.27mm pitch, pin-and-socket, vertical socket connector. Has flight heritage: this is the connector family flying on OreSat's 1U and 2U spacecraft today. Available in through-hole (THT) mounting variants, which gives the mechanical robustness and reworkability benefits described above. Widely stocked, moderate cost, available in a range of pin counts suitable for the pin-budget-per-instance approach already defined for our backplane. **Selected.**

## Comparison Table


| Candidate | Contact Type | Flight Heritage | THT Available | Locking | Pitch | Selected |
| :---- | :---- | :---- | :---- | :---- | :---- | :---- |
| Card-edge (PCIe-style) | Card-edge | None found for backplane use | No | No | N/A | No |
| UWE-3/Kyutech pattern | Pin-and-socket | Yes (multiple missions) | Pattern only, not a specific part | Varies | Varies | No (informed approach) |
| Harwin Gecko-SL | Pin-and-socket | Not confirmed for this use | Not confirmed | Yes (screw-lok) | Small | No |
| Samtec ERM8/ERF8 | Pin-and-socket | Not confirmed for this use | Yes | No | 2mm | No |
| **Samtec SFM/SFH (OreSat)** | **Pin-and-socket** | **Yes (currently flying)** | **Yes** | **No (external retention needed)** | **1.27mm** | **Yes** |


## Decision & Rationale

**We are adopting the Samtec SFM/SFH connector family** (the same family used on OreSat's backplane) as our connector standard for the backplane, per the single-connector-standard approach already defined in the Backplane Subsystem doc.

The two deciding factors were:

1. **Flight heritage:** this is a connector family with a track record on a currently-orbiting spacecraft, not just a component with generic aerospace-adjacent marketing.
2. **THT availability:** through-hole variants give more robust, more reworkable solder joints, and are generally more tolerant of launch vibration than surface-mount-only alternatives.

## Risks / Open Items

- Mechanical retention is not built into this connector (unlike Gecko-SL's screw-lok): retention hardware (card guides, staking, clamps) remains a required deliverable, per the Backplane Subsystem doc, and should be vibration-tested rather than assumed adequate.
- Exact pin count/variant within the SFM/SFH family, and the resulting connector-instance count per board, is still to be worked out using the pin-budget methodology already defined.
- If a specific power rail's current budget exceeds what this pitch/contact size can support even with paralleled pins, a dedicated higher-current connector (e.g., ERM8/ERF8) may need to be reconsidered for that rail specifically.
- Procurement lead time and stock availability for the specific variant should be confirmed before this is finalized in the ICD.

## References

- OreSat backplane design: https://github.com/oresat/oresat-backplane
- NASA review of the CompactPCI card-edge connector family in spaceflight applications: https://nepp.nasa.gov/docuploads/1B921377-60B3-423F-82CDD7CC44080A3E/CompactPCI%20Report,%208-3-07.pdf
- Backplane Subsystem Info doc
