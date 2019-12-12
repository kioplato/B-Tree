/*******************************************************************************
 * File: FD.h
 * Purpose: API for the file descriptors array and each file's cached data.
*******************************************************************************/

#ifndef FD_H
#define FD_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../BF.h"  // BF layer.
#include "../AM.h"  // AM layer interface.

#define AM_MAX_OPEN_FILES 20

#define CALL_FD(func_call)	\
{							\
	int code = func_call;	\
	if (code != AME_OK)		\
		AM_errno = code;	\
		return code;		\
}

// The cache of an opened file.
struct cache_t {
	size_t index_root;  // The index's root.
	char attrType1;     // The type of the record's first field.
	int attrLength1;    // The length of the record's first field.
	char attrType2;     // The type of the record's second field.
	int attrLength2;    // The length of the record's second field.
	size_t n_entries;   // The number of entries a data block can store.
};

// File descriptor entries for AM layer's array.
struct filedesc_t {
	int filedesc;          // File descriptor in BF layer.
	char* filename;        // Filename of the opened file.
	struct cache_t cache;  // File's cached information.
};

// The array with the cache'd information about each opened file.
struct filedesc_t filedescs[AM_MAX_OPEN_FILES];

/*
 * Initialize the file descriptors array.
 *
 * Always succeeds.
 */
void FD_Init();

/*
 * Insert a new opened file and it's cache.
 *
 * Returns AME_OK on successful insert and sets index variable to FD.
 * Returns AME_MAX_FILES if the maximum number of files reached.
 */
int FD_Insert(int filedesc, int* index, char* filename, char attrType1,
		int attrLength1, char attrType2, int attrLength2, size_t index_root,
		size_t n_entries);

/*
 * Deletes a file descriptor from AM layer.
 *
 * Returns AME_OK on success.
 * Returns AME Error code on failure.
 */
int FD_Delete(int index);

/*
 * Get the BF layer file descriptor.
 *
 * Returns AME_OK on success and writes the BF file descriptor on BF_index.
 * Returns AME_FD_INVALID_INDEX if the provided index is invalid.
 */
int FD_Get_FileDesc(int index, int* BF_index);

/*
 * Get the index root for an opened file.
 *
 * Returns AME_OK on success and writes the block number on index_root.
 * Returns AME_FD_INVALID_INDEX if the provided index is invalid.
 */
int FD_Get_IndexRoot(int index, size_t *index_root);

/*
 * Get the attribute type of the first field.
 *
 * Returns AME_OK on success and sets the attrType1 accordingly.
 * Returns AME_FD_INVALID_INDEX if the provided index is invalid.
 */
int FD_Get_attrType1(int index, char *attrType1);

/*
 * Get the attribute length of the first field.
 *
 * Returns AME_OK on success and sets the attrLength1 accordingly.
 * Returns AME_FD_INVALID_INDEX if the provided index is invalid.
 */
int FD_Get_attrLength1(int index, int *attrLength1);

/*
 * Get the attribute type of the second field.
 *
 * Returns AME_OK on success and sets the attrType2 accordingly.
 * Returns AME_FD_INVALID_INDEX if the provided index is invalid.
 */
int FD_Get_attrType2(int index, char *attrType2);

/*
 * Get the attribute length of the second field.
 *
 * Returns AME_OK on success and sets the attrLength2 accordingly.
 * Returns AME_FD_INVALID_INDEX if the provided index is invalid.
 */
int FD_Get_attrLength2(int index, int *attrLength2);

int FD_Get_Entries(int index, size_t* n_entries);

/*
 * Updates the block number of index's root of file's cached data across all opens.
 *
 * Returns AME_OK on success.
 * Returns AME_FD_INVALID_INDEX if the provided index is invalid.
 */
int FD_Set_IndexRoot(int index, size_t index_root);

/*
 * Checks if the file with name fileName is open at least once.
 *
 * Returns AME_OK on success and sets flag accordingly.
 * Returns AME_ERROR on failure.
 */
int FD_IsOpen(char fileName[], int* flag);

#endif  // #ifndef FD_H
