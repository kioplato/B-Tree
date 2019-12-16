/*******************************************************************************
 * File: RD.h
 * Purpose: Implementation of API for interacting with records.
*******************************************************************************/

#include <stdio.h>

#include "RD.h"
#include "../accessmethod/AM.h"
#include "../filedesc/FD.h"

int RD_Init(Record* record, void* fieldA, void* fieldB)
{
	if (record == NULL) return AME_ERROR;
	if (fieldA == NULL) return AME_ERROR;
	if (fieldB == NULL) return AME_ERROR;

	record->fieldA = fieldA;
	record->fieldB = fieldB;

	return AME_OK;
}

int RD_Key_cmp(int file_desc, void* field1, void* field2, int* flag)
{
	char field_type;  // The type of keys.

	if (field1 == NULL) return AME_ERROR;
	if (field2 == NULL) return AME_ERROR;
	if (flag == NULL) return AME_ERROR;

	CALL_FD(FD_Get_attrType1(file_desc, &field_type));

	if (field_type == 'i') {
		int int_field1 = *(int*)field1;
		int int_field2 = *(int*)field2;
		if (int_field1 < int_field2)
			*flag = -1;
		else if (int_field1 == int_field2)
			*flag = 0;
		else if (int_field1 > int_field2)
			*flag = 1;
	}
	else if (field_type == 'f') {
		float float_field1 = *(float*)field1;
		float float_field2 = *(float*)field2;
		if (float_field1 < float_field2)
			*flag = -1;
		else if (float_field1 == float_field2)
			*flag = 0;
		else if (float_field1 > float_field2)
			*flag = 1;
	}
	else if (field_type == 'c') {
		*flag = strcmp((char*)field1, (char*)field2);
	}

	return AME_OK;
}

int RD_Split_Records(int file_desc_AM, Record records[], size_t n_records, size_t* splitter)
{
	size_t c_splitter;  // The current splitter, used for calculation.
	int cmp_flag;       // Flag used for comparing keys.

	if (splitter == NULL) return AME_ERROR;
	if (records == NULL) return AME_ERROR;

	/* Work towards the left side. */
	for (c_splitter = n_records / 2; c_splitter > 0; --c_splitter) {
		CALL_RD(RD_Key_cmp(file_desc_AM, records[c_splitter].fieldA, records[c_splitter - 1].fieldA, &cmp_flag));
		if (cmp_flag == 1) break;
	}

	printf("Left c_splitter: %ld.\n", c_splitter);
	*splitter = c_splitter;  // Store distance from left edge as temporary maximum.

	/* Work towards the right side. */
	for (c_splitter = n_records / 2; c_splitter < n_records - 1; ++c_splitter) {
		CALL_RD(RD_Key_cmp(file_desc_AM, records[c_splitter].fieldA, records[c_splitter + 1].fieldA, &cmp_flag));
		if (cmp_flag == -1) { break; }
	}

	printf("Right c_splitter: %ld.\n", c_splitter);

	/* Compare the distances between left edge split and right edge split. */
	if (*splitter < (n_records - 1) - c_splitter) *splitter = c_splitter + 1;

	return AME_OK;
}
