/*******************************************************************************
 *  file: test_indexblock.c
 *
 *  purpose: testing of IB functions.
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include "../src/accessmethod/AM.h"
#include "../src/filedesc/FD.h"
#include "../src/BF.h"
#include "../src/block/BL.h"
#include "../src/indexblock/IB.h"
#include "../src/defn.h"

/*
#include "../src/record/RD.h"
#include "../src/datablock/DB.h"
*/

int main(void)
{
	AM_Init();

	char filename[] = "database.db";
	int file_desc_AM;
	int file_desc_BF;

	/* Create the file. */
	printf("> Create the file.\n");

	if (AM_CreateIndex(filename, STRING, 128, FLOAT, sizeof(float)) != AME_OK) {
		fprintf(stderr, "AM_CreateIndex() failed on %s.\n", filename);
		exit(1);
	}
	printf("Created file %s\n", filename);

	/* Open the file. */
	printf("> Open the file.\n");

	if ((file_desc_AM = AM_OpenIndex(filename)) < 0) {
		printf("AM_OpenIndex() failed on %s.\n", filename);
		printf("Returned error: %d.\n", file_desc_AM);
		exit(1);
	}
	printf("> Opened the file.\n");

	/* Create block. */
	printf("> Creating a block.\n");

	CALL_FD(FD_Get_FileDesc(file_desc_AM, &file_desc_BF));

	BF_Block* block = NULL;
	int block_id;
	CALL_BL(BL_CreateBlock(file_desc_BF, &block_id, &block));
	printf("block id: %d.\n", block_id);
	printf("> Created a block.\n");

	/* Make it into index block. */
	printf("> Making block into index block.\n");

	char* key = malloc(sizeof(char) * 128);
	strcpy(key, "AAA");
	CALL_IB(IB_Init(file_desc_AM, block, 10, key, 23));

	/* Get max count pointers. */
	printf("> Getting the max number of pointers.\n");

	size_t n_pointers;
	CALL_IB(IB_Get_MaxCountPointers(file_desc_AM, &n_pointers));
	printf("%ld at most fit in an index block.\n", n_pointers);

	/* Get count pointers. */
	printf("> Getting current number of pointers.\n");

	size_t c_pointers;
	CALL_IB(IB_Get_CountPointers(block, &c_pointers));
	printf("%ld is the current number of pointers.\n", c_pointers);

	/* Print the block. */
	printf("Printing block.\n");
	CALL_IB(IB_Print(file_desc_AM, block));

	/* Insert new key. */
	int insert_flag;
	strcpy(key, "AAC");
	CALL_IB(IB_Insert(file_desc_AM, block, 15, key, 16, &insert_flag));
	if (insert_flag != 1) printf("Error in insert key.\n");

	/* Print the block. */
	printf("Printing block.\n");
	CALL_IB(IB_Print(file_desc_AM, block));

	/* Insert new key. */
	strcpy(key, "AAB");
	CALL_IB(IB_Insert(file_desc_AM, block, 35, key, 36, &insert_flag));
	if (insert_flag != 1) printf("Error in insert key.\n");

	/* Print the block. */
	printf("Printing block.\n");
	CALL_IB(IB_Print(file_desc_AM, block));

	/* Insert new key. This should fail. */
	strcpy(key, "AAF");
	CALL_IB(IB_Insert(file_desc_AM, block, 45, key, 46, &insert_flag));
	if (insert_flag != 1) printf("Error in insert key.\n");
	if (insert_flag == 0) printf("Failed to insert.\n");

	/* Print the block. */
	printf("Printing block.\n");
	CALL_IB(IB_Print(file_desc_AM, block));

	/* Release resources. */
	free(key);

	/* Close off block. */
	BF_Block_SetDirty(block);
	CALL_BF(BF_UnpinBlock(block));
	BF_Block_Destroy(&block);

	AM_CloseIndex(file_desc_AM);

	AM_Close();

	return 0;
}
