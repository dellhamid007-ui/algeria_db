#include "types.h"

/* --- Date helpers --- */

Date parseDate(const char *str) {
    Date d = {0, 0, 0};
    if (!str || *str == '\0') return d;
    sscanf(str, "%d-%d-%d", &d.year, &d.month, &d.day);
    return d;
}

char *dateToString(Date d, char *buf) {
    if (!buf) return NULL;
    if (d.year == 0)
        strcpy(buf, "unknown");
    else if (d.month == 0)
        sprintf(buf, "%04d", d.year);
    else if (d.day == 0)
        sprintf(buf, "%04d-%02d", d.year, d.month);
    else
        sprintf(buf, "%04d-%02d-%02d", d.year, d.month, d.day);
    return buf;
}

int dateCmp(Date a, Date b) {
    if (a.year  != b.year)  return a.year  - b.year;
    if (a.month != b.month) return a.month - b.month;
    return a.day - b.day;
}

int dateIsNull(Date d) {
    return d.year == 0;
}

int ageFromDates(Date dob, Date dod) {
    if (dateIsNull(dob) || dateIsNull(dod)) return -1;
    int age = dod.year - dob.year;
    if (dod.month < dob.month ||
        (dod.month == dob.month && dod.day < dob.day))
        age--;
    return age;
}

/* --- String helpers --- */

void strToLower(char *dst, const char *src) {
    int i = 0;
    while (src[i]) {
        dst[i] = (char)tolower((unsigned char)src[i]);
        i++;
    }
    dst[i] = '\0';
}

int isPalindrome(const char *str) {
    int len = (int)strlen(str);
    int l = 0, r = len - 1;
    while (l < r) {
        while (l < r && !isalpha((unsigned char)str[l])) l++;
        while (l < r && !isalpha((unsigned char)str[r])) r--;
        if (tolower((unsigned char)str[l]) !=
            tolower((unsigned char)str[r])) return 0;
        l++; r--;
    }
    return 1;
}

void trimWhitespace(char *s) {
    if (!s) return;
    char *p = s;
    while (*p && isspace((unsigned char)*p)) p++;
    if (p != s) memmove(s, p, strlen(p) + 1);
    int len = (int)strlen(s);
    while (len > 0 && isspace((unsigned char)s[len-1]))
        s[--len] = '\0';
}
