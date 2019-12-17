/*******************************************************************************
 * File: test_AM_insert1.c
 * Purpose: Test AM insert. This test creates only one data block which is the
 * root of the index. No index blocks are created.
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include "../src/accessmethod/AM.h"
#include "../src/block/BL.h"
#include "../src/filedesc/FD.h"
#include "../src/datablock/DB.h"
#include "../src/defn.h"

int main(void)
{
	srand(13153562);

	char file_name[32] = "database_insert.db";
	int file_desc_AM;

	AM_Init();

	printf(">>>>>>>>>>>>>>>>>>>\n");
	printf("Creating file %s.\n", file_name);
	if (AM_CreateIndex(file_name, INTEGER, sizeof(int), STRING, 8) != AME_OK) {
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
	char value[8] = "Sazerac";

	void* key_ptr = &key;
	void* value_ptr = value;

	printf(">>>>>>>>>>>>>>>>>>>\n");
	printf("Inserting records..\n");
	for (size_t i = 0; i < 10; ++i) {
		key = rand() % 50;
		printf("Inserting record:\n");
		printf("Key: %d.\n", key);
		printf("Value: %s.\n", value);
		AM_InsertEntry(file_desc_AM, key_ptr, value_ptr);
	}
	printf(">>>>>>>>>>>>>>>>>>>\n");

	printf(">>>>>>>>>>>>>>>>>>>\n");
	printf("Getting the data block to print it.\n");
	BF_Block* block;  // The only data block that exists with 10 records.
	size_t datablock_id;  // The data block's id.

	CALL_FD(FD_Get_IndexRoot(file_desc_AM, &datablock_id));
	printf("Index root id: %ld.\n", datablock_id);
	CALL_BL(BL_LoadBlock(file_desc_AM, datablock_id, &block));
	printf("Printing the data block.\n");
	CALL_DB(DB_Print(file_desc_AM, block));
	CALL_BF(BF_UnpinBlock(block));
	BF_Block_Destroy(&block);

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
