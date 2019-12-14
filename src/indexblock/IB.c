
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../BF.h"

int IB_Init(BF_Block* index_block, int pointer1, void* key, int pointer2){
    char* data = NULL;  // The index block's data.
    if (index_block == NULL) return AME_ERROR;
    data = BF_Block_GetData(index_block);
    if (data == NULL) return AME_ERROR;

    if (key = NULL) return AME_ERROR;
    int num_of_pointers = 2;
    memcpy((void*)data, (const void*)&num_of_pointers, sizeof(int));
    data+=sizeof(int);
    memcpy((void*)data, (const void*)&pointer1, sizeof(int));
    data+=sizeof(int);
    memcpy((void*)data, (const void*)key, sizeof(*key));
    data+=sizeof(*key);
    memcpy((void*)data, (const void*)&pointer2, sizeof(int));
  }
