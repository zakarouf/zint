/*-------------------------------------------------
    Z_INTERPRETOR - (ZINT) 2021
--------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "../common.h"
#include "zint_defs.h"
#include "zint_sys.h"

 //#define ZINT_DEBUG_ENABLED     /* Comment this line out if you dont want to do a debug build */
 //#define ZINT_DEV_TEST_ENABLED  /* Instialize Developer Test kit */
 //#define ZINT_DEBUG__SHOW_PUSH_AND_POP_VARS_ENABLED
 //#define ZINT_DEBUG__SHOW_PUSH_AND_POP_SCOPE_ENABLED

 //#define ZINT_DEBUG__LOGGER_ENABLED
 //#define ZINT_DEBUG__LOGGER_LOG_MEMORY



/* Kill Switch if Anything goes wrong ( Usually Used For Debug Purpose ) */
void dieOnCommand(char *msg, int CODE, char * codesnip)
{
    fprintf(stderr, "DIED:%d:%s\n\n>>> %s\n", CODE, msg, codesnip);
    exit(1);
}


/* Basic Macro Functions */

/* Check if Value is in range, 1 if it is... */
#define z_checkIfInRange__MF(val, min, max)\
    (( (val) < min) ? 0 : (( (val) < max) ? 1 : 0 ))

/* Check if Given Char is Digit */
#define z_isDigit__MF(c)\
    (((c) < '0') ? 0 : (((c) <= '9') ? 1 : 0 ))

/* Wrap up int */
#define z_wrapInt__MF(x, MaxValue)\
    (x)%(MaxValue)

/* Check if Character is Alphabate */
#define z_isAlpha__MF(c)\
    (z_checkIfInRange__MF((c), 'A', 'Z'+1) == 1 ? z_checkIfInRange__MF((c), 'a', 'z'+1): 0 )

/* Convert Ascii Char To Int (Single Char) */
#define z_charToInt__MF(c)\
    (c)-'0'

/* Basic Macro Functions END */

/*
// Pre Processor, Processor Lang
const char PROCESSED_SYMBS[] = {
      _ZINT__PreP_VARIABLE_SYMB  // Var
    , '$' // Operators MATHS
    , '@' // Funcs
    , '!' // Keywords
};

------------------------------------------------------------------------------------
            ------------KEYWORDS------------

    STATEMENTS [!]

     !0 (char *, int) => Push Varaible
     !1 (char *, int) => Pop Variable


     !2 (char *, int) => In-build Print Function
     !3 (char *, int) => In-Build Input Function


     !5 (char *, int) => IF Statement
        --Push IF Vars--

        --Get & Check IF--
            --PushVar--
                --Do Someting--
            --DelVar--
        --Get & Check ELSE IF--
            --PushVar--
                --Do Someting--
            --DelVar--
        --Get & Check ELSE--
            --PushVar--
                --Do Someting--
            --DelVar--
        
        --Pop IF Vars--


     !4 (char *, int) => Goto Statement
        --Goto a line of code and Execute from there 
            (NOTE: Goto Requires a Lable with :)


     !6 (char *, int) => Call Function
        -- Call Fuction Set its Variable and Get a Value in Return
        (NOTE: Calls Requires a Lable with @)

     !7 Push Scope
     !8 Pop Scope

     EXTRA:
        for loop   
        while loop
                    => Use !4,!5 (If,esle) & !6 (Goto) To create Loops

            --------------------------------
------------------------------------------------------------------------------------
*/
/*----------------------------*/

#define _ZINT__PreP_VARIABLE_SYMB '#'
#define _ZINT__PreP_KEYWORD_SYMB  '!'
#define _ZINT__PreP_FUNCTION_SYMB '@'
#define _ZINT__PreP_OPERATOR_SYMB 'o'
#define _ZINT__PreP_ESCAPE_SYMB   '\\'

/*----------------------------*/






/********************************************/
/*     Variables & Variables Management     */
/*            ------------------            */
/*                   START                  */
/*                  -------                 */
/********************************************/
#define ZINT_VARIABLE_BLOCKSIZE_SCOPE 4 // Memory Allocated/Freed At once. FOR SCOPESIZE
#define ZINT_VARIABLE_BLOCKSIZE_VSIZE 8 // Memory Allocated/Freed At once. FOR VarSize

typedef double Variable_vartype;

typedef struct _Var_t
{
    Variable_vartype *value;
    //long id;
    int size;
    int type;

}Var_t;

typedef struct
{
    Var_t **VARS;

    int *VarUsed;   // Variable USED in **VARS
    int *VarSize;   // TOTAL SIZE OF **VARS

    int ScopeUsed;     // Space Used      in *VARS, *VarAt, *VarSize
    int ScopeSize;     // Space Available in *VARS, *VarAt, *VarSize

    /*
     *    NOTE: `ScopeUsed` and `*VarUsed` always point to a Clean/Unused Space.
     *        
     *    [ 1 , 1 , 1 , 1 , 1 , 0 , 0 , 0 , 0 , 0 ] <- (1 = Space is Used, 0 = Space is not Used)
     *                          ^
     *                          ScopeUsed is pointing hear, Same with *VarUsed
     */

}VariableS_t;
static VariableS_t gVARIABLES;

#define z_Variable_getVarUsedin__MF(a)\
    gVARIABLES.VarUsed[a]
#define z_Variable_getVarSizein__MF(a)\
    gVARIABLES.VarSize[a]

#define z_Variable_getScopeUsed__MF()\
    gVARIABLES.ScopeUsed
#define z_Variable_getScopeSize__MF()\
    gVARIABLES.ScopeSize



/* Add A Variable To (Scope) **VARS  At Top */
static int z_Variable_pushVariableToScope(int type, int size)
{
    int scope = gVARIABLES.ScopeUsed - 1;
    if (gVARIABLES.VarUsed[scope] >= gVARIABLES.VarSize[scope])
    {
        #ifdef ZINT_DEBUG__SHOW_PUSH_AND_POP_VARS_ENABLED
            printf("\nLOG:VU%d VS%d \n", gVARIABLES.VarUsed[scope], gVARIABLES.VarSize[scope]);
        #endif
        gVARIABLES.VarSize[scope] += ZINT_VARIABLE_BLOCKSIZE_VSIZE;
        gVARIABLES.VARS[scope] = realloc(gVARIABLES.VARS[scope], sizeof(Var_t)* gVARIABLES.VarSize[scope] );
    }

    gVARIABLES.VARS[scope][gVARIABLES.VarUsed[scope]].value = malloc(sizeof(double) * size);
    gVARIABLES.VARS[scope][gVARIABLES.VarUsed[scope]].size = size;
    gVARIABLES.VARS[scope][gVARIABLES.VarUsed[scope]].type = type;
    gVARIABLES.VarUsed[scope] += 1;

    return 0;
}
/* Delete A Variable To (Scope) **VARS  At Top */
static int z_Variable_popVariableFromScope(void)
{
    int scope = gVARIABLES.ScopeUsed - 1;
    int *VarScopeUsed = &gVARIABLES.VarUsed[scope];

    free(gVARIABLES.VARS[scope][*VarScopeUsed].value);
    gVARIABLES.VARS[scope][*VarScopeUsed].size = 0;
    gVARIABLES.VARS[scope][*VarScopeUsed].type = 0;

    if ( (gVARIABLES.VarSize[scope] - gVARIABLES.VarUsed[scope] ) >= ZINT_VARIABLE_BLOCKSIZE_VSIZE )
    {
        gVARIABLES.VarSize[scope] -= ZINT_VARIABLE_BLOCKSIZE_VSIZE;
        #ifdef ZINT_DEBUG_ENABLED
        if (gVARIABLES.VarSize[scope] < 0)
        {
            dieOnCommand("Variable VarSize Reached Negetive\n", ERROR_CODE__DELETION__FAIL_VARSIZE_REACHED_NEGETIVE_VALUE, "");
        }
        #endif
        #ifdef ZINT_DEBUG__SHOW_PUSH_AND_POP_VARS_ENABLED
            printf("\nLOG:VU%d VS%d \n", gVARIABLES.VarUsed[scope], gVARIABLES.VarSize[scope]);
        #endif
        gVARIABLES.VARS[scope] = realloc(gVARIABLES.VARS[scope], sizeof(Var_t)* gVARIABLES.VarSize[scope] );
    }

    gVARIABLES.VarUsed[scope] -= 1;

    return 0;
}
/* Add Scope To Top */
static int z_Variable_pushVariableScope(void)
{
    int scope = gVARIABLES.ScopeUsed;

    if ( gVARIABLES.ScopeUsed >= gVARIABLES.ScopeSize )
    {
        gVARIABLES.ScopeSize += ZINT_VARIABLE_BLOCKSIZE_SCOPE;
        gVARIABLES.VARS = realloc(gVARIABLES.VARS, sizeof(Var_t) * gVARIABLES.ScopeSize);
        gVARIABLES.VarSize = realloc(gVARIABLES.VarSize, sizeof(int) * gVARIABLES.ScopeSize);
        gVARIABLES.VarUsed = realloc(gVARIABLES.VarUsed, sizeof(int) * gVARIABLES.ScopeSize);
    }


    gVARIABLES.VarSize[scope] = 0;
    gVARIABLES.VarUsed[scope] = 0;
    gVARIABLES.ScopeUsed += 1;

    return 0;
}
/* Delete Scope From Top */
static int z_Variable_popVariableScope(void)
{
    int scope = gVARIABLES.ScopeUsed -1;

    while (gVARIABLES.VarUsed[scope] >= 0 )
    {
        #ifdef ZINT_DEBUG__SHOW_PUSH_AND_POP_VARS_ENABLED
            printf("\nLOG:VU%d VS%d \n", gVARIABLES.VarUsed[scope], gVARIABLES.VarSize[scope]);
        #endif
        z_Variable_popVariableFromScope();
    }

    if ( ( gVARIABLES.ScopeSize - gVARIABLES.ScopeUsed ) >= ZINT_VARIABLE_BLOCKSIZE_SCOPE)
    {
        
        gVARIABLES.ScopeSize -= ZINT_VARIABLE_BLOCKSIZE_SCOPE;
        #ifdef ZINT_DEBUG_ENABLED
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
/* Change Variable Value In Current Scope */
static void z_Variable_changeValue(int varID, double value)
{
    int scope = gVARIABLES.ScopeUsed -1;
    #ifdef ZINT_DEBUG_ENABLED
    if (gVARIABLES.VarUsed[scope] <= varID)
    {
        dieOnCommand("VARID IS IS NOT IN USE\n", ERROR_CODE__SYNTAX, "");
    }
    #endif
    
    *gVARIABLES.VARS[scope][varID].value = value;
}
/* Access Variable From Current Scope */
static double z_Variable_accessVariable(int varID)
{
    int scope = gVARIABLES.ScopeUsed -1;

    #ifdef ZINT_DEBUG_ENABLED
    if (gVARIABLES.VarUsed[scope] <= varID)
    {
        char ID[10];
        sprintf(ID, "%d", varID);
        dieOnCommand("VarID is Over the Access Limit", ERROR_CODE__PREP_SYNTAX_VARID_OVER_ACCESS_LIMIT, ID);
    }
    #endif

    return *gVARIABLES.VARS[scope][varID].value;
}
static double *z_Variable_accessVariableMemory(int varID)
{
    int scope = gVARIABLES.ScopeUsed -1;

    #ifdef ZINT_DEBUG_ENABLED
    if (gVARIABLES.VarUsed[scope] <= varID)
    {
        char ID[10];
        sprintf(ID, "%d", varID);
        dieOnCommand("VarID Memory is Over the Access Limit", ERROR_CODE__PREP_SYNTAX_VARID_OVER_ACCESS_LIMIT, ID);
    }
    #endif

    return gVARIABLES.VARS[scope][varID].value;
}
/* Start Variable */
static int z_Variable_init(void)
{
    gVARIABLES.ScopeUsed = 1;
    gVARIABLES.ScopeSize = ZINT_VARIABLE_BLOCKSIZE_SCOPE ;

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
        gVARIABLES.VarSize[i] = ZINT_VARIABLE_BLOCKSIZE_VSIZE;
    }

    gVARIABLES.VARS = malloc(sizeof(Var_t*) * gVARIABLES.ScopeSize);
    for (int i = 0; i < gVARIABLES.ScopeSize; ++i)
    {
        gVARIABLES.VARS[i] = malloc(sizeof(Var_t) * gVARIABLES.VarSize[i]);
        memset(gVARIABLES.VARS[i], 0, gVARIABLES.VarSize[i]);
    }

    return 0;
}
/* Collapse Variable in its Entirity */
static int z_Variable_destroy(void)
{
    for (int i = 0; i <= z_Variable_getScopeUsed__MF(); ++i)
    {

        z_Variable_popVariableScope();
    }

    return 0;
}
/********************************************/
/*     Variables & Variables Management     */
/*            ------------------            */
/*                    END                   */
/*                  -------                 */
/********************************************/



/********************************************/
/*               Math Functions             */
/*            ------------------            */
/*                   START                  */
/*                  -------                 */
/********************************************/
static double z__MATHFUNC__add_func(double a, double b)
{
    return a + b;
}
static double z__MATHFUNC__sub_func(double a, double b)
{
    return a - b;
}
static double z__MATHFUNC__mul_func(double a, double b)
{
    return a * b;
}
static double z__MATHFUNC__div_func(double a, double b)
{
    return a + b;
}
static double z__MATHFUNC__mod_func(double a, double b)
{
    return fmod(a, b);
}

typedef double (*mathFunc_Basic_tf)(double, double);
#define ZINT_MATHFUNCS_BASIC__TOTAL 5

/*
typedef enum
{
      ZINT_MathFunc_Basic__ADD = 0
    , ZINT_MathFunc_Basic__SUB
    , ZINT_MathFunc_Basic__MUL
    , ZINT_MathFunc_Basic__DIV
    , ZINT_MathFunc_Basic__MOD

}ZINT_MathFunc_Basic_ENUMS;
*/
typedef struct
{
    mathFunc_Basic_tf        Basic    [ZINT_MATHFUNCS_BASIC__TOTAL];
    char                     Basic_ch [ZINT_MATHFUNCS_BASIC__TOTAL];
    //ZINT_MathFunc_Basic_ENUMS Basic_enums;
    int                      Basic_Total;

}MathFuncs_t;


MathFuncs_t MathFuncs = {
    .Basic = {
          z__MATHFUNC__add_func
        , z__MATHFUNC__sub_func
        , z__MATHFUNC__mul_func
        , z__MATHFUNC__div_func
        , z__MATHFUNC__mod_func
    },

    .Basic_ch = {
        '+','-','*','/','%'
    },

    .Basic_Total = ZINT_MATHFUNCS_BASIC__TOTAL

};
/********************************************/
/*               Math Functions             */
/*            ------------------            */
/*                    END                   */
/*                  -------                 */
/********************************************/

/*
 *
 *
 *
 *
 */
// Break String into List of String (Into Tokens)

StringLineArr_t gFUNCTIONS;
static void z_BreakStringInto2DString_FUNTION_ASTOKENS (StringLines_t *buffer2D, char * buffer, char *token_breaker)
{
    char * token = strtok(buffer, token_breaker);

    int i;
    for (i = 0; i < buffer2D->y && token != NULL; ++i)
    {
        snprintf(buffer2D->lines[i], buffer2D->x ,"%s", token);
        token = strtok(NULL, token_breaker);
    }

    buffer2D->linesUsed = i;
}

static StringLineArr_t z_BreakStringIntoFunctions (String_t buffer)
{
    char * tmp_buff_ptr = buffer.str;
    const int ls_size_x = 96;
    const int ls_size_y = 200;
    

    char * token = strtok(tmp_buff_ptr, "|");

    int used = 0;


    // Breaking Functions Into Strings
    StringLines_t ln = z__StringLines_createEmpty(96 * 200, 10);
    while(token != NULL)
    {
        if (*token == '@')
        {
            snprintf(ln.lines[used], ln.x, "%s", token);
            used++;
        }
        token = strtok(NULL, "|");
    }


    StringLineArr_t Functions = z__StringLinesArr_createEmpty(used+8, ls_size_x, ls_size_y);
    

    for (int i = 0; i < used; ++i)
    {
        z_BreakStringInto2DString_FUNTION_ASTOKENS (&Functions.fn[i], ln.lines[i], ";");
        z__StringLines_Resize_Y(&Functions.fn[i], Functions.fn[i].linesUsed+1);
    }

    z__StringLines_destroy(&ln);

    Functions.used = used;
    return Functions;

}

// Read File as String
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

/********************************************/
/*             Main Interpretor             */
/*            ------------------            */
/*                   START                  */
/*                  -------                 */
/********************************************/
/* Call Basic Math Funcs */
static Variable_vartype z_operator_callMathFuncs(char *line, int line_width)
{
    int it = 0;
    int type;
    double Val[2];

    sscanf( &line[1] ,"%d", &type);

    char * token = strtok(line, " \t\n");

    while (token != NULL && it < 2)
    {
        if (token[0] == _ZINT__PreP_VARIABLE_SYMB )
        {
            token++;
            Val[it] = atof(token);
            Val[it] = z_Variable_accessVariable(Val[it]);
            it++;
        }
        else if (z_isDigit__MF(token[0]))
        {
            Val[it] = atof(token);
            it++;
        }

        token = strtok(NULL, " \t\n");
    }

    return MathFuncs.Basic[type](Val[0], Val[1]);
}

/* Assingment Operator */
static void z_operator_asign(char * line, int line_width, const int VarPos)
{
    int in = z_findCharInStr((String_t){line, line_width}, '=', 0);

    for (int i = in; i < line_width; ++i)
    {
        if (line[i] == _ZINT__PreP_OPERATOR_SYMB )
        {

            double newVal = z_operator_callMathFuncs(&line[i], line_width-i);
            z_Variable_changeValue(VarPos, newVal);

            return;

        }
        else if (z_isDigit__MF(line[i]))
        {
            
            double newVal;
            sscanf(&line[i], "%lf", &newVal);
            z_Variable_changeValue(VarPos, newVal);
            return;

        }
        
    }
}

/* Do Keyword Stuff */

typedef struct keywordarg
{
    char *last;
    int *i;
}__zint_karg_t;

static void z_KeyWord_0_addvariable_to_scope(void *arg)
{
    char *tok;
    char *last = ((__zint_karg_t*)(arg))->last;
    tok = strtok_r(last, " \n\t",  &last);

    int VarCount = 1;
    if (z_isDigit__MF( tok[0] ))
    {
        VarCount = atoi(tok);
    }

    for (int i = 0; i < VarCount; ++i)
    {
        z_Variable_pushVariableToScope(1, 1);
    }

}
static void z_KeyWord_1_delvariable_from_scope(void *arg)
{
    char *tok;
    char *last = ((__zint_karg_t*)(arg))->last;
    tok = strtok_r(last, " \n\t",  &last);

    int VarCount = 1;
    if (z_isDigit__MF( tok[0] ))
    {
        VarCount = atoi(tok);
    }

    for (int i = 0; i < VarCount; ++i)
    {
        z_Variable_popVariableFromScope();
    }

}

static void z_KeyWord_2_inBuild_Print(void *arg)
{
    char *tok;
    char *last = ((__zint_karg_t*)(arg))->last;
    tok = strtok_r(last, "/",  &last);

    static const char ESCAPE_CHAR[] =
    {
        '$','\n','\\','#','\t', '\b', '|'
    /*   0    1    2   3    4     5     6
     *   
     */
    };

    while(tok != NULL)
    {
        if (tok[0] == _ZINT__PreP_VARIABLE_SYMB)
        {
            char * t = &tok[1];
            int VarPos = atoi(t);
            printf("%.0lf", z_Variable_accessVariable(VarPos));
        }
        else if (tok[0] == '\\')
        {
            putchar(ESCAPE_CHAR[z_charToInt__MF(tok[1])]);
        }
        else
        {
            fputs(tok ,stdout);
        }

        tok = strtok_r(last, "/",  &last);
    }

}

static void z_KeyWord_3_inBuild_Input(void *arg)
{
    char *tok;
    char *last = ((__zint_karg_t*)(arg))->last;
    tok = strtok_r(last, " \n\t",  &last);

    char * t = &tok[1];
    int VarPos = atoi(t);
    double newVal;
    scanf("%lf", &newVal);
    z_Variable_changeValue(VarPos, newVal);

}

static void z_KeyWord_4_GOTO(void *arg)
{
    int *i = ((__zint_karg_t*)(arg))->i;
    char *tok;
    char *last = ((__zint_karg_t*)(arg))->last;
    tok = strtok_r(last, " \n\t",  &last);

     //printf("GOTOPOS:%i s:%s", *i+atoi(tok), tok);

    *i += atoi(tok);
   
}

static char z_operator_compare_0_Eq(double a, double b)
{
    return (a == b)? 1:0;
}
static char z_operator_compare_1_Greater(double a, double b)
{
    return (a > b)? 1:0;
}
static char z_operator_compare_2_Lesser(double a, double b)
{
    return (a < b)? 1:0;
}
static char z_operator_compare_3_NotEq(double a, double b)
{
    return (a != b)? 1:0;
}
typedef char (*Operator_Compare_TF)(double, double);
static const Operator_Compare_TF gOperator_Compare_F[] = {
      z_operator_compare_0_Eq
    , z_operator_compare_1_Greater
    , z_operator_compare_2_Lesser
    , z_operator_compare_3_NotEq
};
static void z_KeyWord_5_COMPARE(void *arg)
{
    int *i = ((__zint_karg_t*)(arg))->i;
    char *tok;
    char *last = ((__zint_karg_t*)(arg))->last;
    tok = strtok_r(last, " \n\t",  &last);

    int call = atoi(tok);
    double a = 0, b = 0;

    // !4 0 #1 #1 5

    tok = strtok_r(last, " \n\t",  &last);
    if (*tok == '#')
    {
        tok++;
        a = atof(tok);
        a = z_Variable_accessVariable((int)a);
    } else {
        a = atof(tok);
    }


    tok = strtok_r(last, " \n\t",  &last);
    if (*tok == '#')
    {
        tok++;
        b = atof(tok);
        b = z_Variable_accessVariable((int)b);
    }
    else
    {
        b = atof(tok);
    }

    if (!gOperator_Compare_F[call](a, b))
    {
        
        tok = strtok_r(last, " \n\t",  &last);
        *i += atoi(tok);
    }

}

static void z_funtion(void *arg, StringLines_t ln ,Var_t * returntype);
static void z__DEV_showallStack(void);
static void z_KeyWord_6_Function_Call(void *arg)
{
    char *tok;

    char *last = ((__zint_karg_t*)(arg))->last;
    tok = strtok_r(last, " \n\t",  &last);

    int call = atoi(tok);

    #ifdef ZINT_DEBUG_ENABLED

    if (gFUNCTIONS.size < call)
    {
        dieOnCommand("Funtion Call Not in Memory", ERROR_CODE__PREP_SYNTAX , tok);
    }

    #endif



    z_Variable_pushVariableScope();


        Var_t a;
        

        z_funtion(NULL, gFUNCTIONS.fn[call], &a);



    z_Variable_popVariableScope();

}
static void z_KeyWord_7_DIE(void *arg)
{
    z_KeyWord_2_inBuild_Print(arg);
    dieOnCommand("Called By Program", 0, "\n");
}

static void z_KeyWord_8_PushScope(void *arg)
{
    
}
static void z_KeyWord_9_PopScope(void *arg)
{
    
}

typedef void (*Keyword_t)(void*);
#define KEYWORD_MAX 10
Keyword_t CALL_keywords[KEYWORD_MAX] = {

          z_KeyWord_0_addvariable_to_scope
        , z_KeyWord_1_delvariable_from_scope
        , z_KeyWord_2_inBuild_Print
        , z_KeyWord_3_inBuild_Input
        , z_KeyWord_4_GOTO
        , z_KeyWord_5_COMPARE
        , z_KeyWord_6_Function_Call

        , z_KeyWord_7_DIE
        /*, z_KeyWord_8_*/
        /*, z_KeyWord_9_*/

};

static void z_funtion(void *arg, const StringLines_t ln ,Var_t * returntype)
{
    int id;

    char *tmp_line = malloc(sizeof(char) * ln.x);
    char *tmp_line_main = malloc(sizeof(char) * ln.x);
    memcpy(tmp_line_main, ln.lines[0], ln.x);

    char * strtok_r__line = tmp_line_main;
    char * token = strtok_r(strtok_r__line, " \n\t", &strtok_r__line);

    if (token[0] != '@')
    {

        dieOnCommand("NOT Pointed To Lable", ERROR_CODE__SYNTAX, token);
    }

    sscanf(&tmp_line_main[1], "%d", &id);

    token = strtok_r(strtok_r__line, " \n\t", &strtok_r__line);

    Var_t *v = arg;
    int var_count = atof(token);



    for (int i = 0; i < var_count ; ++i)
    {

        z_Variable_pushVariableToScope(v[i].type, v[i].size);
        z_Variable_changeValue(i, *v[i].value);

    }





    int i = 0;
    while (1)
    {
        GOTO_z_function_ENDWHILE:;
        i++;

        memcpy(tmp_line, ln.lines[i], ln.x);
        memcpy(tmp_line_main, tmp_line, ln.x);

        strtok_r__line = tmp_line_main;

        
        token = strtok_r(strtok_r__line, " \n\t", &strtok_r__line);
        

        while (token != NULL)
        {
            
            if (*token == _ZINT__PreP_KEYWORD_SYMB )
            {

                char KEY_num = z_charToInt__MF(token[1]);


                CALL_keywords[(int)KEY_num](&(__zint_karg_t){strtok_r__line , &i});
                goto GOTO_z_function_ENDWHILE;
            }

            else if (token[0] == _ZINT__PreP_VARIABLE_SYMB )
            {
                char * t = &token[1];
                int VarPos = atoi(t);
                token = strtok_r(strtok_r__line, " \n\t", &strtok_r__line);
                if (token[0] == '=')
                {
                    token = strtok_r(strtok_r__line, " \n\t", &strtok_r__line);
                    if (token != NULL)
                    {
                        z_operator_asign(tmp_line, ln.x, VarPos);
                    }
                    else
                    {
                        dieOnCommand("No Values Done For Assignment Operetor", ERROR_CODE__SYNTAX, token);
                    }
                }

            }

            else if (token[0] == '@')
            {
                if (token[1] == '@')
                {
                    double val;
                    token = strtok_r(strtok_r__line, " \n\t", &strtok_r__line);
                    if (token != NULL)
                    {
                        if (token[0] == '.' || z_isDigit__MF(token[0]))
                        {
                            val = atof(token);
                        }
                    }

                    free(tmp_line);
                    free(tmp_line_main);
                    *returntype = (Var_t){ NULL, 0, 0 };
                    return;
                }

            }


            

            token = strtok_r(strtok_r__line, " \n\t", &strtok_r__line);
            
            
        }
    }



}

/* Interprepor */
static int z_interpreter(const String_t ZFILE_PreP)
{

    String_t zfile_tmp = z__copyString(ZFILE_PreP);


    gFUNCTIONS = z_BreakStringIntoFunctions(zfile_tmp);

    Var_t a;

    z_funtion(NULL, gFUNCTIONS.fn[0] ,&a);

    
    z__destroyStringLinesArr(&gFUNCTIONS);
    z__deleteString(&zfile_tmp);

    return 0;
}


static int z_start(char const  * filename)
{
    String_t zfile = z_ReadFromFile((char *)filename);
    

    if (zfile.str == NULL)
    {
        dieOnCommand("Cannot Open zFile", ERROR_CODE__MISC__FILE_CANT_BE_OPENED, (char*)filename);
    }

    z_interpreter(zfile);
    z__deleteString(&zfile);

    return 0;
}
/********************************************/
/*             Main Interpretor             */
/*            ------------------            */
/*                   END                    */
/*                  -----                   */
/********************************************/



/********************************************/
/*             Developer Toolkit            */
/*            ------------------            */
/*                   START                  */
/*                  -------                 */
/********************************************/
#ifdef ZINT_DEV_TEST_ENABLED

static void z__DEV_showallStack(void)
{
    fputs("\n\n/********/\nShowing All Scopes In Memory\n/********/\n\n", stdout);

    fputs("SCOPE:\n[", stdout);
    for (int i = 0; i < z_Variable_getScopeSize__MF() ; ++i)
    {
        if (i < z_Variable_getScopeUsed__MF())
        {
            printf(" 1,");
        }
        else {
            printf(" 0,");
        }
    }
    printf("\b ]\n");

    fputs("\n\n/********/\nShowing All Variables In Scopes\n/********/\n\n", stdout);
    for (int i = 0; i < z_Variable_getScopeUsed__MF(); ++i)
    {
        printf("SCOPE:%03d: [", i);
        for (int j = 0; j < z_Variable_getVarSizein__MF(i); ++j)
        {
            if (j < z_Variable_getVarUsedin__MF(i))
            {
                printf(" 1,");
            }
            else
            {
                printf(" 0,");
            }
        }
        printf("\b ]\n");
    }
}

static void z__DEV_show_FunctionCode(StringLineArr_t lns)
{
    printf("Funtions: %d\n", lns.used);
    puts("Showing Code");

    for (int i = 0; i < lns.used; ++i)
    {
        printf("\nFUNC ID: %d\n--\n\n", i);
        for (int line = 0; line < lns.fn[i].y; ++line)
        {
            printf("%s;", lns.fn[i].lines[line]);
        }
    }

}

static void z__DEV_main(char *arg0, char *arg1)
{
    z__DEV_showallStack();
}

#endif
/********************************************/
/*             Developer Toolkit            */
/*            ------------------            */
/*                   END                    */
/*                  -----                   */
/********************************************/



/********************************************/
/*             Argument Phraser             */
/*            ------------------            */
/*                   START                  */
/*                  -------                 */
/********************************************/
int phrase(int argc, char *argv[])
{
    for (int i = 0; i < argc; ++i)
    {
        if (argv[i][0] == '-')
        {
            switch(argv[i][1])
            {
                case 'h':
                    puts(ZAKAROUF__ZINT_HELP);
                break;

                case 'i':
                    z_start(argv[i+1]);
                    
                break;


            /* Call DevToolkit */
            /*------START------*/
                
                case 'D':
                #ifdef ZINT_DEV_TEST_ENABLED
                    z__DEV_main(argv[i], argv[i+1]);
                #else
                    fputs("This Build Of Zint is not Made with Developer mode", stderr);
                #endif
                break;
                
                
            /* Call DevToolkit */
            /*-------END-------*/

                default:
                break;
            }
        }
    }

    return 0;
}
/********************************************/
/*             Argument Phraser             */
/*            ------------------            */
/*                   END                    */
/*                  -----                   */
/********************************************/


int main(int argc, char const *argv[])
{
    
    z_Variable_init();
    

    phrase(argc, (char **)argv);



    //z_Variable_destroy();

    #ifdef ZINT_DEBUG__LOGGER_LOG_MEMORY
        printf("Memory Usage :=> %ldKB\n", zint_sys_getRamUsage()/1024);
    #endif
    return 0;
}
