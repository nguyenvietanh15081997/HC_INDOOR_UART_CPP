
#include "BuildCmdUart.hpp"
#include "../logging/slog.h"
#include "../Sensor/Sensor.hpp"
#include "string.h"

#define TIMEWAIT_UPDATE		1000
#define TIMEWAIT			500
#define TIMECONFIGROOM  	600
#define TIMEWAIT_REMOTE		600

static uint8_t parRetry_cnt = 0x02;
static uint8_t parRsp_Max = 0x00;
static uint8_t parFuture = 0;

static void CmdResetNode(uint16_t uniAdrReset) {
	vrts_CMD_STRUCTURE.adr_dst[0] = uniAdrReset & 0xFF;
	vrts_CMD_STRUCTURE.adr_dst[1] = (uniAdrReset >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.opCode[0] = NODE_RESET & 0xFF;
	vrts_CMD_STRUCTURE.opCode[1] = (NODE_RESET >> 8) & 0xFF;
}

static void CmdLightness_Get(uint16_t adrLightnessGet) {
	vrts_CMD_STRUCTURE.adr_dst[0] = adrLightnessGet & 0xFF;
	vrts_CMD_STRUCTURE.adr_dst[1] = (adrLightnessGet >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.opCode[0] = LIGHTNESS_GET & 0xFF;
	vrts_CMD_STRUCTURE.opCode[1] = (LIGHTNESS_GET >> 8) & 0xFF;
}

static void CmdLightness_Set(uint16_t uniAdrSetDim, uint16_t valueLightness,
		uint16_t transition) {
	vrts_CMD_STRUCTURE.adr_dst[0] = uniAdrSetDim & 0xFF;
	vrts_CMD_STRUCTURE.adr_dst[1] = (uniAdrSetDim >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.opCode[0] = LIGHTNESS_SET & 0xFF;
	vrts_CMD_STRUCTURE.opCode[1] = (LIGHTNESS_SET >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[0] = valueLightness & 0xFF;
	vrts_CMD_STRUCTURE.para[1] = (valueLightness >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[2] = 0;
	vrts_CMD_STRUCTURE.para[3] = transition & 0xFF;
	vrts_CMD_STRUCTURE.para[4] = (transition >> 8) & 0xFF;
}

static void CmdCCT_Get(uint16_t adrCCTGet) {
	vrts_CMD_STRUCTURE.adr_dst[0] = adrCCTGet & 0xFF;
	vrts_CMD_STRUCTURE.adr_dst[1] = (adrCCTGet >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.opCode[0] = LIGHT_CTL_TEMP_GET & 0xFF;
	vrts_CMD_STRUCTURE.opCode[1] = (LIGHT_CTL_TEMP_GET >> 8) & 0xFF;
}

static void CmdCCT_Set(uint16_t uniAdrSetCCT, uint16_t valueCCT,
		uint16_t transition) {
	vrts_CMD_STRUCTURE.adr_dst[0] = uniAdrSetCCT & 0xFF;
	vrts_CMD_STRUCTURE.adr_dst[1] = (uniAdrSetCCT >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.opCode[0] = LIGHT_CTL_TEMP_SET & 0xFF;
	vrts_CMD_STRUCTURE.opCode[1] = (LIGHT_CTL_TEMP_SET >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[0] = valueCCT & 0xFF;
	vrts_CMD_STRUCTURE.para[1] = (valueCCT >> 8) & 0xFF;
	int i;
	for (i = 2; i <= 4; i++) {
		vrts_CMD_STRUCTURE.para[i] = 0;
	}
	vrts_CMD_STRUCTURE.para[5] = transition & 0xFF;
	vrts_CMD_STRUCTURE.para[6] = (transition >> 8) & 0xFF;
}

static void CmdAddGroup(uint16_t uniAdrAddGroup, uint16_t adrGroup) {
	vrts_CMD_STRUCTURE.adr_dst[0] = uniAdrAddGroup & 0xFF;
	vrts_CMD_STRUCTURE.adr_dst[1] = (uniAdrAddGroup >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.opCode[0] = CFG_MODEL_SUB_ADD & 0xFF;
	vrts_CMD_STRUCTURE.opCode[1] = (CFG_MODEL_SUB_ADD >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[0] = uniAdrAddGroup & 0xFF;
	vrts_CMD_STRUCTURE.para[1] = (uniAdrAddGroup >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[2] = adrGroup & 0xFF;
	vrts_CMD_STRUCTURE.para[3] = (adrGroup >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[4] = 0x00;
	vrts_CMD_STRUCTURE.para[5] = 0x10;
}

static void CmdDelGroup(uint16_t uniAdrAddGroup, uint8_t adrGroup) {
	vrts_CMD_STRUCTURE.adr_dst[0] = uniAdrAddGroup & 0xFF;
	vrts_CMD_STRUCTURE.adr_dst[1] = (uniAdrAddGroup >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.opCode[0] = CFG_MODEL_SUB_DEL & 0xFF;
	vrts_CMD_STRUCTURE.opCode[1] = (CFG_MODEL_SUB_DEL >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[0] = uniAdrAddGroup & 0xFF;
	vrts_CMD_STRUCTURE.para[1] = (uniAdrAddGroup >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[2] = adrGroup;
	vrts_CMD_STRUCTURE.para[3] = 0xC0;
	vrts_CMD_STRUCTURE.para[4] = 0x00;
	vrts_CMD_STRUCTURE.para[5] = 0x10;
}

static void CmdAddSence(uint16_t uniAdrSence, uint16_t senceID, uint8_t sRgbid) {
	vrts_CMD_STRUCTURE.adr_dst[0] = uniAdrSence & 0xFF;
	vrts_CMD_STRUCTURE.adr_dst[1] = (uniAdrSence >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.opCode[0] = SCENE_STORE & 0xFF;
	vrts_CMD_STRUCTURE.opCode[1] = (SCENE_STORE >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[0] = senceID & 0xFF;
	vrts_CMD_STRUCTURE.para[1] = (senceID >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[2] = sRgbid;
	vrts_CMD_STRUCTURE.para[3] = vrts_CMD_STRUCTURE.para[4] = 0;
}

static void CmdCallSence(uint16_t adr,uint16_t senceId, uint8_t sRgbId, uint16_t transition) {
	vrts_CMD_STRUCTURE.adr_dst[0] = adr & 0xFF;
	vrts_CMD_STRUCTURE.adr_dst[1] = (adr >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.opCode[0] = SCENE_RECALL & 0xFF;
	vrts_CMD_STRUCTURE.opCode[1] = (SCENE_RECALL >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[0] = senceId & 0xFF;
	vrts_CMD_STRUCTURE.para[1] = (senceId >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[2] = sRgbId;
	vrts_CMD_STRUCTURE.para[3] = transition & 0xFF;
	vrts_CMD_STRUCTURE.para[4] = (transition >> 8) & 0xFF;
}

#define HEADER_CALLMODE_RGB  0x0919
static void CmdCallModeRgb(uint16_t adr, uint8_t srgb){
	vrts_CMD_STRUCTURE.adr_dst[0] = adr & 0xFF;
	vrts_CMD_STRUCTURE.adr_dst[1] = (adr >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.opCode[0] = LIGHTNESS_LINEAR_SET & 0xFF;
	vrts_CMD_STRUCTURE.opCode[1] = (LIGHTNESS_LINEAR_SET >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[0] = HEADER_CALLMODE_RGB & 0xFF;
	vrts_CMD_STRUCTURE.para[1] = (HEADER_CALLMODE_RGB >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[2] = srgb;
}

static void CmdDelSence(uint16_t uniAdrDelSence, uint16_t senceId) {
	vrts_CMD_STRUCTURE.adr_dst[0] = uniAdrDelSence & 0xFF;
	vrts_CMD_STRUCTURE.adr_dst[1] = (uniAdrDelSence >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.opCode[0] = SCENE_DEL & 0xFF;
	vrts_CMD_STRUCTURE.opCode[1] = (SCENE_DEL >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[0] = senceId & 0xFF;
	vrts_CMD_STRUCTURE.para[1] = (senceId >> 8) & 0xFF;
}

static void CmdControlOnOff(uint16_t uniAdrControlOnOff, uint8_t statuOnOff,
		uint16_t transition) {
	vrts_CMD_STRUCTURE.adr_dst[0] = uniAdrControlOnOff & 0xFF;
	vrts_CMD_STRUCTURE.adr_dst[1] = (uniAdrControlOnOff >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.opCode[0] = LIGHTOPCODE_ONOFF & 0xFF;
	vrts_CMD_STRUCTURE.opCode[1] = (LIGHTOPCODE_ONOFF >> 8) & 0xFF;
	if (statuOnOff == 1) {
		vrts_CMD_STRUCTURE.para[0] = statuOnOff;
	}
	if (statuOnOff == 0) {
		vrts_CMD_STRUCTURE.para[0] = statuOnOff;
	}
	vrts_CMD_STRUCTURE.para[1] = 0;
	vrts_CMD_STRUCTURE.para[2] = transition & 0xFF;
	vrts_CMD_STRUCTURE.para[3] = (transition >> 8) & 0xFF;
}

static void CmdHSL_Get(uint16_t adrHSLGet) {
	vrts_CMD_STRUCTURE.adr_dst[0] = adrHSLGet & 0xFF;
	vrts_CMD_STRUCTURE.adr_dst[1] = (adrHSLGet >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.opCode[0] = LIGHTOPCODE_SELECT & 0xFF;
	vrts_CMD_STRUCTURE.opCode[1] = (LIGHTOPCODE_SELECT >> 8) & 0xFF;
}

static void CmdHSL_Set(uint16_t uniAdrHSL, uint16_t h, uint16_t s, uint16_t l,
		uint16_t transition) {
	vrts_CMD_STRUCTURE.adr_dst[0] = uniAdrHSL & 0xFF;
	vrts_CMD_STRUCTURE.adr_dst[1] = (uniAdrHSL >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.opCode[0] = LIGHT_HSL_SET & 0xFF;
	vrts_CMD_STRUCTURE.opCode[1] = (LIGHT_HSL_SET >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[0] = l & 0xFF;
	vrts_CMD_STRUCTURE.para[1] = (l >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[2] = h & 0xFF;
	vrts_CMD_STRUCTURE.para[3] = (h >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[4] = s & 0xFF;
	vrts_CMD_STRUCTURE.para[5] = (s >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[6] = 0;
	vrts_CMD_STRUCTURE.para[7] = transition & 0xFF;
	vrts_CMD_STRUCTURE.para[8] = (transition >> 8) & 0xFF;
}

static void CmdSetTimePoll(uint16_t uniAdrSensor, uint16_t timePoll) {
	vrts_CMD_STRUCTURE.adr_dst[0] = uniAdrSensor & 0xFF;
	vrts_CMD_STRUCTURE.adr_dst[1] = (uniAdrSensor >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.opCode[0] = SENSOR_DESCRIP_GET & 0xFF;
	vrts_CMD_STRUCTURE.opCode[1] = (SENSOR_DESCRIP_GET >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[0] = (timePoll >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[1] = timePoll & 0xFF;
	vrts_CMD_STRUCTURE.para[2] = parFuture;
}

static void CmdOnoff_NoAck(uint16_t uniAdrControlOnOff, uint8_t statuOnOff,
		uint16_t transition) {
	vrts_CMD_STRUCTURE.adr_dst[0] = uniAdrControlOnOff & 0xFF;
	vrts_CMD_STRUCTURE.adr_dst[1] = (uniAdrControlOnOff >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.opCode[0] = G_ONOFF_SET_NOACK & 0xFF;
	vrts_CMD_STRUCTURE.opCode[1] = (G_ONOFF_SET_NOACK >> 8) & 0xFF;
	if (statuOnOff == 1) {
		vrts_CMD_STRUCTURE.para[0] = statuOnOff;
	}
	if (statuOnOff == 0) {
		vrts_CMD_STRUCTURE.para[0] = statuOnOff;
	}
	vrts_CMD_STRUCTURE.para[1] = 0;
	vrts_CMD_STRUCTURE.para[2] = transition & 0xFF;
	vrts_CMD_STRUCTURE.para[3] = (transition >> 8) & 0xFF;
}

static void CmdLightness_Set_NoAck(uint16_t uniAdrSetDim, uint16_t valueLightness,
		uint16_t transition) {
	vrts_CMD_STRUCTURE.adr_dst[0] = uniAdrSetDim & 0xFF;
	vrts_CMD_STRUCTURE.adr_dst[1] = (uniAdrSetDim >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.opCode[0] = LIGHTNESS_SET_NOACK & 0xFF;
	vrts_CMD_STRUCTURE.opCode[1] = (LIGHTNESS_SET_NOACK >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[0] = valueLightness & 0xFF;
	vrts_CMD_STRUCTURE.para[1] = (valueLightness >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[2] = 0;
	vrts_CMD_STRUCTURE.para[3] = transition & 0xFF;
	vrts_CMD_STRUCTURE.para[4] = (transition >> 8) & 0xFF;
}

static void CmdCCT_Set_NoAck(uint16_t uniAdrSetCCT, uint16_t valueCCT,
		uint16_t transition) {
	vrts_CMD_STRUCTURE.adr_dst[0] = uniAdrSetCCT & 0xFF;
	vrts_CMD_STRUCTURE.adr_dst[1] = (uniAdrSetCCT >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.opCode[0] = LIGHT_CTL_TEMP_SET_NOACK & 0xFF;
	vrts_CMD_STRUCTURE.opCode[1] = (LIGHT_CTL_TEMP_SET_NOACK >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[0] = valueCCT & 0xFF;
	vrts_CMD_STRUCTURE.para[1] = (valueCCT >> 8) & 0xFF;
	int i;
	for (i = 2; i <= 4; i++) {
		vrts_CMD_STRUCTURE.para[i] = 0;
	}
	vrts_CMD_STRUCTURE.para[5] = transition & 0xFF;
	vrts_CMD_STRUCTURE.para[6] = (transition >> 8) & 0xFF;
}

static void CmdHSL_Set_NoAck(uint16_t uniAdrHSL, uint16_t h, uint16_t s,
		uint16_t l, uint16_t transition) {
	vrts_CMD_STRUCTURE.adr_dst[0] = uniAdrHSL & 0xFF;
	vrts_CMD_STRUCTURE.adr_dst[1] = (uniAdrHSL >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.opCode[0] = LIGHT_HSL_SET_NOACK & 0xFF;
	vrts_CMD_STRUCTURE.opCode[1] = (LIGHT_HSL_SET_NOACK >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[0] = l & 0xFF;
	vrts_CMD_STRUCTURE.para[1] = (l >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[2] = h & 0xFF;
	vrts_CMD_STRUCTURE.para[3] = (h >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[4] = s & 0xFF;
	vrts_CMD_STRUCTURE.para[5] = (s >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[6] = 0;
	vrts_CMD_STRUCTURE.para[7] = transition & 0xFF;
	vrts_CMD_STRUCTURE.para[8] = (transition >> 8) & 0xFF;
}

void CmdUpdateLight(uint16_t cmd, uint16_t adr, uint16_t cmdLength) {
	vrts_CMD_STRUCTURE.HCI_CMD_GATEWAY[0] = cmd & 0xFF;
	vrts_CMD_STRUCTURE.HCI_CMD_GATEWAY[1] = (cmd>>8) & 0xFF;
	vrts_CMD_STRUCTURE.opCode00[0] = 0;
	vrts_CMD_STRUCTURE.opCode00[1] = 0;
	vrts_CMD_STRUCTURE.opCode00[2] = 0;
	vrts_CMD_STRUCTURE.opCode00[3] = 0;
	vrts_CMD_STRUCTURE.retry_cnt = 0;
	vrts_CMD_STRUCTURE.rsp_max = parRsp_Max;
	vrts_CMD_STRUCTURE.adr_dst[0] = adr & 0xFF;
	vrts_CMD_STRUCTURE.adr_dst[1] = (adr >>8 ) & 0xFF;
	vrts_CMD_STRUCTURE.opCode[0] = LIGHTOPCODE_UPDATE & 0xFF;
	vrts_CMD_STRUCTURE.opCode[1] = (LIGHTOPCODE_UPDATE >> 8) & 0xFF;

	uartSendDev_t vrts_UartUpdate;
	vrts_UartUpdate.length = cmdLength;
	vrts_UartUpdate.dataUart = vrts_CMD_STRUCTURE;
	vrts_UartUpdate.timeWait = TIMEWAIT_UPDATE;
	pthread_mutex_trylock(&vrpth_SendUart);
	if(!gvrb_Provision){
		bufferUartUpdate.push_back(vrts_UartUpdate);
	}
	pthread_mutex_unlock(&vrpth_SendUart);
}

void FunctionPer(uint16_t cmd, functionTypeDef Func, uint16_t unicastAdr,
		uint16_t adrGroup, uint8_t parStatusOnOff, uint16_t parLightness,
		uint16_t parCCT, uint16_t parSenceId, uint16_t parTimePoll,
		uint16_t parL, uint16_t parH, uint16_t parS, uint16_t transition_par_t,
		uint8_t cmdLength)
{
	vrts_CMD_STRUCTURE.HCI_CMD_GATEWAY[0] = cmd & 0xFF;
	vrts_CMD_STRUCTURE.HCI_CMD_GATEWAY[1] = (cmd>>8) & 0xFF;
	vrts_CMD_STRUCTURE.opCode00[0] = 0;
	vrts_CMD_STRUCTURE.opCode00[1] = 0;
	vrts_CMD_STRUCTURE.opCode00[2] = 0;
	vrts_CMD_STRUCTURE.opCode00[3] = 0;
	vrts_CMD_STRUCTURE.retry_cnt = parRetry_cnt;
	vrts_CMD_STRUCTURE.rsp_max = parRsp_Max;
	uint64_t timeWait = TIMEWAIT;
	if(Func == ResetNode_typedef){
		CmdResetNode(unicastAdr);
	}
	else if(Func == Lightness_Get_typedef){
		CmdLightness_Get(unicastAdr);
	}
	else if(Func == AddGroup_typedef){
		gvrb_AddGroupLight = true;
		CmdAddGroup(unicastAdr, adrGroup);
		timeWait = TIMECONFIGROOM;
	}
	else if(Func == DelGroup_typedef){
		gvrb_AddGroupLight = false;
		CmdDelGroup(unicastAdr, adrGroup);
	}
	else if(Func == ControlOnOff_typedef){
		CmdControlOnOff(unicastAdr,parStatusOnOff,transition_par_t);
	}
	else if(Func == ControlOnoff_NoAck_typedef){
		CmdOnoff_NoAck(unicastAdr,parStatusOnOff,transition_par_t);
	}
	else if (Func == SetTimePoll_typedef){
		CmdSetTimePoll(unicastAdr, parTimePoll);
	}
	else if (Func == CCT_Set_typedef)
	{
		CmdCCT_Set(unicastAdr,parCCT,transition_par_t);
	}
	else if(Func == CCT_Set_NoAck_typedef){
		CmdCCT_Set_NoAck(unicastAdr,parCCT,transition_par_t);
	}
	else if (Func == Lightness_Set_typedef){
		CmdLightness_Set(unicastAdr, parLightness,transition_par_t);
	}
	else if(Func == Lightness_Set_NoAck_typedef){
		CmdLightness_Set_NoAck(unicastAdr, parLightness,transition_par_t);
	}
//	else if (Func == UpdateLight_typedef){
//		CmdUpdateLight(unicastAdr);
//	}
	else if(Func == AddSence_typedef){
		gSceneIdDel = parSenceId;
		CmdAddSence(unicastAdr, parSenceId,0);
		timeWait = TIMECONFIGROOM;
	}
	else if(Func == DelSence_typedef){
		gSceneIdDel = parSenceId;
		CmdDelSence(unicastAdr, parSenceId);
	}
	else if(Func == CallSence_typedef){
		CmdCallSence(unicastAdr,parSenceId,0,transition_par_t);
	}
	else if(Func == HSL_Set_typedef){
		CmdHSL_Set(unicastAdr, parH, parS, parL,transition_par_t);
	}
	else if(Func == CCT_Get_typedef){
		CmdCCT_Get(unicastAdr);
	}
	else if(Func == HSL_Get_typedef){
		CmdHSL_Get(unicastAdr);
	}
	else if(Func == HSL_Set_NoAck_typedef){
		CmdHSL_Set_NoAck(unicastAdr, parH, parS, parL,transition_par_t);
	}
	else if(Func == SceneForRGB_vendor_typedef){
		gSceneIdDel = parSenceId;
		CmdAddSence(unicastAdr,parSenceId,parStatusOnOff);
	}
	else if(Func == CallSceneRgb_vendor_typedef){
		CmdCallSence(unicastAdr,parSenceId, 0, transition_par_t);
	}
	else if(Func == CallModeRgb_vendor_typedef){
		CmdCallModeRgb(unicastAdr, parStatusOnOff);
	}
	else if(Func == DelSceneRgb_vendor_typedef){
		gSceneIdDel = parSenceId;
		CmdDelSence(unicastAdr,parSenceId);
	}
	uartSendDev_t vrts_DataUartSend;
	vrts_DataUartSend.length = cmdLength;
	vrts_DataUartSend.dataUart = vrts_CMD_STRUCTURE;
	vrts_DataUartSend.timeWait = timeWait;
	pthread_mutex_trylock(&vrpth_SendUart);
	bufferDataUart.push_back(vrts_DataUartSend);
	pthread_mutex_unlock(&vrpth_SendUart);

#if !PRINTUART
	printf("%x %x ",vrts_DataUartSend.dataUart.HCI_CMD_GATEWAY[0],vrts_DataUartSend.dataUart.HCI_CMD_GATEWAY[1]);
	for (int i = 0; i < 4; i++) {
		printf("%x ", vrts_DataUartSend.dataUart.opCode00[i]);
	}
	printf("%x %x ",vrts_DataUartSend.dataUart.retry_cnt, vrts_DataUartSend.dataUart.rsp_max);
	printf("%x %x ",vrts_DataUartSend.dataUart.adr_dst[0],vrts_DataUartSend.dataUart.adr_dst[1]);
	printf("%x %x ",vrts_DataUartSend.dataUart.opCode[0], vrts_DataUartSend.dataUart.opCode[1]);
	for (int j = 0; j < vrts_DataUartSend.length - 12; j++) {
		printf("%x ",vrts_DataUartSend.dataUart.para[j]);
	}
	printf("\n");
#endif
}

static void SetSceneForRemote_DC(uint16_t adrRemote_DC, uint8_t buttonId,
		uint8_t modeId, uint16_t sceneId, uint16_t appID, uint8_t SrgbID) {
	vrts_CMD_STRUCTURE.adr_dst[0] = adrRemote_DC & 0xFF;
	vrts_CMD_STRUCTURE.adr_dst[1] = (adrRemote_DC >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.opCode[0] = RD_OPCODE_SCENE_SEND & 0xFF;
	vrts_CMD_STRUCTURE.opCode[1] = (VENDOR_ID) & 0xFF;
	vrts_CMD_STRUCTURE.para[0] = (VENDOR_ID >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[1] = STATUS_CMD_SCENE & 0xFF;
	vrts_CMD_STRUCTURE.para[2] = (STATUS_CMD_SCENE >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[3] = (HEADER_SCENE_REMOTE_DC_SET) & 0xFF;
	vrts_CMD_STRUCTURE.para[4] = HEADER_SCENE_REMOTE_DC_SET >> 8 & 0xFF;
	vrts_CMD_STRUCTURE.para[5] = buttonId;
	vrts_CMD_STRUCTURE.para[6] = modeId;
	vrts_CMD_STRUCTURE.para[7] = sceneId & 0xFF;
	vrts_CMD_STRUCTURE.para[8] = (sceneId >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[9] = appID & 0xFF;
	vrts_CMD_STRUCTURE.para[10] = (appID >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[11] = SrgbID;
	int i;
	for (i = 0; i < 7; i++) {
		vrts_CMD_STRUCTURE.para[i + 12] = 0;
	}
}

static void SetSceneForRemote_AC(uint16_t adrRemote_AC, uint8_t buttonId,
		uint8_t modeId, uint16_t sceneId, uint16_t appID, uint8_t SrgbID) {
	vrts_CMD_STRUCTURE.adr_dst[0] = adrRemote_AC & 0xFF;
	vrts_CMD_STRUCTURE.adr_dst[1] = (adrRemote_AC >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.opCode[0] = RD_OPCODE_SCENE_SEND & 0xFF;
	vrts_CMD_STRUCTURE.opCode[1] = (VENDOR_ID) & 0xFF;
	vrts_CMD_STRUCTURE.para[0] = (VENDOR_ID >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[1] = STATUS_CMD_SCENE & 0xFF;
	vrts_CMD_STRUCTURE.para[2] = (STATUS_CMD_SCENE >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[3] = (HEADER_SCENE_REMOTE_AC_SET) & 0xFF;
	vrts_CMD_STRUCTURE.para[4] = HEADER_SCENE_REMOTE_AC_SET >> 8 & 0xFF;
	vrts_CMD_STRUCTURE.para[5] = buttonId;
	vrts_CMD_STRUCTURE.para[6] = modeId;
	vrts_CMD_STRUCTURE.para[7] = sceneId & 0xFF;
	vrts_CMD_STRUCTURE.para[8] = (sceneId >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[9] = appID & 0xFF;
	vrts_CMD_STRUCTURE.para[10] = (appID >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[11] = SrgbID;
	int i;
	for (i = 0; i < 7; i++) {
		vrts_CMD_STRUCTURE.para[i + 12] = 0;
	}
}

static void DelSceneForRemote_DC(uint16_t adrRemote_DC, uint8_t buttonId,
		uint8_t modeId) {
	vrts_CMD_STRUCTURE.adr_dst[0] = adrRemote_DC & 0xFF;
	vrts_CMD_STRUCTURE.adr_dst[1] = (adrRemote_DC >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.opCode[0] = RD_OPCODE_SCENE_SEND & 0xFF;
	vrts_CMD_STRUCTURE.opCode[1] = (VENDOR_ID) & 0xFF;
	vrts_CMD_STRUCTURE.para[0] = (VENDOR_ID >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[1] = STATUS_CMD_SCENE & 0xFF;
	vrts_CMD_STRUCTURE.para[2] = (STATUS_CMD_SCENE >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[3] = (HEADER_SCENE_REMOTE_DC_DEL) & 0xFF;
	vrts_CMD_STRUCTURE.para[4] = HEADER_SCENE_REMOTE_DC_DEL >> 8 & 0xFF;
	vrts_CMD_STRUCTURE.para[5] = buttonId;
	vrts_CMD_STRUCTURE.para[6] = modeId;
}

static void DelSceneForRemote_AC(uint16_t adrRemote_AC, uint8_t buttonId,
		uint8_t modeId) {
	vrts_CMD_STRUCTURE.adr_dst[0] = adrRemote_AC & 0xFF;
	vrts_CMD_STRUCTURE.adr_dst[1] = (adrRemote_AC >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.opCode[0] = RD_OPCODE_SCENE_SEND & 0xFF;
	vrts_CMD_STRUCTURE.opCode[1] = (VENDOR_ID) & 0xFF;
	vrts_CMD_STRUCTURE.para[0] = (VENDOR_ID >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[1] = STATUS_CMD_SCENE & 0xFF;
	vrts_CMD_STRUCTURE.para[2] = (STATUS_CMD_SCENE >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[3] = (HEADER_SCENE_REMOTE_AC_DEL) & 0xFF;
	vrts_CMD_STRUCTURE.para[4] = HEADER_SCENE_REMOTE_AC_DEL >> 8 & 0xFF;
	vrts_CMD_STRUCTURE.para[5] = buttonId;
	vrts_CMD_STRUCTURE.para[6] = modeId;
}

static void SetSceneForSensor_LightPir(uint16_t adrLightPir, uint16_t sceneID,
		RD_Sensor_data_tdef dataScene_Pir_light) {
	vrts_CMD_STRUCTURE.adr_dst[0] = adrLightPir & 0xFF;
	vrts_CMD_STRUCTURE.adr_dst[1] = (adrLightPir >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.opCode[0] = RD_OPCODE_SCENE_SEND & 0xFF;
	vrts_CMD_STRUCTURE.opCode[1] = (VENDOR_ID) & 0xFF;
	vrts_CMD_STRUCTURE.para[0] = (VENDOR_ID >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[1] = STATUS_CMD_SCENE & 0xFF;
	vrts_CMD_STRUCTURE.para[2] = (STATUS_CMD_SCENE >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[3] = (HEADER_SCENE_LIGHT_PIR_SET) & 0xFF;
	vrts_CMD_STRUCTURE.para[4] = (HEADER_SCENE_LIGHT_PIR_SET >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[5] = sceneID & 0xFF;
	vrts_CMD_STRUCTURE.para[6] = (sceneID >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[7] = (dataScene_Pir_light.data >> 24) & 0xFF;
	vrts_CMD_STRUCTURE.para[8] = (dataScene_Pir_light.data >> 16) & 0xFF;
	vrts_CMD_STRUCTURE.para[9] = (dataScene_Pir_light.data >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[10] = 0x00;
}

static void DelSceneForSensor_LightPir(uint16_t adrLightPir, uint16_t sceneID) {
	vrts_CMD_STRUCTURE.adr_dst[0] = adrLightPir & 0xFF;
	vrts_CMD_STRUCTURE.adr_dst[1] = (adrLightPir >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.opCode[0] = RD_OPCODE_SCENE_SEND & 0xFF;
	vrts_CMD_STRUCTURE.opCode[1] = VENDOR_ID & 0xFF;
	vrts_CMD_STRUCTURE.para[0] = (VENDOR_ID >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[1] = STATUS_CMD_SCENE & 0xFF;
	vrts_CMD_STRUCTURE.para[2] = (STATUS_CMD_SCENE >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[3] = (HEADER_SCENE_LIGHT_PIR_DEL) & 0xFF;
	vrts_CMD_STRUCTURE.para[4] = (HEADER_SCENE_LIGHT_PIR_DEL >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[5] = sceneID & 0xFF;
	vrts_CMD_STRUCTURE.para[6] = (sceneID >> 8) & 0xFF;
}

static void SetSceneForSensor_Light(uint16_t adrLight, uint16_t sceneID,
		uint16_t condition, uint16_t low_lux, uint16_t hight_lux,
		uint8_t srgbID) {
	vrts_CMD_STRUCTURE.adr_dst[0] = adrLight & 0xFF;
	vrts_CMD_STRUCTURE.adr_dst[1] = (adrLight >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.opCode[0] = RD_OPCODE_SCENE_SEND & 0xFF;
	vrts_CMD_STRUCTURE.opCode[1] = (VENDOR_ID) & 0xFF;
	vrts_CMD_STRUCTURE.para[0] = (VENDOR_ID >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[1] = STATUS_CMD_SCENE & 0xFF;
	vrts_CMD_STRUCTURE.para[2] = (STATUS_CMD_SCENE >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[3] = (HEADER_SCENE_LIGHT_SENSOR_SET) & 0xFF;
	vrts_CMD_STRUCTURE.para[4] = (HEADER_SCENE_LIGHT_SENSOR_SET >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[5] = sceneID & 0xFF;
	vrts_CMD_STRUCTURE.para[6] = (sceneID >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[7] = (condition >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[8] = condition & 0xFF;
	vrts_CMD_STRUCTURE.para[9] = low_lux & 0xFF;
	vrts_CMD_STRUCTURE.para[10] = (low_lux >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[11] = hight_lux & 0xFF;
	vrts_CMD_STRUCTURE.para[12] = (hight_lux >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[13] = srgbID;
}

static void Balance_Lightness(uint16_t adrLight, uint16_t lightness, uint16_t destinantion){
	vrts_CMD_STRUCTURE.adr_dst[0] = adrLight & 0xFF;
	vrts_CMD_STRUCTURE.adr_dst[1] = (adrLight>>8) & 0xFF;
	vrts_CMD_STRUCTURE.opCode[0] = RD_OPCODE_SCENE_SEND & 0xFF;
	vrts_CMD_STRUCTURE.opCode[1] = (VENDOR_ID) & 0xFF;
	vrts_CMD_STRUCTURE.para[0] = (VENDOR_ID>>8) & 0xFF;
	vrts_CMD_STRUCTURE.para[1] = STATUS_CMD_SCENE & 0xFF;
	vrts_CMD_STRUCTURE.para[2] = (STATUS_CMD_SCENE>>8) & 0xFF;
	vrts_CMD_STRUCTURE.para[3] = (HEADER_SCENE_LIGHT_SENSOR_SET_BALANCE) & 0xFF;
	vrts_CMD_STRUCTURE.para[4] = (HEADER_SCENE_LIGHT_SENSOR_SET_BALANCE>>8) & 0xFF;
	vrts_CMD_STRUCTURE.para[5] = lightness & 0xFF;
	vrts_CMD_STRUCTURE.para[6] = (lightness >>8) & 0xFF;
	vrts_CMD_STRUCTURE.para[7] = (destinantion) & 0xFF;
	vrts_CMD_STRUCTURE.para[8] = (destinantion>>8) & 0xFF;
}

static void DelBalance_Lightness(uint16_t adrLight) {
	vrts_CMD_STRUCTURE.adr_dst[0] = adrLight & 0xFF;
	vrts_CMD_STRUCTURE.adr_dst[1] = (adrLight >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.opCode[0] = RD_OPCODE_SCENE_SEND & 0xFF;
	vrts_CMD_STRUCTURE.opCode[1] = (VENDOR_ID) & 0xFF;
	vrts_CMD_STRUCTURE.para[0] = (VENDOR_ID >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[1] = STATUS_CMD_SCENE & 0xFF;
	vrts_CMD_STRUCTURE.para[2] = (STATUS_CMD_SCENE >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[3] = (HEADER_SCENE_LIGHT_SENSOR_DEL) & 0xFF;
	vrts_CMD_STRUCTURE.para[4] = (HEADER_SCENE_LIGHT_SENSOR_DEL >> 8) & 0xFF;
}

static void DelSceneForSensor_Light(uint16_t adrLight, uint16_t sceneID) {
	vrts_CMD_STRUCTURE.adr_dst[0] = adrLight & 0xFF;
	vrts_CMD_STRUCTURE.adr_dst[1] = (adrLight >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.opCode[0] = RD_OPCODE_SCENE_SEND & 0xFF;
	vrts_CMD_STRUCTURE.opCode[1] = (VENDOR_ID) & 0xFF;
	vrts_CMD_STRUCTURE.para[0] = (VENDOR_ID >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[1] = STATUS_CMD_SCENE & 0xFF;
	vrts_CMD_STRUCTURE.para[2] = (STATUS_CMD_SCENE >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[3] = (HEADER_SCENE_LIGHT_SENSOR_DEL) & 0xFF;
	vrts_CMD_STRUCTURE.para[4] = (HEADER_SCENE_LIGHT_SENSOR_DEL >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[5] = sceneID & 0xFF;
	vrts_CMD_STRUCTURE.para[6] = (sceneID >> 8) & 0xFF;
}

static void SetSceneForSensor_Pir(uint16_t adrPir, uint16_t sceneID,
		uint16_t condition, uint8_t srgbID) {
	vrts_CMD_STRUCTURE.adr_dst[0] = adrPir & 0xFF;
	vrts_CMD_STRUCTURE.adr_dst[1] = (adrPir >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.opCode[0] = RD_OPCODE_SCENE_SEND & 0xFF;
	vrts_CMD_STRUCTURE.opCode[1] = (VENDOR_ID) & 0xFF;
	vrts_CMD_STRUCTURE.para[0] = (VENDOR_ID >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[1] = STATUS_CMD_SCENE & 0xFF;
	vrts_CMD_STRUCTURE.para[2] = (STATUS_CMD_SCENE >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[3] = (HEADER_SCENE_PIR_SENSOR_SET) & 0xFF;
	vrts_CMD_STRUCTURE.para[4] = (HEADER_SCENE_PIR_SENSOR_SET >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[5] = sceneID & 0xFF;
	vrts_CMD_STRUCTURE.para[6] = (sceneID >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[7] = (condition >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[8] = condition & 0xFF;
	vrts_CMD_STRUCTURE.para[9] = srgbID;
}

static void DelSceneForSensor_Pir(uint16_t adrPir, uint16_t sceneID) {
	vrts_CMD_STRUCTURE.adr_dst[0] = adrPir & 0xFF;
	vrts_CMD_STRUCTURE.adr_dst[1] = (adrPir >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.opCode[0] = RD_OPCODE_SCENE_SEND & 0xFF;
	vrts_CMD_STRUCTURE.opCode[1] = (VENDOR_ID) & 0xFF;
	vrts_CMD_STRUCTURE.para[0] = (VENDOR_ID >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[1] = STATUS_CMD_SCENE & 0xFF;
	vrts_CMD_STRUCTURE.para[2] = (STATUS_CMD_SCENE >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[3] = (HEADER_SCENE_PIR_SENSOR_DEL) & 0xFF;
	vrts_CMD_STRUCTURE.para[4] = (HEADER_SCENE_PIR_SENSOR_DEL >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[5] = sceneID & 0xFF;
	vrts_CMD_STRUCTURE.para[6] = (sceneID >> 8) & 0xFF;
}

static void SetActionTime_PIRSensor(uint16_t adrPir,uint16_t time){
	vrts_CMD_STRUCTURE.adr_dst[0] = adrPir & 0xFF;
	vrts_CMD_STRUCTURE.adr_dst[1] = (adrPir>>8) & 0xFF;
	vrts_CMD_STRUCTURE.opCode[0] = RD_OPCODE_SCENE_SEND & 0xFF;
	vrts_CMD_STRUCTURE.opCode[1] = VENDOR_ID & 0xFF;
	vrts_CMD_STRUCTURE.para[0] = (VENDOR_ID>>8) & 0xFF;
	vrts_CMD_STRUCTURE.para[1] = STATUS_CMD_SCENE & 0xFF;
	vrts_CMD_STRUCTURE.para[2] = (STATUS_CMD_SCENE>>8) & 0xFF;
	vrts_CMD_STRUCTURE.para[3] = (HEADER_SCENE_PIR_SENSOR_TIMEACTION) & 0xFF;
	vrts_CMD_STRUCTURE.para[4] = (HEADER_SCENE_PIR_SENSOR_TIMEACTION>>8) & 0xFF;
	vrts_CMD_STRUCTURE.para[5] = time & 0xFF;
	vrts_CMD_STRUCTURE.para[6] =(time>>8) & 0xFF;
}

static void SetSceneForDoorSensor(uint16_t adrDoorSensor, uint16_t sceneID,
		uint8_t statusDoor, uint8_t srgbID) {
	vrts_CMD_STRUCTURE.adr_dst[0] = adrDoorSensor & 0xFF;
	vrts_CMD_STRUCTURE.adr_dst[1] = (adrDoorSensor >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.opCode[0] = RD_OPCODE_SCENE_SEND & 0xFF;
	vrts_CMD_STRUCTURE.opCode[1] = VENDOR_ID & 0xFF;
	vrts_CMD_STRUCTURE.para[0] = (VENDOR_ID >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[1] = STATUS_CMD_SCENE & 0xFF;
	vrts_CMD_STRUCTURE.para[2] = (STATUS_CMD_SCENE >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[3] = HEADER_SCENE_DOOR_SENSOR_SET & 0xFF;
	vrts_CMD_STRUCTURE.para[4] = (HEADER_SCENE_DOOR_SENSOR_SET >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[5] = sceneID & 0xFF;
	vrts_CMD_STRUCTURE.para[6] = (sceneID >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[7] = statusDoor & 0xFF;
	vrts_CMD_STRUCTURE.para[8] = srgbID & 0xFF;
}

static void DelSceneForDoorSensor(uint16_t adrDoorSensor, uint16_t sceneID){
	vrts_CMD_STRUCTURE.adr_dst[0] = adrDoorSensor & 0xFF;
	vrts_CMD_STRUCTURE.adr_dst[1] = (adrDoorSensor >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.opCode[0] = RD_OPCODE_SCENE_SEND & 0xFF;
	vrts_CMD_STRUCTURE.opCode[1] = VENDOR_ID & 0xFF;
	vrts_CMD_STRUCTURE.para[0] = (VENDOR_ID >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[1] = STATUS_CMD_SCENE & 0xFF;
	vrts_CMD_STRUCTURE.para[2] = (STATUS_CMD_SCENE >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[3] = (HEADER_SCENE_DOOR_SENSOR_DEL) & 0xFF;
	vrts_CMD_STRUCTURE.para[4] = (HEADER_SCENE_DOOR_SENSOR_DEL >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[5] = sceneID & 0xFF;
	vrts_CMD_STRUCTURE.para[6] = (sceneID >> 8) & 0xFF;
}

static void ControlSwitch4(uint16_t adrSwitch4, uint16_t switch1_2,
		uint16_t switch3_4) {
	vrts_CMD_STRUCTURE.adr_dst[0] = adrSwitch4 & 0xFF;
	vrts_CMD_STRUCTURE.adr_dst[1] = (adrSwitch4 >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.opCode[0] = RD_OPCODE_SCENE_SEND & 0xFF;
	vrts_CMD_STRUCTURE.opCode[1] = VENDOR_ID & 0xFF;
	vrts_CMD_STRUCTURE.para[0] = (VENDOR_ID >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[1] = STATUS_CMD_SCENE & 0xFF;
	vrts_CMD_STRUCTURE.para[2] = (STATUS_CMD_SCENE >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[3] = (HEADER_CONTROL_SWITCH4) & 0xFF;
	vrts_CMD_STRUCTURE.para[4] = (HEADER_CONTROL_SWITCH4 >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[5] = (switch1_2 >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[6] = switch1_2 & 0xFF;
	vrts_CMD_STRUCTURE.para[7] = (switch3_4 >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[8] = switch3_4 & 0xFF;
}

static void SetSceneForSwitch4(uint16_t adrSwitch4, uint16_t switch1_2,
		uint16_t switch3_4, uint16_t sceneID) {
	vrts_CMD_STRUCTURE.adr_dst[0] = adrSwitch4 & 0xFF;
	vrts_CMD_STRUCTURE.adr_dst[1] = (adrSwitch4 >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.opCode[0] = RD_OPCODE_SCENE_SEND & 0xFF;
	vrts_CMD_STRUCTURE.opCode[1] = VENDOR_ID & 0xFF;
	vrts_CMD_STRUCTURE.para[0] = (VENDOR_ID >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[1] = STATUS_CMD_SCENE & 0xFF;
	vrts_CMD_STRUCTURE.para[2] = (STATUS_CMD_SCENE >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[3] = (HEADER_SCENE_SWITCH4_SET) & 0xFF;
	vrts_CMD_STRUCTURE.para[4] = (HEADER_SCENE_SWITCH4_SET >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[5] = sceneID & 0xFF;
	vrts_CMD_STRUCTURE.para[6] = (sceneID >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[7] = (switch1_2 >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[8] = switch1_2 & 0xFF;
	vrts_CMD_STRUCTURE.para[9] = (switch3_4 >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[10] = switch3_4 & 0xFF;
}

static void DelSceneForSwitch4(uint16_t adrSwitch4, uint16_t sceneID) {
	vrts_CMD_STRUCTURE.adr_dst[0] = adrSwitch4 & 0xFF;
	vrts_CMD_STRUCTURE.adr_dst[1] = (adrSwitch4 >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.opCode[0] = RD_OPCODE_SCENE_SEND & 0xFF;
	vrts_CMD_STRUCTURE.opCode[1] = VENDOR_ID & 0xFF;
	vrts_CMD_STRUCTURE.para[0] = (VENDOR_ID >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[1] = STATUS_CMD_SCENE & 0xFF;
	vrts_CMD_STRUCTURE.para[2] = (STATUS_CMD_SCENE >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[3] = (HEADER_SCENE_SWITCH4_DEL) & 0xFF;
	vrts_CMD_STRUCTURE.para[4] = (HEADER_SCENE_SWITCH4_DEL >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[5] = (sceneID) & 0xFF;
	vrts_CMD_STRUCTURE.para[6] = (sceneID >> 8) & 0xFF;
}

static void SetSceneForSocket1(uint16_t adrSocket1, uint16_t socket1_2,
		uint16_t socket3_4, uint16_t sceneID) {
	vrts_CMD_STRUCTURE.adr_dst[0] = adrSocket1 & 0xFF;
	vrts_CMD_STRUCTURE.adr_dst[1] = (adrSocket1 >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.opCode[0] = RD_OPCODE_SCENE_SEND & 0xFF;
	vrts_CMD_STRUCTURE.opCode[1] = VENDOR_ID & 0xFF;
	vrts_CMD_STRUCTURE.para[0] = (VENDOR_ID >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[1] = STATUS_CMD_SCENE & 0xFF;
	vrts_CMD_STRUCTURE.para[2] = (STATUS_CMD_SCENE >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[3] = (HEADER_SCENE_SOCKET1_SET) & 0xFF;
	vrts_CMD_STRUCTURE.para[4] = (HEADER_SCENE_SOCKET1_SET >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[5] = (socket1_2 >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[6] = socket1_2 & 0xFF;
	vrts_CMD_STRUCTURE.para[7] = (socket3_4 >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[8] = socket3_4 & 0xFF;
	vrts_CMD_STRUCTURE.para[9] = sceneID & 0xFF;
	vrts_CMD_STRUCTURE.para[10] = (sceneID >> 8) & 0xFF;
}

static void DelSceneForSocket1(uint16_t adrSocket, uint16_t sceneID) {
	vrts_CMD_STRUCTURE.adr_dst[0] = adrSocket & 0xFF;
	vrts_CMD_STRUCTURE.adr_dst[1] = (adrSocket >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.opCode[0] = RD_OPCODE_SCENE_SEND & 0xFF;
	vrts_CMD_STRUCTURE.opCode[1] = VENDOR_ID & 0xFF;
	vrts_CMD_STRUCTURE.para[0] = (VENDOR_ID >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[1] = STATUS_CMD_SCENE & 0xFF;
	vrts_CMD_STRUCTURE.para[2] = (STATUS_CMD_SCENE >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[3] = (HEADER_SCENE_SOCKET1_DEL) & 0xFF;
	vrts_CMD_STRUCTURE.para[4] = (HEADER_SCENE_SOCKET1_DEL >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[5] = (sceneID) & 0xFF;
	vrts_CMD_STRUCTURE.para[6] = (sceneID >> 8) & 0xFF;
}

static void SceneForTouchScreen(uint16_t adrTouchScreen, uint8_t buttonID,
		uint16_t sceneID, uint8_t srgbID) {
	vrts_CMD_STRUCTURE.adr_dst[0] = adrTouchScreen & 0xFF;
	vrts_CMD_STRUCTURE.adr_dst[1] = (adrTouchScreen >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.opCode[0] = RD_OPCODE_SCENE_SEND & 0xFF;
	vrts_CMD_STRUCTURE.opCode[1] = (VENDOR_ID) & 0xFF;
	vrts_CMD_STRUCTURE.para[0] = (VENDOR_ID >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[1] = STATUS_CMD_SCENE & 0xFF;
	vrts_CMD_STRUCTURE.para[2] = (STATUS_CMD_SCENE >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[3] = (HEADER_SCENE_SCREENT_SET) & 0xFF;
	vrts_CMD_STRUCTURE.para[4] = HEADER_SCENE_SCREENT_SET >> 8 & 0xFF;
	vrts_CMD_STRUCTURE.para[5] = buttonID;
	vrts_CMD_STRUCTURE.para[6] = sceneID & 0xFF;
	vrts_CMD_STRUCTURE.para[7] = (sceneID >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[8] = (srgbID) & 0xFF;
}

static void DelSceneForTouchScreen(uint16_t adrTouchScreen, uint8_t buttonID) {
	vrts_CMD_STRUCTURE.adr_dst[0] = adrTouchScreen & 0xFF;
	vrts_CMD_STRUCTURE.adr_dst[1] = (adrTouchScreen >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.opCode[0] = RD_OPCODE_SCENE_SEND & 0xFF;
	vrts_CMD_STRUCTURE.opCode[1] = (VENDOR_ID) & 0xFF;
	vrts_CMD_STRUCTURE.para[0] = (VENDOR_ID >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[1] = STATUS_CMD_SCENE & 0xFF;
	vrts_CMD_STRUCTURE.para[2] = (STATUS_CMD_SCENE >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[3] = (HEADER_SCENE_SCREENT_DEL) & 0xFF;
	vrts_CMD_STRUCTURE.para[4] = HEADER_SCENE_SCREENT_DEL >> 8 & 0xFF;
	vrts_CMD_STRUCTURE.para[5] = buttonID;
}

static void SendTempHumForScreenT(uint16_t adrScreenT, uint16_t temp,
		uint16_t hum) {
	vrts_CMD_STRUCTURE.adr_dst[0] = adrScreenT & 0xFF;
	vrts_CMD_STRUCTURE.adr_dst[1] = (adrScreenT >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.opCode[0] = RD_OPCODE_SCENE_SEND & 0xFF;
	vrts_CMD_STRUCTURE.opCode[1] = VENDOR_ID & 0xFF;
	vrts_CMD_STRUCTURE.para[0] = (VENDOR_ID >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[1] = STATUS_CMD_SCENE & 0xFF;
	vrts_CMD_STRUCTURE.para[2] = (STATUS_CMD_SCENE >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[3] = (HEADER_TEMPHUM_SCREENTOUCH) & 0xFF;
	vrts_CMD_STRUCTURE.para[4] = (HEADER_TEMPHUM_SCREENTOUCH >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[5] = (temp >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[6] = temp & 0xFF;
	vrts_CMD_STRUCTURE.para[7] = (hum >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[8] = hum & 0xFF;
}

static void SendTimeForScreenT(uint16_t adrScreenT, uint8_t hours,
		uint8_t minute) {
	vrts_CMD_STRUCTURE.adr_dst[0] = adrScreenT & 0xFF;
	vrts_CMD_STRUCTURE.adr_dst[1] = (adrScreenT >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.opCode[0] = RD_OPCODE_SCENE_SEND & 0xFF;
	vrts_CMD_STRUCTURE.opCode[1] = VENDOR_ID & 0xFF;
	vrts_CMD_STRUCTURE.para[0] = (VENDOR_ID >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[1] = STATUS_CMD_SCENE & 0xFF;
	vrts_CMD_STRUCTURE.para[2] = (STATUS_CMD_SCENE >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[3] = (HEADER_TIME_SCREENTOUCH) & 0xFF;
	vrts_CMD_STRUCTURE.para[4] = (HEADER_TIME_SCREENTOUCH >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[5] = hours & 0xFF;
	vrts_CMD_STRUCTURE.para[6] = minute & 0xFF;
}

static void AskTypeDevice(uint16_t adr) {
	vrts_CMD_STRUCTURE.adr_dst[0] = adr & 0xFF;
	vrts_CMD_STRUCTURE.adr_dst[1] = (adr >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.opCode[0] = RD_OPCODE_TYPE_SEND & 0xFF;
	vrts_CMD_STRUCTURE.opCode[1] = VENDOR_ID & 0xFF;
	vrts_CMD_STRUCTURE.para[0] = (VENDOR_ID >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[1] = STATUS_CMD_TYPE & 0xFF;
	vrts_CMD_STRUCTURE.para[2] = (STATUS_CMD_TYPE >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[3] = (HEADER_TYPE_ASK) & 0xFF;
	vrts_CMD_STRUCTURE.para[4] = (HEADER_TYPE_ASK >> 8) & 0xFF;
	int i;
	for (i = 0; i < 11; i++) {
		vrts_CMD_STRUCTURE.para[i + 5] = 0x00;
	}
}

static void SetTypeDevice(uint16_t adrSetTypeDevice, uint8_t type,
		uint8_t attrubute, uint8_t application) {
	vrts_CMD_STRUCTURE.adr_dst[0] = adrSetTypeDevice & 0xFF;
	vrts_CMD_STRUCTURE.adr_dst[1] = (adrSetTypeDevice >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.opCode[0] = RD_OPCODE_TYPE_SEND & 0xFF;
	vrts_CMD_STRUCTURE.opCode[1] = VENDOR_ID & 0xFF;
	vrts_CMD_STRUCTURE.para[0] = (VENDOR_ID >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[1] = STATUS_CMD_TYPE & 0xFF;
	vrts_CMD_STRUCTURE.para[2] = (STATUS_CMD_TYPE >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[3] = (HEADER_TYPE_SET) & 0xFF;
	vrts_CMD_STRUCTURE.para[4] = (HEADER_TYPE_SET >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[5] = type & 0xFF;
	vrts_CMD_STRUCTURE.para[6] = attrubute & 0xFF;
	vrts_CMD_STRUCTURE.para[7] = application & 0xFF;
	int i;
	for (i = 0; i < 8; i++) {
		vrts_CMD_STRUCTURE.para[i + 8] = 0x00;
	}
}

static void SaveGateway(uint16_t adrSaveGateway) {
	vrts_CMD_STRUCTURE.adr_dst[0] = adrSaveGateway & 0xFF;
	vrts_CMD_STRUCTURE.adr_dst[1] = (adrSaveGateway >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.opCode[0] = RD_OPCODE_TYPE_SEND & 0xFF;
	vrts_CMD_STRUCTURE.opCode[1] = VENDOR_ID & 0xFF;
	vrts_CMD_STRUCTURE.para[0] = (VENDOR_ID >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[1] = STATUS_CMD_TYPE & 0xFF;
	vrts_CMD_STRUCTURE.para[2] = (STATUS_CMD_TYPE >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[3] = (HEADER_TYPE_SAVEGW) & 0xFF;
	vrts_CMD_STRUCTURE.para[4] = (HEADER_TYPE_SAVEGW >> 8) & 0xFF;
	int i;
	for (i = 0; i < 11; i++) {
		vrts_CMD_STRUCTURE.para[i + 5] = 0x00;
	}
}

static void AskPm(uint16_t adrPmSensor) {
	vrts_CMD_STRUCTURE.adr_dst[0] = adrPmSensor & 0xFF;
	vrts_CMD_STRUCTURE.adr_dst[1] = (adrPmSensor >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.opCode[0] = RD_OPCODE_SCENE_SEND & 0xFF;
	vrts_CMD_STRUCTURE.opCode[1] = VENDOR_ID & 0xFF;
	vrts_CMD_STRUCTURE.para[0] = (VENDOR_ID >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[1] = STATUS_CMD_SCENE & 0xFF;
	vrts_CMD_STRUCTURE.para[2] = (STATUS_CMD_SCENE >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[3] = (HEADER_ASK_PM) & 0xFF;
	vrts_CMD_STRUCTURE.para[4] = (HEADER_ASK_PM >> 8) & 0xFF;
}

static void ControlCurtains(uint16_t adrCurtain, uint8_t controlStt,
		uint8_t percentOpen) {
	vrts_CMD_STRUCTURE.adr_dst[0] = adrCurtain & 0xFF;
	vrts_CMD_STRUCTURE.adr_dst[1] = (adrCurtain >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.opCode[0] = RD_OPCODE_SCENE_SEND & 0xFF;
	vrts_CMD_STRUCTURE.opCode[1] = VENDOR_ID & 0xFF;
	vrts_CMD_STRUCTURE.para[0] = (VENDOR_ID >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[1] = STATUS_CMD_SCENE & 0xFF;
	vrts_CMD_STRUCTURE.para[2] = (STATUS_CMD_SCENE >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[3] = HEADER_CONTROL_CURTAIN & 0xFF;
	vrts_CMD_STRUCTURE.para[4] = (HEADER_CONTROL_CURTAIN >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[5] = controlStt & 0xFF;
	vrts_CMD_STRUCTURE.para[6] = percentOpen & 0xFF;
}

static void SetSceneCurtains(uint16_t adrCurtain, uint16_t sceneID) {
	vrts_CMD_STRUCTURE.adr_dst[0] = adrCurtain & 0xFF;
	vrts_CMD_STRUCTURE.adr_dst[1] = (adrCurtain >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.opCode[0] = RD_OPCODE_SCENE_SEND & 0xFF;
	vrts_CMD_STRUCTURE.opCode[1] = VENDOR_ID & 0xFF;
	vrts_CMD_STRUCTURE.para[0] = (VENDOR_ID >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[1] = STATUS_CMD_SCENE & 0xFF;
	vrts_CMD_STRUCTURE.para[2] = (STATUS_CMD_SCENE >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[3] = HEADER_SET_SCENE_CURTAIN & 0xFF;
	vrts_CMD_STRUCTURE.para[4] = (HEADER_SET_SCENE_CURTAIN >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[5] = sceneID & 0xFF;
	vrts_CMD_STRUCTURE.para[6] = (sceneID >> 8) & 0xFF;
}

static void DelSceneCurtains(uint16_t adrCurtain, uint16_t sceneID) {
	vrts_CMD_STRUCTURE.adr_dst[0] = adrCurtain & 0xFF;
	vrts_CMD_STRUCTURE.adr_dst[1] = (adrCurtain >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.opCode[0] = RD_OPCODE_SCENE_SEND & 0xFF;
	vrts_CMD_STRUCTURE.opCode[1] = VENDOR_ID & 0xFF;
	vrts_CMD_STRUCTURE.para[0] = (VENDOR_ID >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[1] = STATUS_CMD_SCENE & 0xFF;
	vrts_CMD_STRUCTURE.para[2] = (STATUS_CMD_SCENE >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[3] = HEADER_DEL_SCENE_CURTAIN & 0xFF;
	vrts_CMD_STRUCTURE.para[4] = (HEADER_DEL_SCENE_CURTAIN >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[5] = sceneID & 0xFF;
	vrts_CMD_STRUCTURE.para[6] = (sceneID >> 8) & 0xFF;
}

static void AskStatusCurtain(uint16_t adrCurtain) {
	vrts_CMD_STRUCTURE.adr_dst[0] = adrCurtain & 0xFF;
	vrts_CMD_STRUCTURE.adr_dst[1] = (adrCurtain >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.opCode[0] = RD_OPCODE_SCENE_SEND & 0xFF;
	vrts_CMD_STRUCTURE.opCode[1] = VENDOR_ID & 0xFF;
	vrts_CMD_STRUCTURE.para[0] = (VENDOR_ID >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[1] = STATUS_CMD_SCENE & 0xFF;
	vrts_CMD_STRUCTURE.para[2] = (STATUS_CMD_SCENE >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[3] = HEADER_ASK_STATUS_CURTAIN & 0xFF;
	vrts_CMD_STRUCTURE.para[4] = (HEADER_ASK_STATUS_CURTAIN >> 8) & 0xFF;
}

static void DimPercent(uint16_t adrDimPercent, uint8_t percent) {
	vrts_CMD_STRUCTURE.adr_dst[0] = adrDimPercent & 0xFF;
	vrts_CMD_STRUCTURE.adr_dst[1] = (adrDimPercent >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.opCode[0] = RD_OPCODE_SCENE_SEND & 0xFF;
	vrts_CMD_STRUCTURE.opCode[1] = VENDOR_ID & 0xFF;
	vrts_CMD_STRUCTURE.para[0] = (VENDOR_ID >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[1] = STATUS_CMD_SCENE & 0xFF;
	vrts_CMD_STRUCTURE.para[2] = (STATUS_CMD_SCENE >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[3] = HEADER_DIM_PERCENT & 0xFF;
	vrts_CMD_STRUCTURE.para[4] = (HEADER_DIM_PERCENT >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[5] = percent;
}
static void CmdResetAll(){
	vrts_CMD_STRUCTURE.adr_dst[0] = 0xFF;
	vrts_CMD_STRUCTURE.adr_dst[1] = 0xFF;
	vrts_CMD_STRUCTURE.opCode[0] = RD_OPCODE_TYPE_SEND & 0xFF;
	vrts_CMD_STRUCTURE.opCode[1] = VENDOR_ID & 0xFF;
	vrts_CMD_STRUCTURE.para[0] = (VENDOR_ID >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[1] = STATUS_CMD_SCENE & 0xFF;
	vrts_CMD_STRUCTURE.para[2] = (STATUS_CMD_SCENE >> 8) & 0xFF;
	vrts_CMD_STRUCTURE.para[3] = HEADER_RESET_ALL & 0xFF;
	vrts_CMD_STRUCTURE.para[4] = (HEADER_RESET_ALL >> 8) & 0xFF;
}

void Function_Vendor(uint16_t cmd, functionTypeDef Func_vendor, uint16_t adr,
		uint16_t header_destination, uint8_t buttonID_controlCurtain,
		uint8_t modeID_percentOpen, uint8_t status_door,
		uint16_t condition_lightness, uint16_t low_lux_switch1_2_socket1_2,
		uint16_t hight_lux_switch3_4_socket3_4, uint16_t temp, uint16_t hum,
		uint16_t sceneID, uint16_t appID, uint8_t srgbID, uint8_t type_hours,
		uint8_t attrubute_minute, uint8_t application_second,
		uint16_t transition_par_t, uint16_t cmdLength) {
	vrts_CMD_STRUCTURE.HCI_CMD_GATEWAY[0]= cmd & 0xFF;
	vrts_CMD_STRUCTURE.HCI_CMD_GATEWAY[1]= (cmd>>8) & 0xFF;
	vrts_CMD_STRUCTURE.opCode00[0] = vrts_CMD_STRUCTURE.opCode00[1]= \
	vrts_CMD_STRUCTURE.opCode00[2]= vrts_CMD_STRUCTURE.opCode00[3]=0;
	vrts_CMD_STRUCTURE.retry_cnt = parRetry_cnt;
	vrts_CMD_STRUCTURE.rsp_max = parRsp_Max;
	uint16_t timeWait = TIMEWAIT;
	if(Func_vendor == SceneForRemote_DC_vendor_typedef){
		SetSceneForRemote_DC(adr, buttonID_controlCurtain, modeID_percentOpen, sceneID, appID, srgbID);
		timeWait = TIMEWAIT_REMOTE;
	}
	else if(Func_vendor == DelSceneForRemote_DC_vendor_typedef){
		DelSceneForRemote_DC(adr, buttonID_controlCurtain, modeID_percentOpen);
		timeWait = TIMEWAIT_REMOTE;
	}
	else if(Func_vendor == SceneForRemote_AC_vendor_typedef){
		SetSceneForRemote_AC(adr, buttonID_controlCurtain, modeID_percentOpen, sceneID, appID, srgbID);
		timeWait = TIMEWAIT_REMOTE;
	}
	else if(Func_vendor == DelSceneForRemote_AC_vendor_typedef){
		DelSceneForRemote_AC(adr, buttonID_controlCurtain, modeID_percentOpen);
		timeWait = TIMEWAIT_REMOTE;
	}
	else if(Func_vendor == SceneForSensor_LightPir_vendor_typedef){
		RD_Sensor_data_tdef dataScene_Pir_Light_cmd1;
		dataScene_Pir_Light_cmd1.Pir_Conditon = (uint32_t)(condition_lightness>>3);
		dataScene_Pir_Light_cmd1.Light_Conditon =(uint32_t)((condition_lightness)& 0x0000007);
		dataScene_Pir_Light_cmd1.Lux_low = (uint32_t)(low_lux_switch1_2_socket1_2);
		dataScene_Pir_Light_cmd1.Lux_hi = (uint32_t)(hight_lux_switch3_4_socket3_4);
		SetSceneForSensor_LightPir(adr,sceneID,dataScene_Pir_Light_cmd1);
	}
	else if(Func_vendor == DelSceneForSensor_LightPir_vendor_typedef){
		DelSceneForSensor_LightPir(adr, sceneID);
	}
	else if(Func_vendor == SceneForSensor_Light_vendor_typedef){
		SetSceneForSensor_Light(adr,sceneID, condition_lightness, low_lux_switch1_2_socket1_2, hight_lux_switch3_4_socket3_4, srgbID);
	}
	else if(Func_vendor == Balance_Lightness_vendor_typedef){
		Balance_Lightness(adr, condition_lightness, header_destination);
	}
	else if(Func_vendor == DelBalance_Lightness_vendor_typedef){
		DelBalance_Lightness(adr);
	}
	else if(Func_vendor == DelSceneForSensor_Light_vendor_typedef){
		DelSceneForSensor_Light(adr, sceneID);
	}
	else if(Func_vendor == SceneForSensor_Pir_vendor_typedef){
		SetSceneForSensor_Pir(adr,sceneID, condition_lightness, srgbID);
	}
	else if(Func_vendor == DelSceneForSensor_Pir_vendor_typedef){
		DelSceneForSensor_Pir(adr, sceneID);
	}
	else if(Func_vendor == SceneForDoorSensor_vendor_typedef){
		SetSceneForDoorSensor(adr, sceneID, status_door, srgbID);
	}
	else if(Func_vendor == DelSceneForDoorSensor_vendor_typedef){
		DelSceneForDoorSensor(adr, sceneID);
	}
	else if(Func_vendor == ControlSwitch4_vendor_typedef){
		ControlSwitch4(adr, low_lux_switch1_2_socket1_2, hight_lux_switch3_4_socket3_4);
		timeWait = TIMEWAIT_REMOTE;
	}
	else if(Func_vendor == SetSceneForSwitch4_vendor_typedef){
		SetSceneForSwitch4(adr, low_lux_switch1_2_socket1_2, hight_lux_switch3_4_socket3_4, sceneID);
	}
	else if(Func_vendor == DelSceneForSwitch4_vendor_typedef){
		DelSceneForSwitch4(adr, sceneID);
	}
	else if(Func_vendor == SetSceneForSocket1_vendor_typedef){
		SetSceneForSocket1(adr, low_lux_switch1_2_socket1_2, hight_lux_switch3_4_socket3_4, sceneID);
	}
	else if(Func_vendor == DelSceneForSocket1_vendor_typedef){
		DelSceneForSocket1(adr, sceneID);
	}
	else if(Func_vendor == SceneForScreenT_vendor_typedef){
		SceneForTouchScreen(adr, buttonID_controlCurtain, sceneID,srgbID);
	}
	else if(Func_vendor == DelSceneForScreenT_vendor_typedef){
		DelSceneForTouchScreen(adr, buttonID_controlCurtain);
	}
	else if(Func_vendor == SetTimeAction_vendor_typedef){
		SetActionTime_PIRSensor(adr,condition_lightness);
	}
	else if(Func_vendor == SendTempHumForScreenT_vendor_typedef){
		SendTempHumForScreenT(adr, temp, hum);
	}
	else if(Func_vendor == SendTimeForScreenT_vendor_typedef){
		SendTimeForScreenT(adr, type_hours, attrubute_minute);
	}
	else if(Func_vendor == SaveGateway_vendor_typedef){
		SaveGateway(adr);
		timeWait = 0;
	}
	else if(Func_vendor == AskTypeDevice_vendor_typedef){
		AskTypeDevice(adr);
		timeWait = 0;
	}
	else if(Func_vendor == SetTypeDevice_vendor_typedef){
		SetTypeDevice(adr, type_hours, attrubute_minute, application_second);
		timeWait = 0;
	}
	else if(Func_vendor == AskPm_vendor_typedef){
		AskPm(adr);
		timeWait = 0;
	}
	else if(Func_vendor == ControlCurtain_vendor_typedef){
		ControlCurtains(adr, buttonID_controlCurtain, modeID_percentOpen);
	}
	else if(Func_vendor == SetSceneCurtain_vendor_typedef){
		SetSceneCurtains(adr, sceneID);
	}
	else if(Func_vendor == DelSceneCurtain_vendor_typedef){
		DelSceneCurtains(adr, sceneID);
	}
	else if(Func_vendor == AskSceneCurtain_vendor_typedef){
		AskStatusCurtain(adr);
	}
	else if(Func_vendor == DimPercent_vendor_typedef){
		DimPercent(adr,buttonID_controlCurtain);
	}
	else if(Func_vendor == ResetAll_typedef){
		CmdResetAll();
	}
	uartSendDev_t vrts_DataUartSend;
	vrts_DataUartSend.length = cmdLength;
	vrts_DataUartSend.dataUart = vrts_CMD_STRUCTURE;
	vrts_DataUartSend.timeWait = timeWait;
	pthread_mutex_trylock(&vrpth_SendUart);
	bufferDataUart.push_back(vrts_DataUartSend);
//	head = AddTail(vrts_CMD_STRUCTURE);
	pthread_mutex_unlock(&vrpth_SendUart);
}

