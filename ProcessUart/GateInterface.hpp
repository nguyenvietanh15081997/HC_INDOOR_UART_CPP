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

#include "../Include/Include.hpp"
#include "RingBuffer.h"
#include "ShareMessage.h"
#include "OpCode.h"

#define RINGBUFFER_LEN 		(2048)

#define UART_INTERFACE		(1)
#define UART_BAUDRATE		(115200)
#define UART_LENGTHDATA		(8)

#define TEMPBUFF_LENGTH		(64)
#define MESSAGE_MAXLENGTH	(61)
#define MESSAGE_HEADLENGTH	(3)

#define MESSAGE_OPCODE_01	(0x80)
#define MESSAGE_OPCODE_02	(0x90)
#define MESSAGE_OPCODE_03	(0x91)
#define MESSAGE_OPCODE_04	(0xFA)
#define MESSAGE_OPCODE_05	(0x92)

/*flag of provision*/
//extern bool flag_selectmac;
//extern bool flag_getpro_info;
//extern bool flag_getpro_element;
//extern bool flag_provision;
//extern bool flag_mac;
//extern bool flag_check_select_mac;
//extern bool flag_done;
//extern uint8_t deviceKey_json[50];

/*
 * Initiate uart, buffer save data
 *
 * @param null
 * @return null
 */
void GWIF_Init (void);

/*
 * Transmit uart data
 *
 * @param null
 * @return null
 */
void GWIF_WriteMessage (void);

/*
 * Read data to buffer uart and save to ringbuffer
 *
 * @param null
 * @return null
 */
void GWIF_Read2Buffer (void);

/*
 * Check frame data
 *
 * @param null
 * @return null
 */
void GWIF_CheckData (void);

/*
 * Process data after check
 * - Get data
 * - Calculate
 * - Control
 *
 * @param null
 * @return null
 */
int GWIF_ProcessData (void);

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

void* GWITF_WriteUart(void *argv);


#endif /* GATEWAYMANAGER_GATEINTERFACE_H_ */
