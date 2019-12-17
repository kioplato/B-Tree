/*******************************************************************************
 * File: BT.c
 * Purpose: Implementation of API for operating with Data Blocks.
*******************************************************************************/

#include "BT.h"
#include "../accessmethod/AM.h"
#include "../BF.h"
#include "../block/BL.h"
#include "../datablock/DB.h"
#include "../record/RD.h"
#include "../indexblock/IB.h"

int BT_Get_SubtreeRoot(int file_desc_AM, BF_Block* block, void* key, int* pointer)
{
	size_t c_pointers;  // The current number of pointers in block.

	int key_length;  // The length of key in bytes.
	void* current_key;  // For getting the block's keys.

	size_t key_index;  // For iterating the index block's keys.
	size_t ptr_index;  // For iterating the index block's pointers.

	int get_key_status;
	int get_pointer_status;
	int cmp_status;

	if (block == NULL) return AME_ERROR;
	if (key == NULL) return AME_ERROR;
	if (pointer == NULL) return AME_ERROR;

	CALL_FD(FD_Get_attrLength1(file_desc_AM, &key_length));
	current_key = malloc(key_length);

	CALL_IB(IB_Get_CountPointers(block, &c_pointers));

	for (key_index = 0, ptr_index = 0; key_index < c_pointers - 1; ++key_index/*, ++ptr_index*/) {
		CALL_IB(IB_Get_Key(file_desc_AM, block, current_key, key_index, &get_key_status));
		CALL_RD(RD_Key_cmp(file_desc_AM, key, current_key, &cmp_status));
		if (cmp_status == -1) break;
		ptr_index = key_index + 1;  // Point ptr_index to values >= current_key.
	}
	CALL_IB(IB_Get_Pointer(file_desc_AM, block, pointer, ptr_index, &get_pointer_status));

	free(current_key);

	return AME_OK;
}

int BT_Subtree_Insert(int file_desc_AM, int subtree_root, Record record,
		int* pointer1, void** key, int* pointer2, int* splitted)
{
	BF_Block* block = NULL;  // The block we are working on at this recursive step.

	/* Variables for case where subtree_root is data block. */
	size_t c_entry;  // Current entry in data block.
	int is_datablock;  // If the subtree_root is data block.
	int write_flag;  // Successful record insertion or not.
	int get_flag;  // Successful record "getting" or not.
	int cmp_flag;  // For comparing the fetched record with "to be inserted" record.
	size_t splitter;  // If the block is full, where we split the records array.
	BF_Block* new_block;  // New data block for splitting.
	int new_block_id;  // The block id of the new block.
	int entries_flag;  // Status flag for writing the entries.
	int fieldA_length;
	int fieldB_length;
	size_t n_entries;  // Maximum number of entries in a data block.
	/********************************************************/

	int next_subtree_root;  // The root of the next block when descending.


	int file_desc_BF;  // BF file descriptor.


	CALL_FD(FD_Get_FileDesc(file_desc_AM, &file_desc_BF));

	/*
	 * Load the subtree_root.
	 * Check if it's a data block.
	 * If it is insert the record, do splits etc.
	 * Find the pointer-key-pointer to insert at the index.
	 */
	CALL_BL(BL_LoadBlock(file_desc_BF, subtree_root, &block));
	CALL_DB(DB_Is_DataBlock(block, &is_datablock));
	if (is_datablock == 1) {
		CALL_DB(DB_Insert(file_desc_AM, block, record, &write_flag));
		/*
		 * If write is successful life is easy and no splits are required.
		 * You enjoy a sazerac at bahamas.
		 * Data block has empty space.
		 */
		if (write_flag == 0) {
			CALL_DB(DB_Get_MaxEntries(file_desc_AM, &n_entries));
			Record records[n_entries + 1];
			CALL_FD(FD_Get_attrLength1(file_desc_AM, &fieldA_length));
			CALL_FD(FD_Get_attrLength2(file_desc_AM, &fieldB_length));
			for (c_entry = 0; c_entry < n_entries; ++c_entry) {
				records[c_entry].fieldA = malloc(fieldA_length);
				records[c_entry].fieldB = malloc(fieldB_length);
				CALL_DB(DB_Get_Record(file_desc_AM, block, &records[c_entry], c_entry, &get_flag));
				CALL_RD(RD_Key_cmp(file_desc_AM, record.fieldA, records[c_entry].fieldA, &cmp_flag));
				/*
				 * We break at the first bigger record than the one we insert.
				 * Then we shift it one to the right and write over it.
				 */
				if (cmp_flag == -1) {
					/*
					 * records[c_entry + 1] does not need memory.
					 * It points to the memory allocated for records[c_entry].
					 */
					records[c_entry + 1].fieldA = records[c_entry].fieldA;
					records[c_entry + 1].fieldB = records[c_entry].fieldB;
					break;
				}
			}
			records[c_entry].fieldA = malloc(fieldA_length);
			records[c_entry].fieldB = malloc(fieldB_length);
			memcpy((void*)records[c_entry].fieldA, (const void*)record.fieldA, fieldA_length);
			memcpy((void*)records[c_entry].fieldB, (const void*)record.fieldB, fieldB_length);
			/*
			 * The c_entry is the index of the records in data block.
			 * We write at records[c_entry + 1] because in one read from block
			 * we wrote at two indexes at records array.
			 */
			c_entry++;  // We read records[c_entry]. Move to next record.
			for (; c_entry < n_entries; ++c_entry) {
				records[c_entry + 1].fieldA = malloc(fieldA_length);
				records[c_entry + 1].fieldB = malloc(fieldB_length);
				CALL_DB(DB_Get_Record(file_desc_AM, block, &records[c_entry + 1], c_entry, &get_flag));
			}

			/*
			 * At this point we have the records sorted in an array.
			 * We need to split them.
			 * We will write the half smallest records to existing block and
			 * the half bigger records to a new block.
			 */

			CALL_RD(RD_Split_Records(file_desc_AM, records, n_entries + 1, &splitter));

			/*
			 * We have a split point.
			 * Write the [0, splitter - 1] records to existing data block.
			 * Write the [splitter, n_entries + 1] records to new data block.
			 */

			CALL_BL(BL_CreateBlock(file_desc_BF, &new_block_id, &new_block));

			/* We zero out the number of records in existing data block. */
			CALL_DB(DB_Write_Entries(file_desc_AM, block, 0, &entries_flag));

			/* Write the leftmost records to the existing block. */
			// c_entry is the entry in records array.
			for (c_entry = 0; c_entry < splitter; ++c_entry) {
				CALL_DB(DB_Insert(file_desc_AM, block, records[c_entry], &write_flag));
				if (write_flag != 1) return AME_ERROR;
			}

			/* Write the rightmost records to the new block. */
			// c_entry == splitter.
			for (; c_entry < n_entries + 1; ++c_entry) {
				CALL_DB(DB_Insert(file_desc_AM, new_block, records[c_entry], &write_flag));
				if (write_flag != 1) return AME_ERROR;
			}

			BF_Block_SetDirty(new_block);
			CALL_BF(BF_UnpinBlock(new_block));
			BF_Block_Destroy(&new_block);

			/*
			 * The data block got splitted.
			 * Update the flag...
			 */
			*splitted = 1;

			/*
			 * ...and set the pointer-key-pointer for the index block to insert.
			 *
			 * Remember that we wrote the smaller records at the existing block
			 * and the bigger values to the new block.
			 */
			*pointer1 = subtree_root;
			*key = records[splitter].fieldA;
			*pointer2 = new_block_id;

			/*
			 * Free the memory used for splitting the records.
			 * Keep only the memory used for the key.
			 */
			for (c_entry = 0; c_entry < n_entries + 1; ++c_entry) {
				if (c_entry != splitter) {
					free(records[c_entry].fieldA);
					free(records[c_entry].fieldB);
				}
			}
			free(records[splitter].fieldB);
			/*
			 * We will free memory pointed by key when pointer-key-pointer fits
			 * in an index block. At this point we will also set splitted == 0.
			 */
		}
		else if (write_flag == 1) {
			*splitted = 0;
		}

		BF_Block_SetDirty(block);
		CALL_BF(BF_UnpinBlock(block));
		BF_Block_Destroy(&block);

		return AME_OK;
	}
	else if (is_datablock == 0) {
		/*
		 * This is an index block.
		 * We descend to a data block.
		 */

		/*
		 * Get the next subtree root according to the pointers and keys in
		 * the current index block.
		 */
		CALL_BT(BT_Get_SubtreeRoot(file_desc_AM, block, record.fieldA, &next_subtree_root));

		/*
		 * Move down the tree.
		 * However, before descending unpin the block.
		 * We do this because the B+ Tree depth could be bigger than the buffer
		 * BF layer has for pinning blocks.
		 */
		CALL_BF(BF_UnpinBlock(block));
		BF_Block_Destroy(&block);
		CALL_BT(BT_Subtree_Insert(file_desc_AM, next_subtree_root, record, pointer1, key, pointer2, splitted));
		/*
		 * Now that the recursive call returned we need to check if the next
		 * block below splitted. If it did we need to insert the new
		 * pointer-key-pointer.
		 */

		/* The block below didn't split. Life is easy. */
		if (*splitted == 0) return AME_OK;
		else if (*splitted == 1) {  // No sazerac.
			CALL_BL(BL_LoadBlock(file_desc_BF, subtree_root, &block));
			CALL_IB(IB_Insert(file_desc_AM, block, *pointer1, *key, *pointer2, &write_flag));
			if (write_flag == 1) {
				free(*key);  // Free the memory allocated by data block.
				*splitted = 0;  // No more splits for upper layers.
			}
			else if (write_flag == 0) {
				// XXX: Needs serious rework and cleanup.
				int get_status;
				int cmp_status;
				int pointers_flag;
				size_t n_pointers;
				int key_length;
				int pointer_length;
				size_t c_key;
				CALL_IB(IB_Get_MaxCountPointers(file_desc_AM, &n_pointers));
				CALL_FD(FD_Get_attrLength1(file_desc_AM, &key_length));
				pointer_length = sizeof(int);
				void* keys_ptrs = NULL;
				void* offseted_keys_ptrs;
				keys_ptrs = malloc((pointer_length * n_pointers) + (key_length * n_pointers - 1) + key_length + pointer_length);
				offseted_keys_ptrs = keys_ptrs;
				/*
				 * At the start we get the first pointer.
				 * Then, inside the for loop we get key-pointer pairs.
				 */
				CALL_IB(IB_Get_Pointer(file_desc_AM, block, offseted_keys_ptrs, 0, &get_status));
				if (get_status != 1) { fprintf(stderr, "Failed to get pointer in BT_Subtree_Insert().\n"); exit(1); }
				offseted_keys_ptrs += pointer_length;
				for (c_key = 0; c_key < n_pointers - 1; ++c_key) {
					/* Key. */
					CALL_IB(IB_Get_Key(file_desc_AM, block, offseted_keys_ptrs, c_key, &get_status));
					if (get_status != 1) { fprintf(stderr, "Failed to get key in BT_Subtree_Insert().\n"); exit(1); }
					offseted_keys_ptrs += key_length;
					/* Pointer. */
					CALL_IB(IB_Get_Pointer(file_desc_AM, block, offseted_keys_ptrs, c_key + 1, &get_status));
					if (get_status != 1) { fprintf(stderr, "Failed to get pointer in BT_Subtree_Insert().\n"); exit(1); }
					offseted_keys_ptrs += pointer_length;
					/* Comparison. */
					CALL_RD(RD_Key_cmp(file_desc_AM, *key, offseted_keys_ptrs - pointer_length - key_length, &cmp_status));
					if (cmp_status == -1) {
						/* Move the key-pointer to the right. */
						offseted_keys_ptrs -= pointer_length + key_length;
						memmove((void*)offseted_keys_ptrs + key_length + pointer_length, (const void*)offseted_keys_ptrs, key_length + pointer_length);
						break;
					}
				}
				offseted_keys_ptrs -= pointer_length;
				memcpy((void*)offseted_keys_ptrs, (const void*)pointer1, pointer_length);
				offseted_keys_ptrs += pointer_length;
				memcpy((void*)offseted_keys_ptrs, (const void*)*key, key_length);
				offseted_keys_ptrs += key_length;
				memcpy((void*)offseted_keys_ptrs, (const void*)pointer2, pointer_length);
				offseted_keys_ptrs += pointer_length;
				offseted_keys_ptrs += key_length + pointer_length;
				c_key++;
				for (; c_key < n_pointers - 1; ++c_key) {
					/* Key. */
					CALL_IB(IB_Get_Key(file_desc_AM, block, offseted_keys_ptrs, c_key, &get_status));
					if (get_status != 1) { fprintf(stderr, "Failed to get key in BT_Subtree_Insert().\n"); exit(1); }
					offseted_keys_ptrs += key_length;
					/* Pointer. */
					CALL_IB(IB_Get_Pointer(file_desc_AM, block, offseted_keys_ptrs, c_key + 1, &get_status));
					if (get_status != 1) { fprintf(stderr, "Failed to get pointer in BT_Subtree_Insert().\n"); exit(1); }
					offseted_keys_ptrs += pointer_length;
				}

				/*
				 * Now we have one more pointer.
				 * The second pointer overwrote an existing one.
				 */

				/*
				 * We split according to the existing number of pointers.
				 *          |n_pointers / 2
				 * ------------------------
				 * |P|K|P|K|P|Kv|P|(P|K|P)|
				 * ------------------------
				 *               |splitter
				 * This way the left block will have more pointers than right.
				 * Kv is the key for the upper layer to insert.
				 *
				 * So we write pointer[0, splitter - 1] to the left block,
				 * key[splitter - 1] is the key for the upper layer,
				 * pointer[splitter, n_pointers + 1] to the right block.
				 */
				splitter = (n_pointers / 2) + 1;

				offseted_keys_ptrs = keys_ptrs;
				CALL_IB(IB_Write_CountPointers(file_desc_AM, block, 0, &pointers_flag));
				size_t c_pointer;
				for (c_pointer = 0; c_pointer < splitter - 1; ++c_pointer) {
					/* Probably it would be better to BF_Block_GetData() and memcpy from offseted_keys_ptrs to block's data the equivalent bytes. */
					CALL_IB(IB_Insert(file_desc_AM, block, *(int*)offseted_keys_ptrs, offseted_keys_ptrs + pointer_length, *(int*)offseted_keys_ptrs + pointer_length + key_length, &write_flag));
					offseted_keys_ptrs += pointer_length + key_length;
					if (write_flag != 1) { fprintf(stderr, "Failed to write pointer-key-pointer to left index block.\n"); exit(1); }
				}
				offseted_keys_ptrs += pointer_length;  // Point at the key.

				CALL_BL(BL_CreateBlock(file_desc_BF, &new_block_id, &new_block));

				*pointer1 = subtree_root;
				memcpy((void*)*key, (const void*)offseted_keys_ptrs, key_length);
				*pointer2 = new_block_id;

				*splitted = 1;  // Splitted remains true.

				offseted_keys_ptrs += key_length;
				for (c_pointer = splitter; c_pointer < n_pointers + 1; ++c_pointer) {
					CALL_IB(IB_Insert(file_desc_AM, new_block, *(int*)offseted_keys_ptrs, offseted_keys_ptrs + pointer_length, *(int*)offseted_keys_ptrs + pointer_length + key_length, &write_flag));
					offseted_keys_ptrs += pointer_length + key_length;
					if (write_flag != 1) { fprintf(stderr, "Failed to write pointer-key-pointer to right index block.\n"); exit(1); }
				}

				BF_Block_SetDirty(new_block);
				CALL_BF(BF_UnpinBlock(new_block));
				BF_Block_Destroy(&new_block);

				free(keys_ptrs);
			}

			BF_Block_SetDirty(block);
			CALL_BF(BF_UnpinBlock(block));
			BF_Block_Destroy(&block);

		}

		return AME_OK;
	}

	return AME_ERROR;
}
