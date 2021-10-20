#include <stdbool.h>
#include <stdio.h>

#include "../ds.h"
#include "../logger.h"

int total_tests = 0;
int passed_tests = 0;
int failed_tests = 0;

void test_log(char* test_name, bool success) {
    total_tests++;
    if (success) {
        printf("\033[0;32m[✓]: %s\033[0m\n", test_name);
        passed_tests++;
    } else {
        printf("\033[0;31m[ ]: %s\033[0m\n", test_name);
        failed_tests++;
    }
}

void print_test_section(char* section) {
    printf("\n\033[0;1m%s\033[0m\n\n", section);
}

void test_ds() {
    {
        struct partition* part = get_new_partition(NULL, NULL, 10, true);
        test_log("New partition", part->is_free && part->size == 10);
    }

    struct memory* mem = get_new_empty_memory(100, 10);
    test_log("New memory", mem->p == 100 && mem->q == 10 && mem->head->size == 90 && mem->head->is_free);

    {
        struct partition* part = allocate_partition(mem->head, mem->p);
        test_log("Allocate on partition (1/2)", part == NULL);
    }

    {
        int size = 50;
        struct partition* part = allocate_partition(mem->head, size);
        test_log("Allocate on partition (2/2)", part == mem->head && part->is_free == false && part->size == size && part->next->size == (mem->p - mem->q - size) && part->next->is_free);
    }

    {
        deallocate_partition(mem->head);
        test_log("Deallocate partition", mem->head->size == 90 && mem->head->next == NULL);
    }

    {
        allocate_partition(mem->head, 5);
        allocate_partition(mem->head->next, 10);
        allocate_partition(mem->head->next->next, 6);
        allocate_partition(mem->head->next->next->next, 8);
        allocate_partition(mem->head->next->next->next->next, 4);
        allocate_partition(mem->head->next->next->next->next->next, 20);
        print_memory(mem);
        deallocate_partition(mem->head->next);
        deallocate_partition(mem->head->next->next->next);
        deallocate_partition(mem->head->next->next->next->next->next);
        print_memory(mem);

        {
            print_memory(mem);
            first_fit(mem, 15);
            struct partition* part = mem->head->next->next->next->next->next;
            test_log("First fit", part->size == 15 && part->is_free == false);
        }

        {
            struct partition* part = mem->head->next->next->next;
            print_memory(mem);
            best_fit(mem, 5);
            test_log("Best fit", part->size == 5 && part->is_free == false);
        }

        {
            struct partition* part = mem->head->next->next->next->next;
            print_memory(mem);
            next_fit(mem, 3, 16);
            test_log("Next fit", part->size == 3 && part->is_free == false);
        }
    }
}

int main(int argc, char** argv) {
    mute_logs();
    print_test_section("Testing data structures");
    test_ds();
    printf("\n%d/%d tests passed\n", passed_tests, total_tests);
    return 0;
}