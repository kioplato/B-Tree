/*******************************************************************************
 * File: BL.h
 * Purpose: API for creating and interacting with blocks.
*******************************************************************************/

#ifndef BL_H
#define BL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../BF.h"
#include "../accessmethod/AM.h"

#define CALL_BL(func_call)	\
{							\
	int code = func_call;	\
	if (code != AME_OK) {	\
		AM_errno = code;	\
		return code;		\
	}						\
}

/*
 * Create a new block and allocate it.
 *
 * Returns AME_OK on success and sets block to new block.
 * Returns AME_ERROR on failure.
 */
int BL_CreateBlock(int file_desc, int* block_id, BF_Block** block);

int BL_LoadBlock(int file_desc, int block_id, BF_Block** block);

#endif  // #ifndef BL_H
