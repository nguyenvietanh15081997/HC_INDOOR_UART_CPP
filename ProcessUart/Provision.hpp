/*
 * Provision.h handle process provision
 * Link with GateInterface.h to get data define process provision
 */
#ifndef PROVISION_HPP_
#define PROVISION_HPP_

#include "ShareMessage.h"
#include "OpCode.h"
#include <iostream>

using namespace std;

/*message control provision*/
extern uint8_t OUTMESSAGE_ScanStop[3];
extern uint8_t OUTMESSAGE_ScanStart[3];
extern uint8_t OUTMESSAGE_MACSelect[9];
extern uint8_t OUTMESSAGE_GetPro[3];
extern uint8_t OUTMESSAGE_Provision[28];
extern uint8_t OUTMESSAGE_BindingALl[22];
extern uint8_t reset_GW[];

extern uint8_t PRO_deviceKey[37];
extern uint8_t PRO_netKey[37];
extern uint8_t PRO_appKey[37];
extern uint8_t PRO_uuid[37];
extern uint8_t PRO_mac[24];

extern unsigned int adr_Provision;
extern pthread_t vrpth_Pro;


typedef enum statePro{
	statePro_scan,
	statePro_stop,
	statePro_selectMac,
	statePro_getPro,
	statePro_setPro,
	statePro_provision,
	statePro_bindingAll,
	statePro_saveGw,
	statePro_setType,
	statePro_timeoutPro,
	statePro_findDev,
	statePro_null,
	statePro_waitSetPro

}statePro_t;
extern statePro_t stateProvision;

void *Pro_Thread(void *argv);

#endif
