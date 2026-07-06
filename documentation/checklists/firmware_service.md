# Firmware Service Checklist
*One page. Pass/fail only. Full context: [Refactoring Polling-Based Modules to be Event Driven](../design_guidelines/refactoring_polling_services.md), [Keeping the HAL Policy Free](../design_guidelines/keeping_the_hal_policy_free.md)*

## Document Control

- Version: 1
- Author: Ethan Eggert
- Approver: 
- Last Revised date: 07/5/2026
- Approval Status: Unapproved

## Interface Classification (if refactoring)

- [ ] Every function/type classified as Question, Command, or Loop
- [ ] Questions removed, replaced with events pushed by the service
- [ ] Commands kept, but application requests changes over the event bus rather than calling commands directly
- [ ] Loop functions hidden behind a static implementation called from a system tick handler

## Event Design

- [ ] Unique 16-bit service UID chosen, checked against the existing UID registry for collisions
- [ ] Local event codes defined as an enum
- [ ] Composite event IDs built with `OSUSAT_BUILD_EVENT_ID`
- [ ] Public state struct exposes only a read-only snapshot, not internal implementation state

## HAL Boundary

- [ ] HAL layer does not publish directly to the event bus
- [ ] HAL error reporting uses an error enum + registered callback, not a service UID
- [ ] No event-system dependency introduced into ISR code

## Timing & Safety

- [ ] No blocking or long-running calls introduced inside ISR or tick-handler paths
- [ ] Watchdog/fault-detection behavior considered
- [ ] Prescaler/update rate documented if this service runs on a divided tick

## Verification

- [ ] Tested on hardware or in simulation, not just compiled
- [ ] Behavior during fault/safe-mode transition considered (does this service need to quiet down?)

**Reviewed by:** _______________  **Date:** _______________  **Result:** ☐ Pass ☐ Pass with action items ☐ Fail
