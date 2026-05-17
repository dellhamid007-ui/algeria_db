#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <stdarg.h>

#include "raylib.h"
#include "raymath.h"
#include "types.h"
#include "file_parser.h"
#include "list.h"
#include "stack.h"
#include "tree.h"
#include "recursion.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

/* --- Layout --- */
#define WIN_W        1280
#define WIN_H        720
#define SIDEBAR_W    210
#define HEADER_H     54
#define TAB_H        38
#define INPUT_H      34
#define FIELD_GAP    6

/* Computed content region */
#define CX  (SIDEBAR_W + 12)
#define CY  (HEADER_H + TAB_H + 12)
#define CW  (WIN_W - SIDEBAR_W - 24)
#define CH  (WIN_H - CY - 12)

/* --- Algeria-themed palette --- */
#define COL_BG          ((Color){ 12, 12, 22, 255 })
#define COL_SIDEBAR     ((Color){ 16, 18, 30, 255 })
#define COL_HEADER      ((Color){ 16, 18, 30, 255 })
#define COL_CARD        ((Color){ 24, 28, 45, 255 })
#define COL_CARD_HOVER  ((Color){ 32, 38, 58, 255 })
#define COL_GREEN       ((Color){ 0, 120, 60, 255 })
#define COL_GREEN_DIM   ((Color){ 0, 70, 35, 255 })
#define COL_RED         ((Color){ 200, 30, 50, 255 })
#define COL_TEXT        ((Color){ 215, 218, 228, 255 })
#define COL_TEXT_DIM    ((Color){ 110, 115, 135, 255 })
#define COL_TAB         ((Color){ 22, 25, 40, 255 })
#define COL_TAB_ACT     ((Color){ 0, 100, 50, 220 })
#define COL_INPUT_BG    ((Color){ 14, 16, 26, 255 })
#define COL_ACCENT      ((Color){ 60, 210, 120, 255 })
#define COL_DIVIDER     ((Color){ 40, 44, 65, 255 })

/* --- Views --- */
typedef enum { V_LISTS, V_QUEUES, V_STACKS, V_BST, V_RECURSION, V_COUNT } View;

static const char *viewNames[] = {
    "Linked Lists", "Queues", "Stacks", "Binary Search Tree", "Recursion"
};
static const char *viewIcons[] = { "=", ">>", "#", "<>", "~" };

/* --- Tab definitions per view --- */
static const char *tabsLists[] = {
    "All", "Dates", "Events", "Sort A-Z", "Sort Len", "Sort Age",
    "Palindromes", "Similar", "Search DoB", "Search DoD",
    "BiList", "CircList", "Add", "Delete", "Update", NULL
};
static const char *tabsQueues[] = { "By Words", "By Age", "Merged to Q", NULL };
static const char *tabsStacks[] = {
    "View", "Sort A-Z", "Def Words", "Short/Long", "Smallest",
    "Overlap", "Reverse", "Search", "Killed?", "Stack to Q", "Stack to List", NULL
};
static const char *tabsBST[] = {
    "Tree View", "In-Order", "Pre-Order", "Post-Order", "Info",
    "Search", "Add", "Delete", "Update", "LCA", "Name Range",
    "Successor", "Mirror", NULL
};
static const char *tabsRecur[] = {
    "Count", "Palindrome?", "Remove Lines", "Replace Dates",
    "Permute", "Subsequences", "Events Range", "Palindrome Subseq", NULL
};

static const char **allTabs[] = { tabsLists, tabsQueues, tabsStacks, tabsBST, tabsRecur };

/* --- Result buffer --- */
#define MAX_LINES 1000
#define MAX_LLEN  300

typedef struct {
    char lines[MAX_LINES][MAX_LLEN];
    int  count;
} Results;

/* --- Multi-field input system --- */
#define MAX_FIELDS 5
#define FIELD_LEN  256

typedef struct {
    char    text[FIELD_LEN];
    int     len;
    bool    active;
    char    label[32];
    float   width;    /* fraction of CW (0.0-1.0) */
} InputField;

/* --- App state --- */
typedef struct {
    View    view;
    int     tab;
    float   scroll;
    bool    needsRefresh;

    InputField fields[MAX_FIELDS];
    int        fieldCount;

    Results res;

    /* data */
    TList      *pers;
    TListDate  *dates;
    TListEvent *events;
    TTree      *tree;

    int persCount;
    int eventCount;

    /* BST view */
    Vector2  treePan;
    float    treeZoom;
} App;

static const char *DB_PATH = "data/algeria_history.txt";

/* --- Helpers --- */
static void resClear(App *a) { a->res.count = 0; a->scroll = 0; }

static void resAdd(App *a, const char *fmt, ...) {
    if (a->res.count >= MAX_LINES) return;
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(a->res.lines[a->res.count++], MAX_LLEN, fmt, ap);
    va_end(ap);
}

/* Convert Color to unsigned int for raygui style */
static int colorToInt(Color c) {
    return ((int)c.r << 24) | ((int)c.g << 16) | ((int)c.b << 8) | (int)c.a;
}

static void loadData(App *a) {
    listFree(a->pers);    a->pers   = loadPersonalities(DB_PATH);
    listDateFree(a->dates); a->dates  = loadPersonalityDates(DB_PATH);
    listEventFree(a->events); a->events = loadEvents(DB_PATH);
    treeFree(a->tree);   a->tree   = fillTree(DB_PATH);
    a->persCount = listSize(a->pers);
    a->eventCount = 0;
    for (TListEvent *e = a->events; e; e = e->next) a->eventCount++;
}

/* Setup input fields for a given view+tab */
static void setupFields(App *a) {
    a->fieldCount = 0;
    memset(a->fields, 0, sizeof(a->fields));
    for (int i = 0; i < MAX_FIELDS; i++)
        a->fields[i].width = 0.4f;

    #define F1(lbl, w) do { strncpy(a->fields[0].label, lbl, 31); a->fields[0].width = w; a->fieldCount = 1; } while(0)
    #define F2(l1, w1, l2, w2) do { F1(l1,w1); strncpy(a->fields[1].label,l2,31); a->fields[1].width=w2; a->fieldCount=2; } while(0)
    #define F3(l1,w1,l2,w2,l3,w3) do { F2(l1,w1,l2,w2); strncpy(a->fields[2].label,l3,31); a->fields[2].width=w3; a->fieldCount=3; } while(0)
    #define F4(l1,w1,l2,w2,l3,w3,l4,w4) do { F3(l1,w1,l2,w2,l3,w3); strncpy(a->fields[3].label,l4,31); a->fields[3].width=w4; a->fieldCount=4; } while(0)

    switch (a->view) {
    case V_LISTS:
        switch (a->tab) {
        case 7:  F1("Keyword/Year", 0.4f); break;        /* Similar */
        case 8:  F1("DoB (YYYY-MM-DD)", 0.3f); break;    /* Search DoB */
        case 9:  F1("DoD (YYYY-MM-DD)", 0.3f); break;    /* Search DoD */
        case 12: F4("Name",0.25f,"Definition",0.35f,"DoB",0.15f,"DoD",0.15f); break; /* Add */
        case 13: F1("Name to delete", 0.4f); break;       /* Delete */
        case 14: F4("Name",0.25f,"New Def",0.35f,"New DoB",0.15f,"New DoD",0.15f); break; /* Update */
        default: break;
        }
        break;
    case V_STACKS:
        switch (a->tab) {
        case 7: F1("Name to search", 0.4f); break;   /* Search */
        case 8: F1("Name to check", 0.4f); break;    /* Killed? */
        default: break;
        }
        break;
    case V_BST:
        switch (a->tab) {
        case 5:  F1("Name to search", 0.4f); break;   /* Search */
        case 6:  F4("Name",0.25f,"Definition",0.35f,"DoB",0.15f,"DoD",0.15f); break; /* Add */
        case 7:  F1("Name to delete", 0.4f); break;   /* Delete */
        case 8:  F4("Name",0.25f,"New Def",0.35f,"New DoB",0.15f,"New DoD",0.15f); break; /* Update */
        case 9:  F2("Name 1", 0.3f, "Name 2", 0.3f); break; /* LCA */
        case 10: F2("Min length", 0.2f, "Max length", 0.2f); break; /* Name range */
        case 11: F1("Name", 0.4f); break; /* Successor */
        default: break;
        }
        break;
    case V_RECURSION:
        switch (a->tab) {
        case 0: F1("Name to count", 0.4f); break;     /* Count */
        case 1: F1("Word to check", 0.4f); break;     /* Palindrome */
        case 2: F1("Word to remove", 0.4f); break;    /* Remove lines */
        case 3: F3("Name",0.3f,"New DoB",0.2f,"New DoD",0.2f); break; /* Replace dates */
        case 4: F1("Name to permute", 0.25f); break;  /* Permute */
        case 5: F1("Word", 0.3f); break;              /* Subsequences */
        case 6: F2("Start (YYYY-MM-DD)", 0.25f, "End (YYYY-MM-DD)", 0.25f); break; /* Events range */
        case 7: F1("Word", 0.3f); break;              /* Palindrome subseq */
        default: break;
        }
        break;
    default: break;
    }

    #undef F1
    #undef F2
    #undef F3
    #undef F4
}

/* Styled button using raygui — applies custom colors, then restores defaults */
static bool btnRect(Rectangle r, const char *text, Color bg, Color fg) {
    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL,  colorToInt(bg));
    GuiSetStyle(BUTTON, BASE_COLOR_FOCUSED, colorToInt(ColorBrightness(bg, 0.12f)));
    GuiSetStyle(BUTTON, BASE_COLOR_PRESSED, colorToInt(ColorBrightness(bg, -0.10f)));
    GuiSetStyle(BUTTON, TEXT_COLOR_NORMAL,  colorToInt(fg));
    GuiSetStyle(BUTTON, TEXT_COLOR_FOCUSED, colorToInt(fg));
    GuiSetStyle(BUTTON, TEXT_COLOR_PRESSED, colorToInt(fg));
    GuiSetStyle(BUTTON, BORDER_COLOR_NORMAL, colorToInt(bg));
    GuiSetStyle(BUTTON, BORDER_COLOR_FOCUSED, colorToInt(ColorBrightness(bg, 0.20f)));
    GuiSetStyle(BUTTON, BORDER_COLOR_PRESSED, colorToInt(bg));
    bool clicked = GuiButton(r, text);
    GuiLoadStyleDefault();
    return clicked;
}

/* --- Recursive traversal helper (0=in, 1=pre, 2=post) --- */
static void treeTravRec(TTree *n, App *a, int order) {
    if (!n) return;
    char d1[MAX_DATE], d2[MAX_DATE];
    if (order == 1) resAdd(a, "%-22s  Born: %s  Died: %s", n->name, dateToString(n->dob,d1), dateToString(n->dod,d2));
    treeTravRec(n->left, a, order);
    if (order == 0) resAdd(a, "%-22s  Born: %s  Died: %s", n->name, dateToString(n->dob,d1), dateToString(n->dod,d2));
    treeTravRec(n->right, a, order);
    if (order == 2) resAdd(a, "%-22s  Born: %s  Died: %s", n->name, dateToString(n->dob,d1), dateToString(n->dod,d2));
}

/* --- Stack copy helper --- */
static TStack *copyStack(TStack *stk) {
    TStack *cp = NULL;
    for (TStack *c = stk; c; c = c->next) push(&cp, c->name, c->definition, c->dob, c->dod);
    return cp;
}

/* --- Populate results --- */
static void refreshResults(App *a) {
    resClear(a);
    char d1[MAX_DATE], d2[MAX_DATE];
    const char *f0 = a->fields[0].text;
    const char *f1 = a->fields[1].text;
    const char *f2 = a->fields[2].text;
    const char *f3 = a->fields[3].text;

    switch (a->view) {
    /* ============ LISTS ============ */
    case V_LISTS:
        switch (a->tab) {
        case 0:
            for (TList *c = a->pers; c; c = c->next)
                resAdd(a, "%-22s  %s", c->name, c->definition);
            break;
        case 1:
            for (TListDate *c = a->dates; c; c = c->next)
                resAdd(a, "%-22s  Born: %s  Died: %s", c->name,
                       dateToString(c->dob,d1), dateToString(c->dod,d2));
            break;
        case 2:
            for (TListEvent *c = a->events; c; c = c->next)
                resAdd(a, "%-30s  %s", c->event_name, dateToString(c->date,d1));
            break;
        case 3: {
            TList *s = sortWord(a->pers);
            for (TList *c = s; c; c = c->next) resAdd(a, "%-22s  %s", c->name, c->definition);
            listFree(s); break;
        }
        case 4: {
            TList *s = sortWord2(a->pers);
            for (TList *c = s; c; c = c->next) resAdd(a, "%-22s  (len %d)  %s", c->name, (int)strlen(c->name), c->definition);
            listFree(s); break;
        }
        case 5: {
            TList *s = sortPersonality(a->pers, a->dates);
            for (TList *c = s; c; c = c->next) {
                int age = -1;
                for (TListDate *dd = a->dates; dd; dd = dd->next)
                    if (strcasecmp(dd->name, c->name)==0) { age = ageFromDates(dd->dob,dd->dod); break; }
                resAdd(a, "%-22s  age %d", c->name, age);
            }
            listFree(s); break;
        }
        case 6: {
            TList *p = palindromeName(a->pers);
            if (!p) { resAdd(a, "(no palindromes found)"); break; }
            for (TList *c = p; c; c = c->next) resAdd(a, "%s", c->name);
            listFree(p); break;
        }
        case 7: /* Similar */
            if (f0[0]) {
                TList *s = similarPersonality(a->pers, f0);
                if (!s) { resAdd(a, "No matches for '%s'.", f0); break; }
                for (TList *c = s; c; c = c->next) resAdd(a, "%-22s  %s", c->name, c->definition);
                listFree(s);
            } else resAdd(a, "Enter a keyword or year and press Enter.");
            break;
        case 8: /* Search DoB */
        case 9: /* Search DoD */
            if (f0[0]) {
                Date t = parseDate(f0);
                int found = 0;
                TList *p = a->pers; TListDate *dd = a->dates;
                while (p && dd) {
                    if (dateCmp(a->tab == 8 ? dd->dob : dd->dod, t) == 0) {
                        resAdd(a, "%-22s  %s", p->name, p->definition);
                        resAdd(a, "  Born: %s  Died: %s", dateToString(dd->dob,d1), dateToString(dd->dod,d2));
                        found = 1;
                    }
                    p = p->next; dd = dd->next;
                }
                if (!found) resAdd(a, "No personality %s on %s.", a->tab == 8 ? "born" : "died", f0);
            } else resAdd(a, "Enter a date (YYYY-MM-DD) and press Enter.");
            break;
        case 10: { /* BiList */
            TBiList *bi = mergeNodes(a->pers, a->dates);
            resAdd(a, "-- Bidirectional merged list --");
            for (TBiList *c = bi; c; c = c->next)
                resAdd(a, "%-22s  Born: %s  Died: %s", c->name, dateToString(c->dob,d1), dateToString(c->dod,d2));
            biListFree(bi); break;
        }
        case 11: { /* CircList */
            TCircList *ci = merge2Nodes(a->pers, a->dates);
            resAdd(a, "-- Circular merged list (showing all) --");
            if (ci) {
                TCircList *c = ci; int lim = 50;
                do {
                    resAdd(a, "%-22s  Born: %s  Died: %s", c->name, dateToString(c->dob,d1), dateToString(c->dod,d2));
                    c = c->next; lim--;
                } while (c != ci && lim > 0);
                circListFree(ci);
            }
            break;
        }
        case 12: /* Add */
        case 14: /* Update */
            if (f0[0]) {
                Date dob = parseDate(f2), dod = parseDate(f3);
                bool ok = (a->tab == 12) ? addPersonality(DB_PATH, &a->pers, &a->dates, f0, dob, dod, f1)
                                         : updatePersonality(DB_PATH, &a->pers, &a->dates, f0, f1, dob, dod);
                if (ok) {
                    loadData(a);
                    resAdd(a, "%s '%s' successfully. DB reloaded.", a->tab == 12 ? "Added" : "Updated", f0);
                } else resAdd(a, "Failed to %s.", a->tab == 12 ? "add" : "update");
            } else resAdd(a, "Fill in the fields and press Enter.");
            break;
        case 13: /* Delete */
            if (f0[0]) {
                if (deletepersonality(DB_PATH, &a->pers, &a->dates, f0)) {
                    loadData(a);
                    resAdd(a, "Deleted '%s'. DB reloaded.", f0);
                } else resAdd(a, "'%s' not found.", f0);
            } else resAdd(a, "Enter a name and press Enter.");
            break;
        default: break;
        }
        break;

    /* ============ QUEUES ============ */
    case V_QUEUES: {
        TQueue *q = NULL;
        switch (a->tab) {
        case 0: q = sName(a->pers); resAdd(a, "-- Queue sorted by word count in name --"); break;
        case 1: q = ageP(a->dates); resAdd(a, "-- Queue sorted by age --"); break;
        case 2: {
            TBiList *bi = mergeNodes(a->pers, a->dates);
            q = toQueue(bi); biListFree(bi);
            resAdd(a, "-- Queue from merged list --"); break;
        }
        default: break;
        }
        if (q) {
            for (TQNode *c = q->front; c; c = c->next)
                resAdd(a, "%-22s  Born: %s  Died: %s", c->name, dateToString(c->dob,d1), dateToString(c->dod,d2));
            queueFree(q);
        }
        break;
    }

    /* ============ STACKS ============ */
    case V_STACKS: {
        TBiList *bi = mergeNodes(a->pers, a->dates);
        TStack  *stk = toStack(bi);
        biListFree(bi);

        switch (a->tab) {
        case 0:
            for (TStack *c = stk; c; c = c->next)
                resAdd(a, "%-22s  %s", c->name, c->definition);
            break;
        case 1: {
            TStack *s = sortNameStack(copyStack(stk));
            for (TStack *c = s; c; c = c->next) resAdd(a, "%-22s  %s", c->name, c->definition);
            stackFree(s); break;
        }
        case 2: {
            TStack *ds = definitionStack(copyStack(stk));
            for (TStack *c = ds; c; c = c->next) resAdd(a, "%-22s  %s", c->name, c->definition);
            stackFree(ds); break;
        }
        case 3: {
            TStack *sh = NULL, *lo = NULL;
            pronunciationStack(stk, &sh, &lo);
            resAdd(a, "-- Short descriptions (<=5 words) --");
            for (TStack *c = sh; c; c = c->next) resAdd(a, "  %-22s  %s", c->name, c->definition);
            resAdd(a, "");
            resAdd(a, "-- Long descriptions (>5 words) --");
            for (TStack *c = lo; c; c = c->next) resAdd(a, "  %-22s  %s", c->name, c->definition);
            stackFree(sh); stackFree(lo); break;
        }
        case 4: {
            char *sm = getSmallest(stk);
            if (sm) resAdd(a, "Smallest definition: %s", sm);
            else    resAdd(a, "(empty stack)");
            break;
        }
        case 5: {
            resAdd(a, "-- Continuous/overlapping years --");
            int n = stackSize(stk);
            if (n < 2) { resAdd(a, "Not enough entries."); break; }
            typedef struct { char name[MAX_NAME]; int year; } YI;
            YI *items = malloc(n * sizeof(YI)); int i = 0;
            for (TStack *c = stk; c; c = c->next) {
                strncpy(items[i].name, c->name, MAX_NAME-1);
                items[i].year = c->dob.year ? c->dob.year : c->dod.year; i++;
            }
            for (int j = 1; j < n; j++) { YI key = items[j]; int k = j-1; while (k>=0 && items[k].year>key.year) { items[k+1]=items[k]; k--; } items[k+1]=key; }
            for (int j = 0; j < n-1; j++) {
                if (abs(items[j+1].year - items[j].year) <= 1 && items[j].year && items[j+1].year)
                    resAdd(a, "  %s (%d)  <->  %s (%d)", items[j].name, items[j].year, items[j+1].name, items[j+1].year);
            }
            free(items); break;
        }
        case 6: {
            TStack *rev = recRevStack(copyStack(stk));
            resAdd(a, "-- Reversed stack --");
            for (TStack *c = rev; c; c = c->next) resAdd(a, "%-22s  %s", c->name, c->definition);
            stackFree(rev); break;
        }
        case 7: /* Search */
            if (f0[0]) {
                TStack *found = getInfoPersonality(stk, f0);
                if (found) {
                    resAdd(a, "Name: %s", found->name);
                    resAdd(a, "Def:  %s", found->definition);
                    resAdd(a, "Born: %s  Died: %s", dateToString(found->dob,d1), dateToString(found->dod,d2));
                } else resAdd(a, "'%s' not found in stack.", f0);
            } else resAdd(a, "Enter a name and press Enter.");
            break;
        case 8: /* Killed? */
            if (f0[0]) {
                bool killed = isPersonalityKilled(stk, f0);
                resAdd(a, "'%s' was %skilled.", f0, killed ? "" : "NOT ");
            } else resAdd(a, "Enter a name and press Enter.");
            break;
        case 9: { /* Stack to Queue */
            TQueue *q = stackToQueue(stk);
            resAdd(a, "-- Stack converted to sorted queue --");
            for (TQNode *c = q->front; c; c = c->next)
                resAdd(a, "%-22s  Born: %s  Died: %s", c->name, dateToString(c->dob,d1), dateToString(c->dod,d2));
            queueFree(q); break;
        }
        case 10: { /* Stack to List */
            TList *l = stackToList(stk);
            resAdd(a, "-- Stack converted to sorted list --");
            for (TList *c = l; c; c = c->next) resAdd(a, "%-22s  %s", c->name, c->definition);
            listFree(l); break;
        }
        default: break;
        }
        stackFree(stk);
        break;
    }

    /* ============ BST ============ */
    case V_BST:
        switch (a->tab) {
        case 0: break; /* tree drawn graphically */
        case 1: resAdd(a, "-- In-order traversal --");   treeTravRec(a->tree, a, 0); break;
        case 2: resAdd(a, "-- Pre-order traversal --");  treeTravRec(a->tree, a, 1); break;
        case 3: resAdd(a, "-- Post-order traversal --"); treeTravRec(a->tree, a, 2); break;
        case 4:
            resAdd(a, "Height:   %d", treeHeight(a->tree));
            resAdd(a, "Size:     %d nodes", treeSize(a->tree));
            resAdd(a, "Balanced: %s", isBalancedBST(a->tree) ? "Yes" : "No");
            break;
        case 5: /* Search */
            if (f0[0]) {
                TTree *n = treeSearch(a->tree, f0);
                if (n) {
                    resAdd(a, "Name: %s", n->name);
                    resAdd(a, "Def:  %s", n->definition);
                    resAdd(a, "Born: %s  Died: %s", dateToString(n->dob,d1), dateToString(n->dod,d2));
                } else resAdd(a, "'%s' not found.", f0);
            } else resAdd(a, "Enter a name and press Enter.");
            break;
        case 6: /* Add */
        case 8: /* Update */
            if (f0[0]) {
                Date dob = parseDate(f2), dod = parseDate(f3);
                a->tree = (a->tab == 6) ? addNameBST(a->tree, f0, f1, dob, dod) : updateNameBST(a->tree, f0, f1, dob, dod);
                resAdd(a, "%s '%s' %s BST.", a->tab == 6 ? "Added" : "Updated", f0, a->tab == 6 ? "to" : "in");
            } else resAdd(a, "Fill in the fields and press Enter.");
            break;
        case 7: /* Delete */
            if (f0[0]) {
                a->tree = deleteNameBST(a->tree, f0);
                resAdd(a, "Deleted '%s' from BST.", f0);
            } else resAdd(a, "Enter a name and press Enter.");
            break;
        case 9: /* LCA */
            if (f0[0] && f1[0]) {
                TTree *lca = lowestCommonAncestor(a->tree, f0, f1);
                if (lca) resAdd(a, "Lowest Common Ancestor of '%s' and '%s': %s", f0, f1, lca->name);
                else     resAdd(a, "LCA not found.");
            } else resAdd(a, "Enter both names and press Enter.");
            break;
        case 10: /* Name range */
            if (f0[0] && f1[0]) {
                int lo = atoi(f0), hi = atoi(f1);
                int cnt = countNodesRange(a->tree, lo, hi);
                resAdd(a, "Nodes with name length in [%d, %d]: %d", lo, hi, cnt);
            } else resAdd(a, "Enter min and max name length and press Enter.");
            break;
        case 11: /* Successor */
            if (f0[0]) {
                TTree *s = inOrderSuccessor(a->tree, f0);
                if (s) resAdd(a, "In-order successor of '%s': %s", f0, s->name);
                else   resAdd(a, "No successor found for '%s'.", f0);
            } else resAdd(a, "Enter a name and press Enter.");
            break;
        case 12: /* Mirror */
            a->tree = BSTMirror(a->tree);
            resAdd(a, "Tree has been mirrored.");
            resAdd(a, "Switch to 'Tree View' to see the result.");
            break;
        default: break;
        }
        break;

    /* ============ RECURSION ============ */
    case V_RECURSION:
        switch (a->tab) {
        case 0: /* Count */
            if (f0[0]) {
                int cnt = countOccurrence(DB_PATH, f0);
                resAdd(a, "'%s' appears %d time(s) in the database.", f0, cnt);
            } else resAdd(a, "Enter a name and press Enter.");
            break;
        case 1: /* Palindrome? */
            if (f0[0]) {
                resAdd(a, "'%s' is %sa palindrome.", f0, isPalindromeWord(f0) ? "" : "NOT ");
            } else resAdd(a, "Enter a word and press Enter.");
            break;
        case 2: /* Remove lines */
            if (f0[0]) {
                if (removeOccurrence(DB_PATH, f0)) {
                    loadData(a);
                    resAdd(a, "Removed lines containing '%s'. DB reloaded.", f0);
                } else resAdd(a, "Failed to remove.");
            } else resAdd(a, "Enter a word and press Enter.");
            break;
        case 3: /* Replace dates */
            if (f0[0] && (f1[0] || f2[0])) {
                Date dob = parseDate(f1), dod = parseDate(f2);
                if (replaceOccurrence(DB_PATH, f0, dob, dod)) {
                    loadData(a);
                    resAdd(a, "Replaced dates for '%s'. DB reloaded.", f0);
                } else resAdd(a, "Failed to replace.");
            } else resAdd(a, "Fill in the fields and press Enter.");
            break;
        case 4: /* Permute */
            if (f0[0]) {
                int len = (int)strlen(f0);
                if (len > 8) {
                    resAdd(a, "Name too long for permutation display (max 8 chars).");
                } else {
                    /* generate permutations into results */
                    char buf[FIELD_LEN];
                    strncpy(buf, f0, FIELD_LEN-1); buf[FIELD_LEN-1] = '\0';
                    resAdd(a, "-- Permutations of '%s' --", f0);
                    /* simple inline permutation */
                    int total = 1;
                    for (int i = 1; i <= len; i++) total *= i;
                    resAdd(a, "Total: %d permutations", total);
                    /* show first ~100 */
                    int shown = 0;
                    int *idx = calloc(len, sizeof(int));
                    resAdd(a, "  %s", buf);
                    shown++;
                    int ii = 0;
                    while (ii < len && shown < 200) {
                        if (idx[ii] < ii) {
                            if (ii % 2 == 0) { char t = buf[0]; buf[0] = buf[ii]; buf[ii] = t; }
                            else { char t = buf[idx[ii]]; buf[idx[ii]] = buf[ii]; buf[ii] = t; }
                            resAdd(a, "  %s", buf);
                            shown++;
                            idx[ii]++;
                            ii = 0;
                        } else { idx[ii] = 0; ii++; }
                    }
                    if (shown < total) resAdd(a, "  ... (%d more)", total - shown);
                    free(idx);
                }
            } else resAdd(a, "Enter a short name (max 8 chars) and press Enter.");
            break;
        case 5: /* Subsequences */
            if (f0[0]) {
                int len = (int)strlen(f0);
                if (len > 16) { resAdd(a, "Word too long (max 16 chars)."); break; }
                resAdd(a, "-- Subsequences of '%s' --", f0);
                int total = (1 << len) - 1;
                int shown = 0;
                for (int mask = 1; mask <= total && shown < 200; mask++) {
                    char sub[64]; int k = 0;
                    for (int i = 0; i < len; i++)
                        if (mask & (1 << i)) sub[k++] = f0[i];
                    sub[k] = '\0';
                    resAdd(a, "  %s", sub);
                    shown++;
                }
                if (shown < total) resAdd(a, "  ... (%d more)", total - shown);
            } else resAdd(a, "Enter a word and press Enter.");
            break;
        case 6: /* Events range */
            if (f0[0] && f1[0]) {
                Date start = parseDate(f0), end = parseDate(f1);
                resAdd(a, "-- Events between %s and %s --", f0, f1);
                int found = 0;
                for (TListEvent *c = a->events; c; c = c->next) {
                    if (dateCmp(c->date, start) >= 0 && dateCmp(c->date, end) <= 0) {
                        resAdd(a, "  %-30s  %s", c->event_name, dateToString(c->date, d1));
                        found++;
                    }
                }
                if (!found) resAdd(a, "  No events found in this range.");
            } else resAdd(a, "Enter start and end dates and press Enter.");
            break;
        case 7: /* Palindrome subseq */
            if (f0[0]) {
                int cnt = distinctSubseqWord(f0);
                resAdd(a, "Distinct palindrome subsequences in '%s': %d", f0, cnt);
            } else resAdd(a, "Enter a word and press Enter.");
            break;
        default: break;
        }
        break;

    default: break;
    }
    a->needsRefresh = false;
}

/* --- BST tree drawing --- */
static void drawTreeRec(TTree *node, float x, float y, float spread, float zoom, Vector2 pan) {
    if (!node) return;
    float cx = x + pan.x, cy = y + pan.y;
    float childY = y + 80 * zoom;
    float childSpread = spread * 0.52f;

    if (node->left) {
        float lx = x - spread + pan.x, ly = childY + pan.y;
        DrawLineEx((Vector2){cx, cy}, (Vector2){lx, ly}, 1.5f, COL_DIVIDER);
        drawTreeRec(node->left, x - spread, childY, childSpread, zoom, pan);
    }
    if (node->right) {
        float rx = x + spread + pan.x, ry = childY + pan.y;
        DrawLineEx((Vector2){cx, cy}, (Vector2){rx, ry}, 1.5f, COL_DIVIDER);
        drawTreeRec(node->right, x + spread, childY, childSpread, zoom, pan);
    }

    float r = 24 * zoom;
    DrawCircleV((Vector2){cx, cy}, r, COL_GREEN);
    DrawCircleV((Vector2){cx, cy}, r - 2, COL_CARD);

    char label[20] = {0};
    strncpy(label, node->name, sizeof(label) - 1);
    char *space = strchr(label, ' ');
    if (space) *space = '\0';
    int fs = (int)(11 * zoom);
    if (fs < 5) fs = 5;
    int tw = MeasureText(label, fs);
    DrawText(label, (int)(cx - tw/2), (int)(cy - fs/2), fs, COL_TEXT);
}

/* --- Draw sidebar --- */
static void drawSidebar(App *a) {
    DrawRectangle(0, 0, SIDEBAR_W, WIN_H, COL_SIDEBAR);
    DrawRectangle(0, 0, SIDEBAR_W, HEADER_H, COL_HEADER);
    DrawText("ALGERIA DB", 16, 10, 20, COL_GREEN);
    DrawText("History Database", 16, 32, 12, COL_TEXT_DIM);
    DrawLineEx((Vector2){0, HEADER_H-1}, (Vector2){SIDEBAR_W, HEADER_H-1}, 1, COL_DIVIDER);

    for (int i = 0; i < V_COUNT; i++) {
        float y = HEADER_H + 12 + i * 44;
        Rectangle r = { 8, y, SIDEBAR_W - 16, 38 };
        bool hover = CheckCollisionPointRec(GetMousePosition(), r);
        bool active = ((int)a->view == i);

        if (active) {
            DrawRectangleRounded(r, 0.3f, 4, COL_GREEN_DIM);
            DrawRectangle(0, (int)y, 3, 38, COL_GREEN);
        } else if (hover) {
            DrawRectangleRounded(r, 0.3f, 4, (Color){25, 30, 48, 255});
        }

        Color tc = active ? COL_ACCENT : (hover ? COL_TEXT : COL_TEXT_DIM);
        DrawText(viewIcons[i], 20, (int)(y + 10), 16, tc);
        DrawText(viewNames[i], 42, (int)(y + 11), 15, tc);

        if (hover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && (int)a->view != i) {
            a->view = i;
            a->tab = 0;
            a->needsRefresh = true;
            a->treePan = (Vector2){0, 0};
            setupFields(a);
        }
    }

    DrawLineEx((Vector2){12, WIN_H - 70}, (Vector2){SIDEBAR_W - 12, WIN_H - 70}, 1, COL_DIVIDER);
    DrawText(TextFormat("%d personalities", a->persCount), 16, WIN_H - 55, 13, COL_TEXT_DIM);
    DrawText(TextFormat("%d events", a->eventCount), 16, WIN_H - 38, 13, COL_TEXT_DIM);
    DrawText("data/algeria_history.txt", 16, WIN_H - 18, 10, (Color){60,65,85,255});
}

/* --- Draw header --- */
static void drawHeader(App *a) {
    DrawRectangle(SIDEBAR_W, 0, WIN_W - SIDEBAR_W, HEADER_H, COL_HEADER);
    DrawLineEx((Vector2){SIDEBAR_W, HEADER_H-1}, (Vector2){WIN_W, HEADER_H-1}, 1, COL_DIVIDER);
    DrawText(TextFormat("%s  %s", viewIcons[a->view], viewNames[a->view]), SIDEBAR_W + 20, 17, 22, COL_TEXT);
}

/* --- Draw tabs (scrollable) --- */
static void drawTabs(App *a) {
    DrawRectangle(SIDEBAR_W, HEADER_H, WIN_W - SIDEBAR_W, TAB_H, COL_TAB);
    const char **tabs = allTabs[a->view];
    int x = SIDEBAR_W + 10;
    for (int i = 0; tabs[i]; i++) {
        int tw = MeasureText(tabs[i], 12) + 18;
        Rectangle r = { (float)x, HEADER_H + 5, (float)tw, TAB_H - 10 };
        bool active = (a->tab == i);
        Color bg = active ? COL_TAB_ACT : COL_TAB;
        Color fg = active ? WHITE : COL_TEXT_DIM;
        if (btnRect(r, tabs[i], bg, fg)) {
            if (a->tab != i) {
                a->tab = i;
                a->needsRefresh = true;
                a->treePan = (Vector2){0, 0};
                setupFields(a);
            }
        }
        x += tw + 4;
    }
}

/* --- Draw input fields (using raygui GuiTextBox) --- */
static int drawFields(App *a) {
    if (a->fieldCount == 0) return 0;

    int y = CY;
    int x = CX;
    int totalH = 0;

    /* Style GuiTextBox to match our theme */
    GuiSetStyle(TEXTBOX, BASE_COLOR_NORMAL,   colorToInt(COL_INPUT_BG));
    GuiSetStyle(TEXTBOX, BORDER_COLOR_NORMAL,  colorToInt(COL_DIVIDER));
    GuiSetStyle(TEXTBOX, BORDER_COLOR_FOCUSED, colorToInt(COL_GREEN));
    GuiSetStyle(TEXTBOX, TEXT_COLOR_NORMAL,     colorToInt(COL_TEXT));
    GuiSetStyle(TEXTBOX, TEXT_COLOR_FOCUSED,    colorToInt(COL_TEXT));
    GuiSetStyle(TEXTBOX, BASE_COLOR_FOCUSED,   colorToInt(COL_INPUT_BG));

    for (int i = 0; i < a->fieldCount; i++) {
        InputField *f = &a->fields[i];
        int fw = (int)(CW * f->width) - 8;
        if (fw < 80) fw = 80;

        /* label */
        DrawText(f->label, x, y, 11, COL_TEXT_DIM);

        /* text box via raygui */
        Rectangle box = { (float)x, (float)(y + 14), (float)fw, INPUT_H };
        if (GuiTextBox(box, f->text, FIELD_LEN, f->active)) {
            /* toggle focus on click */
            f->active = !f->active;
        }
        f->len = (int)strlen(f->text);

        /* Tab key cycles fields */
        if (f->active && IsKeyPressed(KEY_TAB)) {
            f->active = false;
            a->fields[(i + 1) % a->fieldCount].active = true;
        }
        /* Enter triggers refresh */
        if (f->active && IsKeyPressed(KEY_ENTER))
            a->needsRefresh = true;

        x += fw + 12;
        totalH = 14 + INPUT_H + FIELD_GAP;
    }

    GuiLoadStyleDefault();  /* restore default raygui style */

    /* Go button */
    Rectangle go = { (float)x, (float)(y + 14), 50, INPUT_H };
    if (btnRect(go, "Go", COL_GREEN, WHITE))
        a->needsRefresh = true;

    return totalH + 4;
}

/* --- Draw content area --- */
static void drawContent(App *a, int inputAreaH) {
    /* BST tree view */
    if (a->view == V_BST && a->tab == 0) {
        BeginScissorMode(CX, CY, CW, CH);
        DrawRectangle(CX, CY, CW, CH, COL_BG);

        if (a->tree) {
            if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
                Vector2 m = GetMousePosition();
                if (m.x > CX && m.x < CX + CW && m.y > CY && m.y < CY + CH) {
                    Vector2 delta = GetMouseDelta();
                    a->treePan.x += delta.x;
                    a->treePan.y += delta.y;
                }
            }
            float wheel = GetMouseWheelMove();
            if (wheel != 0) {
                Vector2 m = GetMousePosition();
                if (m.x > CX && m.x < CX + CW && m.y > CY && m.y < CY + CH) {
                    a->treeZoom += wheel * 0.1f;
                    if (a->treeZoom < 0.3f) a->treeZoom = 0.3f;
                    if (a->treeZoom > 2.5f) a->treeZoom = 2.5f;
                }
            }
            float centerX = CX + CW / 2.0f;
            float topY = CY + 40;
            drawTreeRec(a->tree, centerX, topY, 250 * a->treeZoom, a->treeZoom, a->treePan);
            DrawText("Drag to pan | Scroll to zoom", CX + 8, CY + CH - 20, 11, COL_TEXT_DIM);
        } else {
            DrawText("No tree data loaded.", CX + 20, CY + 20, 16, COL_TEXT_DIM);
        }
        EndScissorMode();
        return;
    }

    /* scrollable text results */
    int contentTop = CY + inputAreaH;
    int contentH = WIN_H - contentTop - 12;

    BeginScissorMode(CX, contentTop, CW, contentH);

    float wheel = GetMouseWheelMove();
    if (wheel != 0) {
        Vector2 m = GetMousePosition();
        if (m.x > CX && m.x < CX + CW && m.y > contentTop)
            a->scroll -= wheel * 40;
    }
    float maxScroll = a->res.count * 26.0f - contentH;
    if (maxScroll < 0) maxScroll = 0;
    a->scroll = Clamp(a->scroll, 0, maxScroll);

    for (int i = 0; i < a->res.count; i++) {
        int y = contentTop + i * 26 - (int)a->scroll;
        if (y < contentTop - 26 || y > WIN_H) continue;

        if (i % 2 == 0)
            DrawRectangle(CX, y, CW, 24, (Color){20, 22, 36, 255});

        /* header lines (start with --) */
        bool isHeader = (a->res.lines[i][0] == '-' && a->res.lines[i][1] == '-');
        DrawText(a->res.lines[i], CX + 10, y + 5, 13, isHeader ? COL_ACCENT : COL_TEXT);
    }
    EndScissorMode();

    /* scrollbar */
    if (maxScroll > 0) {
        float barH = (float)contentH;
        float thumbH = barH * barH / (a->res.count * 26.0f);
        if (thumbH < 20) thumbH = 20;
        float thumbY = contentTop + (a->scroll / maxScroll) * (barH - thumbH);
        DrawRectangleRounded((Rectangle){CX + CW - 5, thumbY, 4, thumbH}, 0.5f, 4, COL_DIVIDER);
    }
}

/* --- Main --- */
int main(void) {
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE);
    InitWindow(WIN_W, WIN_H, "Algeria History Database");
    SetTargetFPS(60);

    App app = {0};
    app.view = V_LISTS;
    app.treeZoom = 1.0f;
    app.needsRefresh = true;

    loadData(&app);
    setupFields(&app);

    while (!WindowShouldClose()) {
        if (app.needsRefresh) refreshResults(&app);

        BeginDrawing();
        ClearBackground(COL_BG);

        drawSidebar(&app);
        drawHeader(&app);
        drawTabs(&app);
        int inputH = drawFields(&app);
        drawContent(&app, inputH);

        DrawLineEx((Vector2){SIDEBAR_W, 0}, (Vector2){SIDEBAR_W, WIN_H}, 1, COL_DIVIDER);

        EndDrawing();
    }

    listFree(app.pers);
    listDateFree(app.dates);
    listEventFree(app.events);
    treeFree(app.tree);
    CloseWindow();
    return 0;
}
