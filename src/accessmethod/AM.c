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

	if (fileName == NULL)
		return AME_ERROR;

	if ((attrType1 == INTEGER && attrLength1 != sizeof(int)) ||
			(attrType1 == FLOAT && attrLength1 != sizeof(float)) ||
			(attrType2 == INTEGER && attrLength2 != sizeof(int)) ||
			(attrType2 == FLOAT && attrLength2 != sizeof(float)))
		return AME_INVALID_LENGTH;

	if ((attrType1 == STRING && (attrLength1 > 255 || attrLength1 < 1)) ||
			(attrType2 == STRING && (attrLength2 > 255 || attrLength2 < 1)))
		return AME_INVALID_LENGTH;

	/* Create and open the file. */
	CALL_BF(BF_CreateFile(fileName));
	CALL_BF(BF_OpenFile(fileName, &file_desc_BF));

	/* Create the metadata block. */
	CALL_BL(BL_CreateBlock(file_desc_BF, &metablock_id, &metablock));
	if (metablock == NULL) {
		fprintf(stderr, "Failed to create metadata block in AM_CreateIndex().\n");
		return AME_ERROR;
	}
	CALL_MT(MT_Init(metablock, attrType1, attrLength1, attrType2, attrLength2));

	/* Close off metadata block. */
	BF_Block_SetDirty(metablock);
	CALL_BF(BF_UnpinBlock(metablock));
	BF_Block_Destroy(&metablock);

	CALL_BF(BF_CloseFile(file_desc_BF));

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

	if (fileName == NULL)
		return AME_ERROR;

	flag = -1;
	CALL_FD(FD_IsOpen(fileName, &flag));
	if (flag == 1) return AME_DELETE_OPEN_FILE;

	if (remove(fileName) == -1) {
		perror("remove() failed");
		return AME_ERROR;
	}

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

	if (fileName == NULL) return AME_ERROR;

	CALL_BF(BF_OpenFile(fileName, &file_desc_BF));

	/* Get metadata block's stored data for caching. */
	metablock = NULL;
	CALL_BL(BL_LoadBlock(file_desc_BF, 0, &metablock));
	if (metablock == NULL) {
		fprintf(stderr, "Metadata block failed to load in AM_OpenIndex().\n");
		return AME_ERROR;
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
	if (flag == 1) return AME_FILE_CLOSE_OPEN_SCAN;

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

	if (fieldA == NULL) return AME_ERROR;
	if (fieldB == NULL) return AME_ERROR;

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
		if (insert_status != 1) return AME_ERROR;
		CALL_FD(FD_Set_IndexRoot(file_desc_AM, first_block_id));
		BF_Block_SetDirty(first_block);
		CALL_BF(BF_UnpinBlock(first_block));
		BF_Block_Destroy(&first_block);
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
	BT_Subtree_Insert(file_desc_AM, index_root, record, &pointer1, &key,
			&pointer2, &splitted);
	/*
	 * The record got inserted.
	 * However, the root could have splitted.
	 * We need to check splitted and create a new root if it's 1.
	 * Update the root at FD cache.
	 */

	if (splitted == 1) {
		CALL_BL(BL_CreateBlock(file_desc_BF, &new_root_id, &new_root));
		CALL_IB(IB_Init(file_desc_AM, new_root, pointer1, key, pointer2));
		free(key);
		BF_Block_SetDirty(new_root);
		CALL_BF(BF_UnpinBlock(new_root));
		BF_Block_Destroy(&new_root);
		CALL_FD(FD_Set_IndexRoot(file_desc_AM, new_root_id));
	}

	return AME_OK;
}

int AM_OpenIndexScan(int indexDesc, int op, void *value)
{
	printf("indexDesc: %d.\n", indexDesc);
	printf("op: %d.\n", op);
	char key_type;
	CALL_FD(FD_Get_attrType1(indexDesc, &key_type));
	printf("Key: ");
	if (key_type == 'i')
		printf("%d.\n", *(int*)value);
	else if (key_type == 'f')
		printf("%f.\n", *(float*)value);
	else if (key_type == 'c')
		printf("%s.\n", (char*)value);

	printf("The AM_OpenIndexScan() isn't yet implemented.\n");

	exit(1);

	return AME_OK;
}
/*
	size_t index_root;
	CALL_FD(FD_Get_IndexRoot(indexDesc, &index_root));
	//αν ειχαμε συνεννοηθει σωστα, τωρα θα εχω στο index_root θα εχω είτε το root, δηλαδη το id ενος index block
	//είτε το id ενος data block, δηλαδη του πρώτου και μοναδικου data block.

	//check the type of block
	BF_Block* root_block=NULL;
	BF_Block_Init(&root_block);
	if (root_block==NULL) return AME_ERROR;

	CALL_FD(FD_Get_FileDesc(indexDesc, &filedesc));


	CALL_BF(BF_GetBlock( filedesc, index_root, root_block)
	int flag;
	CALL_DB(DB_Is_DataBlock(block, &flag));
	if (flag==1)
		int scan_index;
		CALL_IS(IS_Insert(next=1, last_block=index_root, op, indexDesc, void* value, &scan_index))
		return scan_index;
	else //it was index block so we must go down the tree depending on the op
		if (op==EQUAL) //για ισοτητα θελω τον αριστερο pointer ενος key για το οποιο value<key.
			while (flag==0)
				int i=0;
				while (1)
					int pointer, key
					BT_Get_Pair(i, &pointer, &key)








	IS_Insert(int next=1, int last_block, int op, int indexDesc, void* value, *scan_index)

	return AME_OK;
}
*/

void* AM_FindNextEntry(int scanDesc)
{
	printf("scanDesc: %d.\n", scanDesc);

	printf("The AM_FindNextEntry() isn't yet implemented.\n");
	
	exit(1);

	return NULL;
}

int AM_CloseIndexScan(int scanDesc)
{
	printf("scanDesc: %d.\n", scanDesc);

	printf("The AM_CloseIndexScan() isn't yet implemented.\n");

	exit(1);

	return AME_OK;
}

void AM_PrintError(char *errString)
{
	printf("errString: %s.\n", errString);

	printf("The AM_PrintError() isn't yet implemented.\n");

	exit(1);
}

// NOTE: Probably is incomplete.
void AM_Close()
{
	BF_Close();
}
