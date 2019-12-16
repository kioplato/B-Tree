#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../BF.h"

#define CALL_IB(func_call)	\
{							\
	int code = func_call;	\
	if (code != AME_OK) {	\
		AM_errno = code;	\
		return code;		\
	}						\
}

/*
 * Given the first key to insert in the index block and the two pointers,
 * it initializes the index block by inserting
 * num_of_pointers=2, pointer1, key, pointer2
 */
int IB_Init(int file_desc_AM, BF_Block* block, int pointer1, void* key,
		int pointer2);

int IB_Get_CountPointers(BF_Block* block, size_t* c_pointers);

int IB_Get_MaxCountPointers(int file_desc_AM, size_t* n_pointers);

int IB_Write_CountPointers(int file_desc_AM, BF_Block* block, size_t c_pointers,
		int* flag);

int IB_Get_Pointer(int file_desc_AM, BF_Block* block, int* pointer,
		size_t c_pointer, int* flag);

int IB_Get_Key(int file_desc_AM, BF_Block* block, void* key, size_t c_key,
		int* flag);

int IB_Write_Key(int file_desc_AM, BF_Block* block, int pointer1, void* key,
		int pointer2, size_t c_key, int* flag);

int IB_Shift_Right(int file_desc_AM, BF_Block* block, size_t shift_base,
		int* flag);

int IB_Insert(int file_desc_AM, BF_Block* block, int pointer1, void* key,
		int pointer2, int* flag);

int IB_Print(int file_desc_AM, BF_Block* block);
