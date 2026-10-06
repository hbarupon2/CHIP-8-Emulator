#pragma once

#include <stdio.h>

static void clear_screen() {
    printf("\033[2J\033[1;1H");
    fflush(stdout);
}
