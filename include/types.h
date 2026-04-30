#ifndef TYPES_H
#define TYPES_H

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#define MAX_NAME       256
#define MAX_DEF        1024
#define MAX_DATE       32
#define MAX_LINE       2048

typedef struct {
    int year;
    int month;
    int day;
} Date;

/* Personality node – name + definition */
typedef struct TList {
    char          name[MAX_NAME];
    char          definition[MAX_DEF];
    struct TList *next;
} TList;

/* Personality dates node – name + DoB + DoD */
typedef struct TListDate {
    char           name[MAX_NAME];
    Date           dob;
    Date           dod;
    struct TListDate *next;
} TListDate;

/* Event node – event name + date */
typedef struct TListEvent {
    char              event_name[MAX_NAME];
    Date              date;
    struct TListEvent *next;
} TListEvent;

/* Bidirectional (doubly linked) merged node */
typedef struct TBiList {
    char           name[MAX_NAME];
    char           definition[MAX_DEF];
    Date           dob;
    Date           dod;
    struct TBiList *prev;
    struct TBiList *next;
} TBiList;

/* Circular linked list node */
typedef struct TCircList {
    char              name[MAX_NAME];
    char              definition[MAX_DEF];
    Date              dob;
    Date              dod;
    struct TCircList *next;
} TCircList;

/* Stack node */
typedef struct TStack {
    char           name[MAX_NAME];
    char           definition[MAX_DEF];
    Date           dob;
    Date           dod;
    struct TStack *next;
} TStack;

/* Queue node */
typedef struct TQNode {
    char            name[MAX_NAME];
    char            definition[MAX_DEF];
    Date            dob;
    Date            dod;
    struct TQNode  *next;
} TQNode;

typedef struct {
    TQNode *front;
    TQNode *rear;
    int     size;
} TQueue;

/* Binary Search Tree node (key = name, alphabetical) */
typedef struct TTree {
    char           name[MAX_NAME];
    char           definition[MAX_DEF];
    Date           dob;
    Date           dod;
    struct TTree  *left;
    struct TTree  *right;
} TTree;

/* Utility helpers (defined in utils.c) */
Date   parseDate(const char *str);
char  *dateToString(Date d, char *buf);
int    dateCmp(Date a, Date b);
int    dateIsNull(Date d);
int    ageFromDates(Date dob, Date dod);
void   strToLower(char *dst, const char *src);
int    isPalindrome(const char *str);
void   trimWhitespace(char *s);

#endif
