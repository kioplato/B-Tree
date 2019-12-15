/*******************************************************************************
 * File: MT.h
 * Purpose: API for interacting with the file's metadata file.
*******************************************************************************/

#ifndef MT_H
#define MT_H

#include <stdio.h>
#include <string.h>

#include "../BF.h"
#include "../accessmethod/AM.h"

#define CALL_MT(func_call)	\
{							\
	int code = func_call;	\
	if (code != AME_OK) {	\
		AM_errno = code;	\
		return code;		\
	}						\
}

/*
 * Gives the formulation of a metadata block to the provided block.
 *
 * Returns AME_OK on success.
 * Returns AME_ERROR on failure.
 */
int MT_Init(BF_Block* metablock, char attrType1, int attrLength1,
		char attrType2, int attrLength2);

/*
 * Updates the metadata.
 * Currently only the index's root may change.
 *
 * Returns AME_OK on success.
 * Returns AME_ERROR on failure.
 */
int MT_WriteData(BF_Block* metablock, size_t root_index_block);

/*
 * Gets the metadata block's data.
 * Writes the contents to the pointers.
 *
 * Returns AME_OK on success.
 * Returns AME_ERROR on failure.
 */
int MT_GetData(BF_Block* metablock, char* attrType1, int* attrLength1,
		char* attrType2, int* attrLength2, size_t* root_index_block);

#endif  // #ifndef MT_H
