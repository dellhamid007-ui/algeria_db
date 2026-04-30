#include "stack.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* --- Stack primitives --- */

TStack *stackCreate(const char *name, const char *def, Date dob, Date dod) {
    TStack *node = (TStack *)malloc(sizeof(TStack));
    if (!node) return NULL;
    strncpy(node->name,       name ? name : "", MAX_NAME - 1);
    strncpy(node->definition, def  ? def  : "", MAX_DEF  - 1);
    node->name[MAX_NAME-1]       = '\0';
    node->definition[MAX_DEF-1] = '\0';
    node->dob  = dob;
    node->dod  = dod;
    node->next = NULL;
    return node;
}

void push(TStack **top, const char *name, const char *def, Date dob, Date dod) {
    TStack *node = stackCreate(name, def, dob, dod);
    if (!node) return;
    node->next = *top;
    *top = node;
}

TStack *pop(TStack **top) {
    if (!*top) return NULL;
    TStack *node = *top;
    *top = (*top)->next;
    node->next = NULL;
    return node;
}

bool stackIsEmpty(TStack *top) { return top == NULL; }

void stackPrint(TStack *s) {
    char dob_buf[MAX_DATE], dod_buf[MAX_DATE];
    for (TStack *cur = s; cur; cur = cur->next)
        printf("  [%s]  %s | DoB: %s  DoD: %s\n",
               cur->name, cur->definition,
               dateToString(cur->dob, dob_buf),
               dateToString(cur->dod, dod_buf));
}

void stackFree(TStack *s) {
    while (s) { TStack *t = s->next; free(s); s = t; }
}

int stackSize(TStack *s) {
    int c = 0; for (TStack *cur = s; cur; cur = cur->next) c++; return c;
}

/* --- Stack functions --- */

TStack *toStack(TBiList *merged) {
    TStack *stk = NULL;
    for (TBiList *cur = merged; cur; cur = cur->next)
        push(&stk, cur->name, cur->definition, cur->dob, cur->dod);
    return stk;
}

TStack *getInfoPersonality(TStack *stk, const char *name) {
    for (TStack *cur = stk; cur; cur = cur->next)
        if (strcasecmp(cur->name, name) == 0) return cur;
    return NULL;
}

TStack *sortNameStack(TStack *s) {
    TStack *sorted = NULL;
    while (s) {
        TStack *node = pop(&s);
        if (!sorted || strcasecmp(node->name, sorted->name) <= 0) {
            node->next = sorted; sorted = node;
        } else {
            TStack *cur = sorted;
            while (cur->next && strcasecmp(node->name, cur->next->name) > 0)
                cur = cur->next;
            node->next = cur->next;
            cur->next  = node;
        }
    }
    return sorted;
}

TStack *deleteName(TStack *stk, const char *name) {
    TStack *prev = NULL, *cur = stk;
    while (cur && strcasecmp(cur->name, name) != 0) { prev = cur; cur = cur->next; }
    if (!cur) { printf("'%s' not found in stack.\n", name); return stk; }
    if (prev) prev->next = cur->next; else stk = cur->next;
    free(cur);
    return stk;
}

TStack *updateStack(TStack *stk, const char *name,
                    const char *def, Date dob, Date dod) {
    TStack *cur = stk;
    while (cur && strcasecmp(cur->name, name) != 0) cur = cur->next;
    if (!cur) { printf("'%s' not found in stack.\n", name); return stk; }
    if (def && def[0]) strncpy(cur->definition, def, MAX_DEF - 1);
    if (!dateIsNull(dob)) cur->dob = dob;
    if (!dateIsNull(dod)) cur->dod = dod;
    return stk;
}

TQueue *stackToQueue(TStack *stk) {
    TStack *copy = NULL;
    for (TStack *cur = stk; cur; cur = cur->next)
        push(&copy, cur->name, cur->definition, cur->dob, cur->dod);
    TStack *sorted = sortNameStack(copy);

    TQueue *q = queueCreate();
    while (sorted) {
        TStack *node = pop(&sorted);
        enqueue(q, node->name, node->definition, node->dob, node->dod);
        free(node);
    }
    return q;
}

TList *stackToList(TStack *stk) {
    TList *list = NULL, *tail = NULL;
    TStack *copy = NULL;
    for (TStack *cur = stk; cur; cur = cur->next)
        push(&copy, cur->name, cur->definition, cur->dob, cur->dod);
    TStack *sorted = sortNameStack(copy);

    while (sorted) {
        TStack *node = pop(&sorted);
        TList *lnode = listCreate(node->name, node->definition);
        if (!list) { list = tail = lnode; }
        else       { tail->next = lnode; tail = lnode; }
        free(node);
    }
    return list;
}

TStack *addNameStack(TStack *stk, const char *name,
                     const char *definition, Date dob, Date dod) {
    TStack *node = stackCreate(name, definition, dob, dod);
    if (!stk || strcasecmp(name, stk->name) <= 0) {
        node->next = stk; return node;
    }
    TStack *cur = stk;
    while (cur->next && strcasecmp(name, cur->next->name) > 0) cur = cur->next;
    node->next = cur->next;
    cur->next  = node;
    return stk;
}

static int wordCount(const char *s) {
    int count = 0, in_word = 0;
    while (*s) {
        if (!isspace((unsigned char)*s)) { if (!in_word) { count++; in_word = 1; } }
        else in_word = 0;
        s++;
    }
    return count;
}

TStack *definitionStack(TStack *stk) {
    int n = stackSize(stk);
    if (n == 0) return NULL;

    typedef struct { TStack *node; int wc; } Item;
    Item *items = (Item *)malloc(n * sizeof(Item));
    if (!items) return stk;

    int i = 0;
    for (TStack *cur = stk; cur; cur = cur->next) {
        items[i].node = cur;
        items[i].wc   = wordCount(cur->definition);
        i++;
    }
    for (int j = 1; j < n; j++) {
        Item key = items[j]; int k = j - 1;
        while (k >= 0 && items[k].wc > key.wc) { items[k+1] = items[k]; k--; }
        items[k+1] = key;
    }

    TStack *result = NULL;
    for (int j = n - 1; j >= 0; j--)
        push(&result, items[j].node->name, items[j].node->definition,
             items[j].node->dob, items[j].node->dod);
    free(items);
    return result;
}

void pronunciationStack(TStack *stk, TStack **shortStk, TStack **longStk) {
    *shortStk = NULL; *longStk = NULL;
    for (TStack *cur = stk; cur; cur = cur->next) {
        if (wordCount(cur->definition) <= 5)
            push(shortStk, cur->name, cur->definition, cur->dob, cur->dod);
        else
            push(longStk,  cur->name, cur->definition, cur->dob, cur->dod);
    }
}

char *getSmallest(TStack *stk) {
    if (!stk) return NULL;
    TStack *best = stk;
    for (TStack *cur = stk->next; cur; cur = cur->next)
        if (strlen(cur->definition) < strlen(best->definition)) best = cur;
    return best->definition;
}

void continuousSearch(TStack *stk) {
    int n = stackSize(stk);
    if (n < 2) { printf("Not enough entries to compare.\n"); return; }

    typedef struct { char name[MAX_NAME]; int year; } Item;
    Item *items = (Item *)malloc(n * sizeof(Item));
    if (!items) return;

    int i = 0;
    for (TStack *cur = stk; cur; cur = cur->next) {
        strncpy(items[i].name, cur->name, MAX_NAME - 1);
        items[i].name[MAX_NAME-1] = '\0';
        items[i].year = cur->dob.year ? cur->dob.year : cur->dod.year;
        i++;
    }
    for (int j = 1; j < n; j++) {
        Item key = items[j]; int k = j - 1;
        while (k >= 0 && items[k].year > key.year) { items[k+1] = items[k]; k--; }
        items[k+1] = key;
    }
    printf("Continuous/overlapping events:\n");
    for (int j = 0; j < n - 1; j++) {
        if (abs(items[j+1].year - items[j].year) <= 1 &&
            items[j].year != 0 && items[j+1].year != 0)
            printf("  '%s' (%d)  <->  '%s' (%d)\n",
                   items[j].name, items[j].year,
                   items[j+1].name, items[j+1].year);
    }
    free(items);
}

bool isPersonalityKilled(TStack *stk, const char *name) {
    TStack *node = getInfoPersonality(stk, name);
    if (!node) return false;
    char def_lower[MAX_DEF];
    strToLower(def_lower, node->definition);
    return strstr(def_lower, "killed")    != NULL ||
           strstr(def_lower, "martyred")  != NULL ||
           strstr(def_lower, "executed")  != NULL ||
           strstr(def_lower, "assassinated") != NULL;
}

static void pushBottom(TStack **stk, TStack *node) {
    if (!*stk) { node->next = NULL; *stk = node; return; }
    TStack *top = pop(stk);
    pushBottom(stk, node);
    top->next = *stk;
    *stk = top;
}

TStack *recRevStack(TStack *stk) {
    if (!stk || !stk->next) return stk;
    TStack *top = pop(&stk);
    stk = recRevStack(stk);
    pushBottom(&stk, top);
    return stk;
}
