/*******************************************************************************
 * File: test_AM_insert6.c
 * Purpose: Test AM insert. This test creates enough data blocks to split the
 * index blocks, creating three layers of index blocks. Then tests the range
 * queries.
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include "../src/accessmethod/AM.h"
#include "../src/block/BL.h"
#include "../src/indexblock/IB.h"
#include "../src/filedesc/FD.h"
#include "../src/datablock/DB.h"
#include "../src/defn.h"
#include "../src/index/BT.h"

int main(void)
{
	srand(13153562);

	char file_name[32] = "database_insert6.db";
	int file_desc_AM;

	AM_Init();

	printf(">>>>>>>>>>>>>>>>>>>\n");
	printf("Creating file %s.\n", file_name);
	if (AM_CreateIndex(file_name, STRING, 124, STRING, 16) != AME_OK) {
		fprintf(stderr, "Error when creating file %s.\n", file_name);
		return 1;
	}
	printf("Created file %s.\n", file_name);
	printf(">>>>>>>>>>>>>>>>>>>\n");

	printf(">>>>>>>>>>>>>>>>>>>\n");
	printf("Opening file %s.\n", file_name);
	if ((file_desc_AM = AM_OpenIndex(file_name)) < 0) {
		fprintf(stderr, "Error when opening file %s.\n", file_name);
		return 1;
	}
	printf("Opened file %s.\n", file_name);
	printf(">>>>>>>>>>>>>>>>>>>\n");

	char key[124];
	char value[16];

	char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

	void* key_ptr = &key;
	void* value_ptr = value;

	printf(">>>>>>>>>>>>>>>>>>>\n");
	printf("Inserting records..\n");
	size_t i;
	for (i = 0; i < 12; ++i) {
		key[0] = alphabet[rand() % 6];
		key[1] = alphabet[rand() % 6];
		key[2] = '\0';
		value[0] = alphabet[rand() % 6];
		value[1] = alphabet[rand() % 6];
		value[2] = '\0';
		printf("Inserting record %zu:\n", i);
		printf("Key: %s.\n", key);
		printf("Value: %s.\n", value);
		if (AM_InsertEntry(file_desc_AM, key_ptr, value_ptr) != AME_OK) {
			fprintf(stderr, "Failed to insert record.\n");
			exit(1);
		}
	}
	printf(">>>>>>>>>>>>>>>>>>>\n");

	printf(">>>>>>>>>>>>>>>>>>>\n");
	printf("Opening a new scan..\n");
	int scan_index;
	key[0] = 'D';
	key[1] = 'C';
	key[2] = '\0';
	scan_index = AM_OpenIndexScan(file_desc_AM, GREATER_THAN, (void*)key);
	printf(">>>>>>>>>>>>>>>>>>>\n");

	printf(">>>>>>>>>>>>>>>>>>>\n");
	printf("Finding entries..\n");
	char* result = NULL;
	result = (char*)AM_FindNextEntry(scan_index);
	printf("result: %s.\n", result);
	free(result);
	result = (char*)AM_FindNextEntry(scan_index);
	printf("result: %s.\n", result);
	free(result);
	result = (char*)AM_FindNextEntry(scan_index);
	printf("result: %s.\n", result);
	free(result);
	result = (char*)AM_FindNextEntry(scan_index);
	printf("result: %s.\n", result);
	free(result);
	result = (char*)AM_FindNextEntry(scan_index);
	printf("result: %s.\n", result);
	free(result);
	result = (char*)AM_FindNextEntry(scan_index);
	printf("result: %s.\n", result);
	free(result);
	result = (char*)AM_FindNextEntry(scan_index);
	printf("result: %s.\n", result);
	free(result);
	result = (char*)AM_FindNextEntry(scan_index);
	printf("result: %s.\n", result);
	free(result);
	result = (char*)AM_FindNextEntry(scan_index);
	printf("result: %s.\n", result);
	free(result);
	result = (char*)AM_FindNextEntry(scan_index);
	printf("result: %s.\n", result);
	free(result);
	result = (char*)AM_FindNextEntry(scan_index);
	printf("result: %s.\n", result);
	free(result);
	result = (char*)AM_FindNextEntry(scan_index);
	printf("result: %s.\n", result);
	free(result);
	result = (char*)AM_FindNextEntry(scan_index);
	printf("result: %s.\n", result);
	free(result);
	result = (char*)AM_FindNextEntry(scan_index);
	printf("result: %s.\n", result);
	free(result);
	printf(">>>>>>>>>>>>>>>>>>>\n");

	AM_CloseIndexScan(scan_index);

	BF_Block* block = NULL;
	CALL_BL(BL_LoadBlock(file_desc_AM, 1, &block));
	printf("Printing sublist.\n");
	CALL_DB(DB_Print_Sublist(file_desc_AM, block));
	CALL_BF(BF_UnpinBlock(block));
	BF_Block_Destroy(&block);

	printf(">>>>>>>>>>>>>>>>>>>\n");
	printf("Getting the index root block id to print it.\n");
	size_t index_root_id;  // The data block's id.

	CALL_FD(FD_Get_IndexRoot(file_desc_AM, &index_root_id));
	printf("Index root id: %ld.\n", index_root_id);

	size_t n_pointers;
	CALL_IB(IB_Get_MaxCountPointers(file_desc_AM, &n_pointers));
	printf("n_pointers: %ld.\n", n_pointers);

	int file_desc_BF;
	CALL_FD(FD_Get_FileDesc(file_desc_AM, &file_desc_BF));
	int n_blocks;
	CALL_BF(BF_GetBlockCounter(file_desc_BF, &n_blocks));
	printf("n_blocks: %d.\n", n_blocks);

	printf(">>>>>>>>>>>>>>>>>>>\n");
	printf("Closing file %s.\n", file_name);
	if (AM_CloseIndex(file_desc_AM) != AME_OK) {
		fprintf(stderr, "Error when closing file %s.\n", file_name);
		return 1;
	}
	printf("Closed file %s.\n", file_name);
	printf(">>>>>>>>>>>>>>>>>>>\n");

	AM_Close();

	return 0;
}
