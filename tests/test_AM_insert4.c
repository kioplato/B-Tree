/*******************************************************************************
 * File: test_AM_insert4.c
 * Purpose: Test AM insert. This test creates enough data blocks to split the
 * index root in two. This creates one new index block at split and one as the
 * new index root. The keys are integers.
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include "../src/accessmethod/AM.h"
#include "../src/block/BL.h"
#include "../src/indexblock/IB.h"
#include "../src/filedesc/FD.h"
#include "../src/datablock/DB.h"
#include "../src/defn.h"

int main(void)
{
	srand(13153562);

	char file_name[32] = "database_insert4.db";
	int file_desc_AM;

	AM_Init();

	printf(">>>>>>>>>>>>>>>>>>>\n");
	printf("Creating file %s.\n", file_name);
	if (AM_CreateIndex(file_name, INTEGER, sizeof(int), STRING, 16) != AME_OK) {
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

	int key;
	char value[16] = "Tiramisu";

	void* key_ptr = &key;
	void* value_ptr = value;

	printf(">>>>>>>>>>>>>>>>>>>\n");
	printf("Inserting records..\n");
	for (size_t i = 0; i < 100000; ++i) {
		key = rand() % 200000;
		printf("Inserting record %ld:\n", i);
		printf("Key: %d.\n", key);
		printf("Value: %s.\n", value);
		if (AM_InsertEntry(file_desc_AM, key_ptr, value_ptr) != AME_OK) {
			fprintf(stderr, "Failed to insert record.\n");
			exit(1);
		}
	}
	printf(">>>>>>>>>>>>>>>>>>>\n");

	printf(">>>>>>>>>>>>>>>>>>>\n");
	printf("Getting the index root block id to print it.\n");
	BF_Block* block;       // Used to get and print blocks.
	size_t index_root_id;  // The data block's id.

	CALL_FD(FD_Get_IndexRoot(file_desc_AM, &index_root_id));
	printf("Index root id: %ld.\n", index_root_id);

	size_t n_pointers;
	CALL_IB(IB_Get_MaxCountPointers(file_desc_AM, &n_pointers));
	printf("n_pointers: %ld.\n", n_pointers);

	CALL_BL(BL_LoadBlock(file_desc_AM, 1, &block));
	printf("Printing the data block 1.\n");
	CALL_DB(DB_Print(file_desc_AM, block));
	CALL_BF(BF_UnpinBlock(block));
	BF_Block_Destroy(&block);

	CALL_BL(BL_LoadBlock(file_desc_AM, 2, &block));
	printf("Printing the data block 2.\n");
	CALL_DB(DB_Print(file_desc_AM, block));
	CALL_BF(BF_UnpinBlock(block));
	BF_Block_Destroy(&block);

	CALL_BL(BL_LoadBlock(file_desc_AM, 4, &block));
	printf("Printing the data block 4.\n");
	CALL_DB(DB_Print(file_desc_AM, block));
	CALL_BF(BF_UnpinBlock(block));
	BF_Block_Destroy(&block);

	CALL_BL(BL_LoadBlock(file_desc_AM, 40, &block));
	printf("Printing the data block 40.\n");
	CALL_DB(DB_Print(file_desc_AM, block));
	CALL_BF(BF_UnpinBlock(block));
	BF_Block_Destroy(&block);

	CALL_BL(BL_LoadBlock(file_desc_AM, 3, &block));
	printf("Printing the index block 3.\n");
	CALL_IB(IB_Print(file_desc_AM, block));
	CALL_BF(BF_UnpinBlock(block));
	BF_Block_Destroy(&block);

	CALL_BL(BL_LoadBlock(file_desc_AM, 67, &block));
	printf("Printing the index block 67.\n");
	CALL_IB(IB_Print(file_desc_AM, block));
	CALL_BF(BF_UnpinBlock(block));
	BF_Block_Destroy(&block);

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
