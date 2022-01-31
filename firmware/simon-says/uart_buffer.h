/**
 * This buffer should be dumped out over UART in the main polled loop.
 */

#ifndef _UART_BUFFER_H
#define _UART_BUFFER_H

#include "saeclib/src/saeclib_circular_buffer.h"

extern saeclib_circular_buffer_t* uart_buffer;

#endif
