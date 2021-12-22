/*
 * Json.c
 */

#include "JsonProcess.hpp"
#include "../logging/slog.h"
#include "../BuildCmdUart/BuildCmdUart.hpp"
#include "../ScreenTouch/ScreenTouch.hpp"
#include "../ProcessUart/Provision.hpp"
#include "../Mqtt/Mqtt.hpp"


using namespace std;
using namespace rapidjson;

#define DEFAULT			0x0005
#define ZERO			0x0000
#define MINUTE 			0x007e
#define FIVE_MINUTE 	0X009E
#define TEN_MUNITE 		0x00c1
#define TEWNTY_MUNITE 	0x00c2
#define THIRTY_MUNITE 	0x00c3

#define GROUPID_START	49152
#define TIMESLEEP       400000

bool vrb_IsProvision = false;
static bool startProcessRoom = false;
pthread_t vrpth_ProvisionThread;

typedef void (*cb_cmd_function_t)(char *msg);
typedef struct functionProccess{
	string 				commandStr;
	cb_cmd_function_t 	funcProcess;
} functionProcess_t;

static uint16_t Percent2ParamCCT(uint8_t percent) {
	return ((percent * 192) + 800);
}
static uint16_t Percent2ParamDIM(uint8_t percent) {
	return ((percent * 65535) / 100);
}
#if 0
static uint16_t Percent2ParamHSL(uint8_t percent) {
	return ((percent * 65535) / 100);
}
#endif

static uint16_t GetTransition(uint16_t parTime) {
	uint16_t transition;
	switch (parTime) {
	case 65535:
		transition = ZERO;
		break;
	case 1:
		transition = MINUTE;
		break;
	case 5:
		transition = FIVE_MINUTE;
		break;
	case 10:
		transition = TEN_MUNITE;
		break;
	case 20:
		transition = TEWNTY_MUNITE;
		break;
	case 30:
		transition = THIRTY_MUNITE;
		break;
	default:
		transition = DEFAULT;
		break;
	}
	return transition;
}

static void Send_Uart(char *msg) {
	Document document;
	document.Parse(msg);
	if (document.IsObject()) {
		if (document.HasMember("DATA")) {
			const Value &data = document["DATA"];
			if (data.HasMember("UART")) {
				string uart_Str = data["UART"].GetString();
				char * uartConvertChar = new char[uart_Str.length() +1];
				strcpy(uartConvertChar, uart_Str.c_str());
				uint8_t uartArray[48] = {0};
				for (int i =0; i < (int)(strlen(uartConvertChar)/3 +1); i++){
					sscanf((char *)uartConvertChar + i*3,"%2x" ,&uartArray[i]);
				}
				bufferDataUart.push_back(AssignData(uartArray,(strlen(uartConvertChar)/3 +1)));
				delete uartConvertChar;
			}
		}
	}
}

static void Scan(char *msg) {
	Document document;
	document.Parse(msg);
	if (document.IsObject()) {
		vrb_IsProvision = true;
		slog_print(SLOG_INFO, 1, "<provision>provision start");
		gvrb_Provision = true;
		MODE_PROVISION = true;
		stateProvision = statePro_scan;
		pthread_create(&vrpth_Pro, NULL, Pro_Thread, NULL);
	}
}

static void Stop(char *msg) {
	Document document;
	document.Parse(msg);
	if (document.IsObject()) {
		vrb_IsProvision = false;
		slog_print(SLOG_INFO, 1, "<provision>provision stop");
		MODE_PROVISION=false;
		bufferDataUart.push_back(AssignData(OUTMESSAGE_ScanStop,3));
		gvrb_Provision = false;
		stateProvision = statePro_stop;
	}

	StringBuffer sendToDB;
	Writer<StringBuffer> json(sendToDB);

	json.StartObject();
	json.Key("CMD");
	json.String("STOP");
	json.EndObject();

//	cout << sendToDB.GetString() << endl;
	string s = sendToDB.GetString();
	char *msgSend = new char[s.length() + 1];
	strcpy(msgSend, s.c_str());
	mqtt_send(mosq,(char*) TP_PUB, (char*)msgSend);
	delete msgSend;
}

static void Update(char *msg) {
	Document document;
	document.Parse(msg);
	if (document.IsObject()) {
		if (document.HasMember("DATA")) {
			const Value &data = document["DATA"];
			if (data.HasMember("DEVICE_UNICAST_ID") && data["DEVICE_UNICAST_ID"].IsArray()) {
				const Value &deviceUnicast = data["DEVICE_UNICAST_ID"];
				for (SizeType i = 0; i < deviceUnicast.Size(); i++) {
					uint16_t adr = deviceUnicast[i].GetInt();
					CmdUpdateLight(HCI_CMD_GATEWAY_CMD, adr, 12);
				}
			}
		}
	}
}

static void ResetNode(char *msg) {
	Document document;
	document.Parse(msg);
	if (document.IsObject()) {
		if (document.HasMember("DATA")) {
			const Value &data = document["DATA"];
			if (data.HasMember("DEVICE_UNICAST_ID") && data["DEVICE_UNICAST_ID"].IsArray()) {
				const Value &deviceUnicast = data["DEVICE_UNICAST_ID"];
				for (SizeType i = 0; i < deviceUnicast.Size(); i++) {
					uint16_t adr = deviceUnicast[i].GetInt();
					FunctionPer(HCI_CMD_GATEWAY_CMD, ResetNode_typedef, adr,
							NULL8, NULL8, NULL16, NULL16, NULL16, NULL16,
							NULL16, NULL16, NULL16, NULL16, 12);
				}
			}
		}
	}
}

static void ResetHc(char *msg) {
	Function_Vendor(HCI_CMD_GATEWAY_CMD, ResetAll_typedef, NULL16, NULL16,
			NULL8, NULL8, NULL8, NULL16, NULL16, NULL16, NULL16, NULL16, NULL16,
			NULL16, NULL8, NULL8, NULL8, NULL8, NULL16, 17);
}

static void OnOff(char *msg) {
	Document document;
	document.Parse(msg);
	uint16_t time = 0;
	uint16_t adr;
	uint16_t valueOnOff;
	if (document.IsObject()) {
		if (document.HasMember("TIME")) {
			time = document["TIME"].GetInt();
		}
		if (document.HasMember("DATA")) {
			const Value &data = document["DATA"];
			if (data.HasMember("DEVICE_UNICAST_ID") && data.HasMember("VALUE_ONOFF")) {
				adr = data["DEVICE_UNICAST_ID"].GetInt();
				valueOnOff = data["VALUE_ONOFF"].GetInt();
				if (!startProcessRoom) {
					FunctionPer(HCI_CMD_GATEWAY_CMD, ControlOnOff_typedef, adr,
							NULL8, valueOnOff, NULL16, NULL16, NULL16,
							NULL16, NULL16, NULL16, NULL16, GetTransition(time),
							16);
				} else {
					FunctionPer(HCI_CMD_GATEWAY_CMD, ControlOnoff_NoAck_typedef,
							adr, NULL8, valueOnOff, NULL16, NULL16, NULL16,
							NULL16, NULL16, NULL16, NULL16, GetTransition(time),
							16);
				}
			}
		}
	}
}

static void CCT_Set(char *msg) {
	Document document;
	document.Parse(msg);
	uint16_t time = 0;
	uint16_t adr;
	uint16_t valueCCT;
	if (document.IsObject()) {
		if (document.HasMember("TIME")) {
			time = document["TIME"].GetInt();
		}
		if (document.HasMember("DATA")) {
			const Value &data = document["DATA"];
			if (data.HasMember("DEVICE_UNICAST_ID") && data.HasMember("VALUE_CCT")) {
				adr = data["DEVICE_UNICAST_ID"].GetInt();
				valueCCT = data["VALUE_CCT"].GetInt();
				if(!startProcessRoom){
					FunctionPer(HCI_CMD_GATEWAY_CMD, CCT_Set_typedef, adr,
							NULL8, NULL8, NULL16, Percent2ParamCCT(valueCCT),
							NULL16, NULL16, NULL16, NULL16, NULL16,
							GetTransition(time), 19);
				}
				else {
					FunctionPer(HCI_CMD_GATEWAY_CMD, CCT_Set_NoAck_typedef, adr,
							NULL8, NULL8, NULL16, Percent2ParamCCT(valueCCT),
							NULL16, NULL16, NULL16, NULL16, NULL16,
							GetTransition(65535), 19);
				}
			}
		}
	}
}

static void DIM_Set(char *msg) {
	Document document;
	document.Parse(msg);
	uint16_t time = 0;
	uint16_t adr;
	uint16_t valueDim;
	if (document.IsObject()) {
		if (document.HasMember("TIME")) {
			time = document["TIME"].GetInt();
		}
		if (document.HasMember("DATA")) {
			const Value &data = document["DATA"];
			if (data.HasMember("DEVICE_UNICAST_ID") && data.HasMember("VALUE_DIM")) {
				adr = data["DEVICE_UNICAST_ID"].GetInt();
				valueDim = data["VALUE_DIM"].GetInt();
				if (!startProcessRoom) {
					FunctionPer(HCI_CMD_GATEWAY_CMD, Lightness_Set_typedef, adr,
							NULL8, NULL8, Percent2ParamDIM(valueDim),
							NULL16, NULL16, NULL16, NULL16, NULL16, NULL16,
							GetTransition(time), 17);
				} else {
					FunctionPer(HCI_CMD_GATEWAY_CMD,
							Lightness_Set_NoAck_typedef, adr, NULL8, NULL8,
							Percent2ParamDIM(valueDim),
							NULL16, NULL16, NULL16, NULL16, NULL16, NULL16,
							GetTransition(65535), 17);
				}
			}
		}
	}
}

static void PercentDim(char *msg) {
	Document document;
	document.Parse(msg);
	uint16_t time = 0;
	uint16_t adr;
	uint16_t valuePercentDim;
	if (document.IsObject()) {
		if (document.HasMember("TIME")) {
			time = document["TIME"].GetInt();
		}
		if (document.HasMember("DATA")) {
			const Value &data = document["DATA"];
			if (data.HasMember("DEVICE_UNICAST_ID") && data.HasMember("VALUE_PERCENT_DIM")) {
				adr = data["DEVICE_UNICAST_ID"].GetInt();
				valuePercentDim = data["VALUE_PERCENT_DIM"].GetInt();
				Function_Vendor(HCI_CMD_GATEWAY_CMD, DimPercent_vendor_typedef,
						adr, NULL16, valuePercentDim, NULL8, NULL8, NULL16,
						NULL16, NULL16, NULL16, NULL16, NULL16, NULL16, NULL8,
						NULL8, NULL8, NULL8, NULL16, 22);
			}
		}
	}
}

static void HSL_Set(char *msg) {
	Document document;
	document.Parse(msg);
	uint16_t time = 0;
	uint16_t adr;
	uint16_t valueH;
	uint16_t valueS;
	uint16_t valueL;
	if (document.IsObject()) {
		if (document.HasMember("TIME")) {
			time = document["TIME"].GetInt();
		}
		if (document.HasMember("DATA")) {
			const Value &data = document["DATA"];
			if (data.HasMember("DEVICE_UNICAST_ID") && data.HasMember("VALUE_H")
					&& data.HasMember("VALUE_S") && data.HasMember("VALUE_L")) {
				adr = data["DEVICE_UNICAST_ID"].GetInt();
				valueH = data["VALUE_H"].GetInt();
				valueS = data["VALUE_S"].GetInt();
				valueL = data["VALUE_L"].GetInt();
				if (!startProcessRoom) {
					FunctionPer(HCI_CMD_GATEWAY_CMD, HSL_Set_typedef, adr,
							NULL8, NULL8, NULL16, NULL16, NULL16, NULL16,
							valueL, valueH, valueS, GetTransition(time), 21);
				} else {
					FunctionPer(HCI_CMD_GATEWAY_CMD, HSL_Set_NoAck_typedef, adr,
							NULL8, NULL8, NULL16, NULL16, NULL16, NULL16,
							valueL, valueH, valueS, GetTransition(65535), 21);
				}
			}
		}
	}
}

static void AddGroup(char *msg) {
	gvrb_AddGroupLight = true;
	Document document;
	document.Parse(msg);
	uint16_t groupId;
	uint16_t adr;
	if (document.IsObject()) {
		if (document.HasMember("DATA")) {
			const Value &data = document["DATA"];
			if (data.HasMember("GROUP_UNICAST_ID")) {
				groupId = data["GROUP_UNICAST_ID"].GetInt();
			}
			if (data.HasMember("DEVICE_UNICAST_ID") && data["DEVICE_UNICAST_ID"].IsArray()) {
				const Value &temp = data["DEVICE_UNICAST_ID"];
				for (SizeType i = 0; i < temp.Size(); i++) {
					adr = temp[i].GetInt();
					if (groupId >= GROUPID_START) {
						FunctionPer(HCI_CMD_GATEWAY_CMD, AddGroup_typedef, adr,
								groupId, NULL8, NULL16, NULL16, NULL16,
								NULL16, NULL16, NULL16, NULL16, NULL16, 18);
					}
				}
			}
		}
	}
}

static void DelGroup(char *msg) {
	gvrb_AddGroupLight = false;
	Document document;
	document.Parse(msg);
	uint16_t groupId;
	uint16_t adr;
	if (document.IsObject()) {
		if (document.HasMember("DATA")) {
			const Value &data = document["DATA"];
			if (data.HasMember("GROUP_UNICAST_ID")) {
				groupId = data["GROUP_UNICAST_ID"].GetInt();
			}
			if (data.HasMember("DEVICE_UNICAST_ID") && data["DEVICE_UNICAST_ID"].IsArray()) {
				const Value &temp = data["DEVICE_UNICAST_ID"];
				for (SizeType i = 0; i < temp.Size(); i++) {
					adr = temp[i].GetInt();
					if (groupId >= GROUPID_START) {
						FunctionPer(HCI_CMD_GATEWAY_CMD, DelGroup_typedef, adr,
								groupId, NULL8, NULL16, NULL16, NULL16,
								NULL16, NULL16, NULL16, NULL16, NULL16, 18);

					}
				}
			}
		}
	}
}

static void AddScene(char *msg) {
	Document document;
	document.Parse(msg);
	uint16_t sceneId;
	uint8_t srgbId;
	uint16_t adrCCT;
	uint16_t adrRGB_CCT;
	if (document.IsObject()) {
		if (document.HasMember("DATA")) {
			const Value &data = document["DATA"];
			if (data.HasMember("SCENEID")) {
				sceneId = data["SCENEID"].GetInt();
			}
			if (data.HasMember("CCT")) {
				const Value &tempCCT = data["CCT"];
				if (tempCCT.HasMember("DEVICE_UNICAST_ID")  && tempCCT["DEVICE_UNICAST_ID"].IsArray()) {
					const Value &cctObj = tempCCT["DEVICE_UNICAST_ID"];
					for (SizeType i = 0; i < cctObj.Size(); i++) {
						adrCCT = cctObj[i].GetInt();
						FunctionPer(HCI_CMD_GATEWAY_CMD, AddSence_typedef,
								adrCCT,
								NULL8, NULL8, NULL16, NULL16, sceneId, NULL16,
								NULL16, NULL16, NULL16, NULL16, 15);
					}
				}
			}
			if (data.HasMember("RGB_CCT")) {
				const Value &tempRGB_CCT = data["RGB_CCT"];
				for (SizeType i = 0; i < tempRGB_CCT.Size(); i++) {
					const Value &lightRGB_CCT = tempRGB_CCT[i];
					if (lightRGB_CCT.HasMember("DEVICE_UNICAST_ID")
							&& lightRGB_CCT.HasMember("SRGBID")) {
						adrRGB_CCT = lightRGB_CCT["DEVICE_UNICAST_ID"].GetInt();
						srgbId = lightRGB_CCT["SRGBID"].GetInt();
						FunctionPer(HCI_CMD_GATEWAY_CMD,SceneForRGB_vendor_typedef,
								adrRGB_CCT,NULL16,srgbId,NULL16,NULL16,sceneId,NULL16,
								NULL16, NULL16, NULL16,NULL16,15);
					}
				}
			}
		}
	}
}

static void EditScene(char *msg) {
	Document document;
	document.Parse(msg);
	uint16_t sceneId;
	uint16_t adrCCT;
	uint16_t adrRGB_CCT;
	uint16_t sRgbId;
	if (document.IsObject()) {
		if (document.HasMember("DATA")) {
			const Value &data = document["DATA"];
			if (data.HasMember("SCENEID")) {
				sceneId = data["SCENEID"].GetInt();
			}
			if (data.HasMember("CCT_DELSCENE")){
				const Value& cctDel = data["CCT_DELSCENE"];
				for (SizeType i = 0; i < cctDel.Size(); i++) {
					adrCCT = cctDel[i].GetInt();
					FunctionPer(HCI_CMD_GATEWAY_CMD, DelSence_typedef, adrCCT,
					NULL8, NULL8, NULL16, NULL16, sceneId, NULL16, NULL16,
							NULL16, NULL16, NULL16, 14);
				}
			}
			if(data.HasMember("CCT")){
				const Value& cct = data["CCT"];
				if(cct.HasMember("DEVICE_UNICAST_ID") && cct["DEVICE_UNICAST_ID"].IsArray()){
					const Value& arrayAdr = cct["DEVICE_UNICAST_ID"];
					for (SizeType i = 0; i < arrayAdr.Size(); i++) {
						adrCCT = arrayAdr[i].GetInt();
						FunctionPer(HCI_CMD_GATEWAY_CMD, AddSence_typedef,
								adrCCT, NULL8, NULL8, NULL16, NULL16, sceneId,
								NULL16, NULL16, NULL16, NULL16, NULL16, 15);
					}
				}
			}
			if(data.HasMember("RGB_DELSCENE")){
				const Value& arrayRgbCct = data["RGB_DELSCENE"];
				for (SizeType i = 0; i < arrayRgbCct.Size(); i++) {
					adrRGB_CCT = arrayRgbCct[i].GetInt();
					FunctionPer(HCI_CMD_GATEWAY_CMD, DelSceneRgb_vendor_typedef, adrRGB_CCT,
						NULL8, NULL8, NULL16, NULL16, sceneId, NULL16, NULL16,
								NULL16, NULL16, NULL16, 14);
				}
			}
			if(data.HasMember("RGB_CCT")){
				const Value& arrayRgbCctSet = data["RGB_CCT"];
				for (SizeType i = 0; i < arrayRgbCctSet.Size(); i++) {
					const Value &rgbCct = arrayRgbCctSet[i];
					if (rgbCct.HasMember("DEVICE_UNICAST_ID") && rgbCct.HasMember("SRGBID")) {
						adrRGB_CCT = rgbCct["DEVICE_UNICAST_ID"].GetInt();
						sRgbId = rgbCct["SRGBID"].GetInt();
						FunctionPer(HCI_CMD_GATEWAY_CMD,SceneForRGB_vendor_typedef,
								adrRGB_CCT,NULL16,sRgbId,NULL16,NULL16,sceneId,NULL16,
								NULL16, NULL16, NULL16,NULL16,15);
					}
				}
			}
		}
	}
}

static void DelScene(char *msg) {
	Document document;
	document.Parse(msg);
	uint16_t sceneId;
	uint16_t adrCct;
	uint16_t adrRgb;
	if (document.IsObject()) {
		if (document.HasMember("DATA")) {
			const Value &data = document["DATA"];
			if (data.HasMember("SCENEID")) {
				sceneId = data["SCENEID"].GetInt();
				gSceneIdDel = sceneId;
			}
			if (data.HasMember("DEVICE_UNICAST_ID") && data["DEVICE_UNICAST_ID"].IsArray()) {
				const Value &arrayCct = data["DEVICE_UNICAST_ID"];
				for (SizeType i = 0; i < arrayCct.Size(); i++) {
					adrCct = arrayCct[i].GetInt();
					FunctionPer(HCI_CMD_GATEWAY_CMD, DelSence_typedef, adrCct,
							NULL8, NULL8, NULL16, NULL16, sceneId, NULL16,
							NULL16, NULL16, NULL16, NULL16, 14);
				}
			}
			if (data.HasMember("DEVICE_RGB_UNICAST_ID") && data["DEVICE_RGB_UNICAST_ID"].IsArray()) {
				const Value &arrayRgbCct = data["DEVICE_RGB_UNICAST_ID"];
				for (SizeType i = 0; i < arrayRgbCct.Size(); i++) {
					adrRgb = arrayRgbCct[i].GetInt();
					FunctionPer(HCI_CMD_GATEWAY_CMD, DelSceneRgb_vendor_typedef, adrRgb,
							NULL8, NULL8, NULL16, NULL16, sceneId, NULL16,
							NULL16, NULL16, NULL16, NULL16, 14);
				}
			}
		}
	}
}

static void CallScene(char *msg) {
	Document document;
	document.Parse(msg);
	uint16_t sceneId;
	uint16_t transition;
	if(document.IsObject()){
		if(document.HasMember("TIME")){
			transition = document["TIME"].GetInt();
		}
		if(document.HasMember("DATA")){
			const Value &data = document["DATA"];
			if(data.HasMember("SCENEID")){
				sceneId = data["SCENEID"].GetInt();
				FunctionPer(HCI_CMD_GATEWAY_CMD, CallSence_typedef, 65535,
						NULL8, NULL8, NULL16, NULL16, sceneId,
						NULL16, NULL16, NULL16, NULL16,
						GetTransition(transition), 17);
			}
		}
	}
}

static void CallModeRgb(char *msg) {
	Document document;
	document.Parse(msg);
	uint16_t adr;
	uint16_t srgbId;
	if (document.IsObject()) {
		if (document.HasMember("DATA")) {
			const Value &data = document["DATA"];
			if (data.HasMember("DEVICE_UNICAST_ID") && data.HasMember("SRGBID") && data["DEVICE_UNICAST_ID"].IsArray()) {
				srgbId = data["SRGBID"].GetInt();
				const Value& adrObj = data["DEVICE_UNICAST_ID"];
				for (SizeType i = 0; i < adrObj.Size(); i++) {
					adr = adrObj[i].GetInt();
					FunctionPer(HCI_CMD_GATEWAY_CMD, CallModeRgb_vendor_typedef,
							adr, NULL16, srgbId, NULL16, NULL16, NULL16, NULL16,
							NULL16, NULL16, NULL16, NULL16, 15);
				}
			}
		}
	}
}

static void ScanTypeDevice(char *msg) {
	Document document;
	document.Parse(msg);
	uint16_t adr;
	if (document.IsObject()) {
		if (document.HasMember("DATA")) {
			const Value &data = document["DATA"];
			if (data.HasMember("DEVICE_UNICAST_ID")) {
				adr = data["DEVICE_UNICAST_ID"].GetInt();
				Function_Vendor(HCI_CMD_GATEWAY_CMD,
						AskTypeDevice_vendor_typedef, adr, NULL16, NULL8, NULL8,
						NULL8, NULL16, NULL16, NULL16, NULL16, NULL16, NULL16,
						NULL8, NULL8, NULL8, NULL8, NULL8, NULL16, 17);
			}
		}
	}
}

static void SetTypeDevice(char *msg) {
	Document document;
	document.Parse(msg);
	uint16_t adr;
	uint8_t type;
	uint8_t attribute;
	uint8_t appli;
	if(document.IsObject()){
		if(document.HasMember("DATA")){
			const Value& data = document["DATA"];
			if (data.HasMember("DEVICE_UNICAST_ID") && data.HasMember("TYPE")
					&& data.HasMember("ATTRUBUTE")
					&& data.HasMember("APPLICATION")) {
				adr = data["DEVICE_UNICAST_ID"].GetInt();
				type = data["TYPE"].GetInt();
				attribute = data["ATTRUBUTE"].GetInt();
				appli = data["APPLICATION"].GetInt();
				Function_Vendor(HCI_CMD_GATEWAY_CMD,
						SetTypeDevice_vendor_typedef, adr, NULL16,
						NULL8, NULL8, NULL8, NULL16, NULL16, NULL16, NULL16,
						NULL16, NULL16, NULL16, NULL8, type, attribute, appli,
						NULL16, 28);
			}
		}
	}
}

static void SaveGw(char *msg) {
	Document document;
	document.Parse(msg);
	uint16_t adr;
	if (document.IsObject()) {
		if (document.HasMember("DATA")) {
			const Value &data = document["DATA"];
			if (data.HasMember("DEVICE_UNICAST_ID")) {
				adr = data["DEVICE_UNICAST_ID"].GetInt();
				Function_Vendor(HCI_CMD_GATEWAY_CMD, SaveGateway_vendor_typedef,
						adr, NULL16,
						NULL8, NULL8, NULL8, NULL16, NULL16, NULL16, NULL16,
						NULL16, NULL16, NULL16, NULL8, NULL8, NULL8, NULL8,
						NULL16, 17);
			}
		}
	}
}

#define NUM_BUTTONREMOTE			6
string arrayButtonRemote[NUM_BUTTONREMOTE] = {"BUTTON_1","BUTTON_2","BUTTON_3","BUTTON_4","BUTTON_5","BUTTON_6"};
static int ButtonId(string bt) {
	int id = 0;
	for (int i = 0; i < NUM_BUTTONREMOTE; i++) {
		if (bt.compare(arrayButtonRemote[i]) == 0) {
			id = i + 1;
			break;
		}
	}
	return id;
}
static void AddSceneRemoteDc(char *msg) {
	Document document;
	document.Parse(msg);
	uint16_t adr;
	string bt;
	uint16_t buttonId;
	uint16_t modeId;
	uint16_t sceneId;
	if (document.IsObject()) {
		if (document.HasMember("DATA")) {
			const Value &data = document["DATA"];
			if (data.HasMember("DEVICE_UNICAST_ID")
					&& data.HasMember("BUTTONID") && data.HasMember("MODEID")
					&& data.HasMember("SCENEID")) {
				adr = data["DEVICE_UNICAST_ID"].GetInt();
				bt = data["BUTTONID"].GetString();
				buttonId = ButtonId(bt);
				modeId = data["MODEID"].GetInt();
				sceneId = data["SCENEID"].GetInt();
				Function_Vendor(HCI_CMD_GATEWAY_CMD,
						SceneForRemote_DC_vendor_typedef, adr, NULL16, buttonId,
						modeId, NULL8, NULL16, NULL16, NULL16, NULL16, NULL16,
						sceneId, sceneId,
						NULL8, NULL8, NULL8, NULL8, NULL16, 31);
			}
		}
	}
}

static void DelSceneRemoteDc(char *msg) {
	Document document;
	document.Parse(msg);
	uint16_t adr;
	string buttonId;
	uint8_t modeId;
	if (document.IsObject()) {
		if (document.HasMember("DATA")) {
			const Value &data = document["DATA"];
			if (data.HasMember("DEVICE_UNICAST_ID")
					&& data.HasMember("BUTTONID") && data.HasMember("MODEID")) {
				adr = data["DEVICE_UNICAST_ID"].GetInt();
				buttonId = data["BUTTONID"].GetString();
				modeId = data["MODEID"].GetInt();
				Function_Vendor(HCI_CMD_GATEWAY_CMD,
						DelSceneForRemote_DC_vendor_typedef, adr, NULL16,
						ButtonId(buttonId), modeId, NULL8, NULL16, NULL16, NULL16, NULL16,
						NULL16, NULL16, NULL16,
						NULL8, NULL8, NULL8, NULL8, NULL16, 31);
			}
		}
	}
}

static void AddSceneRemoteAc(char *msg) {
	Document document;
	document.Parse(msg);
	uint16_t adr;
	string buttonId;
	uint16_t modeId;
	uint16_t sceneId;
	if (document.IsObject()) {
		if (document.HasMember("DATA")) {
			const Value &data = document["DATA"];
			if (data.HasMember("DEVICE_UNICAST_ID")
					&& data.HasMember("BUTTONID") && data.HasMember("MODEID")
					&& data.HasMember("SCENEID")) {
				adr = data["DEVICE_UNICAST_ID"].GetInt();
				buttonId = data["BUTTONID"].GetString();
				modeId = data["MODEID"].GetInt();
				sceneId = data["SCENEID"].GetInt();
				Function_Vendor(HCI_CMD_GATEWAY_CMD,
						SceneForRemote_AC_vendor_typedef, adr, NULL16, ButtonId(buttonId),
						modeId, NULL8, NULL16, NULL16, NULL16, NULL16, NULL16,
						sceneId, sceneId,
						NULL8, NULL8, NULL8, NULL8, NULL16, 31);
			}
		}
	}
}

static void DelSceneRemoteAc(char *msg) {
	Document document;
	document.Parse(msg);
	uint16_t adr;
	string buttonId;
	uint8_t modeId;
	if (document.IsObject()) {
		if (document.HasMember("DATA")) {
			const Value &data = document["DATA"];
			if (data.HasMember("DEVICE_UNICAST_ID")
					&& data.HasMember("BUTTONID") && data.HasMember("MODEID")) {
				adr = data["DEVICE_UNICAST_ID"].GetInt();
				buttonId = data["BUTTONID"].GetString();
				modeId = data["MODEID"].GetInt();
				Function_Vendor(HCI_CMD_GATEWAY_CMD,
						DelSceneForRemote_AC_vendor_typedef, adr, NULL16,
						ButtonId(buttonId), modeId, NULL8, NULL16, NULL16, NULL16, NULL16,
						NULL16, NULL16, NULL16,
						NULL8, NULL8, NULL8, NULL8, NULL16, 31);
			}
		}
	}
}

static void AddSceneScreenTouch( char *msg){
	Document document;
	document.Parse(msg);
	uint16_t adr;
	uint16_t sceneId;
	uint16_t icon;
	if (document.IsObject()) {
		if (document.HasMember("DATA")) {
			const Value &data = document["DATA"];
			if (data.HasMember("DEVICE_UNICAST_ID") && data.HasMember("SCENEID")
					&& data.HasMember("ICONID")) {
				adr = data["DEVICE_UNICAST_ID"].GetInt();
				sceneId = data["SCENEID"].GetInt();
				icon = data["ICONID"].GetInt();
				SendData2ScreeTouch(st_enum_addscene, adr, sceneId, icon, NULL8,
						NULL16, NULL16, NULL16, NULL8, NULL8, NULL8, NULL8,
						NULL8, NULL8, NULL8);
			}
		}
	}
}

static void EditIconScreenTouch(char *msg) {
	Document document;
	document.Parse(msg);
	uint16_t adr;
	uint16_t sceneId;
	uint16_t icon;
	if (document.IsObject()) {
		if (document.HasMember("DATA")) {
			const Value &data = document["DATA"];
			if (data.HasMember("DEVICE_UNICAST_ID") && data.HasMember("SCENEID")
					&& data.HasMember("ICONID")) {
				adr = data["DEVICE_UNICAST_ID"].GetInt();
				sceneId = data["SCENEID"].GetInt();
				icon = data["ICONID"].GetInt();
				SendData2ScreeTouch(st_enum_editIcon, adr, sceneId, icon, NULL8,
						NULL16, NULL16, NULL16, NULL8, NULL8, NULL8, NULL8,
						NULL8, NULL8, NULL8);
			}
		}
	}
}

static void DelSceneScreenTouch(char *msg) {
	Document document;
	document.Parse(msg);
	uint16_t adr;
	uint16_t sceneId;
	if (document.IsObject()) {
		if (document.HasMember("DATA")) {
			const Value &data = document["DATA"];
			if (data.HasMember("DEVICE_UNICAST_ID")
					&& data.HasMember("SCENEID")) {
				adr = data["DEVICE_UNICAST_ID"].GetInt();
				sceneId = data["SCENEID"].GetInt();
				SendData2ScreeTouch(st_enum_delscene, adr, sceneId,
				NULL8, NULL8, NULL16, NULL16, NULL16, NULL8, NULL8, NULL8,
				NULL8, NULL8, NULL8, NULL8);
			}
		}
	}
}

static void DelAllSceneScreenTouch(char *msg) {
	Document document;
	document.Parse(msg);
	uint16_t adr;
	if (document.IsObject()) {
		if (document.HasMember("DATA")) {
			const Value &data = document["DATA"];
			if (data.HasMember("DEVICE_UNICAST_ID")) {
				adr = data["DEVICE_UNICAST_ID"].GetInt();
				SendData2ScreeTouch(st_enum_DelAllScene, adr, NULL16,
				NULL8, NULL8, NULL16, NULL16, NULL16, NULL8, NULL8, NULL8,
				NULL8, NULL8, NULL8, NULL8);
			}
		}
	}
}

static void DefaultOnOffScreenTouch(char *msg) {
	Document document;
	document.Parse(msg);
	uint16_t adr;
	uint16_t groupId;
	if(document.IsObject()){
		if(document.HasMember("DATA")){
			const Value& data = document["DATA"];
			if(data.HasMember("DEVICE_UNICAST_ID") && data.HasMember("GROUPID")){
				adr = data["DEVICE_UNICAST_ID"].GetInt();
				groupId = data["GROUPID"].GetInt();
				SendData2ScreeTouch(st_enum_DefaultOnOff, adr, groupId,
				NULL8, NULL8, NULL16, NULL16, NULL16, NULL8, NULL8, NULL8,
				NULL8, NULL8, NULL8, NULL8);
			}
		}
	}
}

static void WeatherOutSreenTouch(char *msg) {
	Document document;
	document.Parse(msg);
	uint16_t adr;
	uint16_t statusWeather;
	uint16_t temp;
	if (document.IsObject()) {
		if (document.HasMember("DATA")) {
			const Value &data = document["DATA"];
			if (data.HasMember("DEVICE_UNICAST_ID") && data.HasMember("STATUS")
					&& data.HasMember("TEMPERATURE")) {
				adr = data["DEVICE_UNICAST_ID"].GetInt();
				statusWeather = data["STATUS"].GetInt();
				temp = data["TEMPERATURE"].GetInt();
				SendData2ScreeTouch(st_enum_weatherOutdoor, adr,NULL16,
						NULL8, statusWeather, temp, NULL16,NULL16, NULL8, NULL8, NULL8, NULL8,
						NULL8, NULL8, NULL8);
			}
		}
	}
}

static void WeatherIndoorScreenTouch(char *msg) {
	Document document;
	document.Parse(msg);
	uint16_t adr;
	int temp;
	uint16_t hum;
	uint16_t pm25 = 0;
	if (document.IsObject()) {
		if (document.HasMember("DATA")) {
			const Value &data = document["DATA"];
			if(data.HasMember("PM2.5")){
				pm25 = data["PM2.5"].GetInt();
			}
			if (data.HasMember("DEVICE_UNICAST_ID")
					&& data.HasMember("TEMPERATURE")
					&& data.HasMember("HUMIDITY")) {
				adr = data["DEVICE_UNICAST_ID"].GetInt();
				hum = data["HUMIDITY"].GetInt();
				temp = data["TEMPERATURE"].GetInt();
				int tempSet = temp;
				if(temp < 0){
					tempSet = (temp)*(-1) | 0x8000;
				}
				SendData2ScreeTouch(st_enum_weatherIndoor, adr, NULL16, NULL8,
						NULL8, tempSet, hum, pm25, NULL8, NULL8, NULL8, NULL8,
						NULL8, NULL8, NULL8);
			}
		}
	}
}

static void TimeScreenTouch(char *msg) {
	Document document;
	document.Parse(msg);
	uint16_t adr;
	uint16_t hours;
	uint16_t minute;
	uint16_t second;
	if (document.IsObject()) {
		if (document.HasMember("DATA")) {
			const Value &data = document["DATA"];
			if (data.HasMember("DEVICE_UNICAST_ID") && data.HasMember("HOUR")
					&& data.HasMember("MINUTE") && data.HasMember("SECOND")) {
				adr = data["DEVICE_UNICAST_ID"].GetInt();
				hours = data["HOUR"].GetInt();
				minute = data["MINUTE"].GetInt();
				second = data["SECOND"].GetInt();
				SendData2ScreeTouch(st_enum_getTime, adr, NULL16,
				NULL8, NULL8, NULL16, NULL16, NULL16, NULL8, NULL8, NULL8,
						NULL8, hours, minute, second);
			}
		}
	}
}

static void DateScreenTouch(char *msg) {
	Document document;
	document.Parse(msg);
	uint16_t adr;
	uint16_t year;
	uint16_t month;
	uint16_t date;
	uint16_t day;
	if (document.IsObject()) {
		if (document.HasMember("DATA")) {
			const Value &data = document["DATA"];
			if (data.HasMember("DEVICE_UNICAST_ID") && data.HasMember("YEAR")
					&& data.HasMember("MONTH") && data.HasMember("DATE")
					&& data.HasMember("DAY")) {
				adr = data["DEVICE_UNICAST_ID"].GetInt();
				year = data["YEAR"].GetInt();
				month = data["MONTH"].GetInt();
				date = data["DATE"].GetInt();
				day = data["DAY"].GetInt();
				SendData2ScreeTouch(st_enum_getDate, adr, NULL16,
				NULL8, NULL8, NULL16, NULL16, NULL16, year, month, date, day,
						NULL8,
						NULL8, NULL8);
			}
		}
	}
}

static void AddSceneLightPirSensor(char *msg){
	Document document;
	document.Parse(msg);
	uint16_t sceneId;
	uint16_t adr;
	uint8_t statusPir;
	uint8_t type;
	uint16_t condition;
	uint8_t condition_light = 0;
	uint8_t conditionSensor = 0;
	uint16_t highLux;
	uint16_t lowLux;
	if(document.IsObject()){
		if(document.HasMember("DATA")){
			const Value& data = document["DATA"];
			if (data.HasMember("TYPE") && data.HasMember("DEVICE_UNICAST_ID")
					&& data.HasMember("SCENEID")
					&& data.HasMember("PIR_SENSOR")) {
				adr = data["DEVICE_UNICAST_ID"].GetInt();
				type = data["TYPE"].GetInt();
				sceneId = data["SCENEID"].GetInt();
				const Value& pirSensor = data["PIR_SENSOR"];
				if(pirSensor.HasMember("PIR")){
					statusPir = pirSensor["PIR"].GetInt();
				}
				if(type == 1){
					highLux = lowLux = 0;
				}
				else if(type == 2){
					if(data.HasMember("LIGHT_SENSOR")){
						const Value& lighSensor = data["LIGHT_SENSOR"];
						if(lighSensor.HasMember("CONDITION") && lighSensor.HasMember("LOW_LUX")){
							condition = lighSensor["CONDITION"].GetInt();
							lowLux = lighSensor["LOW_LUX"].GetInt();
							if(condition == 1) {

							}
							else if ((condition == 3) || (condition == 4)) {
								condition_light = 0x01;
							}
							else if ((condition == 5) || (condition == 6)) {
								condition_light = 0x03;
							}
							else if ((condition == 7)) {
								highLux = lighSensor["HIGHT_LUX"].GetInt();
								condition_light = 0x02;
							}
						}
					}
				}
				conditionSensor = (uint8_t)((statusPir << 3) |(condition_light));
				Function_Vendor(HCI_CMD_GATEWAY_CMD,
						SceneForSensor_LightPir_vendor_typedef, adr, NULL16,
						NULL8, NULL8,
						NULL8, conditionSensor, lowLux / 10, highLux / 10,
						NULL8, NULL8, sceneId, NULL16,
						NULL8, NULL8, NULL8, NULL8, NULL16, 23);
			}
		}
	}
}

static void DelSceneLighPirSensor(char *msg) {
	Document document;
	document.Parse(msg);
	uint16_t adr;
	uint16_t sceneId;
	uint16_t condition = 1280;
	if (document.IsObject()) {
		if (document.HasMember("DATA")) {
			const Value &data = document["DATA"];
			if (data.HasMember("DEVICE_UNICAST_ID")
					&& data.HasMember("SCENEID")) {
				adr = data["DEVICE_UNICAST_ID"].GetInt();
				sceneId = data["SCENEID"].GetInt();
				Function_Vendor(HCI_CMD_GATEWAY_CMD,
						DelSceneForSensor_LightPir_vendor_typedef, adr, NULL16,
						NULL8, NULL8, NULL8, condition, NULL16, NULL16, NULL16,
						NULL16, sceneId, sceneId, NULL8, NULL8, NULL8, NULL8,
						NULL16, 19);
			}
		}
	}
}

static void TimeActionPir(char *msg) {
	Document document;
	document.Parse(msg);
	uint16_t adr;
	uint16_t timeAction;
	if (document.IsObject()) {
		if (document.HasMember("DATA")) {
			const Value &data = document["DATA"];
			if (data.HasMember("DEVICE_UNICAST_ID") && data.HasMember("TIME")) {
				adr = data["DEVICE_UNICAST_ID"].GetInt();
				timeAction = data["TIME"].GetInt();
				Function_Vendor(HCI_CMD_GATEWAY_CMD,
						SetTimeAction_vendor_typedef, adr, NULL16, NULL8, NULL8,
						NULL8, timeAction, NULL16, NULL16, NULL16, NULL16, NULL16,
						NULL16, NULL8, NULL8, NULL8, NULL8, NULL16, 19);
			}
		}
	}
}

static void AddSceneDoorSensor(char *msg) {
	Document document;
	document.Parse(msg);
	uint16_t adr;
	uint16_t sceneId;
	uint8_t doorStatus;
	if (document.IsObject()) {
		if (document.HasMember("DATA")) {
			const Value &data = document["DATA"];
			if (data.HasMember("DEVICE_UNICAST_ID")
					&& data.HasMember("DOOR_SENSOR")
					&& data.HasMember("SCENEID")) {
				adr = data["DEVICE_UNICAST_ID"].GetInt();
				sceneId = data["SCENEID"].GetInt();
				const Value &doorSensor = data["DOOR_SENSOR"];
				if (doorSensor.HasMember("DOOR")) {
					doorStatus = doorSensor["DOOR"].GetInt();
					Function_Vendor(HCI_CMD_GATEWAY_CMD,
							SceneForDoorSensor_vendor_typedef, adr, NULL16,
							NULL8, NULL8, doorStatus, NULL16,
							NULL16, NULL16, NULL16, NULL16, sceneId, sceneId,
							NULL, NULL8, NULL8, NULL8, NULL16, 21);
				}
			}
		}
	}
}

static void DelSceneDoorSensor(char *msg) {
	Document document;
	document.Parse(msg);

	uint16_t adr;
	uint16_t sceneId;
	if (document.IsObject()) {
		if (document.HasMember("DATA")) {
			const Value &data = document["DATA"];
			if (data.HasMember("DEVICE_UNICAST_ID")
					&& data.HasMember("SCENEID")) {
				adr = data["DEVICE_UNICAST_ID"].GetInt();
				sceneId = data["SCENEID"].GetInt();
				Function_Vendor(HCI_CMD_GATEWAY_CMD,
						DelSceneForDoorSensor_vendor_typedef, adr, NULL16,
						NULL8, NULL8, NULL8, NULL16, NULL16, NULL16,
						NULL16, NULL16, sceneId, sceneId, NULL8, NULL8, NULL8,
						NULL8, NULL16, 19);
			}
		}
	}
}

static void ControlSwitch4(char *msg) {
	Document document;
	document.Parse(msg);
	uint16_t adr;
	uint8_t relayId;
	uint8_t relayValue;
	if (document.IsObject()) {
		if (document.HasMember("DATA")) {
			const Value &data = document["DATA"];
			if (data.HasMember("DEVICE_UNICAST_ID")) {
				adr = data["DEVICE_UNICAST_ID"].GetInt();
			}
			if (data.HasMember("SWITCH_STATUS")) {
				const Value &switch4 = data["SWITCH_STATUS"];
				if (switch4.HasMember("RELAY1")) {
					relayId = 1;
					relayValue = switch4["RELAY1"].GetInt();
				} else if (switch4.HasMember("RELAY2")) {
					relayId = 2;
					relayValue = switch4["RELAY2"].GetInt();
				} else if (switch4.HasMember("RELAY3")) {
					relayId = 3;
					relayValue = switch4["RELAY3"].GetInt();
				} else if (switch4.HasMember("RELAY4")) {
					relayId = 4;
					relayValue = switch4["RELAY4"].GetInt();
				}
			}
			Function_Vendor(HCI_CMD_GATEWAY_CMD, ControlSwitch4_vendor_typedef,
					adr, NULL16, NULL8, NULL8, NULL8, NULL16,
					(relayId << 8 | relayValue), NULL16,
					NULL16, NULL16, NULL16, NULL16, NULL8, NULL8, NULL8, NULL8,
					NULL16, 23);
		}
	}
}

static void AddSceneSwitch4(char *msg) {
	Document document;
	document.Parse(msg);
	uint16_t adr;
	uint16_t sceneId;
	uint8_t relay1Value;
	uint8_t relay2Value;
	uint8_t relay3Value;
	uint8_t relay4Value;
	if (document.IsObject()) {
		if (document.HasMember("DATA")) {
			const Value &data = document["DATA"];
			if (data.HasMember("DEVICE_UNICAST_ID")) {
				adr = data["DEVICE_UNICAST_ID"].GetInt();
			}
			if (data.HasMember("SCENEID")) {
				sceneId = data["SCENEID"].GetInt();
			}
			if (data.HasMember("SWITCH_STATUS")) {
				const Value &switchStatus = data["SWITCH_STATUS"];
				if (switchStatus.HasMember("RELAY1")) {
					relay1Value = switchStatus["RELAY1"].GetInt();
				}
				if (switchStatus.HasMember("RELAY2")) {
					relay2Value = switchStatus["RELAY2"].GetInt();
				}
				if (switchStatus.HasMember("RELAY3")) {
					relay3Value = switchStatus["RELAY3"].GetInt();
				}
				if (switchStatus.HasMember("RELAY4")) {
					relay4Value = switchStatus["RELAY4"].GetInt();
				}
				Function_Vendor(HCI_CMD_GATEWAY_CMD,
						SetSceneForSwitch4_vendor_typedef, adr, NULL16, NULL8,
						NULL8, NULL8, NULL16, (relay1Value << 8 | relay2Value),
						(relay3Value << 8 | relay4Value),
						NULL16, NULL16, sceneId, NULL16, NULL8, NULL8, NULL8,
						NULL8, NULL16, 23);
			}
		}
	}
}

static void DelSceneSwitch4(char *msg) {
	Document document;
	document.Parse(msg);
	uint16_t adr;
	uint16_t sceneId;
	if (document.IsObject()) {
		if (document.HasMember("DATA")) {
			const Value &data = document["DATA"];
			if (data.HasMember("DEVICE_UNICAST_ID") && data.HasMember("SCENEID")) {
				adr = data["DEVICE_UNICAST_ID"].GetInt();
				sceneId = data["SCENEID"].GetInt();
				Function_Vendor(HCI_CMD_GATEWAY_CMD,
						DelSceneForSwitch4_vendor_typedef, adr, NULL16, NULL8,
						NULL8,
						NULL8, NULL16, NULL16, NULL16, NULL16, NULL16, sceneId,
						NULL16, NULL8, NULL8, NULL8, NULL8, NULL16, 19);
			}
		}
	}
}

static void AddSceneSocket1(char *msg) {

}

static void RequestStatusPmSensor(char *msg) {
	Document document;
	document.Parse(msg);
	uint16_t adr;
	if (document.IsObject()) {
		if (document.HasMember("DATA")) {
			const Value &data = document["DATA"];
			if (data.HasMember("DEVICE_UNICAST_ID")) {
				adr = data["DEVICE_UNICAST_ID"].GetInt();
				Function_Vendor(HCI_CMD_GATEWAY_CMD, AskPm_vendor_typedef, adr,
						NULL16, NULL8, NULL8,
						NULL8, NULL16, NULL16, NULL16, NULL16, NULL16, NULL16,
						NULL16, NULL8, NULL8, NULL8, NULL8, NULL16, 17);
			}
		}
	}
}

static void ControlCurtain(char *msg) {
	Document document;
	document.Parse(msg);
	uint16_t adr;
	uint8_t status;
	uint8_t percent;
	if (document.IsObject()) {
		if (document.HasMember("DATA")) {
			const Value &data = document["DATA"];
			if (data.HasMember("DEVICE_UNICAST_ID")) {
				adr = data["DEVICE_UNICAST_ID"].GetInt();
			}
			if (data.HasMember("CURTAIN_STATUS")) {
				const Value &curtain = data["CURTAIN_STATUS"];
				if (curtain.HasMember("STATUS") && curtain.HasMember("PERCENT")) {
					status = curtain["STATUS"].GetInt();
					percent = curtain["PERCENT"].GetInt();
					Function_Vendor(HCI_CMD_GATEWAY_CMD,
							ControlCurtain_vendor_typedef, adr, NULL16, status,
							percent,
							NULL8, NULL16, NULL16, NULL16, NULL16, NULL16,
							NULL16, NULL16, NULL8, NULL8, NULL8, NULL8, NULL16,
							19);
				}
			}
		}
	}
}

static void AddSceneCurtain(char *msg) {
	Document document;
	document.Parse(msg);
	uint16_t adr;
	uint8_t sceneId;
	if (document.IsObject()) {
		if (document.HasMember("DATA")) {
			const Value &data = document["DATA"];
			if (data.HasMember("DEVICE_UNICAST_ID") && data.HasMember("SCENEID")) {
				adr = data["DEVICE_UNICAST_ID"].GetInt();
				sceneId = data["SCENEID"].GetInt();
				Function_Vendor(HCI_CMD_GATEWAY_CMD,
						SetSceneCurtain_vendor_typedef, adr, NULL16, NULL8,
						NULL8, NULL8,
						NULL16, NULL16, NULL16, NULL16, NULL16, sceneId, NULL16,
						NULL8, NULL8, NULL8, NULL8, NULL16, 19);
			}
		}
	}
}

static void DelSceneCurtain(char *msg) {
	Document document;
	document.Parse(msg);
	uint16_t adr;
	uint8_t sceneId;
	if (document.IsObject()) {
		if (document.HasMember("DATA")) {
			const Value &data = document["DATA"];
			if (data.HasMember("DEVICE_UNICAST_ID") && data.HasMember("SCENEID")) {
				adr = data["DEVICE_UNICAST_ID"].GetInt();
				sceneId = data["SCENEID"].GetInt();
				Function_Vendor(HCI_CMD_GATEWAY_CMD,
						DelSceneCurtain_vendor_typedef, adr, NULL16, NULL8,
						NULL8, NULL8,
						NULL16, NULL16, NULL16, NULL16, NULL16, sceneId, NULL16,
						NULL8, NULL8, NULL8, NULL8, NULL16, 19);
			}
		}
	}
}

static void RequestStatusCurtain(char *msg) {

	Document document;
	document.Parse(msg);
	uint16_t adr;
	if (document.IsObject()) {
		if (document.HasMember("DATA")) {
			const Value &data = document["DATA"];
			if (data.HasMember("DEVICE_UNICAST_ID")) {
				adr = data["DEVICE_UNICAST_ID"].GetInt();
				Function_Vendor(HCI_CMD_GATEWAY_CMD,
						AskSceneCurtain_vendor_typedef, adr, NULL16, NULL8,
						NULL8, NULL8,
						NULL16, NULL16, NULL16, NULL16, NULL16, NULL16, NULL16,
						NULL8, NULL8, NULL8, NULL8, NULL16, 17);
			}
		}
	}
}

static void RequestStatusSensor(char *msg) {
	Document document;
	document.Parse(msg);

	uint16_t adr;
	if(document.IsObject()){
		if(document.HasMember("DATA")){
			const Value& data = document["DATA"];
			if(data.HasMember("DEVICE_UNICAST_ID")){
				adr = data["DEVICE_UNICAST_ID"].GetInt();
				Function_Vendor(HCI_CMD_GATEWAY_CMD, AskPm_vendor_typedef, adr,
				NULL16, NULL8, NULL8,
				NULL8, NULL16, NULL16, NULL16, NULL16, NULL16, NULL16,
				NULL16, NULL8, NULL8, NULL8, NULL8, NULL16, 17);
			}
		}
	}
}

static void StartConfigRoom(char *msg){
	startProcessRoom = true;
}
static void StopConfigRoom(char *msg){
	startProcessRoom = false;
}
static void DelHc(char *msg) {

}

#define MAX_FUNCTION					52
functionProcess_t listCommandMQTT[MAX_FUNCTION] = {
		{"ONOFF",						(OnOff)},
		{"CCT",							(CCT_Set)},
		{"DIM", 						(DIM_Set)},
		{"HSL",							(HSL_Set)},
		{"CALLSCENE",					(CallScene)},
		{"ADDGROUP",					(AddGroup)},
		{"ADDSCENE",					(AddScene)},
		{"UPDATE",						(Update)},
		{"PERCENT_DIM",					(PercentDim)},
		{"DELGROUP", 					(DelGroup)},
		{"EDITSCENE",					(EditScene)},
		{"RESETNODE", 					(ResetNode)},
		{"DELSCENE",					(DelScene)},
		{"CALLMODE_RGB", 				(CallModeRgb)},
		{"SCAN_TYPEDEVICE",				(ScanTypeDevice)},
		{"SET_TYPEDEVICE",				(SetTypeDevice)},
		{"SAVE_GW",						(SaveGw)},
		{"ADDSCENE_REMOTE_DC",			(AddSceneRemoteDc)},
		{"DELSCENE_REMOTE_DC",			(DelSceneRemoteDc)},
		{"ADDSCENE_REMOTE_AC",			(AddSceneRemoteAc)},
		{"DELSCENE_REMOTE_AC",			(DelSceneRemoteAc)},
		{"ADDSCENE_SCREEN_TOUCH",		(AddSceneScreenTouch)},
		{"EDITICON_SCREEN_TOUCH",		(EditIconScreenTouch)},
		{"DELSCENE_SCREEN_TOUCH",		(DelSceneScreenTouch)},
		{"DELALL_SCENE_SCREENTOUCH",	(DelAllSceneScreenTouch)},
		{"DEFAULT_ONOFF_SCREENTOUCH",	(DefaultOnOffScreenTouch)},
		{"WEATHER_OUT_SCREEN_TOUCH",	(WeatherOutSreenTouch)},
		{"WEATHER_IN_SCREEN_TOUCH",		(WeatherIndoorScreenTouch)},
		{"TIME_SCREEN_TOUCH",			(TimeScreenTouch)},
		{"DATE_SCREEN_TOUCH",			(DateScreenTouch)},
		{"ADDSCENE_LIGHT_PIR_SENSOR",	(AddSceneLightPirSensor)},
		{"DELSCENE_LIGHT_PIR_SENSOR",	(DelSceneLighPirSensor)},
		{"TIME_ACTION_PIR",				(TimeActionPir)},
		{"ADDSCENE_DOOR_SENSOR",		(AddSceneDoorSensor)},
		{"DELSCENE_DOOR_SENSOR",		(DelSceneDoorSensor)},
		{"CONTROL_SWITCH4",				(ControlSwitch4)},
		{"ADDSCENE_SWITCH4",			(AddSceneSwitch4)},
		{"DELSCENE_SWITCH4",			(DelSceneSwitch4)},
		{"ADDSCENE_SOCKET1",			(AddSceneSocket1)},
		{"ASK_PM_SENSOR",				(RequestStatusPmSensor)},
		{"CONTROL_CURTAIN",				(ControlCurtain)},
		{"ADDSCENE_CURTAIN",			(AddSceneCurtain)},
		{"DELSCENE_CURTAIN",			(DelSceneCurtain)},
		{"ASK_STATUS_CURTAIN",			(RequestStatusCurtain)},
		{"UPDATE_SENSOR_STATUS",		(RequestStatusSensor)},
		{"SEND_UART", 					(Send_Uart)},
		{"SCAN",						(Scan)},
		{"STOP",						(Stop)},
		{"RESETHC",						(ResetHc)},
		{"START_PROCESS_ROOM",			(StartConfigRoom)},
		{"STOP_PROCESS_ROOM",			(StopConfigRoom)},
		{"DELHC",						(DelHc)}
};

void JsonHandle(char * data) {
	Document document;
	document.Parse(data);
	if (document.IsObject()) {
		if (document.HasMember("CMD")) {
			string cmd = document["CMD"].GetString();
			for (int i = 0; i < MAX_FUNCTION; i++) {
				if (cmd.compare(listCommandMQTT[i].commandStr) == 0) {
					listCommandMQTT[i].funcProcess(data);
					break;
				}
			}
		}
	}
	Document objDelete;
	document.Swap(objDelete);
}
