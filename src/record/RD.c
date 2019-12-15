/*******************************************************************************
 * File: RD.h
 * Purpose: Implementation of API for interacting with records.
*******************************************************************************/

#include <stdio.h>

#include "RD.h"
#include "../accessmethod/AM.h"

int RD_Init(Record* record, void* fieldA, void* fieldB)
{
	if (record == NULL) return AME_ERROR;
	if (fieldA == NULL) return AME_ERROR;
	if (fieldB == NULL) return AME_ERROR;

	record->fieldA = fieldA;
	record->fieldB = fieldB;

	return AME_OK;
}
