/*
 * Linkerlist.c
 *
 *  Created on: Nov 6, 2021
 *      Author: anh
 */

#include "LinkerList.hpp"

vrts_LL_bufferUart head;

static vrts_LL_bufferUart Create_buffer(cmdcontrol_t frameUart){
	vrts_LL_bufferUart newdata;
	newdata = (vrts_LL_bufferUart)malloc(sizeof(struct LL_bufferUart));
	newdata->dataUart = frameUart;
	newdata->next = NULL;
	return newdata;
}

vrts_LL_bufferUart AddTail(cmdcontrol_t frameUart)
{
	vrts_LL_bufferUart temp,p;
	temp=Create_buffer(frameUart);
	if(head == NULL){
		head = temp;
	}
	else{
		p=head;
		while(p->next != NULL){
			p=p->next;
		}
		p->next = temp;
	}
	return head;
}

vrts_LL_bufferUart DellHead(){
	vrts_LL_bufferUart newhead;
	if(head != NULL){
		newhead = head->next;
		free(head);
	}
	return newhead;
}

