#include "recursion.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int countRec(FILE *f, const char *name, char *line, int sz) {
    if (!fgets(line, sz, f)) return 0;
    int c = 0;
    char *p = line;
    while ((p = strcasestr(p, name)) != NULL) { c++; p++; }
    return c + countRec(f, name, line, sz);
}

int countOccurrence(const char *filename, const char *name) {
    FILE *f = fopen(filename, "r");
    if (!f) { perror("countOccurrence"); return -1; }
    char line[MAX_LINE];
    int total = countRec(f, name, line, MAX_LINE);
    fclose(f);
    return total;
}

static void removeRec(FILE *in, FILE *out, const char *word,
                      char *line, int sz) {
    if (!fgets(line, sz, in)) return;
    if (!strcasestr(line, word)) fputs(line, out);
    removeRec(in, out, word, line, sz);
}

bool removeOccurrence(const char *filename, const char *word) {
    char tmp[512];
    snprintf(tmp, sizeof(tmp), "%s.tmp", filename);
    FILE *in  = fopen(filename, "r");
    FILE *out = fopen(tmp, "w");
    if (!in || !out) { if(in) fclose(in); if(out) fclose(out); return false; }
    char line[MAX_LINE];
    removeRec(in, out, word, line, MAX_LINE);
    fclose(in); fclose(out);
    remove(filename);
    rename(tmp, filename);
    return true;
}

static void replaceRec(FILE *in, FILE *out, const char *name,
                       Date dob, Date dod, char *line, int sz) {
    if (!fgets(line, sz, in)) return;
    if (strcasestr(line, name) &&
        classifyLine(line) == LINE_PERSONALITY) {
        char pname[MAX_NAME], def[MAX_DEF];
        Date old_dob, old_dod;
        if (parsePersonalityLine(line, pname, def, &old_dob, &old_dod) &&
            strcasecmp(pname, name) == 0) {
            char dob_buf[MAX_DATE], dod_buf[MAX_DATE];
            dateToString(dob, dob_buf);
            dateToString(dod, dod_buf);
            fprintf(out, "%s = %s {%s} {%s}\n", pname, def, dob_buf, dod_buf);
            replaceRec(in, out, name, dob, dod, line, sz);
            return;
        }
    }
    fputs(line, out);
    replaceRec(in, out, name, dob, dod, line, sz);
}

bool replaceOccurrence(const char *filename, const char *name,
                       Date dob, Date dod) {
    char tmp[512];
    snprintf(tmp, sizeof(tmp), "%s.tmp", filename);
    FILE *in  = fopen(filename, "r");
    FILE *out = fopen(tmp, "w");
    if (!in || !out) { if(in) fclose(in); if(out) fclose(out); return false; }
    char line[MAX_LINE];
    replaceRec(in, out, name, dob, dod, line, MAX_LINE);
    fclose(in); fclose(out);
    remove(filename);
    rename(tmp, filename);
    return true;
}

static void permuteRec(char *name, int start, int len) {
    if (start == len - 1) { printf("  %s\n", name); return; }
    for (int i = start; i < len; i++) {
        char tmp = name[start]; name[start] = name[i]; name[i] = tmp;
        permuteRec(name, start + 1, len);
        tmp = name[start]; name[start] = name[i]; name[i] = tmp;
    }
}

void namePermutation(char *name) {
    if (!name) return;
    printf("Permutations of '%s':\n", name);
    permuteRec(name, 0, (int)strlen(name));
}

static void subseqRec(const char *word, char *current, int wi, int ci) {
    current[ci] = '\0';
    if (ci > 0) printf("  %s\n", current);
    for (int i = wi; word[i]; i++) {
        current[ci] = word[i];
        subseqRec(word, current, i + 1, ci + 1);
    }
}

void subseqName(const char *word) {
    if (!word) return;
    int len = (int)strlen(word);
    char *buf = (char *)malloc(len + 1);
    if (!buf) return;
    printf("Subsequences of '%s':\n", word);
    subseqRec(word, buf, 0, 0);
    free(buf);
}

static void longestSubyearRec(TListEvent *ev, Date date1, Date date2) {
    if (!ev) return;
    if (dateCmp(ev->date, date1) >= 0 && dateCmp(ev->date, date2) <= 0) {
        char buf[MAX_DATE];
        printf("  [%s] : %s\n", ev->event_name, dateToString(ev->date, buf));
    }
    longestSubyearRec(ev->next, date1, date2);
}

void longestSubyear(TListEvent *events, Date date1, Date date2) {
    printf("Events between %d and %d:\n", date1.year, date2.year);
    longestSubyearRec(events, date1, date2);
}

int distinctSubseqWord(const char *event) {
    if (!event) return 0;
    int len = (int)strlen(event);
    int count = 0;
    int total = 1 << len;
    for (int mask = 1; mask < total; mask++) {
        char sub[MAX_NAME];
        int k = 0;
        for (int i = 0; i < len; i++)
            if (mask & (1 << i)) sub[k++] = event[i];
        sub[k] = '\0';
        if (isPalindrome(sub)) count++;
    }
    return count;
}

static bool palRec(const char *s, int l, int r) {
    while (l < r && !isalpha((unsigned char)s[l])) l++;
    while (l < r && !isalpha((unsigned char)s[r])) r--;
    if (l >= r) return true;
    if (tolower((unsigned char)s[l]) != tolower((unsigned char)s[r]))
        return false;
    return palRec(s, l + 1, r - 1);
}

bool isPalindromeWord(const char *event) {
    if (!event || !*event) return false;
    return palRec(event, 0, (int)strlen(event) - 1);
}
