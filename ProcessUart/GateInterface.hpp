/*
 * GateInterface.h manage exchange uart data to AI7688H from gateway telink
 * - Initiate uart
 * - Transmit uart data
 * - Receive uart data
 * - Check uart data
 * - Process uart data
 * GateInterface.h have to include RingBuffer.h and ShareMessage.h
 */

#ifndef GATEINTERFACE_HPP_
#define GATEINTERFACE_HPP_

#include "RingBuffer.h"
#include "ShareMessage.h"
#include "OpCode.h"

#define RINGBUFFER_LEN 		(4096)

#define UART_INTERFACE		(1)
#define UART_BAUDRATE		(115200)
#define UART_LENGTHDATA		(8)

#define TEMPBUFF_LENGTH		(64)
#define MESSAGE_MAXLENGTH	(61)
#define MESSAGE_HEADLENGTH	(3)

/*
 * Thread process message interface with gateway:
 * - initiate
 * - write
 * - read
 * - check
 * - process
 *
 * @param null
 * @return null
 */
void* GWINF_Thread(void *vargp);


#endif /* GATEWAYMANAGER_GATEINTERFACE_H_ */
