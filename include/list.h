#ifndef LIST_H
#define LIST_H

#include "types.h"
#include "file_parser.h"

/* TList primitives */
TList *listCreate(const char *name, const char *def);
void   listPrint(TList *s);
void   listFree(TList *s);
int    listSize(TList *s);

/* Personality/definition list functions */
TList *getPersonality(const char *filename);
TList *sortWord(TList *syn);
TList *sortWord2(TList *syn);
TList *palindromeName(TList *s);
TList *similarPersonality(TList *s, const char *word);
TList *countPersonality(TList *s, Date prt);

/* TListDate primitives */
TListDate *listDateCreate(const char *name, Date dob, Date dod);
void       listDatePrint(TListDate *s);
void       listDateFree(TListDate *s);

TListDate *getDatePersonality(const char *filename);
void       getInfoByDates(TList *s, TListDate *DoB, Date target);
void       getInfoByDates2(TList *s, TListDate *DoD, Date target);
TList     *sortPersonality(TList *s, TListDate *dates);

/* CRUD (operates on both lists + file) */
bool deletepersonality(const char *filename,
                       TList **s, TListDate **a,
                       const char *name);

bool updatePersonality(const char *filename,
                       TList **s, TListDate **a,
                       const char *name,
                       const char *definition,
                       Date dob, Date dod);

bool addPersonality(const char *filename,
                    TList **s, TListDate **a,
                    const char *name,
                    Date dob, Date dod,
                    const char *definition);

/* TListEvent primitives */
TListEvent *listEventCreate(const char *event_name, Date date);
void        listEventPrint(TListEvent *b);
void        listEventFree(TListEvent *b);

bool addEvents(const char *filename,
               TListEvent **b,
               const char *event_name,
               Date date);

/* Bidirectional list */
TBiList *mergeNodes(TList *s, TListDate *a);
void     biListPrint(TBiList *h);
void     biListFree(TBiList *h);

/* Circular list */
TCircList *merge2Nodes(TList *s, TListDate *a);
void       circListPrint(TCircList *h);
void       circListFree(TCircList *h);

/* Queue */
TQueue *queueCreate(void);
void    enqueue(TQueue *q, const char *name, const char *def, Date dob, Date dod);
TQNode *dequeue(TQueue *q);
bool    queueIsEmpty(TQueue *q);
void    queuePrint(TQueue *q);
void    queueFree(TQueue *q);

TQueue *sName(TList *s);
TQueue *ageP(TListDate *a);
TQueue *toQueue(TBiList *merged);

#endif
