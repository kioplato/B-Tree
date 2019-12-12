#include <stdio.h>

#include "../include/AM.h"
#include "../include/bf.h"

#include "MT.h"
#include "BL.h"
#include "FD.h"

int AM_errno = AME_OK;

void AM_Init() {
	/* Initialize global array. */
	CALL_FD(FD_Init());

	/* Initialize scan array. */
	// TODO.

	/* Initialize BF layer. */
	CALL_BF(BF_Init(LRU));

	return;
}

/*
 * Creates an index file with B+-tree.
 * Starts with a single empty B+-tree root block and the file's metadata block.
 */
int AM_CreateIndex(char *fileName, char attrType1, int attrLength1,
	               char attrType2, int attrLength2)
{
	int file_desc;  // The file descriptor of the newly created file.
	BF_Block *metablock;  // The metadata block.
	BF_Block *rootblock;  // The root of the index.

	if (fileName == NULL)
		return AME_ERROR;

	/* Create and open the file. */
	CALL_BF(BF_CreateFile(fileName));
	CALL_BF(BF_OpenFile(fileName, &file_desc));

	/* Create the metadata block. */
	CALL_BL(BL_CreateBlock(file_desc, &metablock));
	CALL_MT(MT_InitMetadataBlock(attrType1, attrLength1, attrType2, attrLength2));

	/* Close off metadata block. */
	BF_Block_SetDirty(metablock);
	CALL_BF(BF_UnpinBlock(metablock));
	BF_Block_Destroy(&metablock);

	return AME_OK;
}

/*
 * Removes the file with name fileName from the disk.
 *
 * Returns AME_OK on success.
 * Returns an error code on failure.
 */
int AM_DestroyIndex(char *fileName)
{
	int is_open;  // Flag about if the fileName is open.

	if (fileName == NULL)
		return AME_ERROR;

	is_open = -1;
	CALL_FD(FD_IsOpen(fileName, &is_open));
	if (is_open == 1)
		return AME_FD_DELETE_OPEN_FILE;

	if (remove(fileName) == -1) {
		perror("remove() failed");
		return AME_ERROR;
	}

	return AME_OK;
}

/*
 * Opens the file, caches the file's metadata.
 *
 * Returns the AM file descriptor for future access.
 */
int AM_OpenIndex(char *fileName) {
	int BF_file_desc;  // The file descriptor as returned from BF layer.
	int FD_file_desc;  // The file descriptor as returned from AM layer, FD API.
	BF_Block* metablock;  // The metadata block.
	char attrType1, attrType2;  // The metadata block's data.
	int attrLength1, attrLength2;
	size_t root_index_block;
	
	if (fileName == NULL) return AME_ERROR;

	CALL_BF(BF_OpenFile(fileName, &BF_file_desc));

	/* Get metadata block's stored data for caching. */
	metablock = NULL;
	BF_Block_Init(&metablock);
	CALL_BF(BF_GetBlock(BF_file_desc, 0, metablock));

	CALL_MT(MT_GetData(metablock, &attrType1, &attrLength1, &attrType2,
				&attrLength2, &root_index_block));

	/* Cache the metadata block's data. */
	CALL_FD(FD_Insert(BF_file_desc, &FD_file_desc, fileName, attrType1,
				attrLength1, attrType2, attrLength2, root_index_block));

	/* Close off the metadata block. */
	CALL_BF(BF_UnpinBlock(metablock));
	BF_Block_Destroy(&metablock);

	return FD_file_desc;
}


int AM_CloseIndex(int fileDesc) {

	return AME_OK;
}


int AM_InsertEntry(int fileDesc, void *value1, void *value2) {
	return AME_OK;
}


int AM_OpenIndexScan(int indexDesc, int op, void *value) {
	int index_root;
	CALL_FD(FD_Get_IndexRoot(int indexDesc, size_t *index_root));
	//αν ειχαμε συνεννοηθει σωστα, τωρα θα εχω στο index_root θα εχω είτε το root, δηλαδη το id ενος index block
	//είτε το id ενος data block, δηλαδη του πρώτου και μοναδικου data block.
	
	//check the type of block
	BF_Block* root_block=NULL;
	BF_Block_Init(&root_block);
	if (root_block==NULL) return AME_ERROR;
	
	CALL_FD(FD_Get_filedesc(indexDesc, &filedesc));
	

	CALL_BF(BF_GetBlock( filedesc, index_root, root_block)
	int flag;
	CALL_DB(DB_Is_DataBlock(block, &flag));
	if (flag==1)
		int scan_index;
		CALL_IS(IS_Insert(next=1, last_block=index_root, op, indexDesc, void* value, &scan_index))
		return scan_index;
	else //it was index block so we must go down the tree depending on the op
		if (op==EQUAL) //για ισοτητα θελω τον αριστερο pointer ενος key για το οποιο value<key.
			while (flag==0)
				int i=0;
				while (1)
					int pointer, key
					BT_Get_Pair(i, &pointer, &key)
					
					
		
	
	
	 
	

	IS_Insert(int next=1, int last_block, int op, int indexDesc, void* value, *scan_index)
	
	return AME_OK;
}


void* AM_FindNextEntry(int scanDesc) {
	int last_block;
	int next;
	int op;
	int index_desc;
	int file_desc;
	CALL_IS(IS_Get_next(scanDesc, &next));
	CALL_IS(IS_Get_last_block(scanDesc, &last_block));
	CALL_IS(IS_Get_op( scanDesc, &op));
	CALL_IS(IS_Get_index_desc(scanDesc, &index_desc));
	CALL_FD(FD_Get_filedesc(index_desc, &file_desc));

	head_bytes=....; //the header bytes of data block ("DBL", c_records, next_block)
	BF_Block* data_block;
	BF_Block_Init(&data_block); 
	CALL_BF(BF_GetBlock(file_desc, last_block,data_block));
	
	
	
	
	
	TODO:c_records = DBL_Get_c_records(data_block); //the numbers of records that last block holds

								 

	
	
	if (op==EQUALS)
		while (next<=c_records) //while kai oxi if wste na ftasei sto prwto equal
			
			if value==DBL_Get_value1(data_block,next)
				if next==c_records
					AM_errno=AME_EOF
					TODO: return DBL_Get_value2(data_block, next)
				else
					new_next=next+1;
					IS_Set_next(int scanDesc, size_t new_next) //update next
					TODO: return DBL_Get_value2(data_block, next) //auti i sinartisi tha prepei na 						xrhsimopoihsei tis FD fun wste na kserei ta attrlenght ktl
			else
				next=next+1;
		
		AM_errno=AME_EOF
		return NULL;

				
	else if (op==NOT EQUAL) //arxizw apo terma aristera toy dentroy, print mexri na vrw ta isa.
				//otan vrw ta isa tote ta prospernaw mexri na vrw ksana ena not equal (an uparxei ki allo)
		while (next<=c_records)
			
			if value!=DBL_Get_value1( data_block, next)
				if next<c_records
					new_next=next+1
				else
					int DBL_GetNextBlock(last_block)
					IS_Set_last_block(int scanDesc, char last_block) //update new last_block
					new_next=1
				IS_Set_next(int scanDesc, size_t new_next) //update next
				return DBL_Get_value2(data_block, next)
			else
				if next<c_records
					next=next+1;
				else if next=c_records //an uparxei epomeno block tote sigoyra tha exei not equal tutple
					if (next==max_records) //ean eftase diladi sto telefteo tuple enos gematou data block 
						DBL_GetNextBlock(last_block) //edw nomizw prepei na elegxw ti tha epistrepsei
									    //wste an einai na thesw analoga to AM_errno
						if last_block=0
							AM_errno=AME_EOF
							return NULL
						IS_Set_last_block(int scanDesc, char last_block) //update new last_block
						c_records = DBL_Get_c_records(data_block);
						next=1
					
						
	else if (op==LESS THAN)
		if attrType=int or float (use operator <)
			if next<=c_records
				if DBL_Get_value1( data_block, next)<value
					if next=c_records
						DBL_GetNextBlock(last_block)
						if last_block=0
							//ερωτημα: τι να βαλω στο next ωστε στην επομενη findEntry να βγαλει EOF?
							//θα μπορουσα να βαλω μια τιμη πχ next=0
							//και να βαλω εδω μεσα εαν ελεγχο if next=0 AM_errno=EOF return NULL
						

						
				else
					AM_errno=AME_EOF
					return NULL
					
				
		if attrType=char (same but use cmp)

}


int AM_CloseIndexScan(int scanDesc) {
	CALL_IS(IS_Close(scanDesc));
	return AME_OK;
}


void AM_PrintError(char *errString) {
	printf("%s\n", errString);
	if (AM_errno==-1)
		char *error_text="AME_OK";
	else if (AM_errno==-2)
		char *error_text="AME_BF_MAX_FILES";
	else if (AM_errno==-3)
		char *error_text="AME_MAX_FILES";
	else if (AM_errno==-4)
		char *error_text="AME_BF_INVALID_FD";
	else if (AM_errno==-5)
		char *error_text="AME_INVALID_FD";
	else if (AM_errno==-6)
		char *error_text="AME_BF_ACTIVE";
	else if (AM_errno==-7)
		char *error_text="AME_FILE_EXISTS";
	else if (AM_errno==-8)
		char *error_text="AME_FULL_MEMORY";
	else if (AM_errno==-9)
		char *error_text="AME_INVALID_BLOCK_ID";
	else if (AM_errno==-10)
		char *error_text="AME_BF_ERROR";
	else if (AM_errno==-11)
		char *error_text="AME_ERROR";
	else if (AM_errno==-12)
		char *error_text="AME_EOF";
	else if (AM_errno==-13)
		char *error_text="AME_FD_INVALID_INDEX";
	else if (AM_errno==-14)
		char *error_text="AME_FD_DELETE_OPEN_FILE";
	

	printf("%s\n", error_text);
}

void AM_Close() {

}
