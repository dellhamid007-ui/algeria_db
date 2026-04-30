#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "file_parser.h"
#include "list.h"
#include "stack.h"
#include "tree.h"
#include "recursion.h"

static const char *DB_FILE = "data/algeria_history.txt";

static TList      *g_pers   = NULL;
static TListDate  *g_dates  = NULL;
static TListEvent *g_events = NULL;

static void loadAll(void) {
    listFree(g_pers);
    listDateFree(g_dates);
    listEventFree(g_events);
    g_pers   = loadPersonalities(DB_FILE);
    g_dates  = loadPersonalityDates(DB_FILE);
    g_events = loadEvents(DB_FILE);
    printf("[DB] Loaded %d personalities and %d events.\n",
           listSize(g_pers), (int)(g_events ? 1 : 0));
}

static void readLine(const char *prompt, char *buf, int sz) {
    printf("%s", prompt);
    fflush(stdout);
    if (!fgets(buf, sz, stdin)) buf[0] = '\0';
    buf[strcspn(buf, "\n")] = '\0';
    trimWhitespace(buf);
}

static Date readDate(const char *prompt) {
    char buf[MAX_DATE];
    readLine(prompt, buf, MAX_DATE);
    return parseDate(buf);
}

/* --- Linked List menu --- */
static void menuList(void) {
    int choice;
    char buf1[MAX_NAME], buf2[MAX_DEF];
    do {
        printf("\n=== Linked List Menu ===\n");
        printf(" 1. Print personality list (name+def)\n");
        printf(" 2. Print date list (name+dob+dod)\n");
        printf(" 3. Print event list\n");
        printf(" 4. Sort personalities alphabetically\n");
        printf(" 5. Sort by name length\n");
        printf(" 6. Sort by age\n");
        printf(" 7. Search by Date of Birth\n");
        printf(" 8. Search by Date of Death\n");
        printf(" 9. Find palindrome names in definitions\n");
        printf("10. Find similar personalities (keyword/year)\n");
        printf("11. Print bidirectional (merged) list\n");
        printf("12. Print circular (merged) list\n");
        printf("13. Add personality\n");
        printf("14. Delete personality\n");
        printf("15. Update personality\n");
        printf("16. Add event\n");
        printf(" 0. Back\n");
        printf("Choice: "); scanf("%d", &choice); getchar();

        switch (choice) {
        case 1:
            printf("\n--- Personality List ---\n");
            listPrint(g_pers);
            break;
        case 2:
            printf("\n--- Date List ---\n");
            listDatePrint(g_dates);
            break;
        case 3:
            printf("\n--- Event List ---\n");
            listEventPrint(g_events);
            break;
        case 4: {
            TList *sorted = sortWord(g_pers);
            printf("\n--- Alphabetically Sorted ---\n");
            listPrint(sorted);
            listFree(sorted);
            break;
        }
        case 5: {
            TList *sorted = sortWord2(g_pers);
            printf("\n--- Sorted by Name Length ---\n");
            listPrint(sorted);
            listFree(sorted);
            break;
        }
        case 6: {
            TList *sorted = sortPersonality(g_pers, g_dates);
            printf("\n--- Sorted by Age ---\n");
            listPrint(sorted);
            listFree(sorted);
            break;
        }
        case 7: {
            Date d = readDate("Enter Date of Birth (YYYY-MM-DD): ");
            getInfoByDates(g_pers, g_dates, d);
            break;
        }
        case 8: {
            Date d = readDate("Enter Date of Death (YYYY-MM-DD): ");
            getInfoByDates2(g_pers, g_dates, d);
            break;
        }
        case 9: {
            TList *palList = palindromeName(g_pers);
            printf("\n--- Palindrome Names Found ---\n");
            listPrint(palList);
            listFree(palList);
            break;
        }
        case 10: {
            readLine("Enter keyword or year: ", buf1, MAX_NAME);
            TList *sim = similarPersonality(g_pers, buf1);
            printf("\n--- Similar Personalities ---\n");
            listPrint(sim);
            listFree(sim);
            break;
        }
        case 11: {
            TBiList *bi = mergeNodes(g_pers, g_dates);
            printf("\n--- Bidirectional List ---\n");
            biListPrint(bi);
            biListFree(bi);
            break;
        }
        case 12: {
            TCircList *circ = merge2Nodes(g_pers, g_dates);
            printf("\n--- Circular List (first 20 nodes) ---\n");
            if (circ) {
                char dob_buf[MAX_DATE], dod_buf[MAX_DATE];
                TCircList *cur = circ;
                int count = 0;
                do {
                    printf("  [%s]  DoB: %s  DoD: %s\n",
                           cur->name,
                           dateToString(cur->dob, dob_buf),
                           dateToString(cur->dod, dod_buf));
                    cur = cur->next;
                    count++;
                } while (cur != circ && count < 20);
                circListFree(circ);
            }
            break;
        }
        case 13: {
            readLine("Name: ",       buf1, MAX_NAME);
            readLine("Definition: ", buf2, MAX_DEF);
            Date dob = readDate("Date of Birth (YYYY-MM-DD): ");
            Date dod = readDate("Date of Death (YYYY-MM-DD): ");
            if (addPersonality(DB_FILE, &g_pers, &g_dates, buf1, dob, dod, buf2))
                printf("Added successfully.\n");
            break;
        }
        case 14: {
            readLine("Name to delete: ", buf1, MAX_NAME);
            if (deletepersonality(DB_FILE, &g_pers, &g_dates, buf1))
                printf("Deleted successfully.\n");
            break;
        }
        case 15: {
            readLine("Name to update: ",    buf1, MAX_NAME);
            readLine("New definition: ",    buf2, MAX_DEF);
            Date dob = readDate("New DoB (YYYY-MM-DD, blank=no change): ");
            Date dod = readDate("New DoD (YYYY-MM-DD, blank=no change): ");
            if (updatePersonality(DB_FILE, &g_pers, &g_dates, buf1, buf2, dob, dod))
                printf("Updated successfully.\n");
            break;
        }
        case 16: {
            readLine("Event name: ", buf1, MAX_NAME);
            Date date = readDate("Event date (YYYY-MM-DD): ");
            if (addEvents(DB_FILE, &g_events, buf1, date))
                printf("Event added.\n");
            break;
        }
        case 0: break;
        default: printf("Invalid choice.\n");
        }
    } while (choice != 0);
}

/* --- Queue menu --- */
static void menuQueue(void) {
    int choice;
    do {
        printf("\n=== Queue Menu ===\n");
        printf(" 1. sName   – queue sorted by word count in name\n");
        printf(" 2. ageP    – queue sorted by age\n");
        printf(" 3. toQueue – merged list to queue\n");
        printf(" 0. Back\n");
        printf("Choice: "); scanf("%d", &choice); getchar();

        switch (choice) {
        case 1: {
            TQueue *q = sName(g_pers);
            printf("\n--- Queue sorted by name word count ---\n");
            queuePrint(q);
            queueFree(q);
            break;
        }
        case 2: {
            TQueue *q = ageP(g_dates);
            printf("\n--- Queue sorted by age ---\n");
            queuePrint(q);
            queueFree(q);
            break;
        }
        case 3: {
            TBiList *bi = mergeNodes(g_pers, g_dates);
            TQueue  *q  = toQueue(bi);
            printf("\n--- Queue from merged list ---\n");
            queuePrint(q);
            queueFree(q);
            biListFree(bi);
            break;
        }
        case 0: break;
        default: printf("Invalid choice.\n");
        }
    } while (choice != 0);
}

/* --- Stack menu --- */
static void menuStack(void) {
    int choice;
    char buf1[MAX_NAME], buf2[MAX_DEF];
    do {
        printf("\n=== Stack Menu ===\n");
        printf(" 1. Build stack from merged list & print\n");
        printf(" 2. Search personality in stack\n");
        printf(" 3. Sort stack alphabetically\n");
        printf(" 4. Add to stack\n");
        printf(" 5. Delete from stack\n");
        printf(" 6. Update in stack\n");
        printf(" 7. Stack -> sorted queue\n");
        printf(" 8. Stack -> sorted list\n");
        printf(" 9. Sort by definition word count\n");
        printf("10. Split short/long descriptions\n");
        printf("11. Get smallest definition\n");
        printf("12. Continuous search (overlapping years)\n");
        printf("13. Is personality killed?\n");
        printf("14. Reverse stack (recursive)\n");
        printf(" 0. Back\n");
        printf("Choice: "); scanf("%d", &choice); getchar();

        TBiList *bi  = mergeNodes(g_pers, g_dates);
        TStack  *stk = toStack(bi);
        biListFree(bi);

        switch (choice) {
        case 1:
            printf("\n--- Stack ---\n");
            stackPrint(stk);
            break;
        case 2: {
            readLine("Name to search: ", buf1, MAX_NAME);
            TStack *found = getInfoPersonality(stk, buf1);
            if (found) {
                char dob_buf[MAX_DATE], dod_buf[MAX_DATE];
                printf("Found: [%s]\n  Def: %s\n  DoB: %s  DoD: %s\n",
                       found->name, found->definition,
                       dateToString(found->dob, dob_buf),
                       dateToString(found->dod, dod_buf));
            } else printf("Not found.\n");
            break;
        }
        case 3: {
            TStack *sorted = sortNameStack(stk);
            printf("\n--- Stack sorted alphabetically ---\n");
            stackPrint(sorted);
            stackFree(sorted);
            stk = NULL;
            break;
        }
        case 4: {
            readLine("Name: ",       buf1, MAX_NAME);
            readLine("Definition: ", buf2, MAX_DEF);
            Date dob = readDate("DoB (YYYY-MM-DD): ");
            Date dod = readDate("DoD (YYYY-MM-DD): ");
            stk = addNameStack(stk, buf1, buf2, dob, dod);
            printf("Added. Stack now has %d items.\n", stackSize(stk));
            break;
        }
        case 5: {
            readLine("Name to delete: ", buf1, MAX_NAME);
            stk = deleteName(stk, buf1);
            break;
        }
        case 6: {
            readLine("Name to update: ",  buf1, MAX_NAME);
            readLine("New definition: ",  buf2, MAX_DEF);
            Date dob = readDate("New DoB: ");
            Date dod = readDate("New DoD: ");
            stk = updateStack(stk, buf1, buf2, dob, dod);
            printf("Updated.\n");
            break;
        }
        case 7: {
            TQueue *q = stackToQueue(stk);
            printf("\n--- Sorted Queue from Stack ---\n");
            queuePrint(q);
            queueFree(q);
            break;
        }
        case 8: {
            TList *list = stackToList(stk);
            printf("\n--- Sorted List from Stack ---\n");
            listPrint(list);
            listFree(list);
            break;
        }
        case 9: {
            TStack *ds = definitionStack(stk);
            printf("\n--- Stack by definition word count ---\n");
            stackPrint(ds);
            stackFree(ds);
            stk = NULL;
            break;
        }
        case 10: {
            TStack *shortS = NULL, *longS = NULL;
            pronunciationStack(stk, &shortS, &longS);
            printf("\n--- Short descriptions ---\n");
            stackPrint(shortS);
            printf("\n--- Long descriptions ---\n");
            stackPrint(longS);
            stackFree(shortS); stackFree(longS);
            break;
        }
        case 11: {
            char *small = getSmallest(stk);
            if (small) printf("Smallest definition: %s\n", small);
            else       printf("Stack is empty.\n");
            break;
        }
        case 12:
            continuousSearch(stk);
            break;
        case 13: {
            readLine("Name: ", buf1, MAX_NAME);
            bool killed = isPersonalityKilled(stk, buf1);
            printf("'%s' was %skilled.\n", buf1, killed ? "" : "NOT ");
            break;
        }
        case 14: {
            TStack *rev = recRevStack(stk);
            printf("\n--- Reversed Stack ---\n");
            stackPrint(rev);
            stackFree(rev);
            stk = NULL;
            break;
        }
        case 0:
            stackFree(stk);
            return;
        default: printf("Invalid choice.\n");
        }
        stackFree(stk);
    } while (choice != 0);
}

/* --- BST menu --- */
static void menuBST(void) {
    int choice;
    char buf1[MAX_NAME], buf2[MAX_DEF], buf3[MAX_NAME];
    TTree *tree = fillTree(DB_FILE);

    do {
        printf("\n=== BST Menu ===\n");
        printf(" 1. In-order traversal\n");
        printf(" 2. Pre-order traversal\n");
        printf(" 3. Post-order traversal\n");
        printf(" 4. Height & Size\n");
        printf(" 5. Search personality\n");
        printf(" 6. Add personality\n");
        printf(" 7. Delete personality\n");
        printf(" 8. Update personality\n");
        printf(" 9. Lowest Common Ancestor\n");
        printf("10. Count nodes in name-length range\n");
        printf("11. In-order successor\n");
        printf("12. Mirror the tree\n");
        printf("13. Is balanced?\n");
        printf(" 0. Back\n");
        printf("Choice: "); scanf("%d", &choice); getchar();

        switch (choice) {
        case 1:
            printf("\n--- In-order ---\n");
            traversalBSTinOrder(tree);
            break;
        case 2:
            printf("\n--- Pre-order ---\n");
            traversalBSTpreOrder(tree);
            break;
        case 3:
            printf("\n--- Post-order ---\n");
            traversalBSTpostOrder(tree);
            break;
        case 4:
            heightSizeBST(tree);
            break;
        case 5:
            readLine("Name: ", buf1, MAX_NAME);
            getInfoNameTree(tree, buf1);
            break;
        case 6: {
            readLine("Name: ",       buf1, MAX_NAME);
            readLine("Definition: ", buf2, MAX_DEF);
            Date dob = readDate("DoB (YYYY-MM-DD): ");
            Date dod = readDate("DoD (YYYY-MM-DD): ");
            tree = addNameBST(tree, buf1, buf2, dob, dod);
            printf("Added.\n");
            break;
        }
        case 7:
            readLine("Name to delete: ", buf1, MAX_NAME);
            tree = deleteNameBST(tree, buf1);
            printf("Deleted.\n");
            break;
        case 8: {
            readLine("Name to update: ", buf1, MAX_NAME);
            readLine("New definition: ", buf2, MAX_DEF);
            Date dob = readDate("New DoB: ");
            Date dod = readDate("New DoD: ");
            tree = updateNameBST(tree, buf1, buf2, dob, dod);
            break;
        }
        case 9: {
            readLine("First name: ",  buf1, MAX_NAME);
            readLine("Second name: ", buf3, MAX_NAME);
            TTree *lca = lowestCommonAncestor(tree, buf1, buf3);
            if (lca) printf("LCA: %s\n", lca->name);
            else     printf("Not found.\n");
            break;
        }
        case 10: {
            int l, h;
            printf("Min name length: "); scanf("%d", &l); getchar();
            printf("Max name length: "); scanf("%d", &h); getchar();
            printf("Count: %d\n", countNodesRange(tree, l, h));
            break;
        }
        case 11: {
            readLine("Name: ", buf1, MAX_NAME);
            TTree *succ = inOrderSuccessor(tree, buf1);
            if (succ) printf("In-order successor: %s\n", succ->name);
            else      printf("No successor found.\n");
            break;
        }
        case 12:
            tree = BSTMirror(tree);
            printf("Tree mirrored.\n");
            break;
        case 13:
            printf("Tree is %sbalanced.\n", isBalancedBST(tree) ? "" : "NOT ");
            break;
        case 0:
            treeFree(tree);
            return;
        default: printf("Invalid choice.\n");
        }
    } while (choice != 0);
    treeFree(tree);
}

/* --- Recursion menu --- */
static void menuRecursion(void) {
    int choice;
    char buf1[MAX_NAME];
    do {
        printf("\n=== Recursion Menu ===\n");
        printf(" 1. Count occurrences of a name in file\n");
        printf(" 2. Remove lines containing a word\n");
        printf(" 3. Replace personality dates\n");
        printf(" 4. Print name permutations\n");
        printf(" 5. Print subsequences of a word\n");
        printf(" 6. Events in date range\n");
        printf(" 7. Count distinct palindrome subsequences\n");
        printf(" 8. Is a word a palindrome? (recursive)\n");
        printf(" 0. Back\n");
        printf("Choice: "); scanf("%d", &choice); getchar();

        switch (choice) {
        case 1: {
            readLine("Name to count: ", buf1, MAX_NAME);
            int cnt = countOccurrence(DB_FILE, buf1);
            printf("'%s' appears %d time(s) in the file.\n", buf1, cnt);
            break;
        }
        case 2: {
            readLine("Word to remove: ", buf1, MAX_NAME);
            if (removeOccurrence(DB_FILE, buf1)) {
                loadAll();
                printf("Lines containing '%s' removed.\n", buf1);
            }
            break;
        }
        case 3: {
            readLine("Name: ", buf1, MAX_NAME);
            Date dob = readDate("New DoB: ");
            Date dod = readDate("New DoD: ");
            if (replaceOccurrence(DB_FILE, buf1, dob, dod)) {
                loadAll();
                printf("Replaced.\n");
            }
            break;
        }
        case 4: {
            readLine("Name to permute: ", buf1, MAX_NAME);
            namePermutation(buf1);
            break;
        }
        case 5: {
            readLine("Word: ", buf1, MAX_NAME);
            subseqName(buf1);
            break;
        }
        case 6: {
            Date d1 = readDate("Start date (YYYY-MM-DD): ");
            Date d2 = readDate("End   date (YYYY-MM-DD): ");
            longestSubyear(g_events, d1, d2);
            break;
        }
        case 7: {
            readLine("Event/word: ", buf1, MAX_NAME);
            int cnt = distinctSubseqWord(buf1);
            printf("Distinct palindrome subsequences: %d\n", cnt);
            break;
        }
        case 8: {
            readLine("Word to check: ", buf1, MAX_NAME);
            printf("'%s' is %sa palindrome.\n", buf1,
                   isPalindromeWord(buf1) ? "" : "NOT ");
            break;
        }
        case 0: break;
        default: printf("Invalid choice.\n");
        }
    } while (choice != 0);
}

int main(void) {
    printf("╔══════════════════════════════════════════╗\n");
    printf("║  Algeria History Database – NSCS 2025   ║\n");
    printf("╚══════════════════════════════════════════╝\n");

    loadAll();

    int choice;
    do {
        printf("\n=== MAIN MENU ===\n");
        printf(" 1. Linked Lists\n");
        printf(" 2. Queues\n");
        printf(" 3. Stacks\n");
        printf(" 4. Binary Search Tree\n");
        printf(" 5. Recursion\n");
        printf(" 6. Reload database from file\n");
        printf(" 0. Exit\n");
        printf("Choice: "); scanf("%d", &choice); getchar();

        switch (choice) {
        case 1: menuList();      break;
        case 2: menuQueue();     break;
        case 3: menuStack();     break;
        case 4: menuBST();       break;
        case 5: menuRecursion(); break;
        case 6: loadAll();       break;
        case 0: printf("Goodbye!\n"); break;
        default: printf("Invalid choice.\n");
        }
    } while (choice != 0);

    listFree(g_pers);
    listDateFree(g_dates);
    listEventFree(g_events);
    return 0;
}
