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

static void readLine(const char *prompt, char *buffer, int size) {
    printf("%s", prompt);
    fflush(stdout);
    if (!fgets(buffer, size, stdin)) buffer[0] = '\0';
    buffer[strcspn(buffer, "\n")] = '\0';
    trimWhitespace(buffer);
}

static Date readDate(const char *prompt) {
    char date_buffer[MAX_DATE];
    readLine(prompt, date_buffer, MAX_DATE);
    return parseDate(date_buffer);
}

/* --- Linked List menu --- */
static void menuList(void) {
    int menu_choice;
    char name_buffer[MAX_NAME], def_buffer[MAX_DEF];
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
        printf("Choice: "); scanf("%d", &menu_choice); getchar();

        switch (menu_choice) {
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
            TList *sorted_list = sortWord(g_pers);
            printf("\n--- Alphabetically Sorted ---\n");
            listPrint(sorted_list);
            listFree(sorted_list);
            break;
        }
        case 5: {
            TList *sorted_list = sortWord2(g_pers);
            printf("\n--- Sorted by Name Length ---\n");
            listPrint(sorted_list);
            listFree(sorted_list);
            break;
        }
        case 6: {
            TList *sorted_list = sortPersonality(g_pers, g_dates);
            printf("\n--- Sorted by Age ---\n");
            listPrint(sorted_list);
            listFree(sorted_list);
            break;
        }
        case 7: {
            Date birth_date = readDate("Enter Date of Birth (YYYY-MM-DD): ");
            getInfoByDates(g_pers, g_dates, birth_date);
            break;
        }
        case 8: {
            Date death_date = readDate("Enter Date of Death (YYYY-MM-DD): ");
            getInfoByDates2(g_pers, g_dates, death_date);
            break;
        }
        case 9: {
            TList *palindrome_list = palindromeName(g_pers);
            printf("\n--- Palindrome Names Found ---\n");
            listPrint(palindrome_list);
            listFree(palindrome_list);
            break;
        }
        case 10: {
            readLine("Enter keyword or year: ", name_buffer, MAX_NAME);
            TList *similar = similarPersonality(g_pers, name_buffer);
            printf("\n--- Similar Personalities ---\n");
            listPrint(similar);
            listFree(similar);
            break;
        }
        case 11: {
            TBiList *bidirectional = mergeNodes(g_pers, g_dates);
            printf("\n--- Bidirectional List ---\n");
            biListPrint(bidirectional);
            biListFree(bidirectional);
            break;
        }
        case 12: {
            TCircList *circular = merge2Nodes(g_pers, g_dates);
            printf("\n--- Circular List (first 20 nodes) ---\n");
            if (circular) {
                char dob_string[MAX_DATE], dod_string[MAX_DATE];
                TCircList *current = circular;
                int node_count = 0;
                do {
                    printf("  [%s]  DoB: %s  DoD: %s\n",
                           current->name,
                           dateToString(current->dob, dob_string),
                           dateToString(current->dod, dod_string));
                    current = current->next;
                    node_count++;
                } while (current != circular && node_count < 20);
                circListFree(circular);
            }
            break;
        }
        case 13: {
            readLine("Name: ",       name_buffer, MAX_NAME);
            readLine("Definition: ", def_buffer, MAX_DEF);
            Date birth_date = readDate("Date of Birth (YYYY-MM-DD): ");
            Date death_date = readDate("Date of Death (YYYY-MM-DD): ");
            if (addPersonality(DB_FILE, &g_pers, &g_dates, name_buffer, birth_date, death_date, def_buffer))
                printf("Added successfully.\n");
            break;
        }
        case 14: {
            readLine("Name to delete: ", name_buffer, MAX_NAME);
            if (deletepersonality(DB_FILE, &g_pers, &g_dates, name_buffer))
                printf("Deleted successfully.\n");
            break;
        }
        case 15: {
            readLine("Name to update: ",    name_buffer, MAX_NAME);
            readLine("New definition: ",    def_buffer, MAX_DEF);
            Date new_birth_date = readDate("New DoB (YYYY-MM-DD, blank=no change): ");
            Date new_death_date = readDate("New DoD (YYYY-MM-DD, blank=no change): ");
            if (updatePersonality(DB_FILE, &g_pers, &g_dates, name_buffer, def_buffer, new_birth_date, new_death_date))
                printf("Updated successfully.\n");
            break;
        }
        case 16: {
            readLine("Event name: ", name_buffer, MAX_NAME);
            Date event_date = readDate("Event date (YYYY-MM-DD): ");
            if (addEvents(DB_FILE, &g_events, name_buffer, event_date))
                printf("Event added.\n");
            break;
        }
        case 0: break;
        default: printf("Invalid choice.\n");
        }
    } while (menu_choice != 0);
}

/* --- Queue menu --- */
static void menuQueue(void) {
    int menu_choice;
    do {
        printf("\n=== Queue Menu ===\n");
        printf(" 1. sName   – queue sorted by word count in name\n");
        printf(" 2. ageP    – queue sorted by age\n");
        printf(" 3. toQueue – merged list to queue\n");
        printf(" 0. Back\n");
        printf("Choice: "); scanf("%d", &menu_choice); getchar();

        switch (menu_choice) {
        case 1: {
            TQueue *queue = sName(g_pers);
            printf("\n--- Queue sorted by name word count ---\n");
            queuePrint(queue);
            queueFree(queue);
            break;
        }
        case 2: {
            TQueue *queue = ageP(g_dates);
            printf("\n--- Queue sorted by age ---\n");
            queuePrint(queue);
            queueFree(queue);
            break;
        }
        case 3: {
            TBiList *bidirectional = mergeNodes(g_pers, g_dates);
            TQueue  *queue  = toQueue(bidirectional);
            printf("\n--- Queue from merged list ---\n");
            queuePrint(queue);
            queueFree(queue);
            biListFree(bidirectional);
            break;
        }
        case 0: break;
        default: printf("Invalid choice.\n");
        }
    } while (menu_choice != 0);
}

/* --- Stack menu --- */
static void menuStack(void) {
    int menu_choice;
    char name_buffer[MAX_NAME], def_buffer[MAX_DEF];
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
        printf("Choice: "); scanf("%d", &menu_choice); getchar();

        TBiList *bidirectional  = mergeNodes(g_pers, g_dates);
        TStack  *stack = toStack(bidirectional);
        biListFree(bidirectional);

        switch (menu_choice) {
        case 1:
            printf("\n--- Stack ---\n");
            stackPrint(stack);
            break;
        case 2: {
            readLine("Name to search: ", name_buffer, MAX_NAME);
            TStack *found_person = getInfoPersonality(stack, name_buffer);
            if (found_person) {
                char dob_string[MAX_DATE], dod_string[MAX_DATE];
                printf("Found: [%s]\n  Def: %s\n  DoB: %s  DoD: %s\n",
                       found_person->name, found_person->definition,
                       dateToString(found_person->dob, dob_string),
                       dateToString(found_person->dod, dod_string));
            } else printf("Not found.\n");
            break;
        }
        case 3: {
            TStack *sorted_stack = sortNameStack(stack);
            printf("\n--- Stack sorted alphabetically ---\n");
            stackPrint(sorted_stack);
            stackFree(sorted_stack);
            stack = NULL;
            break;
        }
        case 4: {
            readLine("Name: ",       name_buffer, MAX_NAME);
            readLine("Definition: ", def_buffer, MAX_DEF);
            Date birth_date = readDate("DoB (YYYY-MM-DD): ");
            Date death_date = readDate("DoD (YYYY-MM-DD): ");
            stack = addNameStack(stack, name_buffer, def_buffer, birth_date, death_date);
            printf("Added. Stack now has %d items.\n", stackSize(stack));
            break;
        }
        case 5: {
            readLine("Name to delete: ", name_buffer, MAX_NAME);
            stack = deleteName(stack, name_buffer);
            break;
        }
        case 6: {
            readLine("Name to update: ",  name_buffer, MAX_NAME);
            readLine("New definition: ",  def_buffer, MAX_DEF);
            Date new_birth_date = readDate("New DoB: ");
            Date new_death_date = readDate("New DoD: ");
            stack = updateStack(stack, name_buffer, def_buffer, new_birth_date, new_death_date);
            printf("Updated.\n");
            break;
        }
        case 7: {
            TQueue *queue = stackToQueue(stack);
            printf("\n--- Sorted Queue from Stack ---\n");
            queuePrint(queue);
            queueFree(queue);
            break;
        }
        case 8: {
            TList *linked_list = stackToList(stack);
            printf("\n--- Sorted List from Stack ---\n");
            listPrint(linked_list);
            listFree(linked_list);
            break;
        }
        case 9: {
            TStack *definition_sorted_stack = definitionStack(stack);
            printf("\n--- Stack by definition word count ---\n");
            stackPrint(definition_sorted_stack);
            stackFree(definition_sorted_stack);
            stack = NULL;
            break;
        }
        case 10: {
            TStack *short_stack = NULL, *long_stack = NULL;
            pronunciationStack(stack, &short_stack, &long_stack);
            printf("\n--- Short descriptions ---\n");
            stackPrint(short_stack);
            printf("\n--- Long descriptions ---\n");
            stackPrint(long_stack);
            stackFree(short_stack); stackFree(long_stack);
            break;
        }
        case 11: {
            char *smallest_definition = getSmallest(stack);
            if (smallest_definition) printf("Smallest definition: %s\n", smallest_definition);
            else       printf("Stack is empty.\n");
            break;
        }
        case 12:
            continuousSearch(stack);
            break;
        case 13: {
            readLine("Name: ", name_buffer, MAX_NAME);
            bool was_killed = isPersonalityKilled(stack, name_buffer);
            printf("'%s' was %skilled.\n", name_buffer, was_killed ? "" : "NOT ");
            break;
        }
        case 14: {
            TStack *reversed_stack = recRevStack(stack);
            printf("\n--- Reversed Stack ---\n");
            stackPrint(reversed_stack);
            stackFree(reversed_stack);
            stack = NULL;
            break;
        }
        case 0:
            stackFree(stack);
            return;
        default: printf("Invalid choice.\n");
        }
        stackFree(stack);
    } while (menu_choice != 0);
}

/* --- BST menu --- */
static void menuBST(void) {
    int menu_choice;
    char name_buffer[MAX_NAME], def_buffer[MAX_DEF], name_buffer2[MAX_NAME];
    TTree *bst = fillTree(DB_FILE);

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
        printf("Choice: "); scanf("%d", &menu_choice); getchar();

        switch (menu_choice) {
        case 1:
            printf("\n--- In-order ---\n");
            traversalBSTinOrder(bst);
            break;
        case 2:
            printf("\n--- Pre-order ---\n");
            traversalBSTpreOrder(bst);
            break;
        case 3:
            printf("\n--- Post-order ---\n");
            traversalBSTpostOrder(bst);
            break;
        case 4:
            heightSizeBST(bst);
            break;
        case 5:
            readLine("Name: ", name_buffer, MAX_NAME);
            getInfoNameTree(bst, name_buffer);
            break;
        case 6: {
            readLine("Name: ",       name_buffer, MAX_NAME);
            readLine("Definition: ", def_buffer, MAX_DEF);
            Date birth_date = readDate("DoB (YYYY-MM-DD): ");
            Date death_date = readDate("DoD (YYYY-MM-DD): ");
            bst = addNameBST(bst, name_buffer, def_buffer, birth_date, death_date);
            printf("Added.\n");
            break;
        }
        case 7:
            readLine("Name to delete: ", name_buffer, MAX_NAME);
            bst = deleteNameBST(bst, name_buffer);
            printf("Deleted.\n");
            break;
        case 8: {
            readLine("Name to update: ", name_buffer, MAX_NAME);
            readLine("New definition: ", def_buffer, MAX_DEF);
            Date new_birth_date = readDate("New DoB: ");
            Date new_death_date = readDate("New DoD: ");
            bst = updateNameBST(bst, name_buffer, def_buffer, new_birth_date, new_death_date);
            break;
        }
        case 9: {
            readLine("First name: ",  name_buffer, MAX_NAME);
            readLine("Second name: ", name_buffer2, MAX_NAME);
            TTree *lowest_common_ancestor = lowestCommonAncestor(bst, name_buffer, name_buffer2);
            if (lowest_common_ancestor) printf("LCA: %s\n", lowest_common_ancestor->name);
            else     printf("Not found.\n");
            break;
        }
        case 10: {
            int min_length, max_length;
            printf("Min name length: "); scanf("%d", &min_length); getchar();
            printf("Max name length: "); scanf("%d", &max_length); getchar();
            printf("Count: %d\n", countNodesRange(bst, min_length, max_length));
            break;
        }
        case 11: {
            readLine("Name: ", name_buffer, MAX_NAME);
            TTree *successor = inOrderSuccessor(bst, name_buffer);
            if (successor) printf("In-order successor: %s\n", successor->name);
            else      printf("No successor found.\n");
            break;
        }
        case 12:
            bst = BSTMirror(bst);
            printf("Tree mirrored.\n");
            break;
        case 13:
            printf("Tree is %sbalanced.\n", isBalancedBST(bst) ? "" : "NOT ");
            break;
        case 0:
            treeFree(bst);
            return;
        default: printf("Invalid choice.\n");
        }
    } while (menu_choice != 0);
    treeFree(bst);
}

/* --- Recursion menu --- */
static void menuRecursion(void) {
    int menu_choice;
    char name_buffer[MAX_NAME];
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
        printf("Choice: "); scanf("%d", &menu_choice); getchar();

        switch (menu_choice) {
        case 1: {
            readLine("Name to count: ", name_buffer, MAX_NAME);
            int count = countOccurrence(DB_FILE, name_buffer);
            printf("'%s' appears %d time(s) in the file.\n", name_buffer, count);
            break;
        }
        case 2: {
            readLine("Word to remove: ", name_buffer, MAX_NAME);
            if (removeOccurrence(DB_FILE, name_buffer)) {
                loadAll();
                printf("Lines containing '%s' removed.\n", name_buffer);
            }
            break;
        }
        case 3: {
            readLine("Name: ", name_buffer, MAX_NAME);
            Date new_birth_date = readDate("New DoB: ");
            Date new_death_date = readDate("New DoD: ");
            if (replaceOccurrence(DB_FILE, name_buffer, new_birth_date, new_death_date)) {
                loadAll();
                printf("Replaced.\n");
            }
            break;
        }
        case 4: {
            readLine("Name to permute: ", name_buffer, MAX_NAME);
            namePermutation(name_buffer);
            break;
        }
        case 5: {
            readLine("Word: ", name_buffer, MAX_NAME);
            subseqName(name_buffer);
            break;
        }
        case 6: {
            Date start_date = readDate("Start date (YYYY-MM-DD): ");
            Date end_date = readDate("End   date (YYYY-MM-DD): ");
            longestSubyear(g_events, start_date, end_date);
            break;
        }
        case 7: {
            readLine("Event/word: ", name_buffer, MAX_NAME);
            int distinct_count = distinctSubseqWord(name_buffer);
            printf("Distinct palindrome subsequences: %d\n", distinct_count);
            break;
        }
        case 8: {
            readLine("Word to check: ", name_buffer, MAX_NAME);
            printf("'%s' is %sa palindrome.\n", name_buffer,
                   isPalindromeWord(name_buffer) ? "" : "NOT ");
            break;
        }
        case 0: break;
        default: printf("Invalid choice.\n");
        }
    } while (menu_choice != 0);
}

int main(void) {
    printf("╔══════════════════════════════════════════╗\n");
    printf("║  Algeria History Database – NSCS 2025   ║\n");
    printf("╚══════════════════════════════════════════╝\n");

    loadAll();

    int menu_choice;
    do {
        printf("\n=== MAIN MENU ===\n");
        printf(" 1. Linked Lists\n");
        printf(" 2. Queues\n");
        printf(" 3. Stacks\n");
        printf(" 4. Binary Search Tree\n");
        printf(" 5. Recursion\n");
        printf(" 6. Reload database from file\n");
        printf(" 0. Exit\n");
        printf("Choice: "); scanf("%d", &menu_choice); getchar();

        switch (menu_choice) {
        case 1: menuList();      break;
        case 2: menuQueue();     break;
        case 3: menuStack();     break;
        case 4: menuBST();       break;
        case 5: menuRecursion(); break;
        case 6: loadAll();       break;
        case 0: printf("Goodbye!\n"); break;
        default: printf("Invalid choice.\n");
        }
    } while (menu_choice != 0);

    listFree(g_pers);
    listDateFree(g_dates);
    listEventFree(g_events);
    return 0;
}
