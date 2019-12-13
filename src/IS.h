/*******************************************************************************
 * File: IS.h
 * Purpose: API for interacting with the index scan array
*******************************************************************************/

#ifndef IS_H
#define IS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "BF.h"
#include "filedesc/FD.h"
#include "accessmethod/AM.h"

#define AM_MAX_SCAN_FILES 20

#define CALL_IS(func_call)	\
{							\
	int code = func_call;	\
	if (code != AME_OK)		\
		AM_errno = code;	\
		return code;		\
}


/////////////////////////////////////////////

#define AME_IS_MAX_FILES -15
#define AME_IS_INVALID_INDEX -16

///////////////////////////////////////////////

struct scan_t {
	int next;
	int last_block;
	int op;
	int index_desc;
	void* value;
};

struct scan_t scanIndexArray[AM_MAX_SCAN_FILES];

void IS_Init()
{
	for (size_t i = 0; i < AM_MAX_SCAN_FILES; ++i){
		scanIndexArray[i].index_desc=-1;
	}
}

int IS_Insert(int next, int last_block, int op, int index_desc, void* value, *scan_index)
{
	/* File available index. */
	for (size_t i = 0; i < AM_MAX_SCAN_FILES; ++i) {
		if (scanIndexArray[i].index_desc == -1) {
			scanIndexArray[i].next=next;
			scanIndexArray[i].last_block=last_block;
			scanIndexArray[i].op=op;
			scanIndexArray[i].index_desc=index_desc;
			scanIndexArray[i].value= (void*)malloc(sizeof(value));     SKATAAAAAAAAAAAA
			memcpy((void*)scanIndexArray[i].value, (const void*)value, sizeof(value));
			*scan_index = i;
			return AME_OK;
		}
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

int FD_Get_filedesc(int index, size_t *filedesc)
{
	if (filedescs[index].filedesc == -1)
		return AME_FD_INVALID_INDEX;
	*filedesc = filedescs[index].filedesc;
	return AME_OK;
}

#endif  // #ifndef IS_H
