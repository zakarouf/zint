#ifndef ZINT_SELF_STRING_H
#define ZINT_SELF_STRING_H

typedef struct ZINT_STRING
{
    char * str;
    int size;

}String_t;

typedef struct
{
    char **lines;
    unsigned int x;
    unsigned int y;
    unsigned linesUsed;
}StringLines_t;

typedef struct
{
    StringLines_t *fn;
    int size;
    int used;
}StringLineArr_t;


#define z__fillString_MF(String, val)\
    memset(String.str, (val), String.size)

int z_findCharInStr(String_t str, char c, int fromIndex);

String_t z__createString(int size);
void z__deleteString(String_t * s);
String_t z__copyString(String_t str);

StringLines_t z__StringLines_createEmpty(int x, int y);
void z__StringLines_Resize_Y (StringLines_t *ln , unsigned int newsize);
void z__StringLines_destroy(StringLines_t *strLines);


StringLineArr_t z__StringLinesArr_createEmpty(int size, int x, int y);
void z__StringLinesArr_resize(StringLineArr_t *lns, int newsize);
void z__destroyStringLinesArr(StringLineArr_t *lns);


#endif