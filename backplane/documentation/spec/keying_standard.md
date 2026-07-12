# Backplane Keying Scheme

## Document Control

- Version: 1
- Author: Ethan Eggert
- Approver:
- Last Revised Date: 07/11/2026
- Approval Status: Unapproved

## Purpose

This document defines how the backplane prevents a board from being inserted incorrectly (reversed, rotated, or into the wrong slot) and is a supporting design doc for the keying/orientation deliverable already required in the Backplane Subsystem Info doc and the ICD template.

## Problem Statement

Because every subsystem board uses the same connector standard (per the connector trade study), a board that shares a connector footprint with another slot's board is not prevented from being physically mated into the wrong place. There are two distinct failure modes to prevent:

1. **Reversed/rotated insertion:** a board flipped 180°, or otherwise mated backwards relative to its own correct slot. Given the shared power lines running through each connector, this is the higher-severity case: a reversed board can put power directly onto a ground or signal pin, risking damage to that board and potentially others sharing the same backplane rail.

2. **Wrong-slot insertion:** a board that mates fine in its own orientation, but into a slot intended for a different subsystem. Lower risk of immediate electrical damage if all slots share an identical pin mapping, but still a risk if any slot carries a subsystem-specific signal, and a risk to mission success even if nothing shorts (e.g., the OBC not finding the board it expects at boot).

## Layered Approach

No single mechanism should be relied on alone. Mechanical prevention first with visual and electronic checks as backups.

### Layer 1: Asymmetric connector placement (prevents reversed/rotated insertion)

Place each board's backplane-facing connector(s) at a position that is not symmetric about the board's centerline or rotation axis. A board that's flipped or rotated then physically cannot align its connector(s) with the backplane's receptacles.

### Layer 2: Slot-specific guide pins (prevents wrong-slot insertion)

Add a mechanical guide pin and matching hole pattern that is unique per board type/slot. For example, a guide pin at a specific position only matches the hole pattern on the board intended for that slot. A board can partially align its connector without the guide pin engaging, but cannot fully seat and reach proper connector mating depth without it.

- **Action item:** define the guide pin position/diameter table per slot as part of the pinout ICD.

### Layer 3: Visual labeling (aid for integration)

Silkscreen each board with its intended slot name/number, and label the backplane itself at each slot to match. This exists purely to help someone doing integration catch a mistake before attempting to seat a mis-mated board.

### Layer 4: Electronic slot-ID check

Reserve a small number of spare pins (already budgeted in the pin-budget methodology) as hard-wired slot-ID lines. For example, a unique ground/no-connect pattern per slot that a board can read on power-up. On boot, each board confirms it's reading the slot ID it expects before proceeding past a minimal safe power-up state.

## Open Items

- Define the specific guide pin position/diameter table per slot, and add it to the pinout ICD
- Confirm the number of spare pins available for the electronic slot-ID scheme against the existing pin-budget worksheet
- Decide the exact slot-ID encoding (e.g., grounded pin pattern vs. a small resistor-divider ID vs. a single-wire ID read over an ADC pin)

## References

- Backplane Subsystem Info doc: keying/orientation deliverable, pin-budget methodology
- Backplane Connector Trade Study: connector standard selection (Samtec SFM/SFH)
