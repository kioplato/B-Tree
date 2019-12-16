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

/*
 * Split the records array in the most optimal way.
 * Takes care of multiple identical key values.
 * Must be at least one value different than the others.
 *
 * n_records: the number of records in records array.
 * splitter: where the right block starts.
 *
 * Returns AME_OK on success.
 * Returns AME_ERROR on failure.
 */
int RD_Split_Records(int file_desc_AM, Record records[], size_t n_records, size_t* splitter);

#endif  // #ifndef RD_H
