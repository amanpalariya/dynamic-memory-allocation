#ifndef CS303_DS_H
#define CS303_DS_H

#include <stdbool.h>
#include <sys/time.h>

struct process {
    int s;  // Size of process in MBs
    int d;  // Duration of process in seconds
    struct timeval arrival_time;
};

struct partition {
    struct partition* prev;
    struct partition* next;
    int size;  // Size of partition in MBs
    int is_free;
};

struct memory {
    int p;  // Total memory in MBs
    int q;  // Memory reserved for OS
    struct partition* head;
};

struct process_queue_node {
    struct process* proc;
    struct process_queue_node* prev;
    struct process_queue_node* next;
};

struct process_queue {
    int max_size;
    int size;
    struct process_queue_node* head;
    struct process_queue_node* tail;
};

struct stats {
    long turnaround_time_num;
    int turnaround_time_den;
    int memory_utilization_num;
    int memory_utilization_den;
};

struct stats* get_empty_stats();

struct process* get_new_process(int s, int d, struct timeval arrival_time);

struct partition* get_new_partition(struct partition* prev, struct partition* next, int size, int is_free);

struct memory* get_new_empty_memory(int p, int q);

/*
Allocated memory on partition `part`
Returns NULL is memory could not be allocated
        `part` is memory was allocated
*/
struct partition* allocate_partition(struct partition* part, int process_size);

void deallocate_partition(struct partition* part);

struct partition* first_fit(struct memory* mem, int process_size);

struct partition* best_fit(struct memory* mem, int process_size);

struct partition* next_fit(struct memory* mem, int process_size, int starting_address);

int get_address_of_partition(struct memory* mem, struct partition* part);

float get_percentage_memory_utilization(struct memory* mem);

void print_memory(struct memory* mem);

bool is_queue_full(struct process_queue* queue);

bool is_queue_empty(struct process_queue* queue);

void init_queue(struct process_queue* queue, int max_size);

struct process_queue* get_new_empty_queue(int max_size);

bool enqueue(struct process_queue* queue, struct process* proc);

struct process* dequeue(struct process_queue* queue);

struct process* peek_queue(struct process_queue* queue);

void free_queue(struct process_queue* queue);

void free_process(struct process* proc);

void free_partition(struct partition* part);

void free_memory(struct memory* mem);

#endif