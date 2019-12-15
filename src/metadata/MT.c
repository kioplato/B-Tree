/*******************************************************************************
 * File: MT.c
 * Purpose: Implementation of API for interacting with the file's metadata file.
*******************************************************************************/

#include <stdio.h>
#include <string.h>

#include "../BF.h"
#include "../accessmethod/AM.h"

#define DESCCODE "BTREE"

int MT_Init(BF_Block* metablock, char attrType1, int attrLength1,
		char attrType2, int attrLength2)
{
	char desccode[] = DESCCODE;  // The code that varifies the file is an index file.
	char* metadata = NULL;       // The metadata block's data.
	char* offseted_metadata = NULL;  // The offseted metadata block's data.
	size_t root_index_block = 0;     // At the start there isn't an index block.

	if (metablock == NULL) return AME_ERROR;
	
	metadata = BF_Block_GetData(metablock);
	if (metadata == NULL) return AME_ERROR;

	offseted_metadata = metadata;
	memcpy((void*)offseted_metadata, (const void*)desccode, strlen(desccode));
	offseted_metadata += strlen(desccode);
	memcpy((void*)offseted_metadata, (const void*)&attrType1, sizeof(attrType1));
	offseted_metadata += sizeof(attrType1);
	memcpy((void*)offseted_metadata, (const void*)&attrLength1, sizeof(attrLength1));
	offseted_metadata += sizeof(attrLength1);
	memcpy((void*)offseted_metadata, (const void*)&attrType2, sizeof(attrType2));
	offseted_metadata += sizeof(attrType2);
	memcpy((void*)offseted_metadata, (const void*)&attrLength2, sizeof(attrLength2));
	offseted_metadata += sizeof(attrLength2);
	memcpy((void*)offseted_metadata, (const void*)&root_index_block, sizeof(root_index_block));

	return AME_OK;
}

int MT_GetData(BF_Block* metablock, char* attrType1, int* attrLength1,
		char* attrType2, int* attrLength2, size_t* root_index_block)
{
	char desccode[] = DESCCODE;  // The code that verifies the file is an index file.
	char* metadata;  // The metadata block's data.
	char* offseted_metadata;  // The offseted metadata for metadata's data traverse.

	if (metablock == NULL) return AME_ERROR;

	metadata = NULL;
	metadata = BF_Block_GetData(metablock);
	if (metadata == NULL) return AME_ERROR;

	offseted_metadata = metadata + strlen(desccode);  // Skip the descriptor code.
	memcpy((void*)attrType1, (const void*)offseted_metadata, sizeof(char));
	offseted_metadata += sizeof(char);
	memcpy((void*)attrLength1, (const void*)offseted_metadata, sizeof(int));
	offseted_metadata += sizeof(int);
	memcpy((void*)attrType2, (const void*)offseted_metadata, sizeof(char));
	offseted_metadata += sizeof(char);
	memcpy((void*)attrLength2, (const void*)offseted_metadata, sizeof(int));
	offseted_metadata += sizeof(int);
	memcpy((void*)root_index_block, (const void*)offseted_metadata, sizeof(size_t));

	return AME_OK;
}

int MT_WriteData(BF_Block* metablock, size_t root_index_block)
{
	char desccode[] = DESCCODE;  // The code that verifies the file is an index file.
	char* metadata = NULL;
	char* offseted_metadata = NULL;
	size_t* src = NULL;  // Source for memcpy.
	char* dst = NULL;    // Destination for memcpy.
	size_t write_bytes;  // Number of bytes to copy for memcpy.

	if (metablock == NULL) return AME_ERROR;

	metadata = BF_Block_GetData(metablock);
	if (metadata == NULL) return AME_ERROR;

	offseted_metadata = metadata;
	offseted_metadata += strlen(desccode);
	offseted_metadata += sizeof(char);  // Skip type of first field.
	offseted_metadata += sizeof(int);   // Skip length of first field.
	offseted_metadata += sizeof(char);  // Skip type of second field.
	offseted_metadata += sizeof(int);   // Skip length of second field.

	src = &root_index_block;
	dst = offseted_metadata;
	write_bytes = sizeof(root_index_block);
	memcpy((void*)dst, (const void*)src, write_bytes);

	return AME_OK;
}
