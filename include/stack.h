#ifndef STACK_H
#define STACK_H

#include "types.h"
#include "list.h"

/* Stack primitives */
TStack *stackCreate(const char *name, const char *def, Date dob, Date dod);
void    push(TStack **top, const char *name, const char *def, Date dob, Date dod);
TStack *pop(TStack **top);
bool    stackIsEmpty(TStack *top);
void    stackPrint(TStack *s);
void    stackFree(TStack *s);
int     stackSize(TStack *s);

/* Stack functions */
TStack *toStack(TBiList *merged);
TStack *getInfoPersonality(TStack *stk, const char *name);
TStack *sortNameStack(TStack *s);
TStack *deleteName(TStack *stk, const char *name);
TStack *updateStack(TStack *stk, const char *name,
                    const char *def, Date dob, Date dod);
TQueue *stackToQueue(TStack *stk);
TList  *stackToList(TStack *stk);
TStack *addNameStack(TStack *stk, const char *name,
                     const char *definition, Date dob, Date dod);
TStack *definitionStack(TStack *stk);
void    pronunciationStack(TStack *stk,
                           TStack **shortStk, TStack **longStk);
char   *getSmallest(TStack *stk);
void    continuousSearch(TStack *stk);
bool    isPersonalityKilled(TStack *stk, const char *name);
TStack *recRevStack(TStack *stk);

#endif
