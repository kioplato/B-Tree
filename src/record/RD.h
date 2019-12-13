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
	if (code != AME_OK)		\
		AM_errno = code;	\
		return code;		\
}

/*
 * Initialize a record with the provided fields.
 *
 * Returns AME_OK on success.
 * Returns AME_ERROR on failure.
 */
int RD_Init(Record* record, void* fieldA, void* fieldB);

#endif  // #ifndef RD_H
