/*******************************************************************************
 * File: FD.h
 * Purpose: API for interacting with the file descriptors array and each file's
 * cached data.
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/bf.h"
#include "../include/AM.h"

#define AM_MAX_OPEN_FILES 20

#define CALL_FD(func_call)	\
{							\
	int code = func_call;	\
	if (code != AME_OK)		\
		AM_errno = code;	\
		return code;		\
}

struct cache_t {
	size_t index_root;
	char attrType1;
	int attrLength1;
	char attrType2;
	int attrLength2;
};

struct filedesc_t {
	int filedesc;
	char* filename;
	struct cache_t cache;
};

struct filedesc_t filedescs[AM_MAX_OPEN_FILES];

void FD_Init()
{
	for (size_t i = 0; i < AM_MAX_OPEN_FILES; ++i)
		filedescs[i].filedesc = -1;
}

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

int FD_Get_filedesc(int index, size_t *filedesc){
	if (filedescs[index].filedesc == -1)
		return AME_FD_INVALID_INDEX;
	*filedesc = filedescs[index].filedesc;
	return AME_OK;
}


int FD_Get_IndexRoot(int index, size_t *index_root)
{
	if (filedescs[index].filedesc == -1)
		return AME_FD_INVALID_INDEX;
	*index_root = filedescs[index].cache.index_root;
	return AME_OK;
}

int FD_Get_attrType1(int index, char *attrType1)
{
	if (filedescs[index].filedesc == -1)
		return AME_FD_INVALID_INDEX;
	*attrType1 = filedescs[index].cache.attrType1;
	return AME_OK;
}

int FD_Get_attrLenght1(int index, int *attrLength1)
{
	if (filedescs[index].filedesc == -1)
		return AME_FD_INVALID_INDEX;
	*attrLength1 = filedescs[index].cache.attrLength1;
	return AME_OK;
}

int FD_Get_attrType2(int index, char *attrType2)
{
	if (filedescs[index].filedesc == -1)
		return AME_FD_INVALID_INDEX;
	*attrType2 = filedescs[index].cache.attrType2;
	return AME_OK;
}

int FD_Get_attrLenght2(int index, int *attrLength2)
{
	if (filedescs[index].filedesc == -1)
		return AME_FD_INVALID_INDEX;
	*attrLength2 = filedescs[index].cache.attrLength2;
	return AME_OK;
}

/*
 * Update the index root for the file and it's other possible opens.
 *
 * Returns AME_OK on success.
 * Returns AME_FD_INVALID_INDEX if the index does not contain an open file.
 */
int FD_Set_IndexRoot(int index, size_t index_root) {
	char* filename;  // The index's filename.

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
 * Returns AME_OK on success.
 * Returns an AM error code on failure.
 */
int FD_IsOpen(char fileName[], int* is_open) {
	if (fileName == NULL)
		return AME_ERROR;

	for (size_t i = 0; i < AM_MAX_OPEN_FILES; ++i) {
		if (strcmp(filedescs[i].filename, fileName) == 0) {
			*is_open = 1;
			return AME_OK;
		}
	}

	*is_open = 0;
	return AME_OK;
}
