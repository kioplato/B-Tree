/*******************************************************************************
 * File: MT.h
 * Purpose: API for creating and interacting with blocks.
*******************************************************************************/

#ifndef BL_H
#define BL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "BF.h"
#include "AM.h"

#define CALL_BL(func_call)	\
{							\
	int code = func_call;	\
	if (code != AME_OK)		\
		AM_errno = code;	\
		return code;		\
}

/*
 * Create a new block and allocate it.
 *
 * Returns AME_OK on success and sets block to new block.
 * Returns AME_ERROR on failure.
 */
// TODO: return block id as well.
int BL_CreateBlock(int file_desc, BF_Block** block)
{
	*block = NULL;
	BF_Block_Init(block);
	if (*block == NULL) return AME_ERROR;
	
	CALL_BF(BF_AllocateBlock(file_desc, *block));

	return AME_OK;
}

int BL_LoadBlock(int file_desc, int block_id, BF_Block** block)
{
	*block = NULL;
	BF_Block_Init(block);
	if (*block == NULL) return AME_ERROR;

	CALL_BF(BF_GetBlock(file_desc, block_id, *block));

	return AME_OK;
}

#endif  // #ifndef BL_H
