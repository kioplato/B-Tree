/*******************************************************************************
 * File: FD.h
 * Purpose: API for interacting with the file descriptors array and each file's
 * cached data.
*******************************************************************************/

#ifndef FD_H
#define FD_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "BF.h"  // BF layer.
#include "AM.h"  // AM layer interface.

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
void FD_Init()
{
	for (size_t i = 0; i < AM_MAX_OPEN_FILES; ++i)
		filedescs[i].filedesc = -1;
}

/*
 * Insert a new opened file and it's cache.
 *
 * Returns AME_OK on successful insert and sets index variable to FD.
 * Returns AME_MAX_FILES if the maximum number of files reached.
 */
int FD_Insert(int filedesc, int* index, char* filename, char attrType1,
		int attrLength1, char attrType2, int attrLength2, size_t index_root,
		size_t n_entries)
{
	/* File available index. */
	for (size_t i = 0; i < AM_MAX_OPEN_FILES; ++i) {
		if (filedescs[i].filedesc == -1) {
			filedescs[i].filedesc = filedesc;
			filedescs[i].filename = (char*)malloc(sizeof(char) * (strlen(filename) + 1));
			strcpy(filedescs[i].filename, filename);
			filedescs[i].cache.index_root = index_root;
			filedescs[i].cache.attrType1 = attrType1;
			filedescs[i].cache.attrLength1 = attrLength1;
			filedescs[i].cache.attrType2 = attrType2;
			filedescs[i].cache.attrLength2 = attrLength2;
			filedescs[i].cache.n_entries = n_entries;
			*index = i;
			return AME_OK;
		}
	}
	return AME_MAX_FILES;
}

/*
 * Deletes a file descriptor from AM layer.
 *
 * Returns AME_OK
 */
int FD_Delete(int index)
{
	int flag;  // Flag about if there is an open index.

	if (index >= AM_MAX_OPEN_FILES)
		return AME_FD_INVALID_INDEX;
	if (index < 0)
		return AME_FD_INVALID_INDEX;
	if (filedescs[index].filedesc == -1)
		return AME_FD_INVALID_INDEX;

	flag = -1;
	// TODO.
	//CALL_IS(IS_IsOpen(index, &flag));
	if (flag == 1) return AME_FD_CLOSE_OPEN_SCAN;

	filedescs[index].filedesc = -1;
	free(filedescs[index].filename);

	return AME_OK;
}

/*
 * Get the BF layer file descriptor.
 *
 * Returns AME_OK on success and writes the BF file descriptor on BF_index.
 * Returns AME_FD_INVALID_INDEX if the provided index is invalid.
 */
int FD_Get_FileDesc(int index, int* BF_index)
{
	if (index >= AM_MAX_OPEN_FILES)
		return AME_FD_INVALID_INDEX;
	if (index < 0)
		return AME_FD_INVALID_INDEX;
	if (filedescs[index].filedesc == -1)
		return AME_FD_INVALID_INDEX;

	*BF_index = filedescs[index].filedesc;

	return AME_OK;
}

/*
 * Get the index root for an opened file.
 *
 * Returns AME_OK on success and writes the block number on index_root.
 * Returns AME_FD_INVALID_INDEX if the provided index is invalid.
 */
int FD_Get_IndexRoot(int index, size_t *index_root)
{
	if (index >= AM_MAX_OPEN_FILES)
		return AME_FD_INVALID_INDEX;
	if (index < 0)
		return AME_FD_INVALID_INDEX;
	if (filedescs[index].filedesc == -1)
		return AME_FD_INVALID_INDEX;

	*index_root = filedescs[index].cache.index_root;

	return AME_OK;
}

/*
 * Get the attribute type of the first field.
 *
 * Returns AME_OK on success and sets the attrType1 accordingly.
 * Returns AME_FD_INVALID_INDEX if the provided index is invalid.
 */
int FD_Get_attrType1(int index, char *attrType1)
{
	if (index >= AM_MAX_OPEN_FILES)
		return AME_FD_INVALID_INDEX;
	if (index < 0)
		return AME_FD_INVALID_INDEX;
	if (filedescs[index].filedesc == -1)
		return AME_FD_INVALID_INDEX;

	*attrType1 = filedescs[index].cache.attrType1;

	return AME_OK;
}

/*
 * Get the attribute length of the first field.
 *
 * Returns AME_OK on success and sets the attrLength1 accordingly.
 * Returns AME_FD_INVALID_INDEX if the provided index is invalid.
 */
int FD_Get_attrLength1(int index, int *attrLength1)
{
	if (index >= AM_MAX_OPEN_FILES)
		return AME_FD_INVALID_INDEX;
	if (index < 0)
		return AME_FD_INVALID_INDEX;
	if (filedescs[index].filedesc == -1)
		return AME_FD_INVALID_INDEX;

	*attrLength1 = filedescs[index].cache.attrLength1;

	return AME_OK;
}

/*
 * Get the attribute type of the second field.
 *
 * Returns AME_OK on success and sets the attrType2 accordingly.
 * Returns AME_FD_INVALID_INDEX if the provided index is invalid.
 */
int FD_Get_attrType2(int index, char *attrType2)
{
	if (index >= AM_MAX_OPEN_FILES)
		return AME_FD_INVALID_INDEX;
	if (index < 0)
		return AME_FD_INVALID_INDEX;
	if (filedescs[index].filedesc == -1)
		return AME_FD_INVALID_INDEX;

	*attrType2 = filedescs[index].cache.attrType2;

	return AME_OK;
}

/*
 * Get the attribute length of the second field.
 *
 * Returns AME_OK on success and sets the attrLength2 accordingly.
 * Returns AME_FD_INVALID_INDEX if the provided index is invalid.
 */
int FD_Get_attrLength2(int index, int *attrLength2)
{
	if (index >= AM_MAX_OPEN_FILES)
		return AME_FD_INVALID_INDEX;
	if (index < 0)
		return AME_FD_INVALID_INDEX;
	if (filedescs[index].filedesc == -1)
		return AME_FD_INVALID_INDEX;

	*attrLength2 = filedescs[index].cache.attrLength2;

	return AME_OK;
}

int FD_Get_Entries(int index, size_t* n_entries)
{
	if (index >= AM_MAX_OPEN_FILES)
		return AME_FD_INVALID_INDEX;
	if (index < 0)
		return AME_FD_INVALID_INDEX;
	if (filedescs[index].filedesc == -1)
		return AME_FD_INVALID_INDEX;

	*n_entries = filedescs[index].cache.n_entries;

	return AME_OK;
}

/*
 * Updates the block number of index's root of file's cached data across all opens.
 *
 * Returns AME_OK on success.
 * Returns AME_FD_INVALID_INDEX if the provided index is invalid.
 */
int FD_Set_IndexRoot(int index, size_t index_root)
{
	char* filename;  // The index's filename.

	if (index >= AM_MAX_OPEN_FILES)
		return AME_FD_INVALID_INDEX;
	if (index < 0)
		return AME_FD_INVALID_INDEX;
	if (filedescs[index].filedesc == -1)
		return AME_FD_INVALID_INDEX;

	filename = filedescs[index].filename;

	for (size_t i = 0; i < AM_MAX_OPEN_FILES; ++i) {
		if (strcmp(filename, filedescs[i].filename) == 0)
			filedescs[i].cache.index_root = index_root;
	}

	return AME_OK;
}

/*
 * Checks if the file with name fileName is open at least once.
 *
 * Returns AME_OK on success and sets flag accordingly.
 * Returns AME_ERROR on failure.
 */
int FD_IsOpen(char fileName[], int* flag)
{
	if (fileName == NULL)
		return AME_ERROR;

	for (size_t i = 0; i < AM_MAX_OPEN_FILES; ++i) {
		if (strcmp(filedescs[i].filename, fileName) == 0) {
			*flag = 1;
			return AME_OK;
		}
	}

	*flag = 0;
	return AME_OK;
}

#endif  // #ifndef FD_H
