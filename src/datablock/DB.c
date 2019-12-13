/*******************************************************************************
 * File: DB.c
 * Purpose: API for interacting with Data Blocks.
*******************************************************************************/

#include <stdio.h>
#include <string.h>

#include "../BF.h"
#include "../accessmethod/AM.h"
#include "../filedesc/FD.h"
#include "../defn.h"
#include "DB.h"

int DB_Init(BF_Block* block, size_t next_block)
{
	char* data = NULL;  // The block's data.
	char* offseted_data = NULL;  // The offseted data for data traversal.
	char datacode[] = DATACODE;
	size_t c_entries = 0;  // The block's current number of entries.

	if (block == NULL) return AME_ERROR;

	data = NULL;
	data = BF_Block_GetData(block);
	if (data == NULL) return AME_ERROR;

	offseted_data = data;
	memcpy((void*)offseted_data, (const void*)datacode, strlen(datacode));
	offseted_data += strlen(datacode);
	memcpy((void*)offseted_data, (const void*)&c_entries, sizeof(c_entries));
	offseted_data += sizeof(c_entries);
	memcpy((void*)offseted_data, (const void*)&next_block, sizeof(next_block));

	return AME_OK;
}

int DB_Is_DataBlock(BF_Block* block, int* flag)
{
	char* data;
	char datacode[] = DATACODE;  // The code for Data Blocks.

	if (block == NULL) return AME_ERROR;

	data = NULL;
	data = BF_Block_GetData(block);
	if (data == NULL) return AME_ERROR;

	if (memcmp((const void*)data, (const void*)datacode, strlen(datacode)) == 0)
		*flag = 1;
	else
		*flag = 0;

	return AME_OK;
}

int DB_Get_NextBlock(BF_Block* block, size_t* next_block)
{
	char* data;           // Block's data.
	char* offseted_data;  // For traversing the data.

	if (block == NULL) return AME_ERROR;
	if (next_block == NULL) return AME_ERROR;

	data = NULL;
	data = BF_Block_GetData(block);
	if (data == NULL) return AME_ERROR;

	offseted_data = data;
	offseted_data += strlen(DATACODE);
	offseted_data += sizeof(size_t);

	memcpy((void*)next_block, (const void*)offseted_data, sizeof(size_t));

	return AME_OK;
}

int DB_Get_Entries(BF_Block* block, size_t* c_entries)
{
	char* data;           // Block's data.
	char* offseted_data;  // For traversing the data.
	
	if (block == NULL) return AME_ERROR;
	if (c_entries == NULL) return AME_ERROR;

	data = NULL;
	data = BF_Block_GetData(block);
	if (data == NULL) return AME_ERROR;

	offseted_data = data;
	offseted_data += strlen(DATACODE);

	memcpy((void*)c_entries, (const void*)offseted_data, sizeof(size_t));

	return AME_OK;
}

int DB_Get_MaxEntries(int file_desc, size_t* n_entries)
{
	char datacode[] = DATACODE;  // Used to calculate records per block.
	size_t metadata_size;    // The metadata size of a Data Block.
	size_t record_max_size;
	int lenA;
	int lenB;

	if (n_entries == NULL) return AME_ERROR;

	/* Metadata contains the data code, current block entries and next block. */
	metadata_size = strlen(datacode) + sizeof(size_t) + sizeof(size_t);

	CALL_FD(FD_Get_attrLength1(file_desc, &lenA));
	CALL_FD(FD_Get_attrLength2(file_desc, &lenB));
	record_max_size = lenA + lenB;

	/* Calculate max entries. */
	*n_entries = (BF_BLOCK_SIZE - metadata_size) / record_max_size;

	return AME_OK;
}

int DB_Get_Record(int file_desc, BF_Block* block, Record* record, size_t c_entry, int* flag)
{
	size_t n_entries;  // The maximum number of entries in a Data Block.
	size_t c_entries;  // The current amount of entries in the Data Block.
	size_t record_max_size;
	int lenA;
	int lenB;

	char* data;  // The block's data.
	char* offseted_data;  // Block's data for traversing contents.

	if (block == NULL) return AME_ERROR;
	if (record == NULL) return AME_ERROR;
	if (flag == NULL) return AME_ERROR;

	CALL_DB(DB_Get_MaxEntries(file_desc, &n_entries));

	if (c_entry >= n_entries) {
		*flag = -1;
		return AME_OK;
	}

	CALL_DB(DB_Get_Entries(block, &c_entries));

	if (c_entry >= c_entries) {
		*flag = 0;
		return AME_OK;
	}

	CALL_FD(FD_Get_attrLength1(file_desc, &lenA));
	CALL_FD(FD_Get_attrLength2(file_desc, &lenB));
	record_max_size = lenA + lenB;

	data = NULL;
	data = BF_Block_GetData(block);
	if (data == NULL) return AME_ERROR;

	offseted_data = data;
	offseted_data += strlen(DATACODE);
	offseted_data += sizeof(size_t);
	offseted_data += sizeof(size_t);

	offseted_data += record_max_size * c_entry;
	
	memcpy((void*)record->fieldA, (const void*)offseted_data, record->lengthA);
	offseted_data += record->lengthA;
	memcpy((void*)record->fieldB, (const void*)offseted_data, record->lengthB);

	*flag = 1;

	return AME_OK;
}

int DB_Write_Record(int file_desc, BF_Block* block, Record record, size_t c_entry, int* flag)
{
	size_t n_entries;  // The maximum number of entries a block can store.
	size_t c_entries;  // The current number of entries in block.

	size_t record_size;  // The maximum record size in bytes.
	int lengthA;      // The length of the first field.
	int lengthB;      // The length of the second field.

	char* data;          // Block's data.
	char* offseted_data; // Block's data for traversal.
	
	char datacode[] = DATACODE;  // The Data Block code for validity.
	size_t metadata_size;        // The size of metadata in bytes.

	char typeA;
	char typeB;

	size_t write_bytes;

	/* Check for invalid arguments. */
	if (block == NULL) return AME_ERROR;
	if (flag == NULL) return AME_ERROR;
	if (record.fieldA == NULL) return AME_ERROR;
	if (record.fieldB == NULL) return AME_ERROR;

	/* Check the maximum number of records. */
	CALL_DB(DB_Get_MaxEntries(file_desc, &n_entries));
	if (c_entry >= n_entries) {
		*flag = -1;
		return AME_OK;
	}

	/* Get block's current entries. */
	CALL_DB(DB_Get_Entries(block, &c_entries));
	if (c_entry >= c_entries) {
		*flag = 0;
		return AME_OK;
	}

	/* Get the types of the fields. */
	CALL_FD(FD_Get_attrType1(file_desc, &typeA));
	CALL_FD(FD_Get_attrType2(file_desc, &typeB));

	/* Get the lengths of the fields. */
	CALL_FD(FD_Get_attrLength1(file_desc, &lengthA));
	CALL_FD(FD_Get_attrLength2(file_desc, &lengthB));
	record_size = lengthA + lengthB;

	data = NULL;
	data = BF_Block_GetData(block);
	if (data == NULL) return AME_ERROR;

	offseted_data = data;
	offseted_data += strlen(datacode);
	offseted_data += sizeof(size_t);  // Skip current entries.
	offseted_data += sizeof(size_t);  // Skip next block.
	offseted_data += c_entry * record_size;

	/* Calculate the write_bytes for first field. */
	/* Write only the initialized bytes. */
	if (typeA == INTEGER || typeA == FLOAT) {
		write_bytes = lengthA;
	}
	else if (typeA == STRING) {
		write_bytes = strlen((char*)record.fieldA) + 1; // + 1 for NULL byte.
	}
	memcpy((void*)offseted_data, (const void*)record.fieldA, write_bytes);

	/* Progress the offseted_data by the maximum length of first field. */
	offseted_data += lengthA;

	if (typeB == INTEGER || typeB == FLOAT) {
		write_bytes = lengthB;
	}
	else if (typeB == STRING) {
		write_bytes = strlen((char*)record.fieldB) + 1; // + 1 for NULL byte.
	}
	memcpy((void*)offseted_data, (const void*)record.fieldB, write_bytes);

	*flag = 1;

	return AME_OK;
}

int DB_Shift_Records_Right(int file_desc, BF_Block* block, size_t shift_base, int* flag)
{
	size_t c_entries;  // The number of entries the current block has.
	size_t n_entries;  // The maximum number of entries the current block has.

	size_t record_size;  // The maximum size of a record.
	size_t lengthA;  // The length of the first field.
	size_t lengthB;  // The length of the second field.

	char* data;
	char* offseted_data;
	char datacode[] = DATACODE;

	size_t move_bytes;

	if (block == NULL) return AME_ERROR;
	if (flag == NULL) return AME_ERROR;

	/* We guarantee the Data Block is not full. */
	CALL_DB(DB_Get_Entries(block, &n_entries));
	if (shift_base >= n_entries) {
		*flag = 0;
		return AME_OK;
	}

	CALL_DB(DB_Get_Entries(block, &c_entries));
	if (shift_base >= c_entries) {
		*flag = 0;
		return AME_OK;
	}

	CALL_FD(FD_Get_attrLength1(file_desc, &lengthA));
	CALL_FD(FD_Get_attrLength2(file_desc, &lengthB));
	record_size = lengthA + lengthB;

	data = NULL;
	data = BF_Block_GetData(block);
	if (data == NULL) return AME_ERROR;

	offseted_data = data;
	offseted_data += strlen(datacode);
	offseted_data += sizeof(size_t);  // Skip c_entries.
	offseted_data += sizeof(size_t);  // Skip next_block.
	offseted_data += shift_base * record_size;

	move_bytes = (shift_base - c_entries) * record_size;

	/*
	 * Could have implemented this with a reverse iteration of the records.
	 * Get_Record and then write it after record_size bytes.
	 * I chose to implement it using memmove.
	 * Probably the Get_Record would have been cleaner.
	 */
	memmove((void*)(offseted_data + record_size), (void*)offseted_data, move_bytes);

	*flag = 1;

	return AME_OK;
}

int DB_Insert(int file_desc, BF_Block* block, Record record, int* flag)
{
	size_t n_entries;  // Total entries a Data Block can store.
	size_t c_entries;  // Current entries the data block has.
	int status;        // The status of the record we get from block.
	Record c_record;   // Record for iterating block.

	if (block == NULL) return AME_ERROR;

	CALL_DB(DB_Get_MaxEntries(file_desc, &n_entries));

	/* Get the block's current entries. */
	CALL_DB(DB_Get_Entries(block, &c_entries));

	/* If block is full set flag and return. We guarantee block isn't full. */
	if (c_entries == n_entries) {
		*flag = 0;
		return AME_OK;
	}

	/* Find correct insert position to maintain sortness. */
	for (size_t i = 0; i < c_entries; ++i) {

		CALL_DB(DB_Get_Record(file_desc, block, &c_record, i, &status));

		/* Check the status of the record. Status = 0 or -1 shouldn't happen. */
		if (status == 1) {  // Record successfully fetched.
			// TODO: Compare the records.
			CALL_DB(DB_Shift_Records_Right(file_desc, block, i, &status));
			if (status != 1) return AME_ERROR;  // status should be 1.
			CALL_DB(DB_Write_Record(file_desc, block, c_record, i, &status));
			if (status != 1) return AME_ERROR;  // Write should succeed.
		} else
			return AME_ERROR;
	}
	
	/* Insert record here. */
	/* Data Block isn't full because we checked it. */
	CALL_DB(DB_Write_Record(file_desc, block, record, c_entries, &status));
	if (status != 1) return AME_ERROR;

	return AME_OK;
}
