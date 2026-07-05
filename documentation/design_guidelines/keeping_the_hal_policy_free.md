# Keeping the HAL Policy Free

## Document Control

- Version: 1
- Author: Ethan Eggert
- Approver: 
- Last Revised date: 07/5/2026
- Approval Status: Unapproved

## Overview

When working with services in firmware you may find that some need signals from the HAL for error reporting, state tracking, or hardware events in general (e.g., an error reporting service wanting signals for CANBus faults). Your first instinct might be to create a new service ID for the HAL and integrate it with the event bus, but this introduces a few different architecture smells:

- The HAL (lowest level interface) now depends on the event system, which would have been previously reserved for higher level applications and services
- ISR timing sensitive code paths now may end up blocking or queuing to publish an event
- The HAL UID space starts to mirror the service UID space, which is a leaky abstraction
- The HAL now encodes "policy," saying that "this failure matters" instead of just reporting a signal

The HAL should not rely on the event bus to report facts or signals to higher level layers.

## Solution

Instead of using the event bus to publish signals, each HAL that needs error reporting functionality should instead implement three features into its interface:

1. An error enum (e.g., `uart_error_t` that contains different failure modes for UART operations)
2. A callback function type the HAL can call to report an error
3. A function to register an error callback with the HAL, so services can hook into HAL signals

Then, add to the HAL's global state the new callback function & any context. In each scenario where an error signal must be published, simply determine the error cause and call the callback.

For some HALs like communication protocols, you can also add callback function types for TX and RX events, and follow the same pattern for publishing error signals.
