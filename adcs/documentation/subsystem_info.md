# Subsystem Info

# ADCS Subsystem

## Document Control

- Version: 1
- Author: Ethan Eggert
- Approver: 
- Last Revised date: 07/5/2026
- Approval Status: Unapproved

Status: DRAFT - not yet scoped or reviewed. This is a proposed starting point intended to be reviewed and revised before committing to any meaningful work.

# Background and context

The Attitude Determination and Control Subsystem (ADCS) is responsible for sensing and controlling the satellite's orientation. The mission's imaging payload (NDVI, cloud formation observation, etc.) needs some deliberate attitude control; a fully passive, uncontrolled system (permanent magnet + hysteresis rods alone) aligns the satellite to the local geomagnetic field line, not to nadir, and only coincidentally points at Earth for parts of the orbit near the magnetic poles. That isn't sufficient for a payload that needs to point roughly at the ground on a repeatable basis to capture usable imagery.

This document instead proposes **magnetorquer-assisted coarse nadir pointing** as the v1 baseline: passive hysteresis damping for detumble, plus driven magnetorquer coils and a lightweight control loop for coarse Earth-pointing. This sits between pure-passive (too little control for an imaging mission) and full active control with reaction wheels (too much hardware/firmware/testing effort for a first flight unit from a small team). Fine pointing via reaction wheels remains an explicit v2+ possibility only if the payload's actual requirements demand better than what magnetic-only control can deliver.

# A note on achievable pointing accuracy

Magnetorquers can only produce torque perpendicular to the local magnetic field at any given instant, so full 3-axis control authority is never available all at once. A controller converges toward nadir over the course of an orbit as the field direction rotates, rather than correcting all axes immediately. Published magnetorquer-only nadir-pointing results are generally in the range of roughly 10-20° mean pointing accuracy.

At a typical CubeSat LEO altitude (~500-600 km), a 15° pointing error translates to a ground miss distance on the order of 130+ km (altitude * tan(error)). Whether that's acceptable depends entirely on the camera's field of view and the imaging concept of operations, not on the NDVI math itself (NDVI has no inherent precision-pointing requirement). This baseline is only workable if the payload commits to:

- A wide FOV lens, wide enough that a 100+ km ground-track miss still lands inside the frame
- **Attitude-gated capture**: triggering the shutter based on the live attitude estimate (only imaging when within some acceptable cone of nadir) rather than on a blind timer
- An opportunistic-sampling mission concept (periodically capturing vegetation/cloud data across whatever ground track results) rather than a repeat-pass, precisely-targeted change-detection concept

If the mission concept actually requires reliably re-imaging a specific target on demand, this baseline is not sufficient and reaction wheels should be pulled forward from the deferred list.

# ADCS Responsibilities (v1 — Magnetorquer-Assisted Coarse Pointing)

1. Damps residual rotational energy after deployment using hysteresis rods (retained from the passive design) so the satellite detumbles without needing active control immediately after release
2. Provides attitude knowledge via:
    1. 3-axis magnetometer for magnetic field vector measurement
    2. 3-axis MEMS rate gyroscope for short-term angular rate/propagation between updates
    3. Coarse sun sensors (photodiodes per face) for a rough sun-vector estimate
3. Runs a lightweight attitude control law (e.g., B-dot for detumble, a cross-product/PD-style controller for coarse nadir pointing) on the ADCS MCU
4. Drives magnetorquer coils to actuate coarse pointing:
    1. Individually wound air-core (or ferrite-core, if sizing requires it) coils for X, Y, and Z axes, mounted against/near their respective faces for maximum loop area. A central driver PCB hosts the H-bridges, current sensing, and protection, with each wound coil wired back to it over a per-axis connector
    2. The Z-axis coil is wound on the same rod-core hardware used for passive hysteresis damping, where practical
5. Publishes a live "within pointing cone" flag alongside the attitude estimate, so the payload can gate image capture on actual attitude rather than firing on a blind timer
6. Logs attitude knowledge and control performance data for downlink to characterize actual on-orbit pointing accuracy and validate (or correct) the assumptions this scope is based on
7. Hands off attitude state and mode transitions (detumble -> coarse point -> safe) to the OBC over the shared messaging standard

# Deferred (v2+ candidate scope)

- Reaction wheels for fine 3-axis pointing, if the payload's actual pointing requirement turns out to be tighter than magnetorquer-only control can deliver
- Star tracker or other high-precision attitude determination hardware
- GPS-based real-time orbit/field-vector estimation (v1 can use an onboard field model e.g., IGRF, combined with ground-uploaded orbital elements, rather than a dedicated GPS receiver, to keep hardware scope down)

These are intentionally left out of v1. If on-orbit characterization data (see responsibility 5 above) shows magnetorquer-only pointing isn't sufficient for the payload, that's the trigger to revisit this list with the full team and faculty advisors, rather than a default assumption to design around now.

# Scope and deliverables


| Deliverable | Description |
| :---- | :---- |
| Pointing requirement confirmation | Define Camera FOV, required pointing accuracy/knowledge, and confirm if attitude-gated, opportunistic-capture is an acceptable concept of operations for mission imagery requirements |
| Magnetic torque & damping sizing analysis | Sizing calculation/simulation for hysteresis rod configuration, magnetorquer dipole moment (all 3 wound coils), and expected disturbance/control torques |
| Wound coil design (X, Y, Z) | Design/winding of individual coils for each axis (air-core or ferrite-core as sizing requires), sized for target dipole moment and mounted for maximum loop area on/near their respective faces; Z-axis coil shares core hardware with the passive hysteresis rod where practical |
| Torquer driver board | Single PCB hosting H-bridge drivers, current sensing, and protection circuitry for all three axes, with a per-axis connector wiring out to each wound coil |
| Attitude-gated capture interface | Define and implement the "within pointing cone" signal/flag published to the payload/OBC so image capture can be gated on live attitude rather than a timer |
| Attitude knowledge sensor PCB design | Schematic/layout for magnetometer, gyroscope, and sun sensors, with an interface to the OBC/ADCS MCU |
| Control law firmware | Detumble (B-dot) and coarse nadir-pointing control law implementation |
| Sensor fusion firmware | Basic filtering (e.g., complementary or simple Kalman filter) to combine magnetometer/gyro/sun-sensor data into an attitude estimate |
| On-orbit characterization plan | Plan for verifying actual pointing performance on-orbit using downlinked attitude knowledge and control performance data |

# Resources

- Magnetorquer-only attitude control literature for CubeSats: background on achievable pointing accuracy and the underactuation limitation: https://www.ri.cmu.edu/app/uploads/2020/06/magnetorquer_only.pdf
- Reference example of a commercial wound-coil magnetorquer board (single PCB driver + separate wound coils per axis, similar to the approach proposed here): https://nanoavionics.com/cubesat-components/cubesat-magnetorquer-satbus-mtq/
- CSSWE passive magnetic attitude control system — reference for hysteresis rod/detumble sizing, still relevant even though v1 adds active control on top: https://lasp.colorado.edu/csswe/system/subsystems/adcs/
- The CubeSat Design Specification, for any magnetic material or deployable constraints: [CDS spec](../spec/CDS+REV14_1+2022-02-09.pdf)
- Low Cost Self-Wound Magnetorquer Guide: [ResearchGate](https://www.researchgate.net/publication/361548818_A_Guide_to_Self-Built_Low-Cost_Magnetorquers_as_will_be_used_in_the_3U_CubeSat_SOURCE)
- Low Cost ADCS System: [ResearchGate](https://www.researchgate.net/publication/363862530_Low-Cost_Attitude_Determination_and_Control_System_of_the_Student-Built_3U_CubeSat_SOURCE)

# Next steps

- [ ] Confirm camera FOV and required pointing accuracy/knowledge with the payload team; confirm attitude-gated opportunistic capture is an acceptable concept of operations
- [ ] Size hysteresis rods, permanent magnet (if retained), and magnetorquer dipole moments against satellite mass properties and disturbance torques
- [ ] Select magnetometer, gyroscope, and sun sensor parts
- [ ] Design and wind X, Y, Z coils; determine mounting locations for maximum loop area
- [ ] Design the central torquer driver board (H-bridges, current sense, protection) and per-axis coil connectors
- [ ] Design and lay out attitude knowledge sensor PCB
- [ ] Implement detumble (B-dot) control law firmware
- [ ] Implement coarse nadir-pointing control law and sensor fusion firmware
- [ ] Implement and expose the "within pointing cone" attitude-gating flag for the payload
- [ ] Write on-orbit characterization/downlink plan
- [ ] Write kickoff doc requirements once scope above is confirmed with the team
