/*******************************************************************************
 * File: RD.h
 * Purpose: API for interacting with records.
*******************************************************************************/

#ifndef RD_H
#define RD_H

#include "AM.h"
#include "FD.h"

int RD_Init(int file_desc, Record* record, void* fieldA, void* fieldB)
{
	if (record == NULL) return AME_ERROR;
	if (fieldA == NULL) return AME_ERROR;
	if (fieldB == NULL) return AME_ERROR;

	record->fieldA = fieldA;
	record->fieldB = fieldB;

	return AME_OK;
}

#endif  // #ifndef RD_H
