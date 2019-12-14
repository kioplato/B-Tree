/*******************************************************************************
 * File: IS.h
 * Purpose: API for interacting with the index scan array
*******************************************************************************/

#ifndef IS_H
#define IS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "BF.h"
#include "filedesc/FD.h"
#include "accessmethod/AM.h"

#define AM_MAX_SCAN_FILES 20

#define CALL_IS(func_call)	\
{							\
	int code = func_call;	\
	if (code != AME_OK)		\
		AM_errno = code;	\
		return code;		\
}


/////////////////////////////////////////////

#define AME_IS_MAX_FILES -15
#define AME_IS_INVALID_INDEX -16
#define AME_IS_OPEN -17

///////////////////////////////////////////////

/*
 * next: the record inside a data block to be examined.
 * last_block: the latest block in which we search for records.
 * index_desc: the index of the filedescs[] array.
 */

struct scan_t {
	int next;
	int last_block;
	int op;
	int index_desc;
	void* value;
};

struct scan_t scanIndexArray[AM_MAX_SCAN_FILES];

/*
 * Initializes the scanIndexArray[AM_MAX_SCAN_FILES].
 */
void IS_Init();


/*
 * Checks if there is available index on scanIndexArray
 * so the scan can be executed. It returns the first available index.
 * Returns AME_OK if there is, AME_IS_MAX_FILES otherwise.
 */
int IS_OpenScan(int *scan_index);


/*
 * Inserts to scanIndexArray[scan_index] the data which AM_FindNextEntry will use.
 * Returns AME_OK if the scan can be opened.
 * Return AME_IS_MAX_FILES if the scanIndexArray[] is full.
 */
int IS_Insert(int next, int last_block, int op, int index_desc, void* value, int scan_index);


/*
* Gets the next field of struct of scanIndexArray[index].
* Returns AME_IS_INVALID_INDEX if the scan doesnt exist.
* Returns AME_OK if the scan exists.
*/
int IS_Get_next(int index, size_t *next);


/*
 * Gets the last_block field of struct of scanIndexArray[index].
 * Returns AME_IS_INVALID_INDEX if the scan doesnt exist.
 * Returns AME_OK if the scan exists.
 */
int IS_Get_last_block(int index, char *last_block);


/*
 * Gets the op of the opened scan.
 * Returns AME_IS_INVALID_INDEX if the scan doesnt exist.
 * Returns AME_OK if the scan exists.
 */
int IS_Get_op(int index, char *op);


/*
 * Gets the index_desc field of struct of scanIndexArray[index]
 * that is, the index of the filedescs[index_desc], of the opened file.
 * Returns AME_IS_INVALID_INDEX if the scan doesnt exist.
 * Returns AME_OK if the scan exists.
 */
int IS_Get_index_desc(int index, int *index_desc);


/*
 * Gets the value pointer of the opened scan.
 * Returns AME_IS_INVALID_INDEX if the scan doesnt exist.
 * Returns AME_OK if the scan exists.
 */
int IS_Get_value(int index, void** value);


/*
 * Sets the 'next' field of struct of scanIndexArray[index].
 * Returns AME_IS_INVALID_INDEX if the scan doesnt exist.
 * Returns AME_OK if the scan exists.
 */
int IS_Set_next(int index, size_t next);


/*
 * Sets the 'last_block' field of struct of scanIndexArray[index].
 * Returns AME_IS_INVALID_INDEX if the scan doesnt exist.
 * Returns AME_OK if the scan exists.
 */
int IS_Set_last_block(int index, char last_block);



/*
 * Closes the scan by initializing the value of 'index_desc' field.
 * of struct of scanIndexArray[index].
 * Returns AME_IS_INVALID_INDEX if the scan doesnt exist.
 * Returns AME_OK if the scan exists.
 */
int IS_Close(int index);


/*
 * Checks if at least one scan of an opened file with index_desc is open.
 * Returns AME_IS_OPEN if a scan is open.
 * Returns AME_OK if no scan exists.
 */
int IS_IsOpen(int index_desc, int* flag);

#endif  // #ifndef IS_H
