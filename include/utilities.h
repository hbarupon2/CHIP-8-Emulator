#pragma once

#include <stdio.h>

static void clear_screen() {
    /* Home first. Erasing with 2J before the cursor moves makes Terminal.app
       push the screen into scrollback and leave the window on the old text. */
    printf("\033[H\033[2J\033[3J");
    fflush(stdout);
}

static uint64_t now_ns() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t) ts.tv_sec * 1000000000ull + (uint64_t) ts.tv_nsec;
}

static void sleep_until(uint64_t time) {
    uint64_t now = now_ns();
    if (now >= time)
        return;
    uint64_t delta = time - now;
    struct timespec ts = {
        .tv_sec = delta / 1000000000ull,
        .tv_nsec = delta % 1000000000ull
    };
    nanosleep(&ts, NULL);
}