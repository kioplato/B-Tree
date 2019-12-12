/*******************************************************************************
 * File: IS.h
 * Purpose: API for interacting with the index scan array
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/bf.h"
#include "../include/AM.h"

#define AM_MAX_SCAN_FILES 20

#define CALL_IS(func_call)	\
{							\
	int code = func_call;	\
	if (code != AME_OK)		\
		AM_errno = code;	\
		return code;		\
}


/////////////////////////////////////////////
int scanIndexArray[AM_MAX_SCAN_FILES][3]; //next (counter inside data block), last_block, index_desc


#define AME_IS_MAX_FILES -15
#define AME_IS_INVALID_INDEX -16

///////////////////////////////////////////////
void IS_Init()
{
	for (size_t i = 0; i < AM_MAX_SCAN_FILES; ++i){
		scanIndexArray[i][0] = -1;
		scanIndexArray[i][1] = -1;
		scanIndexArray[i][2] = -1;
	}
}

int IS_Insert(int next, int last_block, int index_desc, *scan_index)
{
	/* File available index. */
	for (size_t i = 0; i < AM_MAX_SCAN_FILES; ++i) {
		if (scanIndexArray[i][2] == -1) {
			scanIndexArray[i][0]=next;
			scanIndexArray[i][1]=last_block;
			scanIndexArray[i][2]=index_desc;
			*scan_index = i;
			return AME_OK;
		}
	}
	return AME_IS_MAX_FILES;
}


int IS_Get_next(int index, size_t *next)
{
	if (scanIndexArray[index][2] == -1)
		return AME_IS_INVALID_INDEX;
	*next = scanIndexArray[index][0];
	return AME_OK;
}

int IS_Get_last_block(int index, char *last_block)
{
	if (scanIndexArray[index][2] == -1)
		return AME_IS_INVALID_INDEX;
	*last_block = scanIndexArray[index][1];
	return AME_OK;
}

int IS_Get_index_desc(int index, int *index_desc)
{
	if (scanIndexArray[index][2] == -1)
		return AME_IS_INVALID_INDEX;
	*index_desc = scanIndexArray[index][2];
	return AME_OK;
}


int IS_Set_next(int index, size_t next)
{
	if (scanIndexArray[index][2] == -1)
		return AME_IS_INVALID_INDEX;
	scanIndexArray[index][0] = next; 
	return AME_OK;
}

int IS_Set_last_block(int index, char last_block)
{
	if (scanIndexArray[index][2] == -1)
		return AME_IS_INVALID_INDEX;
	scanIndexArray[index][1] = last_block;
	return AME_OK;
}


int IS_Close(int index){
	if (scanIndexArray[index][2] == -1)
		return AME_IS_INVALID_INDEX;
	else{
		scanIndexArray[index][2] = -1;
		return AME_OK;
	}
}


