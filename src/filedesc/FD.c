/*******************************************************************************
 * File: FD.c
 * Purpose: Implementation of the API for the file descriptors array and each
 *          file's cached data.
*******************************************************************************/

#include <stdio.h>

#include "../accessmethod/AM.h"
#include "FD.h"

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
		int attrLength1, char attrType2, int attrLength2, size_t index_root)
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
			*index = i;
			return AME_OK;
		}
	}
	return AME_MAX_FILES;
}

int FD_Delete(int index)
{
	if (index >= AM_MAX_OPEN_FILES)
		return AME_FD_INVALID_INDEX;
	if (index < 0)
		return AME_FD_INVALID_INDEX;
	if (filedescs[index].filedesc == -1)
		return AME_FD_INVALID_INDEX;

	filedescs[index].filedesc = -1;
	free(filedescs[index].filename);

	return AME_OK;
}

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
		if (filedescs[i].filedesc != -1) {
			if (strcmp(filename, filedescs[i].filename) == 0)
				filedescs[i].cache.index_root = index_root;
		}
	}

	return AME_OK;
}

int FD_IsOpen(char fileName[], int* flag)
{
	if (fileName == NULL)
		return AME_ERROR;

	for (size_t i = 0; i < AM_MAX_OPEN_FILES; ++i) {
		if (filedescs[i].filedesc != -1) {
			if (strcmp(filedescs[i].filename, fileName) == 0) {
				*flag = 1;
				return AME_OK;
			}
		}
	}

	*flag = 0;
	return AME_OK;
}
