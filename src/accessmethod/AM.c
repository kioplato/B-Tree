#include <stdio.h>

#include "AM.h"  // The AM layer interface.
#include "../BF.h"  // The BF layer interface.

#include "../metadata/MT.h"  // Metadata block API.
#include "../block/BL.h"  // General Block API.
#include "../filedesc/FD.h"  // File descriptor array API.
#include "../datablock/DB.h"  // Data blocks API.
#include "../record/RD.h"  // Record API.

int AM_errno = AME_OK;

int AM_Init() {
	/* Initialize global array. */
	FD_Init();

	/* Initialize scan array. */
	// TODO.

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
	int file_desc;  // The file descriptor of the newly created file.
	int metablock_id;  // The block id of metadata block.
	BF_Block *metablock;  // The metadata block.

	if (fileName == NULL)
		return AME_ERROR;

	/* Create and open the file. */
	CALL_BF(BF_CreateFile(fileName));
	CALL_BF(BF_OpenFile(fileName, &file_desc));

	/* Create the metadata block. */
	CALL_BL(BL_CreateBlock(file_desc, &metablock_id, &metablock));
	CALL_MT(MT_Init(metablock, attrType1, attrLength1, attrType2, attrLength2));

	/* Close off metadata block. */
	BF_Block_SetDirty(metablock);
	CALL_BF(BF_UnpinBlock(metablock));
	BF_Block_Destroy(&metablock);

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
	BF_Block* metablock = NULL;
	// The metablock's data. Temporary placeholders.
	char attrType1, attrType2;
	int attrLength1, attrLength2;
	size_t root_index_block;
	
	if (fileName == NULL) return AME_ERROR;

	CALL_BF(BF_OpenFile(fileName, &file_desc_BF));

	/* Get metadata block's stored data for caching. */
	metablock = NULL;
	BF_Block_Init(&metablock);
	CALL_BF(BF_GetBlock(file_desc_BF, 0, metablock));

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

	// Get BF layer file descriptor.
	CALL_FD(FD_Get_FileDesc(fileDesc, &BF_index));

	// Get the index root from the cache.
	CALL_FD(FD_Get_IndexRoot(fileDesc, &index_root));

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

/*
 * Searches the correct data block to insert record to.
 * Returns 
 * Calls DBL API to insert the record.
 *
 * Returns the block id of the block that inserted the record to.
 * If returned value is different from the one provided it means that the
 * index or the data block split.
 */
int BT_Subtree_Insert(int file_desc, size_t subtree_root, size_t* overflow_root, Record record)
{
	BF_Block* block = NULL;  // The block we are working on at this recursive step.
	int flag = -1;           // Successful data block insertion or not.
	//size_t datablock_id = 0;

	/*
	 * Subtree does not have data block.
	 * Create datablock, set overflow_root to new block id and insert record.
	 * Record insertion should always succeed.
	 */
	if (subtree_root == 0) {
		CALL_BL(BL_CreateBlock(file_desc, (int*)overflow_root, &block));
		CALL_DB(DB_Init(block, 0));  // Does not have next block.
		CALL_DB(DB_Insert(file_desc, block, record, &flag));
		if (flag != 1) return AME_ERROR;
		BF_Block_SetDirty(block);
		CALL_BF(BF_UnpinBlock(block));
		BF_Block_Destroy(&block);
		return AME_OK;
	}

	/* Subtree exists. Load it's root. */
	CALL_BL(BL_LoadBlock(file_desc, subtree_root, &block));
	
	// If it's a data block call insert on it.
	// If insert fails 
	
	// Go to the next index block.
	
	// .. unpin on advance and reload on back tracking ..
}

int AM_InsertEntry(int file_desc, void* fieldA, void* fieldB)
{
	size_t index_root;  // The index of the root.
	// NOTE: does overflow_root contain the new root or the new index block?
	size_t overflow_root;  // If the index root splits, this is the new root (?).

	if (fieldA == NULL) return AME_ERROR;
	if (fieldB == NULL) return AME_ERROR;

	CALL_FD(FD_Get_IndexRoot(file_desc, &index_root));

	/* Formulate the record. Record struct with void values and lengths. */
	Record record;
	if (RD_Init(file_desc, &record, fieldA, fieldB) != AME_OK) return AME_ERROR;

	BT_Subtree_Insert(file_desc, index_root, &overflow_root, record);

	// if index root does not exist create data block and insert record.
	// subtree_insert takes care of that.
	
	// Recursive iteration through the B+ tree up to data block.
	// Call function to insert record to data block.
	// Function returns the block id of the block that inserted the record.

	return AME_OK;
}

int AM_OpenIndexScan(int indexDesc, int op, void *value)
{
	int index_root;
	CALL_FD(FD_Get_IndexRoot(int indexDesc, size_t *index_root));
	//αν ειχαμε συνεννοηθει σωστα, τωρα θα εχω στο index_root θα εχω είτε το root, δηλαδη το id ενος index block
	//είτε το id ενος data block, δηλαδη του πρώτου και μοναδικου data block.

	//check the type of block
	BF_Block* root_block=NULL;
	BF_Block_Init(&root_block);
	if (root_block==NULL) return AME_ERROR;

	CALL_FD(FD_Get_filedesc(indexDesc, &filedesc));


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

void* AM_FindNextEntry(int scanDesc)
{
	
}

int AM_CloseIndexScan(int scanDesc)
{
	return AME_OK;
}

void AM_PrintError(char *errString)
{

}

void AM_Close()
{

}
