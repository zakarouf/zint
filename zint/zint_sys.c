#include <stdio.h>
#include <stdlib.h>

#include "zint_sys.h"
#include "../common.h"

#ifdef ZINT_DEBUG__LOGGER_LOG_MEMORY
long zint_sys_getRamUsage(void)
{
    struct rusage usage;
    int ret;
    ret = getrusage(RUSAGE_SELF, &usage);
    return usage.ru_maxrss;
}
#endif

#ifdef ZINT_DEBUG__LOGGER_ENABLED
void zint_log(char **stl, int lines)
{
    #ifdef ZINT_SYS__COLORS_ENABLED
        color256_set(2);
    #endif

        fprintf(stdout, "LOG:%s\n", stl[0] );
        for (int i = 1; i < lines; ++i)
        {
            fprintf(stdout, "\t\tSUB[%d]:%s\n", i, stl[i] );
        }

    #ifdef ZINT_SYS__COLORS_ENABLED
        color_reset();
    #endif
}
#endif

/* Kill Switch if Anything goes wrong ( Usually Used For Debug Purpose ) */
void dieOnCommand(char *msg, int CODE, char * codesnip)
{
    #ifdef ZINT_SYS__COLORS_ENABLED
        color256_set(1);
    #endif
    fprintf(stderr, "DIED:%d:%s\n\n>>> %s\n", CODE, msg, codesnip);

    #ifdef ZINT_SYS__COLORS_ENABLED
        color_reset();
    #endif

    exit(1);
}
