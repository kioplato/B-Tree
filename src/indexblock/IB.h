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
int IB_Init(BF_Block* index_block, int pointer1, void* key, int pointer2);
