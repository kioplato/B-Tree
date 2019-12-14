/*******************************************************************************
 * File: IB.c
 * Purpose: Implementation of API for interacting with Index Blocks.
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../BF.h"
#include "../accessmethod/AM.h"
#include "../filedesc/FD.h"

int IB_Init(BF_Block* index_block, int pointer1, void* key, int pointer2){
    char* data = NULL;  // The index block's data.
    if (index_block == NULL) return AME_ERROR;
    data = BF_Block_GetData(index_block);
    if (data == NULL) return AME_ERROR;

    if (key == NULL) return AME_ERROR;
    int num_of_pointers = 2;
    memcpy((void*)data, (const void*)&num_of_pointers, sizeof(int));
    data+=sizeof(int);
    memcpy((void*)data, (const void*)&pointer1, sizeof(int));
    data+=sizeof(int);
    memcpy((void*)data, (const void*)key, sizeof(*key));
    data+=sizeof(*key);
    memcpy((void*)data, (const void*)&pointer2, sizeof(int));

	return AME_OK;
}


int IB_GetPointer(BF_Block *index_block, void* key, int* pointer, int indexDesc){
    char* data = NULL;
    if (index_block == NULL) return AME_ERROR;
    data = BF_Block_GetData(index_block);
    if (data == NULL) return AME_ERROR;
    int num_of_pointers;
    memcpy((const void*)&num_of_pointers, (void*)data, sizeof(int));
    // Skip the number of pointers metadata in index block
    // data points to the first pointer (P0)
    data+=sizeof(int);
    //temporaly hold the pointer
    memcpy((const void*)pointer, (void*)data, sizeof(int));
    num_of_keys=num_of_pointers-1;
    data+=sizeof(int); // Skip the first pointer, data points at first key
    char Type1 = filedescs[indexDesc].cache.attrType1;
    int Lenght1 = filedescs[indexDesc].cache.attrLength1;

    /*
     * We must check the keys in index block in order to choose the correct
     * pointer of the index block. In order to do that, we need to know the key
     * type, that is, the type (i,f,c) and its' lenght in case of char type.
     * We test cases for each type: int, float or char
     */

    if (Type1=='i'){
        int key_i;
        memcpy((const void*)&key_i, (void*)data, sizeof(int));
        if (*key<key_i){
          return AME_OK;
        }
        data+=sizeof(int); //data points to second pointer (P1)
        memcpy((const void*)pointer, (void*)data, sizeof(int));
        data+=sizeof(int); //data points to second key (K2)
        for (int k=2; k<=num_of_keys; k++){
          memcpy((const void*)&key_i, (void*)data, sizeof(int));
          if (*key<key_i)
            return AME_OK;
          else{
            data+=sizeof(int); //data points to next pointer
            memcpy((const void*)pointer, (void*)data, sizeof(int));
            if (k==num_of_keys)
              return AME_OK;
            data+=sizeof(int); //data points to next key
          }
        }
    }

    if (Type1=='f'){
        int key_f;
        memcpy((const void*)&key_f, (void*)data, sizeof(float));
        if (*key<key_f){
          return AME_OK;
        }
        data+=sizeof(float); //data points to second pointer (P1)
        memcpy((const void*)pointer, (void*)data, sizeof(int));
        data+=sizeof(int); //data points to second key (K2)
        for (int k=2; k<=num_of_keys; k++){
          memcpy((const void*)&key_f, (void*)data, sizeof(float));
          if (*key<key_f)
            return AME_OK;
          else{
            data+=sizeof(float); //data points to next pointer
            memcpy((const void*)pointer, (void*)data, sizeof(int));
            if (k==num_of_keys)
              return AME_OK;
            data+=sizeof(int); //data points to next key
          }
        }
    }


    if (Type1=='c'){
        int key_c;
        memcpy((const void*)&key_c, (void*)data, sizeof(Lenght1));
        if (strcmp(key, &key_c)<0){
          return AME_OK;
        }
        data+=sizeof(Lenght1); //data points to second pointer (P1)
        memcpy((const void*)pointer, (void*)data, sizeof(int));
        data+=sizeof(int); //data points to second key (K2)
        for (int k=2; k<=num_of_keys; k++){
          memcpy((const void*)&key_f, (void*)data, sizeof(Lenght1));
          if (strcmp(key, &key_c)<0)
            return AME_OK;
          else{
            data+=sizeof(Lenght1); //data points to next pointer
            memcpy((const void*)pointer, (void*)data, sizeof(int));
            if (k==num_of_keys)
              return AME_OK;
            data+=sizeof(int); //data points to next key
          }
        }
    }

    return AME_ERROR

}
