/*******************************************************************************
 * File: MT.h
 * Purpose: API for creating and interacting with blocks.
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/bf.h"
#include "../include/AM.h"

#define CALL_BL(func_call)	\
{							\
	int code = func_call;	\
	if (code != AME_OK)		\
		AM_errno = code;	\
		return code;		\
}

/*
 * Create a new block and allocate it.
 * Return the new block at block.
 */
int BL_CreateBlock(int file_desc, BF_Block** block) {
	*block = NULL;
	BF_Block_Init(block);
	if (*block == NULL) return AME_ERROR;
	
	CALL_BF(BF_AllocateBlock(file_desc, *block));

	return AME_OK;
}
