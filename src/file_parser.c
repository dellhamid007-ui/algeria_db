#include "file_parser.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

LineType classifyLine(const char *line) {
    if (!line) return LINE_EMPTY;
    while (*line && isspace((unsigned char)*line)) line++;
    if (*line == '\0')  return LINE_EMPTY;
    if (*line == '#')   return LINE_COMMENT;
    if (*line == '{')   return LINE_DATE;
    if (strchr(line, '=')) return LINE_PERSONALITY;
    if (strchr(line, ':')) return LINE_EVENT;
    return LINE_UNKNOWN;
}

/* Extract first {...} from src starting at *pos */
static bool extractBraces(const char *src, int *pos, char *out, int out_sz) {
    int len = (int)strlen(src);
    while (*pos < len && src[*pos] != '{') (*pos)++;
    if (*pos >= len) return false;
    (*pos)++;
    int i = 0;
    while (*pos < len && src[*pos] != '}' && i < out_sz - 1)
        out[i++] = src[(*pos)++];
    out[i] = '\0';
    if (*pos < len) (*pos)++;
    return i > 0;
}

bool parsePersonalityLine(const char *line,
                           char *name_out,
                           char *def_out,
                           Date *dob_out,
                           Date *dod_out) {
    if (!line || !name_out || !def_out || !dob_out || !dod_out) return false;

    *dob_out = (Date){0,0,0};
    *dod_out = (Date){0,0,0};

    const char *eq = strchr(line, '=');
    if (!eq) return false;

    int name_len = (int)(eq - line);
    if (name_len <= 0 || name_len >= MAX_NAME) return false;
    strncpy(name_out, line, name_len);
    name_out[name_len] = '\0';
    trimWhitespace(name_out);

    char rest[MAX_LINE];
    strncpy(rest, eq + 1, MAX_LINE - 1);
    rest[MAX_LINE - 1] = '\0';

    char date_buf[MAX_DATE];
    int pos = 0;
    if (extractBraces(rest, &pos, date_buf, MAX_DATE))
        *dob_out = parseDate(date_buf);
    if (extractBraces(rest, &pos, date_buf, MAX_DATE))
        *dod_out = parseDate(date_buf);

    char def_raw[MAX_DEF];
    int brk = 0;
    while (rest[brk] && rest[brk] != '{') brk++;
    if (brk >= MAX_DEF) brk = MAX_DEF - 1;
    strncpy(def_raw, rest, brk);
    def_raw[brk] = '\0';
    trimWhitespace(def_raw);
    strncpy(def_out, def_raw, MAX_DEF - 1);
    def_out[MAX_DEF - 1] = '\0';

    return name_out[0] != '\0';
}

bool parseEventLine(const char *line,
                    char *event_name_out,
                    Date *date_out) {
    if (!line || !event_name_out || !date_out) return false;
    *date_out = (Date){0,0,0};

    const char *colon = strchr(line, ':');
    if (!colon) return false;

    int name_len = (int)(colon - line);
    if (name_len <= 0 || name_len >= MAX_NAME) return false;
    strncpy(event_name_out, line, name_len);
    event_name_out[name_len] = '\0';
    trimWhitespace(event_name_out);

    char date_str[MAX_DATE];
    strncpy(date_str, colon + 1, MAX_DATE - 1);
    date_str[MAX_DATE - 1] = '\0';
    trimWhitespace(date_str);
    *date_out = parseDate(date_str);

    return event_name_out[0] != '\0';
}

bool parseDateLine(const char *line, Date *date_out) {
    if (!line || !date_out) return false;
    *date_out = (Date){0,0,0};

    const char *p = strchr(line, '{');
    if (!p) return false;
    p++;
    char buf[MAX_DATE];
    int i = 0;
    while (*p && *p != '}' && i < MAX_DATE - 1) buf[i++] = *p++;
    buf[i] = '\0';
    *date_out = parseDate(buf);
    return !dateIsNull(*date_out);
}

TList *loadPersonalities(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) { perror("loadPersonalities"); return NULL; }

    TList *head = NULL, *tail = NULL;
    char line[MAX_LINE];

    while (fgets(line, MAX_LINE, f)) {
        trimWhitespace(line);
        if (classifyLine(line) != LINE_PERSONALITY) continue;

        char name[MAX_NAME], def[MAX_DEF];
        Date dob, dod;
        if (!parsePersonalityLine(line, name, def, &dob, &dod)) continue;

        TList *node = (TList *)malloc(sizeof(TList));
        if (!node) continue;
        strncpy(node->name,       name, MAX_NAME - 1);
        strncpy(node->definition, def,  MAX_DEF  - 1);
        node->name[MAX_NAME-1] = '\0';
        node->definition[MAX_DEF-1] = '\0';
        node->next = NULL;

        if (!head) { head = tail = node; }
        else       { tail->next = node; tail = node; }
    }
    fclose(f);
    return head;
}

TListDate *loadPersonalityDates(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) { perror("loadPersonalityDates"); return NULL; }

    TListDate *head = NULL, *tail = NULL;
    char line[MAX_LINE];

    while (fgets(line, MAX_LINE, f)) {
        trimWhitespace(line);
        if (classifyLine(line) != LINE_PERSONALITY) continue;

        char name[MAX_NAME], def[MAX_DEF];
        Date dob, dod;
        if (!parsePersonalityLine(line, name, def, &dob, &dod)) continue;

        TListDate *node = (TListDate *)malloc(sizeof(TListDate));
        if (!node) continue;
        strncpy(node->name, name, MAX_NAME - 1);
        node->name[MAX_NAME-1] = '\0';
        node->dob  = dob;
        node->dod  = dod;
        node->next = NULL;

        if (!head) { head = tail = node; }
        else       { tail->next = node; tail = node; }
    }
    fclose(f);
    return head;
}

TListEvent *loadEvents(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) { perror("loadEvents"); return NULL; }

    TListEvent *head = NULL, *tail = NULL;
    char line[MAX_LINE];

    while (fgets(line, MAX_LINE, f)) {
        trimWhitespace(line);
        if (classifyLine(line) != LINE_EVENT) continue;

        char ename[MAX_NAME];
        Date date;
        if (!parseEventLine(line, ename, &date)) continue;

        TListEvent *node = (TListEvent *)malloc(sizeof(TListEvent));
        if (!node) continue;
        strncpy(node->event_name, ename, MAX_NAME - 1);
        node->event_name[MAX_NAME-1] = '\0';
        node->date = date;
        node->next = NULL;

        if (!head) { head = tail = node; }
        else       { tail->next = node; tail = node; }
    }
    fclose(f);
    return head;
}

bool rewriteFile(const char *filename,
                 TList      *personalities,
                 TListDate  *dates,
                 TListEvent *events) {
    FILE *f = fopen(filename, "w");
    if (!f) { perror("rewriteFile"); return false; }

    fprintf(f, "# Algeria History Database\n");
    fprintf(f, "# FORMAT: NAME = DEFINITION {DOB} {DOD}  |  EVENT : DATE  |  {DATE}\n\n");

    TList    *p = personalities;
    TListDate *d = dates;
    while (p && d) {
        char dob_buf[MAX_DATE], dod_buf[MAX_DATE];
        dateToString(d->dob, dob_buf);
        dateToString(d->dod, dod_buf);

        if (!dateIsNull(d->dob) && !dateIsNull(d->dod))
            fprintf(f, "%s = %s {%s} {%s}\n", p->name, p->definition, dob_buf, dod_buf);
        else if (!dateIsNull(d->dob))
            fprintf(f, "%s = %s {%s}\n", p->name, p->definition, dob_buf);
        else
            fprintf(f, "%s = %s\n", p->name, p->definition);

        p = p->next;
        d = d->next;
    }

    fprintf(f, "\n");

    TListEvent *ev = events;
    while (ev) {
        char date_buf[MAX_DATE];
        dateToString(ev->date, date_buf);
        fprintf(f, "%s : %s\n", ev->event_name, date_buf);
        ev = ev->next;
    }

    fclose(f);
    return true;
}
