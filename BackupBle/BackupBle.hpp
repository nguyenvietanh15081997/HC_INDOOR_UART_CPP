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

void BACKUP_updateNetKey();
void BACKUP_updateNewDevKey();
void BACKUP_updateAppKey();
void BACKUP_gw();
void BACKUP_dev() ;
void BACKUP_callback();

#endif /* BACKUPBLE_BACKUPBLE_HPP_ */
