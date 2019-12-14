/*******************************************************************************
 * File: IS.h
 * Purpose: API for interacting with the index scan array
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/bf.h"
#include "../include/AM.h"
#include "IS.h"




void IS_Init()
{
	for (size_t i = 0; i < AM_MAX_SCAN_FILES; ++i){
		scanIndexArray[i].index_desc = -1;
	}
}


int IS_OpenScan(int *scan_index){
  for (size_t i = 0; i < AM_MAX_SCAN_FILES; ++i){
    if (scanIndexArray[i].index_desc == -1) {
      *scan_index = i;
      return AME_OK;
  }
  return AME_IS_MAX_FILES;
}



int IS_Insert(int next, int last_block, int op, int index_desc, void* value, int scan_index)
{

		if (scanIndexArray[scan_index].index_desc == -1) {
			scanIndexArray[scan_index].next=next;
			scanIndexArray[scan_index].last_block=last_block;
			scanIndexArray[scan_index].op=op;
			scanIndexArray[scan_index].index_desc=index_desc;
			scanIndexArray[scan_index].value= (void*)malloc(sizeof(value));
			memcpy((void*)scanIndexArray[scan_index].value, (const void*)value, sizeof(value));
			return AME_OK;
	   }
	return AME_IS_MAX_FILES;
}


int IS_Get_next(int index, size_t *next)
{
	if (scanIndexArray[index].index_desc == -1)
		return AME_IS_INVALID_INDEX;
	*next = scanIndexArray[index].next;
	return AME_OK;
}

int IS_Get_last_block(int index, char *last_block)
{
	if (scanIndexArray[index].index_desc == -1)
		return AME_IS_INVALID_INDEX;
	*last_block = scanIndexArray[index].last_block;
	return AME_OK;
}

int IS_Get_op(int index, char *op)
{
	if (scanIndexArray[index].index_desc == -1)
		return AME_IS_INVALID_INDEX;
	*op = scanIndexArray[index].op;
	return AME_OK;
}


int IS_Get_index_desc(int index, int *index_desc)
{
	if (scanIndexArray[index].index_desc == -1)
		return AME_IS_INVALID_INDEX;
	*index_desc = scanIndexArray[index].index_desc;
	return AME_OK;
}

int IS_Get_value(int index, void *value)
{
	if (scanIndexArray[index].index_desc == -1)
		return AME_IS_INVALID_INDEX;
	*value = *(scanIndexArray[index].value);
	return AME_OK;
}



int IS_Set_next(int index, size_t next)
{
	if (scanIndexArray[index].index_desc == -1)
		return AME_IS_INVALID_INDEX;
	scanIndexArray[index].next = next;
	return AME_OK;
}

int IS_Set_last_block(int index, char last_block)
{
	if (scanIndexArray[index].index_desc == -1)
		return AME_IS_INVALID_INDEX;
	scanIndexArray[index].last_block = last_block;
	return AME_OK;
}


int IS_Close(int index){
	if (scanIndexArray[index].index_desc == -1)
		return AME_IS_INVALID_INDEX;
	else{
		scanIndexArray[index].index_desc = -1;
		return AME_OK;
	}
}

int IS_IsOpen(int index_desc){
	int i;
	for (i=0; i<AM_MAX_SCAN_FILES; i++){
		if (scanIndexArray[i].index_desc == index_desc){
			return AME_IS_OPEN;
		}
	}
	return AME_OK;
}
