/*
 * RingBuffer.c
 */

#include "RingBuffer.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

bool ring_init(ringbuffer_t *cb, size_t capacity, size_t sz){
	cb->buffer = malloc(capacity * sz);  // cap phat bo nho dong
	if(cb->buffer == NULL){
		printf("ring_init FAIL! \n");
		return false;
	}
	//printf("ring_init success! \n");

	cb->buffer_end = (char *)cb->buffer + capacity * sz;
	cb->capacity = capacity;
	cb->count = 0;
	cb->sz = sz;
	cb->head = cb->buffer;
	cb->tail = cb->buffer;
	return true;
}
bool ring_free(ringbuffer_t *cb)
{
	if(cb->buffer){
		free(cb->buffer);
		cb->buffer = NULL;
	}else{
		return false;
	}
	return true;
}
bool ring_push_head(ringbuffer_t *cb, const void *item)
{
	if(cb->count == cb->capacity){
		//printf("Ringbuffer is Full ! \n");
		return false;
	}

	memcpy(cb->head, item, cb->sz);
	cb->head = (char*)cb->head + cb->sz;
	if(cb->head == cb->buffer_end)
	cb->head = cb->buffer;
	cb->count++;

	return true;
}
bool ring_pop_tail(ringbuffer_t *cb, void *item)
{
	if(cb->count == 0){
		//printf("Ringbuffer is Empty ! \n");
		return false;
	}

	memcpy(item, cb->tail, cb->sz);
	cb->tail = (char*)cb->tail + cb->sz;
	if(cb->tail == cb->buffer_end)
	cb->tail = cb->buffer;
	cb->count--;
	return true;
}
