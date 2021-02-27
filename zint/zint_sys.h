#ifndef ZAKAROUF__ZINT_SYS_H
#define ZAKAROUF__ZINT_SYS_H

#include "zint_config.h"

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

long zint_sys_getRamUsage(void);

#ifdef ZINT_DEBUG__LOGGER_ENABLED
void zint_log(char **, int);
#endif

void dieOnCommand(char *msg, int CODE, char * codesnip);

#endif