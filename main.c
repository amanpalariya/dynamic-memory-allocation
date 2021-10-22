#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "ds.h"
#include "helper.h"
#include "logger.h"
#include "simulator.h"

char* get_algo_name_from_enum(enum placement_algo algo) {
    switch (algo) {
        case FIRST_FIT:
            return "First fit";
            break;
        case BEST_FIT:
            return "Best fit";
            break;
        case NEXT_FIT:
            return "Next fit";
            break;
    }
    return "Unknown";
}

int main(int argc, char** argv) {
    srand(time(NULL));

    int p = 1000;  // Total main memory
    int q = 200;   // Memory reserved for OS
    int n = 10;    // n>=1
    int m = 10;    // m>=10
    int t = 10;    // t>=5
    int T = 2;     // Simulation duration in minutes
    struct stats* stat = get_empty_stats();
    int MAX_QUEUE_SIZE = 10;
    enum placement_algo algo = FIRST_FIT;

    scanf("%d %d %d %d %d %d %d %d", &p, &q, &n, &m, &t, &T, &MAX_QUEUE_SIZE, (int*)&algo);

    bool error = false;

    if (p <= 0 || q <= 0 || q >= p) {
        log_error("Please provide valid memory size and reserved memory, got (p: %dMB, q: %dMB)", p, q);
        error = true;
    }
    if (n <= 0 || m <= 0 || t <= 0 || T <= 0) {
        log_error("n, m, t, and T must be positive integers, got (n: %d, m: %dMB, t: %dsec, T: %dmin)", n, m, t, T);
        error = true;
    }
    if (MAX_QUEUE_SIZE <= 0) {
        log_error("Maximum queue size should be positive integer, got %d", MAX_QUEUE_SIZE);
        error = true;
    }
    if (algo != FIRST_FIT && algo != BEST_FIT && algo != NEXT_FIT) {
        log_error("Placement algorithm should be either 0 (first fit), 1 (best fit), or 2 (next fit), got %d", algo);
        error = true;
    }

    if (error) {
        return 1;
    }

    float r = randint(0.1 * n * 1e4, 1.2 * n * 1e4) / 1e4;  // Number of process spawing per second

    log_info("RUNNING SIMULATION WITH FOLLOWING CONFIG");
    log_info("p: %dMB", p);
    log_info("q: %dMB", q);
    log_info("n: %d", n);
    log_info("m: %dMB", m);
    log_info("t: %dsec", t);
    log_info("T: %dmin", T);
    log_info("r: %.2f", r);
    log_info("Algo: %s", get_algo_name_from_enum(algo));

    run(p, q, n, m, t, r, algo, MAX_QUEUE_SIZE, stat);
    sleep(T * 60);

    float avg_turnaround_time = (stat->turnaround_time_den == 0 ? 0 : (1.0f * stat->turnaround_time_num) / stat->turnaround_time_den);
    float avg_mem_util = (stat->memory_utilization_den == 0 ? 0 : (stat->memory_utilization_num / stat->memory_utilization_den));
    log_stat("Avg. turnaround time: %.2fms, Avg. memory util: %.2f%", avg_turnaround_time, avg_mem_util);
    return 0;
}