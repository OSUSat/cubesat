# Subsystem Info

# UHF Comms Subsystem

## Document Control

- Version: 1
- Author: Ethan Eggert
- Approver: 
- Last Revised date: 07/5/2026
- Approval Status: Unapproved

Status: DRAFT - not yet scoped or reviewed. This is a proposed starting point intended to be reviewed and revised before committing to any meaningful work.

# Background and context

The UHF Comms subsystem is responsible for uplink (commands from ground station) and downlink (telemetry, beacon, and payload data) over the amateur UHF band. Most university-class CubeSats, especially first missions, use low-cost, well-documented amateur UHF systems rather than higher-frequency or higher-data-rate commercial links, because amateur UHF hardware is cheap, widely available, well-understood by ham radio operators who can help with ground station operations, and doesn't require the same regulatory/licensing complexity as commercial spectrum. This document proposes a simple, half-duplex beacon + command/telemetry link as the v1 baseline, deferring anything higher-throughput to a later revision if the payload's data volume actually requires it.

# UHF Comms Responsibilities (v1, baseline)

1. Transmits a low-rate beacon at a fixed interval containing basic housekeeping telemetry (battery voltage, temperature, subsystem health), so we and the amateur radio community can track satellite health without needing to command it
2. Receives uplinked commands from the ground station and passes them to the OBC over CAN using the packet messaging standard
3. Transmits downlinked telemetry and payload data queued by the OBC
4. Operates half-duplex in the amateur UHF band (435-438 MHz), using AX.25 framing at 1200 bps AFSK as the baseline data rate/protocol, since it's widely supported by ground station software and amateur operators
    1. 9600 bps GMSK considered as a stretch goal once the basic link is proven, if payload data volume requires faster downlink
5. Implements basic ACK/retry per the packet messaging standard so dropped packets are re-requested rather than silently lost
6. Enforces the CDS-mandated RF inhibit, preventing any transmission until the post-deployment timer set by the launch provider has elapsed

# Explicitly Deferred (v2+ candidate scope)

- Higher-throughput downlink (9600+ bps GMSK, or S-band/higher-frequency links) - only pursue if payload data volume shows that the 1200 bps baseline is a bottleneck
- Onboard encryption/authentication of the command uplink beyond what's needed for amateur band compliance
- Custom RF front-end / antenna design beyond a simple deployable dipole or monopole, unless link budget analysis shows it's necessary

# Scope and deliverables


| Deliverable | Description |
| :---- | :---- |
| Link budget analysis | Analysis confirming 1200 bps AFSK at chosen transmit power closes the link for the mission's orbit and ground station setup and is enough to downlink the required experimental data |
| Frequency coordination | Amateur frequency coordination through IARU (International Amateur Radio Union) satellite frequency coordination panel. Should be started early, as it can take significant lead time |
| UHF transceiver hardware selection or design | Evaluate off-the-shelf amateur CubeSat transceiver modules vs. an in-house design |
| Antenna design & deployment mechanism | Deployable dipole/monopole antenna design and its release mechanism |
| RF inhibit implementation | Hardware/firmware implementation of the CDS-mandated post-deployment RF inhibit timer |
| Beacon firmware | Firmware to assemble and transmit the periodic housekeeping beacon |
| Command/telemetry firmware | Firmware to receive uplinked commands, hand off to OBC, and transmit queued downlink data with ACK/retry |
| Ground station compatibility testing | Verification that the downlink is receivable/decodable with common amateur ground station software |

# Resources

- CubeSat Design Specification Rev 14.1 — RF inhibit and deployment timer requirements: [spec/CDS+REV14_1+2022-02-09.pdf](../../documentation/spec/CDS+REV14_1+2022-02-09.pdf)
- IARU Amateur Satellite Frequency Coordination: https://www.iaru.org/satellite/

# Next steps

- [ ] Confirm expected payload data volume to validate if 1200 bps baseline is sufficient
- [ ] Start IARU frequency coordination process (long lead time, start this early regardless of hardware progress)
- [ ] Decide: off-the-shelf transceiver module vs. in-house design, based on team bandwidth and budget
- [ ] Perform link budget analysis for chosen orbit and ground station
- [ ] Design/select antenna and deployment mechanism
- [ ] Design RF inhibit circuit and firmware timer logic
- [ ] Implement beacon and command/telemetry firmware
- [ ] Test against ground station hardware/software before integration
- [ ] Finish subsystem info doc requirements once scope above is confirmed

# Project team

| Team members | Roles | Experience Level/Description |
| :---- | :---- | :---- |
| | | |
