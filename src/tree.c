#include "tree.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static TTree *nodeCreate(const char *name, const char *def, Date dob, Date dod) {
    TTree *n = (TTree *)malloc(sizeof(TTree));
    if (!n) return NULL;
    strncpy(n->name,       name ? name : "", MAX_NAME - 1);
    strncpy(n->definition, def  ? def  : "", MAX_DEF  - 1);
    n->name[MAX_NAME-1]       = '\0';
    n->definition[MAX_DEF-1] = '\0';
    n->dob   = dob;
    n->dod   = dod;
    n->left  = n->right = NULL;
    return n;
}

TTree *treeInsert(TTree *root, const char *name, const char *def,
                  Date dob, Date dod) {
    if (!root) return nodeCreate(name, def, dob, dod);
    int cmp = strcasecmp(name, root->name);
    if (cmp < 0)      root->left  = treeInsert(root->left,  name, def, dob, dod);
    else if (cmp > 0) root->right = treeInsert(root->right, name, def, dob, dod);
    else {
        if (def && def[0]) strncpy(root->definition, def, MAX_DEF - 1);
        if (!dateIsNull(dob)) root->dob = dob;
        if (!dateIsNull(dod)) root->dod = dod;
    }
    return root;
}

TTree *treeSearch(TTree *root, const char *name) {
    if (!root) return NULL;
    int cmp = strcasecmp(name, root->name);
    if (cmp == 0) return root;
    if (cmp < 0)  return treeSearch(root->left,  name);
    return             treeSearch(root->right, name);
}

void treeFree(TTree *root) {
    if (!root) return;
    treeFree(root->left);
    treeFree(root->right);
    free(root);
}

int treeHeight(TTree *root) {
    if (!root) return 0;
    int lh = treeHeight(root->left);
    int rh = treeHeight(root->right);
    return 1 + (lh > rh ? lh : rh);
}

int treeSize(TTree *root) {
    if (!root) return 0;
    return 1 + treeSize(root->left) + treeSize(root->right);
}

TTree *toTree(TStack *stk) {
    TTree *root = NULL;
    for (TStack *cur = stk; cur; cur = cur->next)
        root = treeInsert(root, cur->name, cur->definition, cur->dob, cur->dod);
    return root;
}

TTree *fillTree(const char *filename) {
    TList    *pers  = loadPersonalities(filename);
    TListDate *dates = loadPersonalityDates(filename);
    TTree *root = NULL;

    TList    *p = pers;
    TListDate *d = dates;
    while (p && d) {
        root = treeInsert(root, p->name, p->definition, d->dob, d->dod);
        p = p->next; d = d->next;
    }
    listFree(pers);
    listDateFree(dates);
    return root;
}

TTree *getInfoNameTree(TTree *tr, const char *name) {
    TTree *node = treeSearch(tr, name);
    if (!node) { printf("'%s' not found in tree.\n", name); return NULL; }
    char dob_buf[MAX_DATE], dod_buf[MAX_DATE];
    printf("Name: %s\nDefinition: %s\nDoB: %s  DoD: %s\n",
           node->name, node->definition,
           dateToString(node->dob, dob_buf),
           dateToString(node->dod, dod_buf));
    return node;
}

TTree *addNameBST(TTree *tr, const char *name,
                  const char *def, Date dob, Date dod) {
    return treeInsert(tr, name, def, dob, dod);
}

static TTree *findMin(TTree *root) {
    while (root->left) root = root->left;
    return root;
}

TTree *deleteNameBST(TTree *tr, const char *name) {
    if (!tr) return NULL;
    int cmp = strcasecmp(name, tr->name);
    if      (cmp < 0) tr->left  = deleteNameBST(tr->left,  name);
    else if (cmp > 0) tr->right = deleteNameBST(tr->right, name);
    else {
        if (!tr->left)  { TTree *r = tr->right; free(tr); return r; }
        if (!tr->right) { TTree *l = tr->left;  free(tr); return l; }
        TTree *successor = findMin(tr->right);
        strncpy(tr->name,       successor->name,       MAX_NAME - 1);
        strncpy(tr->definition, successor->definition, MAX_DEF  - 1);
        tr->dob = successor->dob;
        tr->dod = successor->dod;
        tr->right = deleteNameBST(tr->right, successor->name);
    }
    return tr;
}

TTree *updateNameBST(TTree *tr, const char *name,
                     const char *def, Date dob, Date dod) {
    TTree *node = treeSearch(tr, name);
    if (!node) { printf("'%s' not found.\n", name); return tr; }
    if (def && def[0]) strncpy(node->definition, def, MAX_DEF - 1);
    if (!dateIsNull(dob)) node->dob = dob;
    if (!dateIsNull(dod)) node->dod = dod;
    return tr;
}

static void printNode(TTree *n) {
    char dob_buf[MAX_DATE], dod_buf[MAX_DATE];
    printf("  [%s]  DoB: %s  DoD: %s\n",
           n->name,
           dateToString(n->dob, dob_buf),
           dateToString(n->dod, dod_buf));
}

void traversalBSTinOrder(TTree *tr) {
    if (!tr) return;
    traversalBSTinOrder(tr->left);
    printNode(tr);
    traversalBSTinOrder(tr->right);
}

void traversalBSTpreOrder(TTree *tr) {
    if (!tr) return;
    printNode(tr);
    traversalBSTpreOrder(tr->left);
    traversalBSTpreOrder(tr->right);
}

void traversalBSTpostOrder(TTree *tr) {
    if (!tr) return;
    traversalBSTpostOrder(tr->left);
    traversalBSTpostOrder(tr->right);
    printNode(tr);
}

void heightSizeBST(TTree *tr) {
    printf("Height: %d  |  Size: %d\n", treeHeight(tr), treeSize(tr));
}

TTree *lowestCommonAncestor(TTree *tr,
                              const char *word1, const char *word2) {
    if (!tr) return NULL;
    int cmp1 = strcasecmp(word1, tr->name);
    int cmp2 = strcasecmp(word2, tr->name);
    if (cmp1 < 0 && cmp2 < 0) return lowestCommonAncestor(tr->left,  word1, word2);
    if (cmp1 > 0 && cmp2 > 0) return lowestCommonAncestor(tr->right, word1, word2);
    return tr;
}

int countNodesRange(TTree *tr, int l, int h) {
    if (!tr) return 0;
    int len = (int)strlen(tr->name);
    int count = (len >= l && len <= h) ? 1 : 0;
    return count + countNodesRange(tr->left, l, h) + countNodesRange(tr->right, l, h);
}

TTree *inOrderSuccessor(TTree *tr, const char *word) {
    TTree *successor = NULL;
    TTree *cur = tr;
    while (cur) {
        int cmp = strcasecmp(word, cur->name);
        if (cmp < 0) { successor = cur; cur = cur->left; }
        else          cur = cur->right;
    }
    return successor;
}

TTree *BSTMirror(TTree *tr) {
    if (!tr) return NULL;
    TTree *tmp  = tr->left;
    tr->left    = BSTMirror(tr->right);
    tr->right   = BSTMirror(tmp);
    return tr;
}

static int checkHeight(TTree *root) {
    if (!root) return 0;
    int lh = checkHeight(root->left);
    if (lh == -1) return -1;
    int rh = checkHeight(root->right);
    if (rh == -1) return -1;
    int diff = lh - rh;
    if (diff < -1 || diff > 1) return -1;
    return 1 + (lh > rh ? lh : rh);
}

bool isBalancedBST(TTree *tr) {
    return checkHeight(tr) != -1;
}

static void collectInOrder(TTree *root, TTree **arr, int *idx) {
    if (!root) return;
    collectInOrder(root->left, arr, idx);
    arr[(*idx)++] = root;
    collectInOrder(root->right, arr, idx);
}

static TTree *buildBalanced(TTree **arr, int start, int end) {
    if (start > end) return NULL;
    int mid = (start + end) / 2;
    TTree *root = nodeCreate(arr[mid]->name, arr[mid]->definition,
                             arr[mid]->dob,  arr[mid]->dod);
    root->left  = buildBalanced(arr, start,   mid - 1);
    root->right = buildBalanced(arr, mid + 1, end);
    return root;
}

static TTree **mergeSortedArrays(TTree **a, int na,
                                  TTree **b, int nb,
                                  int *total) {
    *total = na + nb;
    TTree **merged = (TTree **)malloc(*total * sizeof(TTree *));
    if (!merged) return NULL;
    int i = 0, j = 0, k = 0;
    while (i < na && j < nb) {
        if (strcasecmp(a[i]->name, b[j]->name) <= 0) merged[k++] = a[i++];
        else                                           merged[k++] = b[j++];
    }
    while (i < na) merged[k++] = a[i++];
    while (j < nb) merged[k++] = b[j++];
    return merged;
}

TTree *BTSMerge(TTree *tr1, TTree *tr2) {
    int n1 = treeSize(tr1), n2 = treeSize(tr2);

    TTree **arr1 = (TTree **)malloc(n1 * sizeof(TTree *));
    TTree **arr2 = (TTree **)malloc(n2 * sizeof(TTree *));
    if (!arr1 || !arr2) { free(arr1); free(arr2); return NULL; }

    int idx1 = 0, idx2 = 0;
    collectInOrder(tr1, arr1, &idx1);
    collectInOrder(tr2, arr2, &idx2);

    int total;
    TTree **merged = mergeSortedArrays(arr1, n1, arr2, n2, &total);
    free(arr1); free(arr2);
    if (!merged) return NULL;

    TTree *root = buildBalanced(merged, 0, total - 1);
    free(merged);
    return root;
}
