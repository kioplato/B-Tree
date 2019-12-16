/*******************************************************************************
 * File: IB.c
 * Purpose: Implementation of API for interacting with Index Blocks.
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "IB.h"
#include "../record/RD.h"
#include "../BF.h"
#include "../accessmethod/AM.h"
#include "../filedesc/FD.h"
#include "../defn.h"

int IB_Init(int file_desc_AM, BF_Block* block, int pointer1, void* key, int pointer2)
{
	char* data = NULL;  // The index block's data.
	char* offseted_data = NULL;  // The block's data for iterating block.

    size_t num_of_pointers = 2;  // The number of pointers.

	char key_type;  // The variable type of key.
	int key_length; // The length of key in bytes.
	size_t write_bytes;  // The bytes to write.

	if (block == NULL) return AME_ERROR;
	if (key == NULL) return AME_ERROR;

	data = NULL;
	data = BF_Block_GetData(block);
	if (data == NULL) return AME_ERROR;

	CALL_FD(FD_Get_attrType1(file_desc_AM, &key_type));
	CALL_FD(FD_Get_attrLength1(file_desc_AM, &key_length));

	if (key_type == INTEGER || key_type == FLOAT) {
		write_bytes = key_length;
	}
	else if (key_type == STRING) {
		write_bytes = strlen((char*)key) + 1;
	} else {
		fprintf(stderr, "Invalid type of key in IB_Init().\n");
		return AME_ERROR;
	}

	offseted_data = data;
	memcpy((void*)offseted_data, (const void*)&num_of_pointers, sizeof(size_t));
	offseted_data += sizeof(size_t);
	memcpy((void*)offseted_data, (const void*)&pointer1, sizeof(int));
	offseted_data += sizeof(int);
	memcpy((void*)offseted_data, (const void*)key, write_bytes);
	offseted_data += key_length;
	memcpy((void*)offseted_data, (const void*)&pointer2, sizeof(int));

	return AME_OK;
}

int IB_Get_CountPointers(BF_Block* block, size_t* c_pointers)
{
	char* data;           // Block's data.
	char* offseted_data;  // For traversing the data.

	if (block == NULL) return AME_ERROR;
	if (c_pointers == NULL) return AME_ERROR;

	data = NULL;
	data = BF_Block_GetData(block);
	if (data == NULL) return AME_ERROR;

	offseted_data = data;
	memcpy((void*)c_pointers, (const void*)offseted_data, sizeof(size_t));

	return AME_OK;
}

int IB_Get_MaxCountPointers(int file_desc_AM, size_t* n_pointers)
{
	size_t metadata_size;  // The metadata size of index block.

	int key_length;  // The length of key in bytes.
	size_t pointer_length;  // The length of a pointer.

	if (n_pointers == NULL) return AME_ERROR;

	/* Metadata contains only the number of pointers. */
	metadata_size = sizeof(size_t);

	CALL_FD(FD_Get_attrLength1(file_desc_AM, &key_length));
	pointer_length = sizeof(int);

	/* Calculate max pointers. */
	*n_pointers = (BF_BLOCK_SIZE - metadata_size - pointer_length) / (pointer_length + key_length) + 1;

	return AME_OK;
}

int IB_Write_CountPointers(int file_desc_AM, BF_Block* block, size_t c_pointers, int* flag)
{
	char* data;           // The block's data.
	char* offseted_data;  // For iterating over block's data.

	size_t n_pointers;  // The number of maximum pointers in an index block.

	if (block == NULL) return AME_ERROR;
	if (flag == NULL) return AME_ERROR;

	/* If we are trying to write c_pointers more than maximum number of pointers. */
	CALL_IB(IB_Get_MaxCountPointers(file_desc_AM, &n_pointers));
	if (c_pointers > n_pointers) {
		*flag = 0;
		return AME_OK;
	}

	data = NULL;
	data = BF_Block_GetData(block);
	if (data == NULL) return AME_ERROR;

	offseted_data = data;
	memcpy((void*)offseted_data, (const void*)&c_pointers, sizeof(size_t));

	*flag = 1;

	return AME_OK;
}

int IB_Get_Pointer(int file_desc_AM, BF_Block* block, int* pointer,
		size_t c_pointer, int* flag)
{
	size_t n_pointers;  // Maximum number of pointers in index block.
	size_t c_pointers;  // Current number of pointers in block.

	size_t pointer_key_length;  // Length of key + pointer in bytes.
	size_t pointer_length;      // Length of a pointer in bytes.
	int key_length;             // Length of a key in bytes.

	char* data;
	char* offseted_data;

	if (block == NULL) return AME_ERROR;
	if (pointer == NULL) return AME_ERROR;
	if (flag == NULL) return AME_ERROR;

	CALL_IB(IB_Get_MaxCountPointers(file_desc_AM, &n_pointers));
	if (c_pointer >= n_pointers) {
		*flag = -1;
		return AME_OK;
	}

	CALL_IB(IB_Get_CountPointers(block, &c_pointers));
	if (c_pointer >= c_pointers) {
		*flag = 0;
		return AME_OK;
	}

	CALL_FD(FD_Get_attrLength1(file_desc_AM, &key_length));
	pointer_length = sizeof(int);
	pointer_key_length = key_length + pointer_length;

	data = NULL;
	data = BF_Block_GetData(block);
	if (data == NULL) return AME_ERROR;

	offseted_data = data;
	offseted_data += sizeof(size_t);  // Skip the count pointers.
	offseted_data += c_pointer * pointer_key_length;

	memcpy((void*)pointer, (const void*)offseted_data, pointer_length);

	*flag = 1;

	return AME_OK;
}

int IB_Get_Key(int file_desc_AM, BF_Block* block, void* key, size_t c_key, int* flag)
{
	size_t n_pointers;  // The maximum number of pointers in index block.
	size_t c_pointers;  // The current number of pointers in index block.

	size_t n_keys;  // The maximum number of keys in index block.
	size_t c_keys;  // The current number of keys in index block.

	int key_length;
	size_t key_pointer_pair_size;

	char* data;
	char* offseted_data;

	if (block == NULL) return AME_ERROR;
	if (key == NULL) return AME_ERROR;
	if (flag == NULL) return AME_ERROR;

	CALL_IB(IB_Get_MaxCountPointers(file_desc_AM, &n_pointers));
	n_keys = n_pointers - 1;
	if (c_key >= n_keys) {
		*flag = -1;
		return AME_OK;
	}

	CALL_IB(IB_Get_CountPointers(block, &c_pointers));
	c_keys = c_pointers - 1;
	if (c_key >= c_keys) {
		*flag = 0;
		return AME_OK;
	}

	CALL_FD(FD_Get_attrLength1(file_desc_AM, &key_length));
	key_pointer_pair_size = key_length + sizeof(int);

	data = NULL;
	data = BF_Block_GetData(block);
	if (data == NULL) return AME_ERROR;

	offseted_data = data;
	offseted_data += sizeof(size_t);  // Skip the number of pointers.
	offseted_data += sizeof(int);  // Skip the first pointer.
	offseted_data += key_pointer_pair_size * c_key;  // Go to the requested key.

	memcpy((void*)key, (const void*)offseted_data, key_length);

	*flag = 1;

	return AME_OK;
}

int IB_Write_Key(int file_desc_AM, BF_Block* block, int pointer1, void* key,
		int pointer2, size_t c_key, int* flag)
{
	size_t n_pointers;  // The max number of pointers a block can store.
	size_t c_pointers;  // The current number of entries in block.

	size_t n_keys;  // The max number of keys in an index block.
	size_t c_keys;  // The current number of keys in block.

	int pointers_flag;  // Status flag for updating block's c_pointers.

	size_t key_pointer_size;  // The size of key and pointer in bytes.
	int key_length;           // The length of key in bytes.
	int pointer_length;       // The length of pointer in bytes.

	char key_type;  // The variable type of key.

	size_t write_bytes;  // The bytes to write.

	char* data;           // The block's data.
	char* offseted_data;  // For iterating block's data.

	if (block == NULL) return AME_ERROR;
	if (key == NULL) return AME_ERROR;
	if (flag == NULL) return AME_ERROR;

	CALL_IB(IB_Get_MaxCountPointers(file_desc_AM, &n_pointers));
	n_keys = n_pointers - 1;
	if (c_key >= n_keys) {
		*flag = -1;
		return AME_OK;
	}

	CALL_IB(IB_Get_CountPointers(block, &c_pointers));
	c_keys = c_pointers - 1;
	if (c_key > c_keys) {
		*flag = 0;
		return AME_OK;
	}

	/* If we don't overwrite an existing key, increase block's number of pointers. */
	if (c_key == c_keys) {
		c_pointers++;
		CALL_IB(IB_Write_CountPointers(file_desc_AM, block, c_pointers, &pointers_flag));
		if (pointers_flag != 1) {
			fprintf(stderr, "Failed to write new number of pointers in IB_Write_Key().\n");
			return AME_ERROR;
		}
	}

	CALL_FD(FD_Get_attrType1(file_desc_AM, &key_type));

	CALL_FD(FD_Get_attrLength1(file_desc_AM, &key_length));
	pointer_length = sizeof(int);
	key_pointer_size = key_length + pointer_length;

	data = NULL;
	data = BF_Block_GetData(block);
	if (data == NULL) return AME_ERROR;

	offseted_data = data;
	offseted_data += sizeof(size_t);  // Skip metadata.
	offseted_data += sizeof(int);     // Skip first pointer.

	offseted_data += c_key * key_pointer_size;

	/* Calculate the write_bytes for key. */
	if (key_type == INTEGER || key_type == FLOAT) {
		write_bytes = key_length;
	}
	else if (key_type == STRING) {
		write_bytes = strlen((char*)key) + 1;  // +1 for NULL byte.
	} else {
		fprintf(stderr, "Invalid type of key in IB_Write_Key().\n");
		exit(1);
	}

	/* Write left pointer. */
	offseted_data -= pointer_length;
	memcpy((void*)offseted_data, (const void*)&pointer1, pointer_length);
	offseted_data += pointer_length;

	/* Write the key. */
	memcpy((void*)offseted_data, (const void*)key, write_bytes);
	offseted_data += key_length;

	/* Write right pointer. */
	memcpy((void*)offseted_data, (const void*)&pointer2, pointer_length);

	*flag = 1;

	return AME_OK;
}

int IB_Shift_Right(int file_desc_AM, BF_Block* block, size_t shift_base,
		int* flag)
{
	size_t n_pointers;
	size_t c_pointers;

	size_t n_keys;
	size_t c_keys;

	size_t key_pointer_size;
	int key_length;
	int pointer_length;

	char* data;
	char* offseted_data;

	size_t move_bytes;

	int pointers_flag;

	if (block == NULL) return AME_ERROR;
	if (flag == NULL) return AME_ERROR;

	/* Check if the block is full. */
	CALL_IB(IB_Get_MaxCountPointers(file_desc_AM, &n_pointers));
	CALL_IB(IB_Get_CountPointers(block, &c_pointers));
	if (c_pointers == n_pointers) {
		*flag = -2;
		return AME_OK;
	}

	n_keys = n_pointers - 1;
	c_keys = c_pointers - 1;

	/* Check if shift_base is invalid. */
	if (shift_base >= n_keys) {
		*flag = -1;
		return AME_OK;
	}

	if (shift_base >= c_keys) {
		*flag = 0;
		return AME_OK;
	}

	CALL_FD(FD_Get_attrLength1(file_desc_AM, &key_length));
	pointer_length = sizeof(int);
	key_pointer_size = key_length + pointer_length;

	data = NULL;
	data = BF_Block_GetData(block);
	if (data == NULL) return AME_ERROR;

	offseted_data = data;
	offseted_data += sizeof(size_t);  // Skip metadata.
	offseted_data += sizeof(pointer_length);  // Skip first pointer.
	offseted_data += shift_base * key_pointer_size;

	move_bytes = (c_keys - shift_base) * key_pointer_size;

	memmove((void*)(offseted_data + key_pointer_size),
			(const void*)offseted_data, move_bytes);

	c_pointers++;
	CALL_IB(IB_Write_CountPointers(file_desc_AM, block, c_pointers, &pointers_flag));
	if (pointers_flag != 1) {
		fprintf(stderr, "Failed to write updated count pointers in IB_Shift_Right().\n");
		return AME_ERROR;
	}

	*flag = 1;

	return AME_OK;
}

int IB_Insert(int file_desc_AM, BF_Block* block, int pointer1, void* key,
		int pointer2, int* flag)
{
	size_t n_pointers;  // Maximum number pointers an index block can store.
	size_t c_pointers;  // Current number of pointers in block.
	size_t c_keys;      // The number of keys in the index block.

	int key_length;     // The length of keys in bytes.

	void* current_key = NULL;  // Fetched key for comparing.

	size_t c_key;  // For iterating n-th key.

	int get_status;    // The status of the get key operation.
	int shift_status;  // The status of the shift keys operation.
	int write_status;  // The status of the write key operation.

	int cmp_flag;  // Flag for comparing the keys.

	if (block == NULL) return AME_ERROR;
	if (key == NULL) return AME_ERROR;

	CALL_IB(IB_Get_MaxCountPointers(file_desc_AM, &n_pointers));

	CALL_IB(IB_Get_CountPointers(block, &c_pointers));

	if (c_pointers == n_pointers) {
		*flag = 0;
		return AME_OK;
	}

	c_keys = c_pointers - 1;

	CALL_FD(FD_Get_attrLength1(file_desc_AM, &key_length));

	current_key = malloc(key_length);  // Memory for key.

	for (c_key = 0; c_key < c_keys; ++c_key) {
		CALL_IB(IB_Get_Key(file_desc_AM, block, current_key, c_key, &get_status));

		if (get_status == 1) {
			CALL_RD(RD_Key_cmp(file_desc_AM, key, current_key, &cmp_flag));

			if (cmp_flag == -1) {
				CALL_IB(IB_Shift_Right(file_desc_AM, block, c_key, &shift_status));
				if (shift_status != 1) {
					fprintf(stderr, "Failed to shift in IB_Shift_Right().\n");
					exit(1);  // Shifting should always succeed.
				}
				break;
			}
		} else {
			fprintf(stderr, "Failed to get key in IB_Insert().\n");
			exit(1);
		}
	}

	free(current_key);

	*flag = 1;

	/* Write the key and it's pointers at the calculated position. */
	CALL_IB(IB_Write_Key(file_desc_AM, block, pointer1, key, pointer2, c_key, &write_status));
	if (write_status != 1) {
		fprintf(stderr, "Failed to write key in IB_Insert().\n");
		exit(1);
	}

	return AME_OK;
}

int IB_Print(int file_desc_AM, BF_Block* block)
{
	size_t c_pointers;

	char key_type;
	int key_length;

	void* key;
	int pointer;

	int get_pointer_status;
	int get_key_status;

	if (block == NULL) return AME_ERROR;

	CALL_IB(IB_Get_CountPointers(block, &c_pointers));

	/*
	 * Index blocks cannot be empty.
	 * At least two pointers and a key will be present.
	 * However we check it.
	 */

	if (c_pointers == 0) {
		printf("Block is empty.\n");
		return AME_OK;
	}

	/* Get key type and length. */
	CALL_FD(FD_Get_attrType1(file_desc_AM, &key_type));
	CALL_FD(FD_Get_attrLength1(file_desc_AM, &key_length));

	/* Allocate memory for key. */
	key = malloc(key_length);

	/* Print the first pointer. */
	CALL_IB(IB_Get_Pointer(file_desc_AM, block, &pointer, 0, &get_pointer_status));
	if (get_pointer_status != 1) return AME_ERROR;
	printf("ptr: %d.\n", pointer);

	/* Print key pointer pairs. */
	for (size_t c_key = 0; c_key < c_pointers - 1; ++c_key) {
		CALL_IB(IB_Get_Key(file_desc_AM, block, key, c_key, &get_key_status));
		if (get_key_status != 1) return AME_ERROR;
		CALL_IB(IB_Get_Pointer(file_desc_AM, block, &pointer, c_key + 1, &get_pointer_status));
		if (get_pointer_status != 1) return AME_ERROR;
		if (key_type == INTEGER) printf("key: %d.\n", *(int*)key);
		if (key_type == FLOAT) printf("key: %f.\n", *(float*)key);
		if (key_type == STRING) printf("key: %s.\n", (char*)key);
		printf("ptr: %d.\n", pointer);
	}

	free(key);

	return AME_OK;
}

int IB_GetPointer(BF_Block *index_block, void* key, int* pointer, int indexDesc){
    char* data = NULL;
    if (index_block == NULL) return AME_ERROR;
    data = BF_Block_GetData(index_block);
    if (data == NULL) return AME_ERROR;
    int num_of_pointers;
	memcpy((void*)&num_of_pointers, (const void*)data, sizeof(int));
    // Skip the number of pointers metadata in index block
    // data points to the first pointer (P0)
    data+=sizeof(int);
    //temporaly hold the pointer
    memcpy((void*)pointer, (const void*)data, sizeof(int));
    int num_of_keys=num_of_pointers-1;
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
        memcpy((void*)&key_i, (const void*)data, sizeof(int));
        if (*(int*)key<key_i){
          return AME_OK;
        }
        data+=sizeof(int); //data points to second pointer (P1)
        memcpy((void*)pointer, (const void*)data, sizeof(int));
        data+=sizeof(int); //data points to second key (K2)
        for (int k=2; k<=num_of_keys; k++){
          memcpy((void*)&key_i, (const void*)data, sizeof(int));
          if (*(int*)key<key_i)
            return AME_OK;
          else{
            data+=sizeof(int); //data points to next pointer
            memcpy((void*)pointer, (const void*)data, sizeof(int));
            if (k==num_of_keys)
              return AME_OK;
            data+=sizeof(int); //data points to next key
          }
        }
    }

    if (Type1=='f'){
        float key_f;
        memcpy((void*)&key_f, (const void*)data, sizeof(float));
        if (*(float*)key<key_f){
          return AME_OK;
        }
        data+=sizeof(float); //data points to second pointer (P1)
        memcpy((void*)pointer, (const void*)data, sizeof(int));
        data+=sizeof(int); //data points to second key (K2)
        for (int k=2; k<=num_of_keys; k++){
          memcpy((void*)&key_f, (const void*)data, sizeof(float));
          if (*(float*)key<key_f)
            return AME_OK;
          else{
            data+=sizeof(float); //data points to next pointer
            memcpy((void*)pointer, (const void*)data, sizeof(int));
            if (k==num_of_keys)
              return AME_OK;
            data+=sizeof(int); //data points to next key
          }
        }
    }


    if (Type1=='c'){
        char* key_c = NULL;
        key_c = malloc(sizeof(char) * Lenght1);
        memcpy((void*)key_c, (const void*)data, sizeof(Lenght1));
        if (strcmp(key, key_c)<0){
          free(key_c);
          return AME_OK;
        }
        data+=sizeof(Lenght1); //data points to second pointer (P1)
        memcpy((void*)pointer, (const void*)data, sizeof(int));
        data+=sizeof(int); //data points to second key (K2)
        for (int k=2; k<=num_of_keys; k++){
          memcpy((void*)key_c, (const void*)data, sizeof(Lenght1));
          if (strcmp(key, key_c)<0) {
            free(key_c);
            return AME_OK;
          }
          else{
            data+=sizeof(Lenght1); //data points to next pointer
            memcpy((void*)pointer, (const void*)data, sizeof(int));
            if (k==num_of_keys) {
              free(key_c);
              return AME_OK;
            }
            data+=sizeof(int); //data points to next key
          }
        }
    }

    return AME_ERROR;

}
