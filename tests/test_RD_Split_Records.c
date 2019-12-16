/*******************************************************************************
 *  file: test_RD_Split_Records.c
 *
 *  purpose: testing of RD_Split_Records function.
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include "../src/record/RD.h"
#include "../src/filedesc/FD.h"
/*
#include "../src/BF.h"
#include "../src/accessmethod/AM.h"
#include "../src/datablock/DB.h"
#include "../src/block/BL.h"
*/

int main(void) {
	int index;
	char filename[] = "filename1.db";
	FD_Init();
	CALL_FD(FD_Insert(0, &index, filename, 'c', 8, 'f', sizeof(float), 0));

	Record records[6];

	for (size_t c_record = 0; c_record < 6; ++c_record) {
		records[c_record].fieldA = malloc(sizeof(char) * 8);
		records[c_record].fieldB = malloc(sizeof(float));
	}

	/* Edit those numbers to test different test cases. */
	strcpy((char*)records[0].fieldA, "AAC");
	strcpy((char*)records[1].fieldA, "AAC");
	strcpy((char*)records[2].fieldA, "AAC");
	strcpy((char*)records[3].fieldA, "AAC");
	strcpy((char*)records[4].fieldA, "AAC");
	strcpy((char*)records[5].fieldA, "AAD");
	/****************************************************/

	size_t splitter;

	CALL_RD(RD_Split_Records(index, records, 6, &splitter));

	printf("Splitter: %ld.\n", splitter);

	printf("The blocks will split into:\n");
	printf("Left block.\n");
	for (size_t c_record = 0; c_record < splitter; ++c_record)
		printf("%s.\n", (char*)records[c_record].fieldA);

	printf("Right block.\n");
	for (size_t c_record = splitter; c_record < 6; ++c_record)
		printf("%s.\n", (char*)records[c_record].fieldA);

	for (size_t c_record = 0; c_record < 6; ++c_record) {
		free(records[c_record].fieldA);
		free(records[c_record].fieldB);
	}

	CALL_FD(FD_Delete(index));

	return 0;
}
