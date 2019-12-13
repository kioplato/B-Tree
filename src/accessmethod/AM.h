#ifndef AM_H
#define AM_H

#include <stdio.h>

#include "../BF.h"

/* Error codes */
extern int AM_errno;

#define AME_OK -1             /* Successful execution. */
#define AME_BF_MAX_FILES -2   /* Maximum open files reached for BF. */
#define AME_MAX_FILES -3      /* Maximum open files reached for AM. */
#define AME_BF_INVALID_FD -4  /* Invalid file descriptor used in BF. */
#define AME_INVALID_FD -5     /* Invalid file descriptor used in AM. */
#define AME_BF_ACTIVE -6      /* BF layer is already initialized. */
#define AME_FILE_EXISTS -7    /* File already exists. */
#define AME_FULL_MEMORY -8    /* The memory is full with unpined blocks. */
#define AME_INVALID_BLOCK_ID -9  /* The requested block id does not exist in file. */
#define AME_BF_ERROR -10      /* Generic BF layer failure. */
#define AME_ERROR -11     /* Generic AM layer failure. */
#define AME_EOF -12       /* There are not any more records that match. */
#define AME_FD_INVALID_INDEX -13  /* Accessing empty index in FD array. */
#define AME_DELETE_OPEN_FILE -14  /* Deleting an open file. */
#define AME_FD_CLOSE_OPEN_SCAN -15  /* Closing a file with an open scan. */

// Convert BF error codes to AME error codes.
int convert(BF_ErrorCode code);
/***************/

#define EQUAL 1
#define NOT_EQUAL 2
#define LESS_THAN 3
#define GREATER_THAN 4
#define LESS_THAN_OR_EQUAL 5
#define GREATER_THAN_OR_EQUAL 6

typedef struct Record {
	void* fieldA;
	int lengthA;
	void* fieldB;
	int lengthB;
} Record;

int AM_Init(void);

int AM_CreateIndex(
  char *fileName,   /* όνομα αρχείου */
  char attrType1,   /* τύπος πρώτου πεδίου: 'c' (συμβολοσειρά), 'i' (ακέραιος), 'f' (πραγματικός) */
  int attrLength1,  /* μήκος πρώτου πεδίου: 4 γιά 'i' ή 'f', 1-255 γιά 'c' */
  char attrType2,   /* τύπος πρώτου πεδίου: 'c' (συμβολοσειρά), 'i' (ακέραιος), 'f' (πραγματικός) */
  int attrLength2   /* μήκος δεύτερου πεδίου: 4 γιά 'i' ή 'f', 1-255 γιά 'c' */
);

int AM_DestroyIndex(
  char *fileName  /* όνομα αρχείου */
);

int AM_OpenIndex(
  char *fileName  /* όνομα αρχείου */
);

int AM_CloseIndex(
  int fileDesc  /* αριθμός που αντιστοιχεί στο ανοιχτό αρχείο */
);

int AM_InsertEntry(
  int fileDesc,  /* αριθμός που αντιστοιχεί στο ανοιχτό αρχείο */
  void *value1,  /* τιμή του πεδίου-κλειδιού προς εισαγωγή */
  void *value2   /* τιμή του δεύτερου πεδίου της εγγραφής προς εισαγωγή */
);

int AM_OpenIndexScan(
  int fileDesc,  /* αριθμός που αντιστοιχεί στο ανοιχτό αρχείο */
  int op,        /* τελεστής σύγκρισης */
  void *value    /* τιμή του πεδίου-κλειδιού προς σύγκριση */
);

/*
 * On failure returns NULL and sets AM_errno accordingly.
 */
void *AM_FindNextEntry(
  int scanDesc  /* αριθμός που αντιστοιχεί στην ανοιχτή σάρωση */
);

int AM_CloseIndexScan(
  int scanDesc /* αριθμός που αντιστοιχεί στην ανοιχτή σάρωση */
);

void AM_PrintError(
  char *errString /* κείμενο για εκτύπωση */
);

void AM_Close();

#endif /* #ifndef AM_H */
