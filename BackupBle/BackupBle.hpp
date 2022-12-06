/*
 * BackupBle.hpp
 *
 *  Created on: May 12, 2022
 *      Author: anh
 */

#ifndef BACKUPBLE_BACKUPBLE_HPP_
#define BACKUPBLE_BACKUPBLE_HPP_


#include "../Include/Include.hpp"
#include <sqlite3.h>
typedef enum{
	inforGw,
	inforDev,
	infoAppKey,
	infoNetKey,
}type_get_db;

typedef struct infoDev{
	uint16_t adr;
	string devKey;
}infoDev_t;

void BACKUP_updateNetKey(string netkey, infoDev_t gw, uint32_t index);
void BACKUP_updateNewDevKey(infoDev_t gw);
void BACKUP_updateAppKey(string appkey);
void BACKUP_gw(infoDev_t gw);
void BACKUP_dev(vector<infoDev_t> listDev);
void BACKUP_DeviceUnicastIdMax(vector<infoDev_t> listDev);

void BACKUP_Init();
void BACKUP2HC_callback(string db);

#endif /* BACKUPBLE_BACKUPBLE_HPP_ */
