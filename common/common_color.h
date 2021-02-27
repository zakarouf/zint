#ifndef ZAKAROUF__ZINT_COMMON_COLOR_H
#define ZAKAROUF__ZINT_COMMON_COLOR_H

#define color256_set(code)\
    printf("\x1b[38;5;%d", code)

#define color256_B_set(code)\
    printf("\x1b[48;5;%d", code)

#define color_reset() \
    printf("\x1b[0m")

#endif