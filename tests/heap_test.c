#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../src/heap.h"

// length of max line in a test file
#define LINELEN 256

// static inline key fabs(key a) {
//     return a > 0 ? a : -a;
// }

static inline int
iabs(int a)
{
    return a > 0 ? a : -a;
}

// assume vals are ints
static inline void
load(const char *path, key *keys, int32_t *vals, int num_inputs)
{
    FILE *file = fopen(path, "r");
    if (file == NULL) {
        fprintf(stderr, "path not valid: %s\n", path);
        exit(1);
    }
    char line[LINELEN];
    for (int i = 0; i < num_inputs; i++) {
        if (fgets(line, LINELEN, file) == NULL)
            fprintf(stderr, "got to end of file early\n");
        switch (line[0]) {
        case 'i':; // needed because who knows why
            char *next_key = strtok(&(line[2]), " \t\n"); // cut off 'i '
            char *next_val = strtok(NULL, " \t\n");
#ifdef FLOAT
            keys[i] = strtof(next_key, NULL); // HARD CODED TYPE for keys FIXME
#else
            keys[i] = atoi(next_key); // HARD CODED TYPE for keys FIXME
#endif
            vals[i] = atoi(next_val);
            break;
        default:
            break;
        }
    }
}

// **********************************************************

static int
new_heap_empty(void)
{
    struct heap *H = init_heap();
    printf("testing new heap is empty: ");
    return hempty(H);
}

static int
simple_heap_inserts(void)
{
    struct heap *H = init_heap();
    printf("testing 6 simple elements: ");

    int a = 10;
    hinsert(H, &a, 41);
    int b = 29;
    hinsert(H, &b, 23);
    int c = 21;
    hinsert(H, &c, 43);
    int d = 39;
    hinsert(H, &d, 4);
    int e = 7;
    hinsert(H, &e, 39);
    int f = 38;
    hinsert(H, &f, 14);
    int correct[] = {21, 10, 7, 29, 38, 39};

    int goodsofar = 1;
    for (int i = 0; i < 6; i++) {
        int next = *((int *)hremove_max(H));
        goodsofar = goodsofar && (next == correct[i]);
    }

    free_heap(H);
    return goodsofar;
}

// EXACT copy of twofiveseven_heap_inserts, except the 257 -> 6
static int
simple_heap_inserts_file(void)
{
    struct heap *H = init_heap();
    printf("testing simple elements in file: "); // CHANGED
    key keys[6];
    int vals[6];
    load("tests/heap/simple_heap_inserts.input", keys, vals, 6); // CHANGED
    for (int i = 0; i < 6; i++) { hinsert(H, &(vals[i]), keys[i]); }
    key correct_keys[6];
    int correct_vals[6];
    load("tests/heap/simple_heap_inserts.output",
         correct_keys,
         correct_vals,
         6); // CHANGED

    int goodsofar = 1;
    for (int i = 0; i < 6 && goodsofar; i++) {
        int next = *((int *)hremove_max(H));
        int diff = iabs(next - correct_vals[i]);
        goodsofar = goodsofar && (diff == 0);
    }

    free_heap(H);
    return goodsofar;
}

static int
twofiveseven_heap_inserts(void)
{
    struct heap *H = init_heap();
    printf("testing 257 elements: ");
    key keys[257];
    int vals[257];
    load("tests/heap/twofiveseven_heap_inserts.input", keys, vals, 257);
    for (int i = 0; i < 257; i++) { hinsert(H, &(vals[i]), keys[i]); }
    key correct_keys[257];
    int correct_vals[257];
    load("tests/heap/twofiveseven_heap_inserts.output",
         correct_keys,
         correct_vals,
         257);

    int goodsofar = 1;
    for (int i = 0; i < 257 && goodsofar; i++) {
        int next = *((int *)hremove_max(H));
        int diff = iabs(next - correct_vals[i]);
        goodsofar = goodsofar && (diff == 0);
    }

    free_heap(H);
    return goodsofar;
}

static int
large_heap_inserts_with_dups(void)
{
    struct heap *H = init_heap();
    printf("testing 1047 elements: ");
    key keys[1047];
    int vals[1047];
    load("tests/heap/large_heap_inserts_with_dups.input", keys, vals, 1047);
    for (int i = 0; i < 1047; i++) { hinsert(H, &(vals[i]), keys[i]); }
    key correct_keys[1047];
    int correct_vals[1047];
    load("tests/heap/large_heap_inserts_with_dups.output",
         correct_keys,
         correct_vals,
         1047);

    int goodsofar = 1;
    for (int i = 0; i < 1047 && goodsofar; i++) {
        int next = *((int *)hremove_max(H));
#ifdef DEBUG
        printf("actual %d, correct: %d\n", next, correct_vals[i]);
#endif
        int diff = iabs(next - correct_vals[i]);
        goodsofar = goodsofar && (diff == 0);
    }

    free_heap(H);
    return goodsofar;
}

static int
simple_removes(void)
{
    struct heap *H = init_heap();
    printf("testing simple removes ");

    int goodsofar = 1;
    int a = 10;
    struct hnode *a_node = hinsert(H, &a, 41);
    int b = 29;
    struct hnode *b_node = hinsert(H, &b, 23);
    int c = 21;
    // hnode *c_node = hinsert(H, &c, 43);
    hinsert(H, &c, 43);

    goodsofar = goodsofar && (10 == *(int *)hremove(H, a_node));
#ifdef DEBUG
    printf("goodsofar: %d\n", goodsofar);
#endif

    int d = 39;
    // hnode *d_node = hinsert(H, &d, 4);
    hinsert(H, &d, 4);
    int e = 7;
    // hnode *e_node = hinsert(H, &e, 39);
    hinsert(H, &e, 39);

    goodsofar = goodsofar && (29 == *(int *)hremove(H, b_node));
#ifdef DEBUG
    printf("goodsofar: %d\n", goodsofar);
#endif

    int f = 38;
    // hnode *f_node = hinsert(H, &f, 14);
    hinsert(H, &f, 14);
    int correct[] = {21, 7, 38, 39};

    for (int i = 0; i < 4; i++) {
        int next = *((int *)hremove_max(H));
        goodsofar = goodsofar && (next == correct[i]);
#ifdef DEBUG
        printf("goodsofar: %d\n", goodsofar);
#endif
    }

    free_heap(H);
    return goodsofar;
}

int
main()
{
    // array of all tests, all return ints and take void
    int (*tests[])(void) = {
        new_heap_empty,
        simple_heap_inserts,
        simple_heap_inserts_file,
        twofiveseven_heap_inserts,
        large_heap_inserts_with_dups,
        simple_removes,
    };
    size_t len = sizeof(tests) / sizeof(tests[0]);
    for (size_t i = 0; i < len; i++) {
        int passed = tests[i]();
        if (passed) {
            printf("\033[32mPASSED\033[0m\n");
        } else {
            printf("\033[31mFAILED\033[0m\n");
            return 1;
        }
    }

    return 0;
}
