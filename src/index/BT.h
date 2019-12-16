/*******************************************************************************
 * File: BT.h
 * Purpose: API for operating on B+-Tree.
*******************************************************************************/

#ifndef BT_H
#define BT_H

#include <stdio.h>

#include "../record/RD.h"

#define CALL_BT(func_call)	\
{							\
	int code = func_call;	\
	if (code != AME_OK) {	\
		AM_errno = code;	\
		return code;		\
	}						\
}

/*
 * Get the pointer that the key should live at.
 * The returned block id could be a data block or index block.
 *
 * Returns AME_OK on success.
 * Returns AME_ERROR on failure.
 */
int BT_Get_SubtreeRoot(int file_desc_AM, BF_Block* block, void* key, int* pointer);

/*
 * Searches the correct data block to insert record to.
 * Returns
 * Calls DBL API to insert the record.
 *
 * Returns the block id of the block that inserted the record to.
 * If returned value is different from the one provided it means that the
 * index or the data block split.
 */
int BT_Subtree_Insert(int file_desc, size_t subtree_root, size_t* overflow_root, Record record);

#endif  // #ifndef BT_H
