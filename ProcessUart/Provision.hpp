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

extern bool isProvision;
extern bool flag_selectmac;
extern bool flag_getpro_info;
extern bool flag_getpro_element;
extern bool flag_provision;
extern bool flag_mac;
extern bool flag_check_select_mac;
extern bool flag_done;
extern bool flag_setpro;
extern bool flag_admitpro;
extern bool flag_checkadmitpro;

extern bool flag_set_type;
extern bool flag_checkHB;
extern bool flag_checkSaveGW;
extern bool flag_checkTypeDEV;
extern bool hasSelectMac;
extern bool isPir_light;

extern uint64_t Timeout_CheckDataBuffer1;
extern bool Pro_startCount;
extern unsigned char scanNotFoundDev;
extern unsigned int adr_heartbeat;

extern uint16_t unicastId;
extern pthread_t tmp ;
extern bool vrb_CheckingClock;

extern uint8_t PRO_deviceKey[37];
extern uint8_t PRO_netKey[37];
extern uint8_t PRO_appKey[37];
extern uint8_t PRO_uuid[37];
extern uint8_t PRO_mac[24];

void StopCountCheckTimeout();

void InitTimeoutRspProvision();

void *ProvisionThread (void *argv);


#endif
