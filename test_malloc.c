#include "mymalloc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Test Functions
void test_malloc() {
    printf("Testing my_malloc...\n");
    void* ptr1 = my_malloc(16);
    void* ptr2 = my_malloc(32);
    if (!ptr1 || !ptr2) {
        printf("my_malloc failed.\n");
        exit(1);
    }
    memset(ptr1, 0, 16);
    memset(ptr2, 0, 32);
    my_free(ptr1);
    my_free(ptr2);
    printf("my_malloc test passed.\n");
}

void test_free() {
    printf("Testing my_free...\n");
    void* ptr = my_malloc(16);
    if (!ptr) {
        printf("my_malloc failed.\n");
        exit(1);
    }
    my_free(ptr);
    printf("my_free test passed.\n");
}

void test_realloc() {
    printf("Testing my_realloc...\n");
    void* ptr = my_malloc(16);
    if (!ptr) {
        printf("my_malloc failed.\n");
        exit(1);
    }
    ptr = my_realloc(ptr, 32);
    if (!ptr) {
        printf("my_realloc failed.\n");
        exit(1);
    }
    memset(ptr, 0, 32);
    my_free(ptr);
    printf("my_realloc test passed.\n");
}

// Main Function
int main(int argc, char* argv[]) {
    if (argc > 1 && strcmp(argv[1], "-t") == 0) {
        int test_num = atoi(argv[2]);
        printf("Running test %d...\n", test_num);
        switch (test_num) {
            case 1:
                test_malloc();
                break;
            case 2:
                test_free();
                break;
            case 3:
                test_realloc();
                break;
            default:
                printf("Invalid test number. Use 1 for malloc, 2 for free, 3 for realloc.\n");
                return 1;
        }
    } else {
        printf("Running all tests...\n");
        test_malloc();
        test_free();
        test_realloc();
    }

    printf("All tests passed.\n");
    return 0;
}
