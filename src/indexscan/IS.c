/*******************************************************************************
 * File: IS.h
 * Purpose: API for interacting with the index scan array.
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../BF.h"
#include "../accessmethod/AM.h"
#include "IS.h"
#include "../filedesc/FD.h"

void IS_Init()
{
	for (size_t i = 0; i < AM_MAX_SCAN_FILES; ++i)
		scanIndexArray[i].index_desc_AM = -1;
}

int IS_Insert(int next, int last_block, int op, int index_desc_AM, void* value,
		int* scan_index)
{
	int key_length;
	CALL_FD(FD_Get_attrLength1(index_desc_AM, &key_length));

	for (size_t i = 0; i < AM_MAX_SCAN_FILES; ++i) {
		if (scanIndexArray[i].index_desc_AM == -1) {
			scanIndexArray[i].index_desc_AM = index_desc_AM;
			scanIndexArray[i].last_block = last_block;
			scanIndexArray[i].next = next;
			scanIndexArray[i].op = op;
			scanIndexArray[i].value = malloc(key_length);
			memcpy((void*)scanIndexArray[i].value, (const void*)value, key_length);
			*scan_index = i;
			return AME_OK;
		}
	}

	return AME_IS_MAX_FILES;
}

int IS_Get_next(int index, int* next)
{
	if (next == NULL) return AME_ERROR;

	if (scanIndexArray[index].index_desc_AM == -1)
		return AME_IS_INVALID_INDEX;

	*next = scanIndexArray[index].next;

	return AME_OK;
}

int IS_Get_last_block(int index, int* last_block)
{
	if (last_block == NULL) return AME_ERROR;

	if (scanIndexArray[index].index_desc_AM == -1)
		return AME_IS_INVALID_INDEX;

	*last_block = scanIndexArray[index].last_block;

	return AME_OK;
}

int IS_Get_op(int index, int* op)
{
	if (op == NULL) return AME_ERROR;

	if (scanIndexArray[index].index_desc_AM == -1)
		return AME_IS_INVALID_INDEX;

	*op = scanIndexArray[index].op;

	return AME_OK;
}

int IS_Get_index_desc(int index, int* index_desc)
{
	if (index_desc == NULL) return AME_ERROR;

	if (scanIndexArray[index].index_desc_AM == -1)
		return AME_IS_INVALID_INDEX;

	*index_desc = scanIndexArray[index].index_desc_AM;

	return AME_OK;
}

int IS_Get_value(int index, void** value)
{
	if (value == NULL) return AME_ERROR;

	if (scanIndexArray[index].index_desc_AM == -1)
		return AME_IS_INVALID_INDEX;

	*value = scanIndexArray[index].value;

	return AME_OK;
}

int IS_Set_next(int index, int next)
{
	if (scanIndexArray[index].index_desc_AM == -1)
		return AME_IS_INVALID_INDEX;

	scanIndexArray[index].next = next;

	return AME_OK;
}

int IS_Set_last_block(int index, int last_block)
{
	if (scanIndexArray[index].index_desc_AM == -1)
		return AME_IS_INVALID_INDEX;

	scanIndexArray[index].last_block = last_block;

	return AME_OK;
}

int IS_Close(int index)
{
	if (scanIndexArray[index].index_desc_AM == -1)
		return AME_IS_INVALID_INDEX;

	free(scanIndexArray[index].value);

	scanIndexArray[index].index_desc_AM = -1;

	return AME_OK;
}

int IS_IsOpen(int index_desc, int* flag)
{
	if (flag == NULL) return AME_ERROR;

	for (size_t i = 0; i < AM_MAX_SCAN_FILES; ++i){
		if (scanIndexArray[i].index_desc_AM == index_desc) {
			*flag = 1;
			return AME_OK;
		}
	}
	*flag = 0;

	return AME_OK;
}
