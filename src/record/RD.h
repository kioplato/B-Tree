/*******************************************************************************
 * File: RD.h
 * Purpose: API for interacting with records.
*******************************************************************************/

#ifndef RD_H
#define RD_H

#include <stdio.h>

#include "../accessmethod/AM.h"

#define CALL_RD(func_call)	\
{							\
	int code = func_call;	\
	if (code != AME_OK) {	\
		AM_errno = code;	\
		return code;		\
	}						\
}

typedef struct Record {
	void* fieldA;
	void* fieldB;
} Record;

/*
 * Initialize a record with the provided fields.
 *
 * Returns AME_OK on success.
 * Returns AME_ERROR on failure.
 */
int RD_Init(Record* record, void* fieldA, void* fieldB);

/*
 * Checks if field1 is less than, equal or more than field2.
 * If field1 < field2 then flag = -1.
 * If field1 = field2 then flag = 0.
 * If field1 > field2 then flag = 1.
 *
 * Returns AME_OK on success.
 * Returns AME_ERROR on failure.
 */
int RD_Key_cmp(int file_desc, void* field1, void* field2, int* flag);

#endif  // #ifndef RD_H
