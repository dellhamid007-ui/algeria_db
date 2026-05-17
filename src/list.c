#include "list.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static int compare_age(const void *a, const void *b) {
    typedef struct { char name[MAX_NAME]; char definition[MAX_DEF]; int age; } PersonItem;
    const PersonItem *pa = (const PersonItem *)a;
    const PersonItem *pb = (const PersonItem *)b;
    return pa->age - pb->age;
}

static int compare_word_count(const void *a, const void *b) {
    typedef struct { TList *node; int word_count; } NameItem;
    const NameItem *na = (const NameItem *)a;
    const NameItem *nb = (const NameItem *)b;
    return na->word_count - nb->word_count;
}

static int compare_age_value(const void *a, const void *b) {
    typedef struct { TListDate *node; int age_value; } AgeItem;
    const AgeItem *aa = (const AgeItem *)a;
    const AgeItem *ab = (const AgeItem *)b;
    return aa->age_value - ab->age_value;
}

/* --- TList primitives --- */

TList *listCreate(const char *name, const char *def) {
    TList *node = (TList *)malloc(sizeof(TList));
    if (!node) return NULL;
    strncpy(node->name,       name ? name : "", MAX_NAME - 1);
    strncpy(node->definition, def  ? def  : "", MAX_DEF  - 1);
    node->name[MAX_NAME-1] = '\0';
    node->definition[MAX_DEF-1] = '\0';
    node->next = NULL;
    return node;
}

void listPrint(TList *s) {
    for (TList *cur = s; cur; cur = cur->next)
        printf("  [%s] -> %s\n", cur->name, cur->definition);
}

void listFree(TList *s) {
    while (s) { TList *t = s->next; free(s); s = t; }
}

int listSize(TList *s) {
    int c = 0;
    for (TList *cur = s; cur; cur = cur->next) c++;
    return c;
}

TList *getPersonality(const char *filename) {
    return loadPersonalities(filename);
}

TListDate *getDatePersonality(const char *filename) {
    return loadPersonalityDates(filename);
}

void getInfoByDates(TList *s, TListDate *DoB, Date target) {
    TListDate *d = DoB;
    TList     *p = s;
    int found = 0;
    while (p && d) {
        if (dateCmp(d->dob, target) == 0) {
            printf("Name: %s\n  Definition: %s\n  DoB: ", p->name, p->definition);
            char buf[MAX_DATE]; printf("%s\n", dateToString(d->dob, buf));
            found = 1;
        }
        p = p->next; d = d->next;
    }
    if (!found) printf("No personality found with that date of birth.\n");
}

void getInfoByDates2(TList *s, TListDate *DoD, Date target) {
    TListDate *d = DoD;
    TList     *p = s;
    int found = 0;
    while (p && d) {
        if (dateCmp(d->dod, target) == 0) {
            printf("Name: %s\n  Definition: %s\n  DoD: ", p->name, p->definition);
            char buf[MAX_DATE]; printf("%s\n", dateToString(d->dod, buf));
            found = 1;
        }
        p = p->next; d = d->next;
    }
    if (!found) printf("No personality found with that date of death.\n");
}

static TList *insertSorted(TList *sorted, TList *node) {
    node->next = NULL;
    if (!sorted || strcasecmp(node->name, sorted->name) <= 0) {
        node->next = sorted;
        return node;
    }
    TList *cur = sorted;
    while (cur->next && strcasecmp(node->name, cur->next->name) > 0)
        cur = cur->next;
    node->next = cur->next;
    cur->next  = node;
    return sorted;
}

TList *sortWord(TList *syn) {
    TList *sorted = NULL;
    for (TList *cur = syn; cur; cur = cur->next) {
        TList *node = listCreate(cur->name, cur->definition);
        sorted = insertSorted(sorted, node);
    }
    return sorted;
}

TList *sortWord2(TList *syn) {
    TList *sorted = NULL;
    for (TList *cur = syn; cur; cur = cur->next) {
        TList *node = listCreate(cur->name, cur->definition);
        node->next = NULL;
        if (!sorted || strlen(node->name) <= strlen(sorted->name)) {
            node->next = sorted; sorted = node;
        } else {
            TList *p = sorted;
            while (p->next && strlen(node->name) > strlen(p->next->name))
                p = p->next;
            node->next = p->next;
            p->next    = node;
        }
    }
    return sorted;
}

TList *sortPersonality(TList *s, TListDate *dates) {
    if (!s || !dates) return NULL;

    int count = 0;
    TList *person = s; TListDate *date = dates;
    while (person && date) { count++; person = person->next; date = date->next; }

    typedef struct { char name[MAX_NAME]; char definition[MAX_DEF]; int age; } PersonItem;
    PersonItem *person_items = (PersonItem *)malloc(count * sizeof(PersonItem));
    if (!person_items) return NULL;

    person = s; date = dates;
    for (int i = 0; i < count; i++) {
        strncpy(person_items[i].name, person->name, MAX_NAME - 1);
        strncpy(person_items[i].definition, person->definition, MAX_DEF - 1);
        person_items[i].name[MAX_NAME-1] = '\0';
        person_items[i].definition[MAX_DEF-1] = '\0';
        person_items[i].age = ageFromDates(date->dob, date->dod);
        person = person->next; date = date->next;
    }

    qsort(person_items, count, sizeof(PersonItem), compare_age);

    TList *sorted_list = NULL, *tail = NULL;
    for (int i = 0; i < count; i++) {
        TList *node = listCreate(person_items[i].name, person_items[i].definition);
        if (!sorted_list) { sorted_list = tail = node; }
        else         { tail->next = node; tail = node; }
    }
    free(person_items);
    return sorted_list;
}

bool deletepersonality(const char *filename,
                       TList **s, TListDate **a,
                       const char *name) {
    TList *prev = NULL, *cur = *s;
    while (cur && strcasecmp(cur->name, name) != 0) { prev = cur; cur = cur->next; }
    if (!cur) { printf("Personality '%s' not found.\n", name); return false; }
    if (prev) prev->next = cur->next; else *s = cur->next;
    free(cur);

    TListDate *dprev = NULL, *dcur = *a;
    while (dcur && strcasecmp(dcur->name, name) != 0) { dprev = dcur; dcur = dcur->next; }
    if (dcur) {
        if (dprev) dprev->next = dcur->next; else *a = dcur->next;
        free(dcur);
    }

    TListEvent *events = loadEvents(filename);
    rewriteFile(filename, *s, *a, events);
    listEventFree(events);
    return true;
}

bool updatePersonality(const char *filename,
                       TList **s, TListDate **a,
                       const char *name,
                       const char *definition,
                       Date dob, Date dod) {
    TList *p = *s;
    while (p && strcasecmp(p->name, name) != 0) p = p->next;
    if (!p) { printf("Personality '%s' not found.\n", name); return false; }
    if (definition && definition[0])
        strncpy(p->definition, definition, MAX_DEF - 1);

    TListDate *d = *a;
    while (d && strcasecmp(d->name, name) != 0) d = d->next;
    if (d) {
        if (!dateIsNull(dob)) d->dob = dob;
        if (!dateIsNull(dod)) d->dod = dod;
    }

    TListEvent *events = loadEvents(filename);
    rewriteFile(filename, *s, *a, events);
    listEventFree(events);
    return true;
}

bool addPersonality(const char *filename,
                    TList **s, TListDate **a,
                    const char *name,
                    Date dob, Date dod,
                    const char *definition) {
    TList *node = listCreate(name, definition ? definition : "");
    node->next = NULL;
    if (!*s) { *s = node; }
    else {
        TList *t = *s; while (t->next) t = t->next;
        t->next = node;
    }

    TListDate *dnode = listDateCreate(name, dob, dod);
    dnode->next = NULL;
    if (!*a) { *a = dnode; }
    else {
        TListDate *t = *a; while (t->next) t = t->next;
        t->next = dnode;
    }

    TListEvent *events = loadEvents(filename);
    rewriteFile(filename, *s, *a, events);
    listEventFree(events);
    return true;
}

TList *similarPersonality(TList *s, const char *word) {
    int target_year = atoi(word);
    TList *result = NULL, *tail = NULL;

    for (TList *cur = s; cur; cur = cur->next) {
        char def_lower[MAX_DEF];
        strToLower(def_lower, cur->definition);
        char word_lower[MAX_NAME];
        strToLower(word_lower, word);
        if (strstr(def_lower, word_lower)) {
            TList *node = listCreate(cur->name, cur->definition);
            if (!result) { result = tail = node; }
            else         { tail->next = node; tail = node; }
        }
    }
    (void)target_year;
    return result;
}

TList *countPersonality(TList *s, Date prt) {
    char year_str[16];
    sprintf(year_str, "%d", prt.year);
    return similarPersonality(s, year_str);
}

TList *palindromeName(TList *s) {
    TList *result = NULL;
    for (TList *cur = s; cur; cur = cur->next) {
        char def_copy[MAX_DEF];
        strncpy(def_copy, cur->definition, MAX_DEF - 1);
        def_copy[MAX_DEF-1] = '\0';

        char *tok = strtok(def_copy, " ,;.-()");
        while (tok) {
            if (strlen(tok) > 1 && isPalindrome(tok)) {
                bool dup = false;
                for (TList *r = result; r; r = r->next)
                    if (strcasecmp(r->name, tok) == 0) { dup = true; break; }
                if (!dup) {
                    TList *node = listCreate(tok, "palindrome");
                    result = insertSorted(result, node);
                }
            }
            tok = strtok(NULL, " ,;.-()");
        }
        if (isPalindrome(cur->name)) {
            bool dup = false;
            for (TList *r = result; r; r = r->next)
                if (strcasecmp(r->name, cur->name) == 0) { dup = true; break; }
            if (!dup) {
                TList *node = listCreate(cur->name, "palindrome name");
                result = insertSorted(result, node);
            }
        }
    }
    return result;
}

/* --- TListDate primitives --- */

TListDate *listDateCreate(const char *name, Date dob, Date dod) {
    TListDate *node = (TListDate *)malloc(sizeof(TListDate));
    if (!node) return NULL;
    strncpy(node->name, name ? name : "", MAX_NAME - 1);
    node->name[MAX_NAME-1] = '\0';
    node->dob  = dob;
    node->dod  = dod;
    node->next = NULL;
    return node;
}

void listDatePrint(TListDate *s) {
    char dob_buf[MAX_DATE], dod_buf[MAX_DATE];
    for (TListDate *cur = s; cur; cur = cur->next)
        printf("  [%s]  DoB: %s  DoD: %s\n",
               cur->name,
               dateToString(cur->dob, dob_buf),
               dateToString(cur->dod, dod_buf));
}

void listDateFree(TListDate *s) {
    while (s) { TListDate *t = s->next; free(s); s = t; }
}

/* --- TListEvent primitives --- */

TListEvent *listEventCreate(const char *event_name, Date date) {
    TListEvent *node = (TListEvent *)malloc(sizeof(TListEvent));
    if (!node) return NULL;
    strncpy(node->event_name, event_name ? event_name : "", MAX_NAME - 1);
    node->event_name[MAX_NAME-1] = '\0';
    node->date = date;
    node->next = NULL;
    return node;
}

void listEventPrint(TListEvent *b) {
    char buf[MAX_DATE];
    for (TListEvent *cur = b; cur; cur = cur->next)
        printf("  [%s] : %s\n", cur->event_name, dateToString(cur->date, buf));
}

void listEventFree(TListEvent *b) {
    while (b) { TListEvent *t = b->next; free(b); b = t; }
}

bool addEvents(const char *filename,
               TListEvent **b,
               const char *event_name,
               Date date) {
    TListEvent *node = listEventCreate(event_name, date);
    if (!*b) { *b = node; }
    else {
        TListEvent *t = *b; while (t->next) t = t->next;
        t->next = node;
    }
    TList     *pers  = loadPersonalities(filename);
    TListDate *dates = loadPersonalityDates(filename);
    rewriteFile(filename, pers, dates, *b);
    listFree(pers);
    listDateFree(dates);
    return true;
}

/* --- Bidirectional list --- */

TBiList *mergeNodes(TList *s, TListDate *a) {
    TBiList *head = NULL, *tail = NULL;
    TList    *p = s;
    TListDate *d = a;
    while (p && d) {
        TBiList *node = (TBiList *)malloc(sizeof(TBiList));
        if (!node) break;
        strncpy(node->name,       p->name,       MAX_NAME - 1);
        strncpy(node->definition, p->definition, MAX_DEF  - 1);
        node->name[MAX_NAME-1]       = '\0';
        node->definition[MAX_DEF-1] = '\0';
        node->dob  = d->dob;
        node->dod  = d->dod;
        node->prev = tail;
        node->next = NULL;
        if (tail) tail->next = node;
        else      head       = node;
        tail = node;
        p = p->next; d = d->next;
    }
    return head;
}

void biListPrint(TBiList *h) {
    char dob_buf[MAX_DATE], dod_buf[MAX_DATE];
    for (TBiList *cur = h; cur; cur = cur->next)
        printf("  [%s]  %s | DoB: %s  DoD: %s\n",
               cur->name, cur->definition,
               dateToString(cur->dob, dob_buf),
               dateToString(cur->dod, dod_buf));
}

void biListFree(TBiList *h) {
    while (h) { TBiList *t = h->next; free(h); h = t; }
}

/* --- Circular list --- */

TCircList *merge2Nodes(TList *s, TListDate *a) {
    TCircList *head = NULL, *tail = NULL;
    TList    *p = s;
    TListDate *d = a;
    while (p && d) {
        TCircList *node = (TCircList *)malloc(sizeof(TCircList));
        if (!node) break;
        strncpy(node->name,       p->name,       MAX_NAME - 1);
        strncpy(node->definition, p->definition, MAX_DEF  - 1);
        node->name[MAX_NAME-1]       = '\0';
        node->definition[MAX_DEF-1] = '\0';
        node->dob  = d->dob;
        node->dod  = d->dod;
        node->next = NULL;
        if (!head) { head = tail = node; node->next = head; }
        else       { tail->next = node; node->next = head; tail = node; }
        p = p->next; d = d->next;
    }
    return head;
}

void circListPrint(TCircList *h) {
    if (!h) return;
    char dob_buf[MAX_DATE], dod_buf[MAX_DATE];
    TCircList *cur = h;
    do {
        printf("  [%s]  %s | DoB: %s  DoD: %s\n",
               cur->name, cur->definition,
               dateToString(cur->dob, dob_buf),
               dateToString(cur->dod, dod_buf));
        cur = cur->next;
    } while (cur != h);
}

void circListFree(TCircList *h) {
    if (!h) return;
    TCircList *cur = h->next;
    while (cur != h) {
        TCircList *t = cur->next;
        free(cur);
        cur = t;
    }
    free(h);
}

/* --- Queue --- */

TQueue *queueCreate(void) {
    TQueue *q = (TQueue *)malloc(sizeof(TQueue));
    if (!q) return NULL;
    q->front = q->rear = NULL;
    q->size  = 0;
    return q;
}

void enqueue(TQueue *q, const char *name, const char *def, Date dob, Date dod) {
    if (!q) return;
    TQNode *node = (TQNode *)malloc(sizeof(TQNode));
    if (!node) return;
    strncpy(node->name,       name ? name : "", MAX_NAME - 1);
    strncpy(node->definition, def  ? def  : "", MAX_DEF  - 1);
    node->name[MAX_NAME-1]       = '\0';
    node->definition[MAX_DEF-1] = '\0';
    node->dob  = dob;
    node->dod  = dod;
    node->next = NULL;
    if (!q->rear) { q->front = q->rear = node; }
    else          { q->rear->next = node; q->rear = node; }
    q->size++;
}

TQNode *dequeue(TQueue *q) {
    if (!q || !q->front) return NULL;
    TQNode *node = q->front;
    q->front = q->front->next;
    if (!q->front) q->rear = NULL;
    q->size--;
    node->next = NULL;
    return node;
}

bool queueIsEmpty(TQueue *q) {
    return !q || q->front == NULL;
}

void queuePrint(TQueue *q) {
    if (!q) return;
    char dob_buf[MAX_DATE], dod_buf[MAX_DATE];
    for (TQNode *cur = q->front; cur; cur = cur->next)
        printf("  [%s] | DoB: %s  DoD: %s\n",
               cur->name,
               dateToString(cur->dob, dob_buf),
               dateToString(cur->dod, dod_buf));
}

void queueFree(TQueue *q) {
    if (!q) return;
    TQNode *cur = q->front;
    while (cur) { TQNode *t = cur->next; free(cur); cur = t; }
    free(q);
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

TQueue *sName(TList *s) {
    TQueue *q = queueCreate();
    int n = listSize(s);
    if (n == 0) return q;

    typedef struct { TList *node; int word_count; } NameItem;
    NameItem *name_items = (NameItem *)malloc(n * sizeof(NameItem));
    if (!name_items) return q;

    int index = 0;
    for (TList *current = s; current; current = current->next) {
        name_items[index].node = current;
        name_items[index].word_count = wordCount(current->name);
        index++;
    }

    qsort(name_items, n, sizeof(NameItem), compare_word_count);

    Date null_date = {0,0,0};
    for (int j = 0; j < n; j++)
        enqueue(q, name_items[j].node->name, name_items[j].node->definition, null_date, null_date);
    free(name_items);
    return q;
}

TQueue *ageP(TListDate *a) {
    int n = 0;
    for (TListDate *cur = a; cur; cur = cur->next) n++;

    TQueue *q = queueCreate();
    if (n == 0) return q;

    typedef struct { TListDate *node; int age_value; } AgeItem;
    AgeItem *age_items = (AgeItem *)malloc(n * sizeof(AgeItem));
    if (!age_items) return q;

    int i = 0;
    for (TListDate *cur = a; cur; cur = cur->next) {
        age_items[i].node = cur;
        age_items[i].age_value = ageFromDates(cur->dob, cur->dod);
        i++;
    }

    qsort(age_items, n, sizeof(AgeItem), compare_age_value);

    for (int j = 0; j < n; j++)
        enqueue(q, age_items[j].node->name, "", age_items[j].node->dob, age_items[j].node->dod);
    free(age_items);
    return q;
}

TQueue *toQueue(TBiList *merged) {
    TQueue *q = queueCreate();
    for (TBiList *cur = merged; cur; cur = cur->next)
        enqueue(q, cur->name, cur->definition, cur->dob, cur->dod);
    return q;
}
