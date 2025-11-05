#ifndef UART_EVENTS_H
#define UART_EVENTS_H

#include "packet.h"
#include <stdbool.h>

void uart_events_init(void);
void uart_events_update(void);
bool uart_events_is_packet_available(void);
void uart_events_get_packet(OSUSatPacket *packet);
void uart_events_send_packet(const OSUSatPacket *packet);

#endif
