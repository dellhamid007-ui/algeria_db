#ifndef TREE_H
#define TREE_H

#include "types.h"
#include "stack.h"

/* BST primitives */
TTree *treeInsert(TTree *root, const char *name, const char *def,
                  Date dob, Date dod);
TTree *treeSearch(TTree *root, const char *name);
void   treeFree(TTree *root);
int    treeHeight(TTree *root);
int    treeSize(TTree *root);

/* BST functions */
TTree *toTree(TStack *stk);
TTree *fillTree(const char *filename);
TTree *getInfoNameTree(TTree *tr, const char *name);
TTree *addNameBST(TTree *tr, const char *name,
                  const char *def, Date dob, Date dod);
TTree *deleteNameBST(TTree *tr, const char *name);
TTree *updateNameBST(TTree *tr, const char *name,
                     const char *def, Date dob, Date dod);

void traversalBSTinOrder(TTree *tr);
void traversalBSTpreOrder(TTree *tr);
void traversalBSTpostOrder(TTree *tr);
void heightSizeBST(TTree *tr);

TTree *lowestCommonAncestor(TTree *tr,
                             const char *word1, const char *word2);
int   countNodesRange(TTree *tr, int l, int h);
TTree *inOrderSuccessor(TTree *tr, const char *word);
TTree *BSTMirror(TTree *tr);
bool  isBalancedBST(TTree *tr);
TTree *BTSMerge(TTree *tr1, TTree *tr2);

#endif
