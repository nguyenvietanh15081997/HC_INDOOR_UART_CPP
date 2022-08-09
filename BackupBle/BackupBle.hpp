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

void BACKUP_updateNetKey(char *db);
void BACKUP_updateNewDevKey();
void BACKUP_updateAppKey(char *db);
void BACKUP_gw(char *db);
void BACKUP_dev(char *db) ;
void BACKUP_callback(char *dir);

#endif /* BACKUPBLE_BACKUPBLE_HPP_ */
