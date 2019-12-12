/*******************************************************************************
 * File: BL.c
 * Purpose: Implementation of API for creating and interacting with blocks.
*******************************************************************************/

#include <stdio.h>

#include "BL.h"
#include "../BF.h"
#include "../accessmethod/AM.h"

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
