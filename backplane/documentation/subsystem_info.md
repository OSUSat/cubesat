# Subsystem Info

# Backplane Subsystem

## Document Control

- Version: 1
- Author: Ethan Eggert
- Approver: 
- Last Revised date: 07/5/2026
- Approval Status: Unapproved

Status: DRAFT - not yet scoped or reviewed. This is a proposed starting point intended to be reviewed and revised before committing to any meaningful work.

# Background and context

The backplane is the shared physical and electrical board that every other subsystem board plugs into for power distribution and inter-board communication, replacing the direct board-to-board stacking approach with a dedicated backplane PCB that all boards mate into independently. This gives up some of the simplicity of self-stacking in exchange for the benefit that any single board can be pulled and reseated without disturbing the rest of the stack, which matters when doing a lot of iterative bring-up and debugging and integration.

**RF:** will be kept off the backplane entirely and routed via coax/harnessing instead of a backplane microstrip.

# Backplane Responsibilities

1. Defines a single connector standard used identically by every subsystem board, and documents its pinout convention in an Interface Control Document (ICD)
2. Distributes power rail(s) from the EPS to every subsystem board through the backplane, with power pins gauged/paralleled to the current budget of each rail (a single small-pitch contact is typically rated for a modest current, but high-current rails may need multiple gathered pins rather than one)
3. Carries the CAN bus between every subsystem board
4. Defines a per-board connector count and pin budget methodology (see below), so each subsystem team can determine how many connector instances their board needs without guessing
5. Defines a keying/orientation scheme that makes reversed or misaligned insertion physically difficult or impossible
6. Provides mechanical retention adequate for the launch vibration environment, verified by test rather than assumed
7. Maintains version control over the pinout ICD and per-board connector count across hardware revisions

# Connector & Pin Budget Sizing Methodology

1. Inventory the signals every board needs access to: communications, power rail(s), a shared ground reference, and a small number of spare/reserved pins for future use
2. Add margin pins for power based on current budget per rail (parallel pins as needed, per current rating of the chosen connector's contacts)
3. Divide the total pin count needed by the pin count of the chosen single connector variant to get the number of connector instances required per board

# Scope and deliverables

| Deliverable | Description |
| :---- | :---- |
| Connector standard selection | Select a single pin-and-socket connector family (e.g., a small-pitch board-to-board connector) used identically across every subsystem board |
| Pin budget & connector count worksheet | Documented methodology and per-subsystem results for how many connector instances each board requires |
| Pinout ICD | Interface Control Document defining the single connector's pin assignment (CAN, power, ground, spares), current rating per pin/gang, and revision history |
| Keying/orientation scheme | Mechanical or connector-level keying that prevents reversed/misaligned insertion |
| Retention hardware design & vibration test plan | Card guides, edge clamps, staking, or connector-integrated locking, verified against the team's expected launch vibration environment by test |
| Backplane PCB design | Physical layout of the backplane PCB itself, including repeated connector footprints per board slot |


# Resources

- NASA review of the CompactPCI card-edge connector family in spaceflight applications: precedent for connector vibration risk in a launch environment: https://nepp.nasa.gov/docuploads/1B921377-60B3-423F-82CDD7CC44080A3E/CompactPCI%20Report,%208-3-07.pdf
- General guidance on connector selection for CubeSats, including latching/locking variants under launch vibration: https://www.harwin.com/blog/how-to-choose-connectors-for-space
- Other backplane architectures used by other university CubeSat teams: UWE-3/Kyutech backplane approach, OreSat backplane
- The CubeSat Design Specification, for any connector/mechanical constraints tied to the deployer envelope: [spec/CDS+REV14_1+2022-02-09.pdf](../../documentation/spec/CDS+REV14_1+2022-02-09.pdf)

# Next steps

- [ ] Revise subsystem doc requirements once scope above is confirmed
- [ ] Select a single connector standard and specific part/pitch
- [ ] Build the pin budget report and determine connector-instance count per subsystem board
- [ ] Draft the pinout ICD
- [ ] Define keying/orientation scheme
- [ ] Design retention hardware and plan a vibration test to verify it
- [ ] Design the backplane PCB
