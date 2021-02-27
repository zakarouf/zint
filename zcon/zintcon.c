#include <stdio.h>
#include <string.h>

#include "../zint_string.h"
#include "../common.h"

extern void dieOnCommand(char *msg, int CODE, char * codesnip);
extern String_t z_ReadFromFile(char *);

#define ZINT_UNFILE__KEYWORD_FuntionStart "fn"
#define ZINT_UNFILE__KEYWORD_FuntionEnd "fnend"
#define ZINT_UNFILE__KEYWORD_VariableDeclare "let"
#define ZINT_UNFILE__KEYWORD_If "if"
#define ZINT_UNFILE__KEYWORD_Else_If "elif"
#define ZINT_UNFILE__KEYWORD_Else "else"
#define ZINT_UNFILE__KEYWORD_WhileLoop_Start "while"
#define ZINT_UNFILE__KEYWORD_WhileLoop_End "wlend"
#define ZINT_UNFILE__KEYWORD_ForLoop_Start "for"
#define ZINT_UNFILE__KEYWORD_ForLoop_End "forend"
#define ZINT_UNFILE__KEYWORD_FunctionCall "call"

void z_convertor__giveErrorAndDie(const char * msg, int ErrorCode , char * funcname ,char *atline, int aroundIndex)
{
	color256_set(1);
	fprintf(stderr, "ERROR:%d:%s\nOn: fn %s\n\t|\t%s\n\t\t ", ErrorCode, msg, funcname, atline);
	for (int i = 0; i < aroundIndex; ++i)
	{
		fputc(' ', stderr);
	}
	fputc('^', stderr);
	color_reset();

}

static StringLines_t z_convertor_UnFile_create_zint_function(char * token, int id)
{
	StringLines_t func = z__StringLines_createEmpty(96, 200);



	snprintf(func.lines[0], "|@%d %d;", id);


	while (token)
	{

	}
}


static StringLineArr_t z_convertor_UnFile_BreakIntoFuntion(String_t s)
{
    const int ln_size_x = 128;
    const int ln_size_y = 1024;

    String_t tmp = z__copyString(s);

    char * tok = strtok(tmp.str, " \n\t");
    int fn_count = -1;

    while(tok != NULL)
    {
        if (strcmp("fn", tok) == 0 )
        {
            fn_count += 1;
        }
    }

    if (fn_count == -1)
    {
    	z_convertor__giveErrorAndDie("No Function Found in File", 0, "", "", 0);
    }

}

static int z_convertor(char const * filename)
{
    String_t z_unfile = z_ReadFromFile((char*)filename);


    if (z_unfile.str == NULL)
    {
        dieOnCommand("Cannot Open zFile", 404, (char*)filename);
    }



     z__deleteString(&z_unfile);
     return 0;
}