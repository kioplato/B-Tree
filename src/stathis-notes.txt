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
