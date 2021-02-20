/*-------------------------------------------------
    Z_INTERPRETOR - (ZINT) 2021
--------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "zint_defs.h"

#define Z_DEBUG_ENABLED   //Comment this line out if you dont want to do a debug build
#define check(s, ...)\
    printf(s, ...);

enum ERROR_CODE
{
      ERROR_CODE__VARIABLE      = 0
    , ERROR_CODE__DELETION__FAIL_VARUSED_REACHED_NEGETIVE_VALUE
    , ERROR_CODE__DELETION__FAIL_VARSIZE_REACHED_NEGETIVE_VALUE
    , ERROR_CODE__DELETION__FAIL_SCOPESIZE_REACHED_NEGETIVE_VALUE

    , ERROR_CODE__SYNTAX        = 400

    , ERROR_CODE__PREP_SYNTAX   = 800
    , ERROR_CODE__PREP_SYNTAX_VARID_OVER_ACCESS_LIMIT

    , ERROR_CODE__MISC          = 1000
    , ERROR_CODE__MISC__FILE_CANT_BE_OPENED
};

// Kill Switch if Anything goes wrong
static void dieOnCommand(char *msg, int CODE, char * codesnip)
{
    fprintf(stderr, "ERROR:%d:%s\n\n\t%s", CODE, msg, codesnip);
    exit(1);
}


//Macro Funcs

// Check if Value is in range, 1 if it is...
#define z_checkIfInRange__MF(val, min, max)\
    (( (val) < min) ? 0 : (( (val) < max) ? 1 : 0 ))

// Check if Given Char is Digit
#define z_isDigit__MF(c)\
    (((c) < '0') ? 0 : (((c) <= '9') ? 1 : 0 ))

// Wrap up int
#define z_wrapInt__MF(x, MaxValue)\
    (x)%(MaxValue)

// Check if Character is Alphabate
#define z_isAlpha__MF(c)\
    (z_checkIfInRange__MF((c), 'A', 'Z'+1) == 1 ? z_checkIfInRange__MF((c), 'a', 'z'+1): 0 )

//Macro Funcs END



// Strings
typedef struct Z_STRING
{
    char * str;
    int size;

}String_t;

String_t z__createString(int size)
{
    return (String_t){

        .str = malloc(sizeof(char) * size),
        .size = size
    };
}

#define z__fillString_MF(String, val)\
    memset(String.str, (val), String.size)

void z__deleteString(String_t * s)
{
    free(s->str);
    s->size = 0;
}

String_t z__copyString(String_t str)
{

    String_t str2 = {
        .str = malloc(sizeof(char) * str.size),
        .size = str.size,
    };

    memcpy(str2.str, str.str, str.size);

    return str2;
}

// Malloc And Free 2d Char, Taken from Ztorg (https://github.com/zakarouf/ztorg)
static char **zse_malloc_2D_array_char (unsigned int x, unsigned int y) {

    char **arr = malloc(y * sizeof(char*));
    for (int i = 0; i < y; ++i)
    {
        arr[i] = (char*)malloc(x * sizeof(char));
    }

    return arr;

}
static void zse_free2dchar(char **mem, int size)
{
    for (int i = 0; i < size; ++i)
    {
        free(mem[i]);
    }
    free(mem);

}

// String END



// KeyWords, Tokens

typedef enum TOKEN_S_enum
{
      TOKEN_var = 0
    , TOKEN_if
    , TOKEN_else
    , TOKEN_elif
    , TOKEN_then
    , TOKEN_endif
    , TOKEN_for
    , TOKEN_forend
    , TOKEN_while
    , TOKEN_wlend
    , TOKEN_fn 
    , TOKEN_fnend
    , TOKEN_call
}TOKEN_S_enum;

typedef struct _
{
    String_t gKEYWORDS[20];
    TOKEN_S_enum sign;

}Keywords_t;

Keywords_t Keywords = {
    .gKEYWORDS = {
          { "var"     , 4 }
        , { "if"      , 3 }
        , { "else"    , 5 }
        , { "elif"    , 5 }
        , { "then"    , 5 }
        , { "endif"   , 6 }
        , { "for"     , 4 }
        , { "forend"  , 7 }
        , { "while"   , 6 }
        , { "wlend"   , 6 }
        , { "fn"      , 3 }
        , { "fnend"   , 6 }
        , { "pub"     , 4 }
        , { "call"    , 5 }
    }
};

//Keyword END

// Pre Processor, Processor Lang
const char PROCESSED_SYMBS[] = {
      '#' // Global Var
    , '$' // Local Var
    , '!' // Global Funcs
    , '@' // Local Funcs
};


// Basic mathFunc
static double z_add_func(double a, double b)
{
    return a + b;
}
static double z_sub_func(double a, double b)
{
    return a - b;
}
static double z_mul_func(double a, double b)
{
    return a * b;
}
static double z_div_func(double a, double b)
{
    return a + b;
}

typedef double (*mathFunc_Basic_tf)(double, double);
#define Z_MATHFUNCS_BASIC__TOTAL 4

mathFunc_Basic_tf MathFuncs_Basic[Z_MATHFUNCS_BASIC__TOTAL] = {
      z_add_func
    , z_sub_func
    , z_mul_func
    , z_div_func
};

typedef enum
{
      Z_MathFunc_Basic__ADD = 0
    , Z_MathFunc_Basic__SUB
    , Z_MathFunc_Basic__MUL
    , Z_MathFunc_Basic__DIV

}Z_MathFunc_Basic_ENUMS;

typedef struct
{
    mathFunc_Basic_tf Basic[Z_MATHFUNCS_BASIC__TOTAL];
    char Basic_ch[Z_MATHFUNCS_BASIC__TOTAL];
    Z_MathFunc_Basic_ENUMS Basic_enums;
    int Basic_Total;


}MathFuncs_t;

MathFuncs_t MathFuncs = {
    .Basic = {
          z_add_func
        , z_sub_func
        , z_mul_func
        , z_div_func
    },

    .Basic_ch = {
        '+','-','*','/'
    },

    .Basic_Total = Z_MATHFUNCS_BASIC__TOTAL

};
// Math Funcs END




// Variable Management
// // Keep Track of Variable
/*
typedef struct _Var_t
{
    double var;
    long int id;


    struct _Var_t *next;
    struct _Var_t *prev;

}Var_t;
*/

#define Z_VARIABLE_BLOCKSIZE_SCOPE 8 // Memory Allocated/Freed At once. FOR SCOPESIZE
#define Z_VARIABLE_BLOCKSIZE_VSIZE 8 // Memory Allocated/Freed At once. FOR VarSize

typedef struct _Var_t
{
    double *value;
    long id;
    int size;
    int type;

}Var_t;

typedef struct
{
    Var_t **VARS;

    int *VarUsed;   // Variable USED in *VARS
    int *VarSize; // TOTAL SIZE OF *VARS

    int ScopeUsed;       // Space Used      in **VARS, *VarAt, *VarSize
    int ScopeSize;     // Space Available in **VARS, *VarAt, *VarSize

    /*
     *    NOTE: `ScopeAt` and `*VarAt` always point to a Clean/Unused Space.
     *        
     *    [ 1 , 1 , 1 , 1 , 1 , 0 , 0 , 0 , 0 , 0 ] <- (1 = Space is Used, 0 = Space is not Used)
     *                          ^
     *                          ScopeAt is pointing hear, Same with *VarAt
     */

}VariableS_t;
static VariableS_t gVARIABLES;

static int z_Variable_changeVariableValue()
{
    return 0;
}

static int z_Variable_addVariableToScope(int type, int size)
{
    int scope = gVARIABLES.ScopeUsed - 1;
    if (gVARIABLES.VarUsed[scope] == gVARIABLES.VarSize[scope])
    {
        gVARIABLES.VarSize[scope] += Z_VARIABLE_BLOCKSIZE_VSIZE;
        gVARIABLES.VARS[scope] = realloc(gVARIABLES.VARS[scope], sizeof(Var_t)* gVARIABLES.VarSize[scope] );
    }

    gVARIABLES.VARS[scope][gVARIABLES.VarUsed[scope]].value = malloc(sizeof(double) * size);
    gVARIABLES.VARS[scope][gVARIABLES.VarUsed[scope]].size = size;
    gVARIABLES.VARS[scope][gVARIABLES.VarUsed[scope]].type = type;
    gVARIABLES.VarUsed[scope] += 1;

    return 0;
}
static int z_Variable_delVariableFromScope(void)
{
    int scope = gVARIABLES.ScopeUsed - 1;

    free(gVARIABLES.VARS[scope][gVARIABLES.VarUsed[scope]].value);
    gVARIABLES.VARS[scope][gVARIABLES.VarUsed[scope]].size = 0;
    gVARIABLES.VARS[scope][gVARIABLES.VarUsed[scope]].type = 0;

    if ( !z_checkIfInRange__MF((gVARIABLES.VarSize[scope] - gVARIABLES.VarUsed[scope]), 0, Z_VARIABLE_BLOCKSIZE_VSIZE) )
    {
        gVARIABLES.VarSize[scope] -= Z_VARIABLE_BLOCKSIZE_VSIZE;
        #ifdef Z_DEBUG_ENABLED
        if (gVARIABLES.VarSize[scope] < 0)
        {
            dieOnCommand("Variable VarSize Reached Negetive\n", ERROR_CODE__DELETION__FAIL_VARSIZE_REACHED_NEGETIVE_VALUE, "");
        }
        #endif
        gVARIABLES.VARS[scope] = realloc(gVARIABLES.VARS[scope], sizeof(Var_t)* gVARIABLES.VarSize[scope] );
    }

    gVARIABLES.VarUsed[scope] -= 1;

    return 0;
}
static int z_Variable_addVariableScope(void)
{
    int scope = gVARIABLES.ScopeUsed;

    if ( gVARIABLES.ScopeUsed == gVARIABLES.ScopeSize )
    {
        gVARIABLES.ScopeSize += Z_VARIABLE_BLOCKSIZE_SCOPE;
        gVARIABLES.VARS = realloc(gVARIABLES.VARS, sizeof(Var_t) * gVARIABLES.ScopeSize);
        gVARIABLES.VarSize = realloc(gVARIABLES.VarSize, sizeof(int) * gVARIABLES.ScopeSize);
        gVARIABLES.VarUsed = realloc(gVARIABLES.VarUsed, sizeof(int) * gVARIABLES.ScopeSize);
    }


    gVARIABLES.VarSize[scope] = 0;
    gVARIABLES.VarUsed[scope] = 0;
    gVARIABLES.ScopeUsed += 1;

    return 0;
}
static int z_Variable_delVariableScope(void)
{
    int scope = gVARIABLES.ScopeUsed -1;

    while (gVARIABLES.VarUsed[scope] != 0)
    {
        z_Variable_delVariableFromScope();
    }

    if ( !z_checkIfInRange__MF(( gVARIABLES.ScopeSize - gVARIABLES.ScopeUsed ), 0, Z_VARIABLE_BLOCKSIZE_SCOPE ) )
    {
        gVARIABLES.ScopeSize -= Z_VARIABLE_BLOCKSIZE_SCOPE;
        #ifdef Z_DEBUG_ENABLED
        if (gVARIABLES.ScopeSize < 0)
        {
            dieOnCommand("Variable Scope Reached Negetive\n", ERROR_CODE__DELETION__FAIL_SCOPESIZE_REACHED_NEGETIVE_VALUE, "");
        }
        #endif
        gVARIABLES.VARS = realloc(gVARIABLES.VARS, sizeof(Var_t) * gVARIABLES.ScopeSize);
        gVARIABLES.VarSize = realloc(gVARIABLES.VarSize, sizeof(int) * gVARIABLES.ScopeSize);
        gVARIABLES.VarUsed = realloc(gVARIABLES.VarUsed, sizeof(int) * gVARIABLES.ScopeSize);
    }

    gVARIABLES.ScopeUsed -= 1;

    return 0;
}


static void z_Variable_changeValue(int varID, double value)
{
    int scope = gVARIABLES.ScopeUsed -1;
    #ifdef Z_DEBUG_ENABLED
    if (gVARIABLES.VarUsed[scope] <= varID)
    {
        dieOnCommand("VARID IS IS NOT IN USE\n", ERROR_CODE__SYNTAX, "");
    }
    #endif
    
    *gVARIABLES.VARS[scope][varID].value = value;
}

static double z_Variable_accessVariable(int varID)
{
    int scope = gVARIABLES.ScopeUsed -1;

    #ifdef Z_DEBUG_ENABLED
    if (gVARIABLES.VarUsed[scope] <= varID)
    {
        dieOnCommand("VarID is Over the Access Limit", ERROR_CODE__PREP_SYNTAX_VARID_OVER_ACCESS_LIMIT, "");
    }
    #endif

    return *gVARIABLES.VARS[scope][varID].value;
}

static int z_Variable_init(void)
{
    gVARIABLES.ScopeUsed = 1;
    gVARIABLES.ScopeSize = Z_VARIABLE_BLOCKSIZE_SCOPE ;

    gVARIABLES.VarUsed   = malloc(sizeof(int) * gVARIABLES.ScopeSize);
    gVARIABLES.VarSize = malloc(sizeof(int) * gVARIABLES.ScopeSize);


    if (   gVARIABLES.VarUsed == NULL 
        || gVARIABLES.VarSize == NULL)
    {
        return -1;
    }

    memset(gVARIABLES.VarUsed, 0, gVARIABLES.ScopeSize);

    for (int i = 0; i < gVARIABLES.ScopeSize; ++i)
    {
        gVARIABLES.VarSize[i] = Z_VARIABLE_BLOCKSIZE_VSIZE;
    }

    gVARIABLES.VARS = malloc(sizeof(Var_t*) * gVARIABLES.ScopeSize);
    for (int i = 0; i < gVARIABLES.ScopeSize; ++i)
    {
        gVARIABLES.VARS[i] = malloc(sizeof(Var_t) * gVARIABLES.VarSize[i]);
        memset(gVARIABLES.VARS[i], 0, gVARIABLES.VarSize[i]);
    }

    return 0;
}

static int z_Variable_destroy(void)
{


    return 0;
}


#define z_Var_getVarUsedin(a)\
    gVARIABLES.VarUsed[a]
#define z_Var_getVarSizein(a)\
    gVARIABLES.VarSize[a]
// Variable END


// Break String into List of String (Into Tokens)
static int z_BreakStringInto2DString_ASTOKENS (char ** buffer2D ,int buffXsize, int buffYsize, char *buffer, char *token_breaker)
{
    char * token;
    int count = 0;

    token = strtok(buffer, token_breaker);

    for (int i = 0; i < buffXsize && token != NULL; ++i)
    {
        sprintf(buffer2D[i], "%s", token);
        token = strtok(NULL, token_breaker);
        count++;
    }

    return count;
}

static String_t z_ReadFromFile(char *file)
{
    int string_size;
    FILE *f = fopen(file, "rb");

    if (f == NULL)
    {
        return (String_t) {
            NULL, 0
        };
    }

    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);  /* same as rewind(f); */

    char *string = malloc(fsize + 1);
    fread(string, 1, fsize, f);
    fclose(f);

    string[fsize] = 0;

    string_size = fsize;
    return (String_t)
            {string, string_size};
}
/*
static double z_operator_asign(char * line)
{

}
*/
static int z_interpreter(const String_t ZFILE_PreP)
{
    String_t zfile_tmp = z__copyString(ZFILE_PreP);

    int x = 100, y = 1000;
    char ** buff2D = zse_malloc_2D_array_char(x, y);

    int count;
    count = z_BreakStringInto2DString_ASTOKENS(buff2D, x, y, zfile_tmp.str, ";");


    for (int i = 0; i < count; ++i)
    {
        char * token = strtok(buff2D[i], " \n\t");



        while (token != NULL)
        {
            if (   token[0] == 'v' 
                && token[1] == 'a'
                && token[2] == 'r')
            {
                token = strtok(NULL, " \n\t");
                int VarCount = 1;
                if (z_isDigit__MF(token[0]))
                {
                    VarCount = atoi(token);

                }
                for (int i = 0; i < VarCount; ++i)
                {
                    z_Variable_addVariableToScope(1, 1);
                }
                

            }

            else if (token[0] == '#')
            {
                char * t = &token[1];
                int VarPos = atoi(t);
                token = strtok(NULL, " \n\t");
                if (token[0] == '=')
                {
                    token = strtok(NULL, " \n\t");
                    if (token != NULL)
                    {
                        z_Variable_changeValue(VarPos, atof(token));
                    }
                    else
                    {
                        dieOnCommand("No Values Done For Assignment Operetor", ERROR_CODE__SYNTAX, token);
                    }
                }

            }
            else if (strcmp(token, "print") == 0)
            {
                token = strtok(NULL, " \n\t");
                if (token[0] == '#')
                {
                    char * t = &token[1];
                    int VarPos = atoi(t);

                    printf("%lf\n", z_Variable_accessVariable(VarPos));
                }
            }
            token = strtok(NULL, " \n\t");

        }
    }


    return 0;
}

static int z_start(char * filename)
{
    String_t zfile = z_ReadFromFile(filename);
    z_interpreter(zfile);

    if (zfile.str == NULL)
    {
        dieOnCommand("Cannot Open zFile", ERROR_CODE__MISC__FILE_CANT_BE_OPENED, filename);
    }

    

    return 0;
}


int main(int argc, char const *argv[])
{
    z_Variable_init();

    z_start("Z.zfile");
    return 0;
}