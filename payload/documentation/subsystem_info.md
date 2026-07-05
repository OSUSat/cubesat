# Subsystem Info

# Payload Subsystem

## Document Control

- Version: 1
- Author: Ethan Eggert
- Approver: 
- Last Revised date: 07/5/2026
- Approval Status: Unapproved

# Background and context

The payload subsystem is responsible for controlling the dual camera system (or the mechanism for switching the camera filter for v1), taking images, and recording sensor data over different parts of the earth to perform climate science. Controlled by the OBC, when it receives a command to record payload data, it will automatically start a procedure to take images in different wavelengths over different sections of the earth. After the sensors have completed collection, they will transfer images in chunks over CAN to the OBC for storage, as well as the processed data from the climate algorithms.

# Payload Responsibilities

1. Receives commands over CANBus from the OBC when in a healthy state
2. Switches between camera filters/image bands to take images
3. Measures NDVI and RBD to detect vegetation health and oceanic algal blooms
4. Frames images in chunks, sending image bytes alongside a checksum for ordered and error-corrected image transmission to the OBC over CAN
5. Sends back the results of the algorithms to the OBC for storage
6. Performs graceful transitions between safe mode and science mode

# Considerations

- OBC needs a write buffer for reliable data storage and transmission, or the payload needs some sort of flow control
- Both OBC and payload need a transaction/retry mechanism in case of power failure or other latchups, for reliability
    - This is provided by the packet standard's CRC & ACK/NACK handshake

# Scope and deliverables


| Deliverable | Description |
| :---- | :---- |
| Switching camera mechanism design | The scope and hardware design for a mechanism for taking images in multiple light wavelengths |
| Climate experiment functionality report | Document detailing which algorithms, how and when they're used, that the payload will use to conduct climate science experiments |
| Payload PCB block diagram | The block diagram for the payload PCB |
| Payload PCB design & manufacturing | The design and manufacture of the payload PCB |
| Software control framework | Software functionality for controlling the different hardware elements |
| Healthy <-> safe mode transition | Software functionality for transitioning between safe mode and healthy mode |
| Error-corrected CAN image transmission | Software protocol for transmitting images over UART to the OBC |
| Payload data transmission over CAN | Software functionality for sending payload data to the OBC for storage |
| Testing framework (throughout project) | Software framework that allows for robust testing without the hardware. Should test software functionality and mock hardware-dependent behavior so the software flows as expected. |

# Resources

- https://www.arducam.com/embedded-camera-module/cameras-for-raspberrypi/raspberry-pi-camera-multi-cam-adapter-stereo.html
- https://www.raspberrypi.com/products/pi-noir-camera-v2/

# Next steps (v1r1)

- [X] Scope and design payload camera mechanism
- [ ] Design payload block diagram
- [X] Design payload hardware
- [ ] Scope software and create block diagram
- [ ] Create hardware mocks
- [ ] Implement and test software functionality
- [ ] Test on real hardware
- [ ] Integrate into system

# Next Steps (v2r1)

- [ ] Scope and design dual-camera system
- [ ] Write climate experiment functionality report
- [ ] Design payload block diagram
- [ ] Design payload hardware
- [ ] Scope software and create block diagram
- [ ] Create hardware mocks
- [ ] Implement and test software functionality
- [ ] Test on real hardware
- [ ] Integrate into system
