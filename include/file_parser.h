#ifndef FILE_PARSER_H
#define FILE_PARSER_H

#include "types.h"

/*
 * File format (algeria_history.txt):
 *   Personality: NAME = DEFINITION {YYYY-MM-DD} {YYYY-MM-DD}
 *   Event:       EVENT NAME : YYYY-MM-DD
 *   Date:        {YYYY-MM-DD}
 *   Lines starting with '#' are comments, empty lines ignored.
 */

typedef enum {
    LINE_PERSONALITY,
    LINE_EVENT,
    LINE_DATE,
    LINE_COMMENT,
    LINE_EMPTY,
    LINE_UNKNOWN
} LineType;

LineType classifyLine(const char *line);

bool parsePersonalityLine(const char *line,
                          char *name_out,
                          char *def_out,
                          Date *dob_out,
                          Date *dod_out);

bool parseEventLine(const char *line,
                    char *event_name_out,
                    Date *date_out);

bool parseDateLine(const char *line, Date *date_out);

TList      *loadPersonalities(const char *filename);
TListDate  *loadPersonalityDates(const char *filename);
TListEvent *loadEvents(const char *filename);

bool rewriteFile(const char *filename,
                 TList      *personalities,
                 TListDate  *dates,
                 TListEvent *events);

#endif
