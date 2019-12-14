/*******************************************************************************
 * File: BT.h
 * Purpose: API for operating on B+-Tree.
*******************************************************************************/

#ifndef RD_H
#define RD_H

#define CALL_BT(func_call)	\
{							\
	int code = func_call;	\
	if (code != AME_OK)		\
		AM_errno = code;	\
		return code;		\
}

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

#endif // #ifndef RD_H
