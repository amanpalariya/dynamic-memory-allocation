#include "helper.h"

#include <stdlib.h>

int randint(int min, int max) {
    if (min > max) return 0;
    return min + rand() % (max - min + 1);
}