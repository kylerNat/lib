#ifndef __STR__
#define __STR__

#include <string.h> //might remove later
#include "misc.h"
#include "logging.h"
#include "memory.h"

struct strand
{
    char* str;
    int len;
};

#define init_strand(string) ((strand) {string, sizeof(string)-1})

bool operator==(strand str, int a)
{
    if(a == 0) return str.str==0;
    log_error("cannot compare strand and int");
    return 0;
}

define_printer(strand s, ("%.*s", s.len, s.str));

#define TAB 9 //tab character

#define isspace(a) ((a) == ' ' || (a) == TAB)

#define isnewline(a) ((a) == '\n' || (a) == '\r')
#define islowercase(a) (('a' <= (a) && (a) <= 'z'))
#define isuppercase(a) (('A' <= (a) && (a) <= 'Z'))
#define islowercasehex(a) (('a' <= (a) && (a) <= 'f'))
#define isuppercasehex(a) (('A' <= (a) && (a) <= 'F'))
#define iswordstart(a) (islowercase(a) || isuppercase(a) || (a) == '_')
#define iswordchar(a) (iswordstart(a) || ('0' <= (a) && (a) <= '9'))
#define isdigit(a) (('0' <= (a) && (a) <= '9'))
#define ishexdigit(a) (isdigit(a) || ('a' <= (a) && (a) <= 'f') || ('A' <= (a) && (a) <= 'F'))
#define isbit(a) (('0' <= (a) && (a) <= '9'))

void skip_whitespace_and_newline(char*& code)
{
    while(isspace(*code) || isnewline(*code)) code++;
}

void skip_newline(char*& code)
{
    while(isnewline(*code)) code++;
}

void goto_next_line(char*& code)
{
    while(*code++ != '\n');
}

int get_next_word(char* code)
{
    if(!iswordstart(*code)) return 0;
    int size = 1;
    while(iswordchar(code[size])) size++;
    return size;
}

strand scan_next_word(char*& code)
{
    char* word = code;
    int word_len = get_next_word(code);
    code += word_len;
    return {word, word_len};
}

int get_line(char* code)
{
    int size = 0;
    while(!isnewline(code[size])) size++;
    return size;
}

//returns the start of the current line
char* get_line_start(char* code)
{
    while(!isnewline(*code)) code--;
    return code+1;
}

bool equal(char* a, char* b)
{
    return a && b && strcmp(a, b) == 0;
}

bool equal(char* a, char* b, int size)
{
    return a && b && strncmp(a, b, size) == 0;
}

bool equal(strand a, strand b)
{
    return a.len==b.len && strncmp(a.str, b.str, a.len) == 0;
}

int find(char* code, char c)
{
    int i = 0;
    while(code[i] != c) i++;
    return i;
}

int find(char* code, char* s)
{
    int i = 0;
    while(!equal(code+i, s)) i++;
    return i;
}

struct concat_helper
{
    char* s;
    int len;
};

concat_helper init_concat_helper()
{
    concat_helper helper;
    helper.s = (char*) malloc(1024); //TODO: this is shitty, fix it
    *helper.s = 0;
    helper.len = 0;
    return helper;
}

concat_helper operator,(concat_helper helper, char* s)
{
    strcat_s(helper.s, available_free_memory(), s);
    helper.len += strlen(s);
    return helper;
}

concat_helper operator,(concat_helper helper, const char* s)
{
    strcat_s(helper.s, available_free_memory(), s);
    helper.len += strlen(s);
    return helper;
}

concat_helper operator,(concat_helper helper, char c)
{
    helper.s[helper.len++] = c;
    helper.s[helper.len] = 0;
    return helper;
}

concat_helper operator,(concat_helper helper, real x)
{

    helper.len += sprintf(helper.s+helper.len, "%g", x);
    return helper;
}

concat_helper operator,(concat_helper helper, double x)
{

    helper.len += sprintf(helper.s+helper.len, "%g", x);
    return helper;
}

concat_helper operator,(concat_helper helper, int x)
{

    helper.len += sprintf(helper.s+helper.len, "%d", x);
    return helper;
}

char* finalize_concat(concat_helper helper)
{
    permalloc(helper.len);
    return helper.s;
}

#define concat(...) (finalize_concat((init_concat_helper(), __VA_ARGS__)))

#endif
