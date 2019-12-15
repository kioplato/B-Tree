/*******************************************************************************
 * File: DB.h
 * Purpose: API for interacting with Data Blocks.
*******************************************************************************/

#ifndef DB_H
#define DB_H

#include <stdio.h>
#include <string.h>

#include "../BF.h"
#include "../accessmethod/AM.h"
#include "../filedesc/FD.h"
#include "../defn.h"
#include "../record/RD.h"

#define DATACODE "DBL" // Code which means the block is data block.

#define CALL_DB(func_call)	\
{							\
	int code = func_call;	\
	if (code != AME_OK)		\
		AM_errno = code;	\
		return code;		\
}

/*
 * Initialize Data Block.
 * Write Data Block's initial metadata with:
 * current entries = 0
 * next block id = next_block
 *
 * Returns AME_OK on success.
 * Returns AME_ERROR on failure.
 */
int DB_Init(BF_Block* block, size_t next_block);

/*
 * Check whether provided block is Data Block.
 * Sets flag = 1 if it is.
 * Sets flag = 0 if it is not.
 *
 * Returns AME_OK on success.
 * Returns AME_ERROR on failure.
 */
int DB_Is_DataBlock(BF_Block* block, int* flag);

/*
 * Get the next block id from the provided Data Block.
 * Writes the next block id to next_block.
 * Checks whether provided block is indeed a Data Block.
 *
 * Returns AME_OK on success.
 * Returns AME_ERROR on failure.
 */
int DB_Get_NextBlock(BF_Block* block, size_t* next_block);

/*
 * Get the current number of entries from the provided Data Block.
 * Writes the current entries to c_entries.
 * Checks whether provided block is indeed a Data Block.
 *
 * Returns AME_OK on success.
 * Returns AME_ERROR on failure.
 */
int DB_Get_Entries(BF_Block* block, size_t* c_entries);

/*
 * Updates the number of entries of block.
 * Overwrites the c_entries over the number of entries of the block.
 * If c_entries > n_entries then flag = 0.
 * If c_entries <= n_entries then flag = 1.
 *
 * Returns AME_OK on success.
 * Returns AME_ERROR on failure.
 */
int DB_Write_Entries(int file_desc, BF_Block* block, size_t c_entries, int* flag);

/*
 * Get the maximum entries a Data Block can store.
 * Writes the maximum entries to n_entries.
 * Checks whether provided block is Data Block.
 *
 * Returns AME_OK on success.
 * Returns AME_ERROR on failure.
 */
int DB_Get_MaxEntries(int file_desc, size_t* n_entries);

/*
 * Get the c_entry-th record from provided Data Block.
 * Writes the record to record.
 * If c_entry >= n_entries then flag = -1.
 * If c_entry >= c_entries then flag = 0.
 * If c_entry < c_entries then flag = 1.
 * Checks whether provided block is Data Block.
 *
 * Returns AME_OK on success.
 * Returns AME_ERROR on failure.
 */
int DB_Get_Record(int file_desc, BF_Block* block, Record* record, size_t c_entry, int* flag);

/*
 * Write the provided record in Data Block at c_entry-th position.
 * If c_entry >= n_entries then flag = -1.
 * If c_entry >= c_entries then flag = 0.
 * If c_entry < c_entries then flag = 1.
 *
 * Returns AME_OK on success.
 * Returns AME_ERROR on failure.
 */
int DB_Write_Record(int file_desc, BF_Block* block, Record record, size_t c_entry, int* flag);

/*
 * Shifts the records inside a Data Block to the right.
 * Shifts the last records starting from and including shift_base.
 * If shift_base >= n_entries flag == -1.
 * If shift_base >= c_entries flag == 0.
 * If shift_base < c_entries flag == 1.
 *
 * block: the Data Block.
 * shift_base: the starting point of the shift. Values from 0 to n_entries - 1.
 *
 * Returns AME_OK code on success.
 * Returns AME_ERROR on failure.
 */
int DB_Shift_Records_Right(int file_desc, BF_Block* block, size_t shift_base, int* flag);

/*
 * Inserts a record in provided Data Block while maintaining sortness.
 * If block is full flag = 0 (record isn't written).
 * If block isn't full flag = 1 (record is written).
 *
 * Returns AME_OK on success.
 * Returns AME_ERROR on failure.
 */
int DB_Insert(int file_desc, BF_Block* block, Record record, int* flag);

#endif  // #ifndef DB_H
