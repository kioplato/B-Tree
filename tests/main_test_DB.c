/*******************************************************************************
 *  file: main_test_DB.c
 *
 *  purpose: testing of DB set of functions.
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include "../src/BF.h"
#include "../src/accessmethod/AM.h"
#include "../src/datablock/DB.h"
#include "../src/block/BL.h"

int main(void) {
	AM_Init();

	char filename1[] = "database_int.db";  // Name of the file to create with integers keys.
	int file_desc_AM1;  // The file descriptor in AM layer for filename1.

	char filename2[] = "database_float.db";  // Name of the file to create with float keys.
	int file_desc_AM2;  // The file descriptor in AM layer for filename2.

	char filename3[] = "database_string.db";  // Name of the file to create with string keys.
	int file_desc_AM3;  // The file descriptor in AM layer for filename3.

	/* Create the files. */
	printf("> Creating the files.\n");

	if (AM_CreateIndex(filename1, INTEGER, sizeof(int), STRING, 16) != AME_OK) {
		fprintf(stderr, "AM_CreateIndex() failed on %s.\n", filename1);
		exit(1);
	}
	printf("Created file %s.\n", filename1);

	if (AM_CreateIndex(filename2, FLOAT, sizeof(float), INTEGER, sizeof(int)) != AME_OK) {
		fprintf(stderr, "AM_CreateIndex() failed on %s.\n", filename2);
		exit(1);
	}
	printf("Created file %s.\n", filename2);

	if (AM_CreateIndex(filename3, STRING, 8, FLOAT, sizeof(float)) != AME_OK) {
		fprintf(stderr, "AM_CreateIndex() failed on %s.\n", filename3);
		exit(1);
	}
	printf("Created file %s.\n", filename3);

	/* Open the files. */
	printf("> Opening the files.\n");

	if ((file_desc_AM1 = AM_OpenIndex(filename1)) < 0) {
		printf("AM_OpenIndex() failed on %s.\n", filename1);
		printf("Returned error: %d.\n", file_desc_AM1);
		exit(1);
	}
	printf("Opened file %s at %d.\n", filename1, file_desc_AM1);

	if ((file_desc_AM2 = AM_OpenIndex(filename2)) < 0) {
		printf("AM_OpenIndex() failed on %s.\n", filename2);
		printf("Returned error: %d.\n", file_desc_AM2);
		exit(1);
	}
	printf("Opened file %s at %d.\n", filename2, file_desc_AM2);

	if ((file_desc_AM3 = AM_OpenIndex(filename3)) < 0) {
		printf("AM_OpenIndex() failed on %s.\n", filename3);
		printf("Returned error: %d.\n", file_desc_AM3);
		exit(1);
	}
	printf("Opened file %s at %d.\n", filename3, file_desc_AM3);

	/* Create two blocks in each file. */
	printf("> Creating two blocks in each file.\n");

	BF_Block* datablock1A = NULL;  // The first data block for filename1.
	BF_Block* datablock1B = NULL;  // The second data block for filename1.
	int block_id1A;  // The block id of datablock1A.
	int block_id1B;  // The block id of datablock1B.

	printf("> Creating the blocks for %s.\n", filename1);
	CALL_BL(BL_CreateBlock(file_desc_AM1, &block_id1A, &datablock1A));
	CALL_BL(BL_CreateBlock(file_desc_AM1, &block_id1B, &datablock1B));
	printf("data block 1 id: %d.\n", block_id1A);
	printf("data block 2 id: %d.\n", block_id1B);

	BF_Block* datablock2A = NULL;  // The first data block for filename2.
	BF_Block* datablock2B = NULL;  // The second data block for filename2.
	int block_id2A;  // The block id of datablock2A.
	int block_id2B;  // The block id of datablock2B.

	printf("> Creating the blocks for %s.\n", filename2);
	CALL_BL(BL_CreateBlock(file_desc_AM2, &block_id2A, &datablock2A));
	CALL_BL(BL_CreateBlock(file_desc_AM2, &block_id2B, &datablock2B));
	printf("data block 1 id: %d.\n", block_id2A);
	printf("data block 2 id: %d.\n", block_id2B);

	BF_Block* datablock3A = NULL;  // The first data block for filename3.
	BF_Block* datablock3B = NULL;  // The second data block for filename3.
	int block_id3A;  // The block id of datablock3A.
	int block_id3B;  // The block id of datablock3B.

	printf("> Creating the blocks for %s.\n", filename3);
	CALL_BL(BL_CreateBlock(file_desc_AM3, &block_id3A, &datablock3A));
	CALL_BL(BL_CreateBlock(file_desc_AM3, &block_id3B, &datablock3B));
	printf("data block 1 id: %d.\n", block_id3A);
	printf("data block 2 id: %d.\n", block_id3B);

	/* Make them into data blocks, and make them point to each other. */
	printf("> Making them into data blocks that point to each other.\n");

	CALL_DB(DB_Init(datablock1A, block_id1B));
	CALL_DB(DB_Init(datablock1B, block_id1A));

	CALL_DB(DB_Init(datablock2A, block_id2B));
	CALL_DB(DB_Init(datablock2B, block_id2A));

	CALL_DB(DB_Init(datablock3A, block_id3B));
	CALL_DB(DB_Init(datablock3B, block_id3A));

	/* Check if they are data blocks. */
	printf("> Checking if they are data blocks.\n");

	int isdata_flag1A = -1;
	int isdata_flag1B = -1;

	printf("> Checking %s blocks.\n", filename1);
	CALL_DB(DB_Is_DataBlock(datablock1A, &isdata_flag1A));
	CALL_DB(DB_Is_DataBlock(datablock1B, &isdata_flag1B));
	if (isdata_flag1A == 1) printf("data block 1 of %s is a data block.\n", filename1);
	if (isdata_flag1B == 1) printf("data block 2 of %s is a data block.\n", filename1);

	int isdata_flag2A = -1;
	int isdata_flag2B = -1;

	printf("> Checking %s blocks.\n", filename2);
	CALL_DB(DB_Is_DataBlock(datablock2A, &isdata_flag2A));
	CALL_DB(DB_Is_DataBlock(datablock2B, &isdata_flag2B));
	if (isdata_flag2A == 1) printf("data block 1 of %s is a data block.\n", filename2);
	if (isdata_flag2B == 1) printf("data block 2 of %s is a data block.\n", filename2);
	
	int isdata_flag3A = -1;
	int isdata_flag3B = -1;

	printf("> Checking %s blocks.\n", filename3);
	CALL_DB(DB_Is_DataBlock(datablock3A, &isdata_flag3A));
	CALL_DB(DB_Is_DataBlock(datablock3B, &isdata_flag3B));
	if (isdata_flag3A == 1) printf("data block 1 of %s is a data block.\n", filename3);
	if (isdata_flag3B == 1) printf("data block 2 of %s is a data block.\n", filename3);

	/* Insert records into the blocks. */
	printf("> Inserting records into the files.\n");

	int field1A;
	char field1B[16];
	Record record1A;  // Record to insert in data block1A.
	Record record1B;  // Record to insert in data block1B.
	Record record1C;  // Record to insert in data block1A
	int insert_flag1A = -1;  // Insert flag for record1A.
	int insert_flag1B = -1;  // Insert flag for record1B.
	int insert_flag1C = -1;  // Insert flag for record1C.

	printf("> Inserting records into %s.\n", filename1);
	field1A = 5;
	strcpy(field1B, "Plato");
	record1A.fieldA = &field1A;
	record1A.fieldB = field1B;
	CALL_DB(DB_Insert(file_desc_AM1, datablock1A, record1A, &insert_flag1A));
	if (insert_flag1A == 1) printf("First record was successfully inserted in data block 1 of %s.\n", filename1);

	field1A = 8;
	strcpy(field1B, "Stathis");
	record1B.fieldA = &field1A;
	record1B.fieldB = field1B;
	CALL_DB(DB_Insert(file_desc_AM1, datablock1B, record1B, &insert_flag1B));
	if (insert_flag1B == 1) printf("Second record was successfully inserted in data block 2 of %s.\n", filename1);

	field1A = 3;
	strcpy(field1B, "Mitsos");
	record1C.fieldA = &field1A;
	record1C.fieldB = field1B;
	CALL_DB(DB_Insert(file_desc_AM1, datablock1A, record1C, &insert_flag1C));
	if (insert_flag1C == 1) printf("Third record was successfully inserted in data block 1 of %s.\n", filename1);

	float field2A;
	int field2B;
	Record record2A;  // Record to insert in data block2A.
	Record record2B;  // Record to insert in data block2B.
	Record record2C;  // Record to insert in data block2A
	int insert_flag2A = -1;  // Insert flag for record2A.
	int insert_flag2B = -1;  // Insert flag for record2B.
	int insert_flag2C = -1;  // Insert flag for record2C.

	printf("> Inserting records into %s.\n", filename2);
	field2A = 5.2;
	field2B = 3;
	record2A.fieldA = &field2A;
	record2A.fieldB = &field2B;
	CALL_DB(DB_Insert(file_desc_AM2, datablock2A, record2A, &insert_flag2A));
	if (insert_flag2A == 1) printf("First record was successfully inserted in data block 1 of %s.\n", filename2);

	field2A = 8.7;
	field2B = 12;
	record2B.fieldA = &field2A;
	record2B.fieldB = &field2B;
	CALL_DB(DB_Insert(file_desc_AM2, datablock2B, record2B, &insert_flag2B));
	if (insert_flag2B == 1) printf("Second record was successfully inserted in data block 2 of %s.\n", filename2);

	field2A = 3.1;
	field2B = -13;
	record2C.fieldA = &field2A;
	record2C.fieldB = &field2B;
	CALL_DB(DB_Insert(file_desc_AM2, datablock2A, record2C, &insert_flag2C));
	if (insert_flag2C == 1) printf("Third record was successfully inserted in data block 1 of %s.\n", filename2);

	char field3A[8];
	float field3B;
	Record record3A;
	Record record3B;
	Record record3C;
	int insert_flag3A = -1;  // Insert flag for record3A.
	int insert_flag3B = -1;  // Insert flag for record3B.
	int insert_flag3C = -1;  // Insert flag for record3C.

	printf("> Inserting records into %s.\n", filename3);
	strcpy(field3A, "Plato");
	field3B = 3.2;
	record3A.fieldA = field3A;
	record3A.fieldB = &field3B;
	CALL_DB(DB_Insert(file_desc_AM3, datablock3A, record3A, &insert_flag3A));
	if (insert_flag3A == 1) printf("First record was successfully inserted in data block 1 of %s.\n", filename3);
	if (insert_flag3A == 0) printf("First record wasn't successfully inserted in data block 1 of %s.\n", filename3);
	if (insert_flag3A == -1) printf("First record wasn't successfully inserted in data block 1 of %s.\n", filename3);

	strcpy(field3A, "Stathis");
	field3B = 3.6;
	record3B.fieldA = field3A;
	record3B.fieldB = &field3B;
	CALL_DB(DB_Insert(file_desc_AM3, datablock3B, record3B, &insert_flag3B));
	if (insert_flag3B == 1) printf("Second record was successfully inserted in data block 2 of %s.\n", filename3);
	if (insert_flag3B == 0) printf("Second record wasn't successfully inserted in data block 2 of %s.\n", filename3);
	if (insert_flag3B == -1) printf("Second record wasn't successfully inserted in data block 2 of %s.\n", filename3);

	strcpy(field3A, "Mitsos");
	field3B = -13.8;
	record3C.fieldA = field3A;
	record3C.fieldB = &field3B;
	CALL_DB(DB_Insert(file_desc_AM3, datablock3A, record3C, &insert_flag3C));
	if (insert_flag3C == 1) printf("Third record was successfully inserted in data block 1 of %s.\n", filename3);
	if (insert_flag3C == 0) printf("Third record wasn't successfully inserted in data block 1 of %s.\n", filename3);
	if (insert_flag3C == -1) printf("Third record wasn't successfully inserted in data block 1 of %s.\n", filename3);

	/* Get the number of records per data block. */
	printf("> Get the number of records in each data block.\n");

	size_t c_entries1A = 0;
	size_t c_entries1B = 0;

	printf("> Get the number of records in %s data blocks.\n", filename1);
	CALL_DB(DB_Get_Entries(datablock1A, &c_entries1A));
	printf("Data block 1A has %ld records.\n", c_entries1A);
	CALL_DB(DB_Get_Entries(datablock1B, &c_entries1B));
	printf("Data block 1B has %ld records.\n", c_entries1B);

	size_t c_entries2A = 0;
	size_t c_entries2B = 0;

	printf("> Get the number of records in %s data blocks.\n", filename2);
	CALL_DB(DB_Get_Entries(datablock2A, &c_entries2A));
	printf("Data block 2A has %ld records.\n", c_entries2A);
	CALL_DB(DB_Get_Entries(datablock2B, &c_entries2B));
	printf("Data block 2B has %ld records.\n", c_entries2B);

	size_t c_entries3A = 0;
	size_t c_entries3B = 0;

	printf("> Get the number of records in %s data blocks.\n", filename3);
	CALL_DB(DB_Get_Entries(datablock3A, &c_entries3A));
	printf("Data block 3A has %ld records.\n", c_entries3A);
	CALL_DB(DB_Get_Entries(datablock3B, &c_entries3B));
	printf("Data block 3B has %ld records.\n", c_entries3B);

	/* Get the records and print them. */
	printf("> Getting the records and printing them for each block.\n");

	int fetched_field1A;
	char fetched_field1B[16];
	Record fetched_record1A;
	Record fetched_record1B;
	Record fetched_record1C;
	int getrecord_flag1A = -1;
	int getrecord_flag1B = -1;
	int getrecord_flag1C = -1;

	printf("> Getting and printing %s records.\n", filename1);
	fetched_record1A.fieldA = &fetched_field1A;
	fetched_record1A.fieldB = fetched_field1B;
	CALL_DB(DB_Get_Record(file_desc_AM1, datablock1A, &fetched_record1A, 0, &getrecord_flag1A));
	if (getrecord_flag1A == 1) printf("Record 0 from data block 1A should have been fetched successfully.\n");
	printf("Field1A: %d.\n", *(int*)fetched_record1A.fieldA);
	printf("Field1B: %s.\n", (char*)fetched_record1A.fieldB);

	fetched_record1B.fieldA = &fetched_field1A;
	fetched_record1B.fieldB = fetched_field1B;
	CALL_DB(DB_Get_Record(file_desc_AM1, datablock1B, &fetched_record1B, 0, &getrecord_flag1B));
	if (getrecord_flag1B == 1) printf("Record 0 from data block 1B should have been fetched successfully.\n");
	printf("Field1A: %d.\n", *(int*)fetched_record1B.fieldA);
	printf("Field1B: %s.\n", (char*)fetched_record1B.fieldB);

	fetched_record1C.fieldA = &fetched_field1A;
	fetched_record1C.fieldB = fetched_field1B;
	CALL_DB(DB_Get_Record(file_desc_AM1, datablock1A, &fetched_record1C, 1, &getrecord_flag1C));
	if (getrecord_flag1A == 1) printf("Record 1 from data block 1A should have been fetched successfully.\n");
	printf("Field1A: %d.\n", *(int*)fetched_record1C.fieldA);
	printf("Field1B: %s.\n", (char*)fetched_record1C.fieldB);

	float fetched_field2A;
	int fetched_field2B;
	Record fetched_record2A;
	Record fetched_record2B;
	Record fetched_record2C;
	int getrecord_flag2A = -1;
	int getrecord_flag2B = -1;
	int getrecord_flag2C = -1;

	printf("> Getting and printing %s records.\n", filename2);
	fetched_record2A.fieldA = &fetched_field2A;
	fetched_record2A.fieldB = &fetched_field2B;
	CALL_DB(DB_Get_Record(file_desc_AM2, datablock2A, &fetched_record2A, 0, &getrecord_flag2A));
	if (getrecord_flag2A == 1) printf("Record 0 from data block 2A should have been fetched successfully.\n");
	printf("Field2A: %f.\n", *(float*)fetched_record2A.fieldA);
	printf("Field2B: %d.\n", *(int*)fetched_record2A.fieldB);

	fetched_record2B.fieldA = &fetched_field2A;
	fetched_record2B.fieldB = &fetched_field2B;
	CALL_DB(DB_Get_Record(file_desc_AM2, datablock2B, &fetched_record2B, 0, &getrecord_flag2B));
	if (getrecord_flag2B == 1) printf("Record 0 from data block 2B should have been fetched successfully.\n");
	printf("Field2A: %f.\n", *(float*)fetched_record2B.fieldA);
	printf("Field2B: %d.\n", *(int*)fetched_record2B.fieldB);

	fetched_record2C.fieldA = &fetched_field2A;
	fetched_record2C.fieldB = &fetched_field2B;
	CALL_DB(DB_Get_Record(file_desc_AM2, datablock2A, &fetched_record2C, 1, &getrecord_flag2C));
	if (getrecord_flag2C == 1) printf("Record 1 from data block 2A should have been fetched successfully.\n");
	printf("Field2A: %f.\n", *(float*)fetched_record2C.fieldA);
	printf("Field2B: %d.\n", *(int*)fetched_record2C.fieldB);

	char fetched_field3A[8];
	float fetched_field3B;
	Record fetched_record3A;
	Record fetched_record3B;
	Record fetched_record3C;
	int getrecord_flag3A = -1;
	int getrecord_flag3B = -1;
	int getrecord_flag3C = -1;

	printf("> Getting and printing %s records.\n", filename3);
	fetched_record3A.fieldA = fetched_field3A;
	fetched_record3A.fieldB = &fetched_field3B;
	CALL_DB(DB_Get_Record(file_desc_AM3, datablock3A, &fetched_record3A, 0, &getrecord_flag3A));
	if (getrecord_flag3A == 1) printf("Record 0 from data block 3A should have been fetched successfully.\n");
	printf("Field3A: %s.\n", (char*)fetched_record3A.fieldA);
	printf("Field3B: %f.\n", *(float*)fetched_record3A.fieldB);

	fetched_record3B.fieldA = fetched_field3A;
	fetched_record3B.fieldB = &fetched_field3B;
	CALL_DB(DB_Get_Record(file_desc_AM3, datablock3B, &fetched_record3B, 0, &getrecord_flag3B));
	if (getrecord_flag3B == 1) printf("Record 0 from data block 3B should have been fetched successfully.\n");
	printf("Field3A: %s.\n", (char*)fetched_record3B.fieldA);
	printf("Field3B: %f.\n", *(float*)fetched_record3B.fieldB);

	fetched_record3C.fieldA = fetched_field3A;
	fetched_record3C.fieldB = &fetched_record3B;
	CALL_DB(DB_Get_Record(file_desc_AM3, datablock3A, &fetched_record3C, 1, &getrecord_flag3C));
	if (getrecord_flag3C == 1) printf("Record 1 from data block 3A should have been fetched successfully.\n");
	printf("Field3A: %s.\n", (char*)fetched_record3C.fieldA);
	printf("Field3B: %f.\n", *(float*)fetched_record3C.fieldB);

	/* Close off data blocks. */
	printf("> Closing off data blocks.\n");

	printf("> Closing off data blocks %s.\n", filename1);
	BF_Block_SetDirty(datablock1A);
	CALL_BF(BF_UnpinBlock(datablock1A));
	BF_Block_Destroy(&datablock1A);

	BF_Block_SetDirty(datablock1B);
	CALL_BF(BF_UnpinBlock(datablock1B));
	BF_Block_Destroy(&datablock1B);

	printf("> Closing off data blocks %s.\n", filename2);
	BF_Block_SetDirty(datablock2A);
	CALL_BF(BF_UnpinBlock(datablock2A));
	BF_Block_Destroy(&datablock2A);

	BF_Block_SetDirty(datablock2B);
	CALL_BF(BF_UnpinBlock(datablock2B));
	BF_Block_Destroy(&datablock2B);

	printf("> Closing off data blocks %s.\n", filename3);
	BF_Block_SetDirty(datablock3A);
	CALL_BF(BF_UnpinBlock(datablock3A));
	BF_Block_Destroy(&datablock3A);

	BF_Block_SetDirty(datablock3B);
	CALL_BF(BF_UnpinBlock(datablock3B));
	BF_Block_Destroy(&datablock3B);

	/* Closing the files. */
	printf("> Closing the files.\n");

	printf("> Closing %s file.\n", filename1);
	AM_CloseIndex(file_desc_AM1);

	printf("> Closing %s file.\n", filename2);
	AM_CloseIndex(file_desc_AM2);

	printf("> Closing %s file.\n", filename3);
	AM_CloseIndex(file_desc_AM3);

	AM_Close();

	return 0;
}
