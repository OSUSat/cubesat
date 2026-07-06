# Writing Interface Control Documents (ICDs)

## Document Control

- Version: 1
- Author: Ethan Eggert
- Approver: 
- Last Revised date: 07/5/2026
- Approval Status: Unapproved

## Overview

An Interface Control Document (ICD) defines and freezes the interface between two or more subsystems, or between a subsystem and an external system, like a ground station or launch vehicle. Each side can be built independently without constantly re-coordinating on every detail. This is different from a subsystem info doc: the info doc describes one subsystem's internal requirements, while an ICD describes only what crosses the boundary between subsystems.

The core idea: an ICD is a pre-agreed contract. If it gets written after both sides are already implemented, most of its value is already gone: the whole point is that both sides can develop against it in parallel without surprises.

## When to Write One

- Any time two or more subsystems (or a subsystem and an external system) share a physical connector, a power rail, or a data protocol
- Before both sides start building against the interface
- Examples already in this project: the backplane connector pinout, the packet standard, and (eventually) the ground station uplink/downlink protocol

## Document Structure

### 1. Document Control Header

- ICD number, revision, date, author
- Owning/primary-maintainer subsystem, and every other subsystem that consumes this interface and must approve changes to it
- Approval signatures from every consuming subsystem

### 2. Scope

- Which systems this ICD governs, specifically
- What's explicitly **not** covered, preventing scope-creep arguments down the line

### 3. Physical Interface *(if applicable)*

- Connector type, part number, pin/contact count
- Full pinout table: pin number, signal name, direction, voltage/current rating, notes
- Mechanical reference (mounting, keying, orientation) if relevant

### 4. Electrical Interface

- Voltage levels (logic high/low, absolute max, nominal)
- Current ratings/limits, per pin or per rail
- Protection responsibility: which side of the interface is responsible for ESD, reverse-polarity, overcurrent protection, etc.
- Timing characteristics if relevant (rise/fall time, setup/hold)

### 5. Protocol / Data Interface *(if applicable)*

- Packet/message format, byte order, framing
- Command/response tables, or a reference to the shared messaging standard if this ICD just adopts it
- Error handling and retry behavior
- Reference to the specific protocol version in use, and where the definition is maintained

### 6. Sequencing & Timing

- Power-up/power-down order across the interface, if sequence matters
- Required handshaking: who initializes first, who waits on whom

### 7. Change Control

- How to propose a change: who reviews, who must approve
- No unilateral changes
- Revision history table: version, date, author, summary of change, approvers

### 8. Traceability

- Which system-level or subsystem-level requirements this ICD satisfies
- Reference to the subsystem info doc(s) of every subsystem it governs

## Common Pitfalls

- Writing the ICD as after-the-fact documentation of an interface that's already built, instead of a pre-agreed contract both sides build against
- One subsystem changing a pin assignment, voltage, or timing detail without re-circulating for sign-off from everyone else who depends on it
- No revision history, so nobody can tell what changed between hardware or firmware runs
- No "out of scope" section, which just moves the scope-creep argument to later, when it's more expensive to resolve

## Maintenance

- ICDs are controlled documents and live in version control and go through the same review process as normal work PRs 
- Any executed test or bring-up procedure that depends on an ICD (e.g., ATP steps) should cite the ICD's specific revision number such that a mismatch between what was tested and what's currently defined is traceable
