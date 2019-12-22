#include <stdio.h>

#include "AM.h"  // The AM layer interface.
#include "../BF.h"  // The BF layer interface.

#include "../metadata/MT.h"  // Metadata block API.
#include "../block/BL.h"  // General Block API.
#include "../filedesc/FD.h"  // File descriptor array API.
#include "../datablock/DB.h"  // Data blocks API.
#include "../record/RD.h"  // Record API.
#include "../indexscan/IS.h"
#include "../indexblock/IB.h"
#include "../index/BT.h"

int convert(BF_ErrorCode code) {
	if (code == BF_OK) return AME_OK;
	if (code == BF_OPEN_FILES_LIMIT_ERROR) return AME_MAX_FILES;
	if (code == BF_INVALID_FILE_ERROR) return AME_BF_INVALID_FD;
	if (code == BF_ACTIVE_ERROR) return AME_BF_ACTIVE;
	if (code == BF_FILE_ALREADY_EXISTS) return AME_FILE_EXISTS;
	if (code == BF_FULL_MEMORY_ERROR) return AME_FULL_MEMORY;
	if (code == BF_INVALID_BLOCK_NUMBER_ERROR) return AME_INVALID_BLOCK_ID;
	if (code == BF_ERROR) return AME_BF_ERROR;
	printf("Invalid BF_ErrorCode passed into convert.\n");
	return AME_ERROR;
}

int AM_errno = AME_OK;

int AM_Init() {
	/* Initialize global array. */
	FD_Init();

	/* Initialize scan array. */
	IS_Init();

	/* Initialize BF layer. */
	CALL_BF(BF_Init(LRU));

	AM_errno = AME_OK;
	return AME_OK;
}

/*
 * Creates an index file with B+-tree.
 * Starts only with the file's metadata block.
 *
 * Returns AME_OK on success.
 * Returns an AM Error on failure.
 */
int AM_CreateIndex(char *fileName, char attrType1, int attrLength1,
	               char attrType2, int attrLength2)
{
	int file_desc_BF;  // The file descriptor of the newly created file.
	int metablock_id;  // The block id of metadata block.
	BF_Block *metablock;  // The metadata block.

	if (fileName == NULL) {
		AM_errno = AME_ERROR;
		return AME_ERROR;
	}

	if ((attrType1 != INTEGER && attrType1 != FLOAT && attrType1 != STRING) ||
			(attrType2 != INTEGER && attrType2 != FLOAT && attrType2 != STRING)) {
		AM_errno = AME_INVALID_TYPE;
		return AME_INVALID_TYPE;
	}

	if ((attrType1 == INTEGER && attrLength1 != sizeof(int)) ||
			(attrType1 == FLOAT && attrLength1 != sizeof(float)) ||
			(attrType2 == INTEGER && attrLength2 != sizeof(int)) ||
			(attrType2 == FLOAT && attrLength2 != sizeof(float))) {
		AM_errno = AME_INVALID_LENGTH;
		return AME_INVALID_LENGTH;
	}

	if ((attrType1 == STRING && (attrLength1 > 255 || attrLength1 < 1)) ||
			(attrType2 == STRING && (attrLength2 > 255 || attrLength2 < 1))) {
		AM_errno = AME_INVALID_LENGTH;
		return AME_INVALID_LENGTH;
	}

	/* Create and open the file. */
	CALL_BF(BF_CreateFile(fileName));
	CALL_BF(BF_OpenFile(fileName, &file_desc_BF));

	/* Create the metadata block. */
	CALL_BL(BL_CreateBlock(file_desc_BF, &metablock_id, &metablock));
	if (metablock == NULL) {
		fprintf(stderr, "Failed to create metadata block in AM_CreateIndex().\n");
		AM_errno = AME_ERROR;
		return AME_ERROR;
	}
	CALL_MT(MT_Init(metablock, attrType1, attrLength1, attrType2, attrLength2));

	/* Close off metadata block. */
	BF_Block_SetDirty(metablock);
	CALL_BF(BF_UnpinBlock(metablock));
	BF_Block_Destroy(&metablock);

	CALL_BF(BF_CloseFile(file_desc_BF));

	AM_errno = AME_OK;
	return AME_OK;
}

/*
 * Removes the file with name fileName from the disk.
 *
 * Returns AME_OK on success.
 * Returns AM Error code on failure.
 */
// TODO: Review. Does the scanDesc need to be checked?
// Probably not. If the file isn't open, scanDesc isn't open as well?
int AM_DestroyIndex(char *fileName)
{
	int flag;  // Flag about if the fileName is open.

	if (fileName == NULL) {
		AM_errno = AME_ERROR;
		return AME_ERROR;
	}

	flag = -1;
	CALL_FD(FD_IsOpen(fileName, &flag));
	if (flag == 1) {
		AM_errno = AME_DELETE_OPEN_FILE;
		return AME_DELETE_OPEN_FILE;
	}

	if (remove(fileName) == -1) {
		AM_errno = AME_FILE_NOT_EXISTS;
		return AME_FILE_NOT_EXISTS;
	}

	AM_errno = AME_OK;
	return AME_OK;
}

/*
 * Opens the file, caches the file's metadata.
 *
 * Returns the AM file descriptor on success.
 * Returns an AM Error code on failure.
 */
int AM_OpenIndex(char *fileName)
{
	// The file descriptor as returned from the BF layer.
	// Will be stored in the AM file descriptors array for accessing BF layer.
	int file_desc_BF;
	// The file descriptor as it's stored in AM layer.
	int file_desc_FD;
	// The metadata block. We read it's data in order to cache it.
	BF_Block* metablock;
	// The metablock's data. Temporary placeholders.
	char attrType1, attrType2;
	int attrLength1, attrLength2;
	size_t root_index_block;

	int is_index;

	if (fileName == NULL) return AME_ERROR;

	CALL_BF(BF_OpenFile(fileName, &file_desc_BF));

	/* Get metadata block's stored data for caching. */
	metablock = NULL;
	CALL_BL(BL_LoadBlock(file_desc_BF, 0, &metablock));
	if (metablock == NULL) {
		fprintf(stderr, "Metadata block failed to load in AM_OpenIndex().\n");
		AM_errno = AME_ERROR;
		return AME_ERROR;
	}

	CALL_MT(MT_Is_IndexFile(metablock, &is_index));
	if (is_index != 1) {
		CALL_BF(BF_UnpinBlock(metablock));
		BF_Block_Destroy(&metablock);
		AM_errno = AME_INVALID_FILETYPE;
		return AME_INVALID_FILETYPE;
	}

	CALL_MT(MT_GetData(metablock, &attrType1, &attrLength1, &attrType2,
				&attrLength2, &root_index_block));

	/* Cache the metadata block's data. */
	CALL_FD(FD_Insert(file_desc_BF, &file_desc_FD, fileName, attrType1,
				attrLength1, attrType2, attrLength2, root_index_block));

	/* Close off the metadata block. */
	CALL_BF(BF_UnpinBlock(metablock));
	BF_Block_Destroy(&metablock);

	return file_desc_FD;
}

int AM_CloseIndex(int fileDesc)
{
	int BF_index;  // The BF layer file descriptor.
	size_t index_root;   // The index's root.
	BF_Block* metablock; // The metadata block for writing new index root.
	int flag;  // Flag about if the file has open scans.

	// Get BF layer file descriptor.
	CALL_FD(FD_Get_FileDesc(fileDesc, &BF_index));

	// Get the index root from the cache.
	CALL_FD(FD_Get_IndexRoot(fileDesc, &index_root));

	// Check if this file open has any open scans.
	CALL_IS(IS_IsOpen(fileDesc, &flag));
	if (flag == 1) {
		AM_errno = AME_FILE_CLOSE_OPEN_SCAN;
		return AME_FILE_CLOSE_OPEN_SCAN;
	}

	// Try to delete the FD. If successful proceed.
	CALL_FD(FD_Delete(fileDesc));

	// Load metadata block.
	CALL_BL(BL_LoadBlock(BF_index, 0, &metablock));

	// Write the new index root to metadata block.
	CALL_MT(MT_WriteData(metablock, index_root));

	// Close off metadata block.
	BF_Block_SetDirty(metablock);
	CALL_BF(BF_UnpinBlock(metablock));
	BF_Block_Destroy(&metablock);

	// Close the file in BF layer.
	CALL_BF(BF_CloseFile(BF_index));

	AM_errno = AME_OK;
	return AME_OK;
}

int AM_InsertEntry(int file_desc_AM, void* fieldA, void* fieldB)
{
	size_t index_root;  // The root of index.

	int file_desc_BF;  // The BF file descriptor.

	BF_Block* first_block;  // First data block to create if index is empty.
	int first_block_id;     // First data block's id.
	int insert_status;      // Insert status of the first record.

	BF_Block* new_root;  // The new root if the root splits.
	int new_root_id;     // The block id of the new root.

	Record record;  // The record to insert.

	if (fieldA == NULL) { AM_errno = AME_ERROR; return AME_ERROR; }
	if (fieldB == NULL) { AM_errno = AME_ERROR; return AME_ERROR; }

	/*
	 * This function makes the record's members point to the same memory as
	 * fieldA and fieldB. The memory is copied into the data block and index
	 * block. However, if in some way we changed the contents of fieldA or
	 * fieldB before writing the record there would be a problem.
	 */
	CALL_RD(RD_Init(&record, fieldA, fieldB));

	CALL_FD(FD_Get_IndexRoot(file_desc_AM, &index_root));

	/*
	 * This is the first insert.
	 * Create the first data block and insert the record.
	 * Set this new data block as index's root.
	 */
	if (index_root == 0) {
		CALL_BL(FD_Get_FileDesc(file_desc_AM, &file_desc_BF));
		CALL_BL(BL_CreateBlock(file_desc_BF, &first_block_id, &first_block));
		CALL_DB(DB_Init(first_block, 0));
		CALL_DB(DB_Insert(file_desc_AM, first_block, record, &insert_status));
		if (insert_status != 1) { AM_errno = AME_ERROR; return AME_ERROR; }
		CALL_FD(FD_Set_IndexRoot(file_desc_AM, first_block_id));
		BF_Block_SetDirty(first_block);
		CALL_BF(BF_UnpinBlock(first_block));
		BF_Block_Destroy(&first_block);
		AM_errno = AME_OK;
		return AME_OK;
	}

	/* Variables used for the recursion. */
	int pointer1;
	void* key = NULL;
	int pointer2;
	int splitted = 0;

	/*
	 * Insert the record in the index.
	 * Key will point to valid memory when it reaches the data block.
	 * That same memory will be used in each split to write the new key for the
	 * upper layer to insert.
	 */
	CALL_BT(BT_Subtree_Insert(file_desc_AM, index_root, record, &pointer1, &key,
			&pointer2, &splitted));
	/*
	 * The record got inserted.
	 * However, the root could have splitted.
	 * We need to check splitted and create a new root if it's 1.
	 * Update the root at FD cache.
	 */

	if (splitted == 1) {
		CALL_FD(FD_Get_FileDesc(file_desc_AM, &file_desc_BF));
		CALL_BL(BL_CreateBlock(file_desc_BF, &new_root_id, &new_root));
		CALL_IB(IB_Init(file_desc_AM, new_root, pointer1, key, pointer2));
		free(key);
		BF_Block_SetDirty(new_root);
		CALL_BF(BF_UnpinBlock(new_root));
		BF_Block_Destroy(&new_root);
		CALL_FD(FD_Set_IndexRoot(file_desc_AM, new_root_id));
	}

	AM_errno = AME_OK;
	return AME_OK;
}

int AM_OpenIndexScan(int file_desc_AM, int op, void* value)
{
	/*
	 * Used in the case where we need to traverse the B+-Tree in order to get
	 * to the required data block specified by the `value`.
	 * ex.: when op == EQUAL.
	 *
	 * It's the B+-Tree's root.
	 */
	int subtree_root;

	/*
	 * Block id of the first block that contains records according to op-value.
	 * In case where we need to start from the leftmost block, for example when
	 * op == LESS THAN, we already know the block id and we don't need to search
	 * for it. This happens because when we split a data block, the half smaller
	 * records are placed in the already existing block. As such, since data
	 * block with block id 1 is the first block and when we split a data block
	 * the existing data block gets the half smallest records then the block
	 * with id 1 is the leftmost.
	 */
	int block_id;

	int scan_index;  // The index of the cached data for this new scan.

	if (op == EQUAL || op == GREATER_THAN || op == GREATER_THAN_OR_EQUAL) {
		CALL_FD(FD_Get_IndexRoot(file_desc_AM, (size_t*)&subtree_root));
		CALL_BT(BT_Get_SubtreeLeaf(file_desc_AM, subtree_root, value, &block_id));
	} else {
		block_id = 1;
	}

	/*
	 * Inserts the required data for future reference by AM_FindNextEntry()
	 * in order to get the next record which satisties the range query.
	 *
	 * next is 0 because AM_FindNextEntry() will search for the first record
	 * in the block_id which satisfies the range query. We don't care to search
	 * for it at this stage.
	 */
	CALL_IS(IS_Insert(0, block_id, op, file_desc_AM, value, &scan_index));

	AM_errno = AME_OK;
	return scan_index;
}

void* AM_FindNextEntry(int scanDesc)
{
	BF_Block* block;  // The opened block we are searching in.

	int key_length;  // The length of key in bytes.
	int val_length;  // The length of value in bytes.

	int last_block;    // Block to search in for records.
	int next;          // The record offset in the block.
	int file_desc_AM;  // The file descriptor in AM.
	int op;            // The operation relative to value for range query.
	void* value = NULL;  // The cached value for the range query.

	Record record;  // Record temp for getting records from data blocks.
	size_t c_entries;  // The last_block's number of entries stored.
	int next_block;    // The block's next block.
	size_t c_entry;    // The index of the current entry in block.

	int get_flag;  // The get status of the record from data block.
	int cmp_flag;  // The result from comparing value to fetched key from block.

	void* rval = NULL;  // The return value. Value field to return.

	int last_entry;  // True if the result is the last entry of the block.

	/*
	 * In some cases the current block won't contain a result for a range query.
	 * Such cases are GREATER_THAN with a value that is the last in the block,
	 * GREATER_THAN or GREATER_THAN_OR_EQUAL with a value that does not exist
	 * but it's bigger than existing keys. These cases happen the first time
	 * AM_FindNextEntry() is executed.
	 *
	 * There is the exception of NOT_EQUAL with a value that is the last record
	 * in the block. In this case we need to read the next block and happens
	 * after the first execution of AM_FindNextEntry().
	 *
	 * TODO: Probably recursion would be cleaner than for looping.
	 * It would work like this: while no result found update next and/or
	 * last_block and repeat for the next record. Probably it would be tail
	 * recursion as well.
	 */
	int requires_next_block;  // True if result not found and needs next block to find one.

	int rv;  // Return value of functions.

	/*
	 * TODO: Functions should set the AM_errno before they return.
	 */
	if ((rv = IS_Get_last_block(scanDesc, &last_block)) != AME_OK) { AM_errno = rv; return NULL; }
	if ((rv = IS_Get_next(scanDesc, &next)) != AME_OK) { AM_errno = rv; return NULL; }
	if ((rv = IS_Get_index_desc(scanDesc, &file_desc_AM)) != AME_OK) { AM_errno = rv; return NULL; }
	if ((rv = IS_Get_op(scanDesc, &op)) != AME_OK) { AM_errno = rv; return NULL; }
	if ((rv = IS_Get_value(scanDesc, &value)) != AME_OK) { AM_errno = rv; return NULL; }

	if (last_block == 0) {
		AM_errno = AME_EOF;
		return rval;
	}

	if ((rv = FD_Get_attrLength1(file_desc_AM, &key_length)) != AME_OK) { AM_errno = rv; return NULL; }
	if ((rv = FD_Get_attrLength2(file_desc_AM, &val_length)) != AME_OK) { AM_errno = rv; return NULL; }
	record.fieldA = malloc(key_length);
	record.fieldB = malloc(val_length);

	if ((rv = BL_LoadBlock(file_desc_AM, last_block, &block)) != AME_OK) { AM_errno = rv; return NULL; }

	if ((rv = DB_Get_Entries(block, &c_entries)) != AME_OK) { AM_errno = rv; return NULL; }

	/*
	 * We search for the first key that satisfies the range query.
	 * We do this because this for two reasons:
	 * 1. It may be the first AM_FindNextEntry() after AM_OpenIndexScan().
	 *    This means that we need to search the block to get to the first record
	 *    that satisfies the range query. To get to the "frontier" let's say.
	 * 2. It may be a NOT EQUAL query. This means that we need to search in the
	 *    whole block since the some records may have `value` keys.
	 */

	for (c_entry = next; c_entry < c_entries; ++c_entry) {
		if ((rv = DB_Get_Record(file_desc_AM, block, &record, c_entry, &get_flag)) != AME_OK) { AM_errno = rv; return NULL; }
		if (get_flag != 1) { AM_errno = AME_ERROR; return NULL; }
		if ((rv = RD_Key_cmp(file_desc_AM, value, record.fieldA, &cmp_flag)) != AME_OK) { AM_errno = rv; return NULL; }
		if (op == EQUAL && cmp_flag == 0) {
			rval = record.fieldB;
			break;
		}
		else if (op == NOT_EQUAL && cmp_flag != 0) {
			rval = record.fieldB;
			break;
		}
		else if (op == LESS_THAN && cmp_flag == 1) {
			rval = record.fieldB;
			break;
		}
		else if (op == GREATER_THAN && cmp_flag == -1) {
			rval = record.fieldB;
			break;
		}
		else if (op == LESS_THAN_OR_EQUAL && (cmp_flag == 0 || cmp_flag == 1)) {
			rval = record.fieldB;
			break;
		}
		else if (op == GREATER_THAN_OR_EQUAL && (cmp_flag == 0 || cmp_flag == -1)) {
			rval = record.fieldB;
			break;
		}
	}

	free(record.fieldA);
	if (rval == NULL) free(record.fieldB);

	/*
	 * At this point, either we found a matching record or the block ran out
	 * out records to search.
	 *
	 * If the latter is happening, we ran out of records in current block while
	 * no record matches the range query, we need to move to the next data
	 * block in the chain only in the case of op == NOT_EQUAL. Is such case we
	 * do a recursive call to get the first record from the next block.
	 *
	 * If the former is happening, if this was the last record of the block
	 * then move to the next record. The last record happens when
	 * c_entry == c_entries - 1 because of break.
	 */

	last_entry = c_entry == c_entries - 1;
	requires_next_block = op == NOT_EQUAL && rval == NULL;
	requires_next_block |= op == GREATER_THAN && rval == NULL;
	requires_next_block |= op == GREATER_THAN_OR_EQUAL && rval == NULL;

	if (last_entry || requires_next_block) {
		if ((rv = DB_Get_NextBlock(block, (size_t*)&next_block)) != AME_OK) { AM_errno = rv; return NULL; }
		if ((rv = IS_Set_last_block(scanDesc, next_block)) != AME_OK) { AM_errno = rv; return NULL; }
		if ((rv = IS_Set_next(scanDesc, 0)) != AME_OK) { AM_errno = rv; return NULL; }
	} else {
		if ((rv = IS_Set_next(scanDesc, c_entry + 1)) != AME_OK) { AM_errno = rv; return NULL; }
	}

	if ((rv = BF_UnpinBlock(block)) != BF_OK) {AM_errno = convert(rv); return NULL; }
	BF_Block_Destroy(&block);

	/*
	 * Just get the first entry of the next block.
	 */
	if (requires_next_block)
		return AM_FindNextEntry(scanDesc);

	if (rval == NULL) AM_errno = AME_EOF;
	else AM_errno = AME_OK;
	return rval;
}

int AM_CloseIndexScan(int scanDesc)
{
	CALL_IS(IS_Close(scanDesc));

	AM_errno = AME_OK;
	return AME_OK;
}

void AM_PrintError(char *errString)
{
	printf("%s: ", errString);
	if (AM_errno == AME_OK)
		printf("successful execution.\n");
	else if (AM_errno == AME_BF_MAX_FILES || AM_errno == AME_MAX_FILES)
		printf("maximum opens reached.\n");
	else if (AM_errno == AME_INVALID_FD || AM_errno == AME_FD_INVALID_INDEX)
		printf("invalid file descriptor.\n");
	else if (AM_errno == AME_FILE_EXISTS)
		printf("file already exists.\n");
	else if (AM_errno == AME_BF_ERROR)
		printf("system fail.\n");
	else if (AM_errno == AME_ERROR)
		printf("NULL memory or failed memory allocation.\n");
	else if (AM_errno == AME_EOF)
		printf("no entries that match range query.\n");
	else if (AM_errno == AME_DELETE_OPEN_FILE)
		printf("deleting an open file.\n");
	else if (AM_errno == AME_FILE_CLOSE_OPEN_SCAN)
		printf("trying to close a file with opened scan(s).\n");
	else if (AM_errno == AME_IS_MAX_FILES)
		printf("maximum open scans reached.\n");
	else if (AM_errno == AME_IS_INVALID_INDEX)
		printf("invalid scan index.\n");
	else if (AM_errno == AME_INVALID_LENGTH)
		printf("invalid field length(s).\n");
	else if (AM_errno == AME_INVALID_TYPE)
		printf("invalid field type(s).\n");
	else if (AM_errno == AME_FILE_NOT_EXISTS)
		printf("deleting non existent file path.\n");
	else if (AM_errno == AME_INVALID_FILETYPE)
		printf("opening unknown file type.\n");

	//else if (AM_errno == AME_INVALID_BLOCK_ID)
	//	printf("this should not happen.\n");
	//else if (AM_errno == AME_FULL_MEMORY)
	//	printf("this should not happen.\n");
	//else if (AM_errno == AME_BF_ACTIVE)
	//	printf("this should not happen.\n");
	//else if (AM_errno == AME_BF_INVALID_FD)
	//	printf("this should not happen.\n");
}

void AM_Close()
{
	BF_Close();
}
