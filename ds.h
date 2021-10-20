#ifndef CS303_DS_H
#define CS303_DS_H

#include <stdbool.h>

struct process {
    int s;  // Size of process in MBs
    int d;  // Duration of process in seconds
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

struct process* get_new_process(int s, int d);

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

void print_memory(struct memory* mem);

void free_process(struct process* proc);

void free_partition(struct partition* part);

void free_memory(struct memory* mem);

#endif