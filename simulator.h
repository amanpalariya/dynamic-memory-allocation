#ifndef CS303_SIMULATOR_H
#define CS303_SIMULATOR_H

#include "ds.h"

enum placement_algo {
    FIRST_FIT = 0,
    BEST_FIT = 1,
    NEXT_FIT = 2
};

void run(int p, int q, int n, int m, int t, int r, enum placement_algo algo, int MAX_QUEUE_SIZE, struct stats* stat);

#endif