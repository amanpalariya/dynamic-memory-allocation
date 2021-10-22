#include "ds.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "logger.h"

struct stats* get_empty_stats() {
    struct stats* stat = (struct stats*)malloc(sizeof(struct stats));
    stat->turnaround_time_num = 0;
    stat->turnaround_time_den = 0;
    stat->memory_utilization_num = 0;
    stat->memory_utilization_den = 0;
}

struct process* get_new_process(int s, int d, struct timeval arrival_time) {
    struct process* proc = (struct process*)malloc(sizeof(struct process));
    proc->s = s;
    proc->d = d;
    proc->arrival_time = arrival_time;
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

int get_address_of_partition(struct memory* mem, struct partition* part) {
    struct partition* iter = mem->head;
    int address = 0;
    while (iter != NULL) {
        if (iter == part) break;
        address += iter->size;
        iter = iter->next;
    }
    return address;
}

float get_percentage_memory_utilization(struct memory* mem) {
    struct partition* iter = mem->head;
    int utilization_in_MB = mem->q;
    while (iter != NULL) {
        utilization_in_MB += iter->is_free ? 0 : iter->size;
        iter = iter->next;
    }
    return (100.0f * utilization_in_MB) / mem->p;
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

void init_process_queue_node(struct process_queue_node* node, struct process* proc, struct process_queue_node* prev, struct process_queue_node* next) {
    node->proc = proc;
    node->prev = prev;
    node->next = next;
}

struct process_queue_node* get_new_empty_process_queue_node() {
    return (struct process_queue_node*)malloc(sizeof(struct process_queue_node));
}

struct process_queue_node* get_new_process_queue_node(struct process* proc, struct process_queue_node* prev, struct process_queue_node* next) {
    struct process_queue_node* node = get_new_empty_process_queue_node();
    init_process_queue_node(node, proc, prev, next);
    return node;
}

void free_process_queue_node(struct process_queue_node* node) {
    free_process(node->proc);
    free(node);
}

bool is_queue_full(struct process_queue* queue) {
    return queue->size == queue->max_size;
}

bool is_queue_empty(struct process_queue* queue) {
    return queue->size == 0;
}

void init_queue(struct process_queue* queue, int max_size) {
    queue->max_size = max_size;
    queue->size = 0;
    queue->head = NULL;
    queue->tail = NULL;
}

struct process_queue* get_new_empty_queue(int max_size) {
    struct process_queue* queue = (struct process_queue*)malloc(sizeof(struct process_queue));
    init_queue(queue, max_size);
    return queue;
}

bool __enqueue(struct process_queue* queue, struct process* proc) {
    struct process_queue_node* node = get_new_process_queue_node(proc, NULL, queue->head);
    if (is_queue_empty(queue)) {
        queue->head = node;
        queue->tail = node;
        queue->size = 1;
        return true;
    } else if (is_queue_full(queue)) {
        return false;
    } else {
        queue->head->prev = node;
        queue->head = node;
        queue->size += 1;
        return true;
    }
}

bool enqueue(struct process_queue* queue, struct process* proc) {
    bool response = __enqueue(queue, proc);
    return response;
}

struct process* __dequeue(struct process_queue* queue) {
    if (is_queue_empty(queue)) {
        return NULL;
    } else if (queue->size == 1) {
        struct process_queue_node* removed_node = queue->tail;
        struct process* proc = removed_node->proc;
        queue->head = NULL;
        queue->tail = NULL;
        queue->size = 0;
        free(removed_node);
        return proc;
    } else {
        struct process_queue_node* removed_node = queue->tail;
        struct process* proc = removed_node->proc;
        queue->tail = queue->tail->prev;
        queue->tail->next = NULL;
        queue->size -= 1;
        free(removed_node);
        return proc;
    }
}

struct process* dequeue(struct process_queue* queue) {
    struct process* response = __dequeue(queue);
    return response;
}

struct process* peek_queue(struct process_queue* queue) {
    if (is_queue_empty(queue)) {
        return NULL;
    } else {
        return queue->tail->proc;
    }
}

void free_queue(struct process_queue* queue) {
    struct process_queue_node* node = queue->head;
    while (node != NULL) {
        free_process_queue_node(node);
        node = node->next;
    }
    free(queue);
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