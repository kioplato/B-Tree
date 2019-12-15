/*******************************************************************************
 * File: BL.c
 * Purpose: Implementation of API for creating and interacting with blocks.
*******************************************************************************/

#include <stdio.h>

#include "BL.h"
#include "../BF.h"
#include "../accessmethod/AM.h"

int BL_CreateBlock(int file_desc_BF, int* block_id, BF_Block** block)
{
	if (block_id == NULL) return AME_ERROR;
	if (block == NULL) return AME_ERROR;

	*block = NULL;
	BF_Block_Init(block);
	if (*block == NULL) return AME_ERROR;
	
	CALL_BF(BF_AllocateBlock(file_desc_BF, *block));

	CALL_BF(BF_GetBlockCounter(file_desc_BF, block_id));
	(*block_id)--;

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
