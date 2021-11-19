/*
 * LinkerList.h
 *
 *  Created on: Nov 6, 2021
 *      Author: anh
 */

#ifndef LINKERLIST_HPP_
#define LINKERLIST_HPP_


#include <stdio.h> 
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "../Include/Include.hpp"

struct LL_bufferUart{
	cmdcontrol_t dataUart;
	struct LL_bufferUart *next;
};
typedef struct LL_bufferUart *vrts_LL_bufferUart;

extern vrts_LL_bufferUart head;

vrts_LL_bufferUart AddTail(cmdcontrol_t frameUart);

vrts_LL_bufferUart DellHead();

#endif /* PROCESSUART_LINKERLIST_H_ */
