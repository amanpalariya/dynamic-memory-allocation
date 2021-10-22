#include "simulator.h"

#include <pthread.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>

#include "ds.h"
#include "helper.h"
#include "logger.h"

struct run_process_args {
    struct process* proc;
    struct partition* part;
    int partition_address;
    pthread_mutex_t* mem_mutex;
    pthread_cond_t* mem_available;
};

struct process_creator_args {
    struct process_queue* queue;
    int r;
    int m;
    int t;
    pthread_mutex_t* queue_mutex;
};

struct process_allocator_args {
    struct process_queue* queue;
    int p;
    int q;
    pthread_mutex_t* mem_mutex;
    pthread_mutex_t* queue_mutex;
    pthread_cond_t* mem_available;
    enum placement_algo algo;
    struct stats* stat;
};

struct run_process_args* get_run_process_args(struct process* proc, struct partition* part, int partition_address, pthread_mutex_t* mem_mutex, pthread_cond_t* mem_available) {
    struct run_process_args* args = (struct run_process_args*)malloc(sizeof(struct run_process_args));
    args->proc = proc;
    args->part = part;
    args->partition_address = partition_address;
    args->mem_mutex = mem_mutex;
    args->mem_available = mem_available;
    return args;
}

struct process_creator_args* get_process_creator_args(struct process_queue* queue, int r, int m, int t, pthread_mutex_t* queue_mutex) {
    struct process_creator_args* args = (struct process_creator_args*)malloc(sizeof(struct process_creator_args));
    args->queue = queue;
    args->m = m;
    args->t = t;
    args->r = r;
    args->queue_mutex = queue_mutex;
    return args;
}

struct process_allocator_args* get_process_allocator_args(struct process_queue* queue, int p, int q, pthread_mutex_t* mem_mutex, pthread_mutex_t* queue_mutex, pthread_cond_t* mem_available, enum placement_algo algo, struct stats* stat) {
    struct process_allocator_args* args = (struct process_allocator_args*)malloc(sizeof(struct process_allocator_args));
    args->queue = queue;
    args->p = p;
    args->q = q;
    args->mem_mutex = mem_mutex;
    args->queue_mutex = queue_mutex;
    args->mem_available = mem_available;
    args->algo = algo;
    args->stat = stat;
    return args;
}

struct timeval get_curr_time() {
    struct timeval t;
    gettimeofday(&t, NULL);
    return t;
}

long get_time_diff_in_millis(struct timeval start, struct timeval end) {
    return (end.tv_sec - start.tv_sec) * 1000000 + end.tv_usec - start.tv_usec;
}

struct process* get_random_process(int m, int t) {
    int size_in_megabyte = 10 * ((5 + randint(0.5 * m, 3.0 * m)) / 10);
    int duration_in_sec = 5 * ((int)((2.5 + randint(0.5 * t, 6.0 * t)) / 5));
    return get_new_process(size_in_megabyte, duration_in_sec, get_curr_time());
}

struct partition* allocate(struct memory* mem, struct process* proc, int* last_address, enum placement_algo algo) {
    switch (algo) {
        case FIRST_FIT:
            return first_fit(mem, proc->s);
            break;
        case BEST_FIT:
            return best_fit(mem, proc->s);
            break;
        case NEXT_FIT:
            return next_fit(mem, proc->s, *last_address);
            break;
    }
}

void* run_process(void* args) {
    struct run_process_args* _args = (struct run_process_args*)(args);
    struct process* proc = _args->proc;
    sleep(proc->d);
    pthread_mutex_t* mem_mutex = _args->mem_mutex;
    pthread_mutex_lock(mem_mutex);
    struct partition* part = _args->part;
    int address = _args->partition_address;
    deallocate_partition(part);
    log_warning("%dMB partition [%d, %d] freed from process (s: %dMB, d: %ds)", part->size, address, address + part->size, proc->s, proc->d);
    pthread_mutex_unlock(mem_mutex);
    pthread_cond_broadcast(_args->mem_available);
}

void* process_creator(void* args) {
    struct process_creator_args* _args = (struct process_creator_args*)(args);
    struct process_queue* queue = _args->queue;
    pthread_mutex_t* queue_mutex = _args->queue_mutex;
    int r = _args->r;
    int m = _args->m;
    int t = _args->t;
    int step_time_in_millis = 10;
    while (true) {
        usleep(step_time_in_millis * 1000);
        if (randint(0, 1000) < (r * step_time_in_millis)) {
            struct process* proc = get_random_process(m, t);
            pthread_mutex_lock(queue_mutex);
            log_info("New process (s: %dMB, d: %ds) generated", proc->s, proc->d);
            if (enqueue(queue, proc)) {
                log_info("Process (s: %dMB, d: %ds) queued", proc->s, proc->d);
            } else {
                log_warning("Process (s: %dMB, d: %ds) could NOT be queued, queue full", proc->s, proc->d);
            }
            pthread_mutex_unlock(queue_mutex);
        }
    }
}

void log_stats(struct stats* stat) {
    float avg_turnaround_time = (stat->turnaround_time_den == 0 ? 0 : (1.0f * stat->turnaround_time_num) / stat->turnaround_time_den);
    float avg_mem_util = (stat->memory_utilization_den == 0 ? 0 : (stat->memory_utilization_num / stat->memory_utilization_den));
    log_info("Avg. turnaround time: %.2fms, Avg. memory util: %.2f%", avg_turnaround_time, avg_mem_util);
}

void* process_allocator(void* args) {
    struct process_allocator_args* _args = (struct process_allocator_args*)(args);
    int p = _args->p;
    int q = _args->q;
    struct process_queue* queue = _args->queue;
    struct memory* mem = get_new_empty_memory(p, q);
    pthread_mutex_t* mem_mutex = _args->mem_mutex;
    pthread_mutex_t* queue_mutex = _args->queue_mutex;
    pthread_cond_t* mem_available = _args->mem_available;
    struct stats* stat = _args->stat;
    enum placement_algo algo = _args->algo;
    int last_address = 0;

    while (true) {
        usleep(10000);
        if (!is_queue_empty(queue)) {
            struct process* proc = peek_queue(queue);
            log_info("Spawing process (s: %dMB, d: %ds)", proc->s, proc->d);

            pthread_mutex_lock(mem_mutex);  // Lock

            struct partition* part = allocate(mem, proc, &last_address, algo);
            if (part != NULL) {
                stat->turnaround_time_num += get_time_diff_in_millis(proc->arrival_time, get_curr_time());
                stat->turnaround_time_den += 1;
                pthread_mutex_lock(queue_mutex);  // Q Lock
                dequeue(queue);
                pthread_mutex_unlock(queue_mutex);  // Q Unlock
                int address = get_address_of_partition(mem, part);
                pthread_t thread_id;
                pthread_create(&thread_id, NULL, run_process, get_run_process_args(proc, part, address, mem_mutex, mem_available));
                log_info("Process (s: %dMB, d: %ds) allocated %dMB partition [%d, %d]", proc->s, proc->d, part->size, address, address + part->size);
                print_memory(mem);
            } else {
                log_warning("Not enough memory for process (s: %dMB, d: %ds)", proc->s, proc->d);
                pthread_cond_wait(mem_available, mem_mutex);  // Condition wait
            }
            stat->memory_utilization_num += get_percentage_memory_utilization(mem);
            stat->memory_utilization_den += 1;

            log_stats(stat);

            pthread_mutex_unlock(mem_mutex);  // Unlock
        }
    }
}

void run(int p, int q, int n, int m, int t, int r, enum placement_algo algo, int MAX_QUEUE_SIZE, struct stats* stat) {
    struct process_queue* queue = get_new_empty_queue(MAX_QUEUE_SIZE);
    pthread_mutex_t* mem_mutex = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
    pthread_mutex_t* queue_mutex = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
    pthread_cond_t* mem_available = (pthread_cond_t*)malloc(sizeof(pthread_cond_t));

    pthread_mutex_init(mem_mutex, NULL);
    pthread_mutex_init(queue_mutex, NULL);
    pthread_cond_init(mem_available, NULL);

    pthread_t process_creator_thread_id, process_allocator_thread_id;
    pthread_create(&process_creator_thread_id, NULL, process_creator, get_process_creator_args(queue, r, m, t, queue_mutex));
    pthread_create(&process_allocator_thread_id, NULL, process_allocator, get_process_allocator_args(queue, p, q, mem_mutex, queue_mutex, mem_available, algo, stat));
}