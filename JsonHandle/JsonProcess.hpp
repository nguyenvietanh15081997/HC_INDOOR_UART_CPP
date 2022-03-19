/*
 * JsonProcess.h process json data
 * Transmit data format json
 */
#ifndef JSONHANDLE_JSONPROCESS_HPP_
#define JSONHANDLE_JSONPROCESS_HPP_

#include "../Include/Include.hpp"

#define LED_DOWNLIGHT_SMT				12001
#define LED_DOWNLIGHT_COB_RONG			12002
#define LED_DOWNLIGHT_COB_HEP			12003
#define LED_DOWNLIGHT_COB_TRANG_TRI		12004
#define LED_PANEL_TRON					12005
#define LED_PANEL_VUONG					12006
#define LED_OP_TRAN						12007
#define LED_OP_TUONG					12008
#define LED_CHIEU_TRANH_GAN_TUONG		12009
#define LED_TRACKLIGHT					12010
#define LED_THA_TRAN					12011
#define LED_CHIEU_GUONG					12012
#define LED_DAY_CCT						12013
#define LED_Tube_M16					12014
#define DEN_BAN							12015
#define LED_FLOODING					12016
#define LED_DAY_RGBCW					14001
#define LED_BULD						14002
#define LED_OP_TRAN_LOA					15001
#define CONG_TAC_CHUYEN_MACH_ONOFF		21001
#define CONG_TAC_CAM_UNG_RD_CT01		22001
#define CONG_TAC_CAM_UNG_RD_CT02		22002
#define CONG_TAC_CAM_UNG_RD_CT03		22003
#define CONG_TAC_CAM_UNG_RD_CT04		22004
#define DIEU_KHIEN_CANH_M2(DC)			23001
#define DIEU_KHIEN_CANH_AC				23002

#define REM_MO							54
#define REM_DONG						55
#define REM_DUNG						56
#define REM_PHANTRAM_MO					57

void JsonHandle(char *data);

#endif
