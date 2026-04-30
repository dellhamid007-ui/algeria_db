#ifndef RECURSION_H
#define RECURSION_H

#include "types.h"
#include "file_parser.h"

int   countOccurrence(const char *filename, const char *name);
bool  removeOccurrence(const char *filename, const char *word);
bool  replaceOccurrence(const char *filename, const char *name,
                        Date dob, Date dod);
void  namePermutation(char *name);
void  subseqName(const char *word);
void  longestSubyear(TListEvent *events, Date date1, Date date2);
int   distinctSubseqWord(const char *event);
bool  isPalindromeWord(const char *event);

#endif
