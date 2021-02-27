#include <stdio.h>

#include "zint_sys.h"


#ifdef ZINT_DEBUG__LOGGER_LOG_MEMORY
long zint_sys_getRamUsage(void)
{
    struct rusage usage;
    int ret;
    ret = getrusage(RUSAGE_SELF, &usage);
    return usage.ru_maxrss;
}
#endif


