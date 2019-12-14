#include "BT.h"
#include "../BF.h"
#include "../BL.h"
#include "../DB.h"

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
		CALL_BL(BL_CreateBlock(file_desc, &block, overflow_root));
		CALL_DB(DB_Init(block));
		CALL_DB(DB_Insert(block, record, &flag));
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
