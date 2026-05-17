# Standard Library Functions Used in the Project

This document lists the standard C library functions (excluding those from `<stdio.h>` and `<stdlib.h>`) that are used in the `algeria_db` project, along with brief descriptions of what each function does.

## `<ctype.h>` (Character Classification and Conversion)

- **`isalpha(int c)`**  
  Checks if the character `c` is an alphabetic letter (A-Z or a-z). Returns non-zero if true, 0 otherwise.

- **`isspace(int c)`**  
  Checks if the character `c` is a whitespace character (space, tab, newline, etc.). Returns non-zero if true, 0 otherwise.

- **`tolower(int c)`**  
  Converts the uppercase letter `c` to its lowercase equivalent. If `c` is not uppercase, returns `c` unchanged.

- **`toupper(int c)`**  
  Converts the lowercase letter `c` to its uppercase equivalent. If `c` is not lowercase, returns `c` unchanged.

## `<string.h>` (String Manipulation)

- **`strcspn(const char *s, const char *reject)`**  
  Computes the length of the initial segment of string `s` that consists of characters not in the string `reject`.

- **`strlen(const char *s)`**  
  Returns the length of the null-terminated string `s` (excluding the null terminator).

- **`strncat(char *dest, const char *src, size_t n)`**  
  Appends up to `n` characters from the string `src` to the end of the string `dest`, ensuring null-termination.

- **`strncmp(const char *s1, const char *s2, size_t n)`**  
  Compares up to `n` characters of the strings `s1` and `s2`. Returns 0 if equal, negative if `s1` < `s2`, positive if `s1` > `s2`.

- **`strstr(const char *haystack, const char *needle)`**  
  Locates the first occurrence of the substring `needle` in the string `haystack`. Returns a pointer to the beginning of the substring, or NULL if not found.