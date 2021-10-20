#include "ds.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "logger.h"

struct process* get_new_process(int s, int d) {
    struct process* proc = (struct process*)malloc(sizeof(struct process));
    proc->s = s;
    proc->d = d;
    return proc;
}

struct partition* get_new_partition(struct partition* prev, struct partition* next, int size, int is_free) {
    struct partition* part = (struct partition*)malloc(sizeof(struct partition));
    part->prev = prev;
    part->next = next;
    part->size = size;
    part->is_free = is_free;
    return part;
}

struct memory* get_new_empty_memory(int p, int q) {
    struct memory* mem = (struct memory*)malloc(sizeof(struct memory));
    mem->p = p;
    mem->q = q;
    mem->head = get_new_partition(NULL, NULL, p - q, true);
    return mem;
}

void compact(struct memory* mem) {
    struct partition* part = mem->head;
    while (part != NULL) {
        struct partition* next_part = part->next;
        if (part->is_free) {
            while (next_part != NULL || next_part->is_free) {
                part->size += next_part->size;
                struct partition* part_to_free = next_part;
                next_part = next_part->next;
                free_partition(part_to_free);
            }
            part->next = next_part;
            if (next_part != NULL) {
                next_part->prev = part;
            }
        }
        part = next_part;
    }
}

struct partition* allocate_partition(struct partition* part, int process_size) {
    if (part == NULL || process_size > part->size || !part->is_free)
        return NULL;
    if (part->size > process_size) {
        struct partition* free_part = get_new_partition(part, part->next, part->size - process_size, true);
        if (part->next != NULL)
            part->next->prev = free_part;
        part->next = free_part;
    }
    part->size = process_size;
    part->is_free = false;
    return part;
}

void deallocate_partition(struct partition* part) {
    if (part->is_free) return;
    part->is_free = true;
    if (part->next != NULL && part->next->is_free) {
        part->size += part->next->size;
        struct partition* part_to_free = part->next;
        part->next = part->next->next;
        if (part->next != NULL) {
            part->next->prev = part;
        }
        free_partition(part_to_free);
    }
    if (part->prev != NULL && part->prev->is_free) {
        part->prev->size += part->size;
        part->prev->next = part->next;
        free_partition(part);
    }
}

struct partition* first_fit(struct memory* mem, int process_size) {
    struct partition* part = mem->head;
    while (part != NULL) {
        if (part->is_free && part->size >= process_size)
            break;
        part = part->next;
    }
    return allocate_partition(part, process_size);
}

struct partition* best_fit(struct memory* mem, int process_size) {
    struct partition* part = mem->head;
    struct partition* best_part = NULL;
    int min_fragmentation = __INT_MAX__;
    while (part != NULL) {
        if (part->is_free && part->size >= process_size) {
            int fragmentation = part->size - process_size;
            if (fragmentation < min_fragmentation) {
                min_fragmentation = fragmentation;
                best_part = part;
            }
        }
        part = part->next;
    }
    return allocate_partition(best_part, process_size);
}

struct partition* next_fit(struct memory* mem, int process_size, int starting_address) {
    struct partition* part = mem->head;
    struct partition* fit = NULL;
    int address = 0;
    while (part != NULL) {
        if (address >= starting_address) break;
        if (part->is_free && part->size >= process_size && fit != NULL)
            fit = part;
        address += part->size;
        part = part->next;
    }
    while (part != NULL) {
        if (part->is_free && part->size >= process_size) {
            fit = allocate_partition(part, process_size);
            break;
        }
        part = part->next;
    }
    return allocate_partition(fit, process_size);
}

void print_memory(struct memory* mem) {
    struct partition* part = mem->head;
    log_info("┌────────┐");
    log_info("│ %s %4d │", part->is_free ? " " : "✓", part->size);
    part = part->next;
    while (part != NULL) {
        log_info("├────────┤");
        log_info("│ %s %4d │", part->is_free ? " " : "✓", part->size);
        part = part->next;
    }
    log_info("└────────┘");
    fflush(stdout);
}

void free_process(struct process* proc) {
    free(proc);
}

void free_partition(struct partition* part) {
    free(part);
}

void free_memory(struct memory* mem) {
    struct partition* part = mem->head;
    while (part != NULL) {
        struct partition* part_to_free = part;
        part = part->next;
        free_partition(part_to_free);
    }
    free(mem);
}