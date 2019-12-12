/*******************************************************************************
 * File: RD.h
 * Purpose: API for interacting with records.
*******************************************************************************/

#ifndef RD_H
#define RD_H

#include <stdio.h>

#include "../accessmethod/AM.h"

int RD_Init(Record* record, void* fieldA, void* fieldB);

#endif  // #ifndef RD_H
