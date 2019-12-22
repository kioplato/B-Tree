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
 * If the data block splits then index blocks insert/split at backtrack.
 *
 * If splitted == 0 when function completes execution it means that the provided
 * subtree root splitted. Most likely you would want to create a new index root
 * and insert the pointer-key-pointer.
 */
int BT_Subtree_Insert(int file_desc_AM, int subtree_root, Record record,
		int* pointer1, void** key, int* pointer2, int* splitted);

/*
 * Searches the B+-Tree and returns the data block id in which the key should
 * exist. It may not exist but that's where it would be if it exists.
 * Writes the data block id at block_id.
 *
 * Returns AME_OK on success.
 * Returns AME_ERROR on failure.
 */
int BT_Get_SubtreeLeaf(int file_desc_AM, int subtree_root, void* key,
		int* block_id);

#endif  // #ifndef BT_H
