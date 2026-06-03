#ifndef claim_h
#define claim_h

#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#define RED "\033[0;31m"
#define GREEN "\033[0;32m"
#define YELLOW "\033[0;33m"
#define BOLD_RED "\033[1;31m"
#define RESET "\033[0m"

bool _eq_int(int a, int b) { return a == b; }
bool _eq_unsigned_int(unsigned int a, unsigned int b) { return a == b; }
bool _eq_long(long a, long b) { return a == b; }
bool _eq_unsigned_long(unsigned long a, unsigned long b) { return a == b; }
bool _eq_long_long(long long a, long long b) { return a == b; }
bool _eq_unsigned_long_long(unsigned long long a, unsigned long long b) { return a == b; }
bool _eq_short(short a, short b) { return a == b; }
bool _eq_unsigned_short(unsigned short a, unsigned short b) { return a == b; }
bool _eq_char(char a, char b) { return a == b; }
bool _eq_unsigned_char(unsigned char a, unsigned char b) { return a == b; }
bool _eq_float(float a, float b) { return a == b; }
bool _eq_double(double a, double b) { return a == b; }
bool _eq_bool(bool a, bool b) { return a == b; }
bool _eq_str(char *a, char *b) { return strcmp(a, b) == 0; }
bool _eq_const_str(const char *a, const char *b) { return strcmp(a, b) == 0; }

#define _FMT(x) _Generic((x), \
    int: "%d", \
    unsigned int: "%u", \
    long: "%ld", \
    unsigned long: "%lu", \
    long long: "%lld", \
    unsigned long long: "%llu", \
    short: "%d", \
    unsigned short: "%u", \
    char: "%c", \
    unsigned char: "%u", \
    float: "%g", \
    double: "%g", \
    bool: "%d", \
    char *: "\"%s\"", \
    const char *: "\"%s\"" \
)

#define _EQ_FN(x) _Generic((x), \
    int: _eq_int, \
    unsigned int: _eq_unsigned_int, \
    long: _eq_long, \
    unsigned long: _eq_unsigned_long, \
    long long: _eq_long_long, \
    unsigned long long: _eq_unsigned_long_long, \
    short: _eq_short, \
    unsigned short: _eq_unsigned_short, \
    char: _eq_char, \
    unsigned char: _eq_unsigned_char, \
    float: _eq_float, \
    double: _eq_double, \
    bool: _eq_bool, \
    char *: _eq_str, \
    const char *: _eq_const_str \
)

#define expect_eq(a, b) do { \
    runner.assertions += 1; \
    if (!_EQ_FN(a)((a), (b))) { \
        runner.assertions_failed += 1; \
        printf(ASSERTION_FAILED " (%s:%d): expected '%s' to equal '%s' (got ", __FILE__, __LINE__, #a, #b); \
        printf(_FMT(a), (a)); \
        printf(", expected "); \
        printf(_FMT(b), (b)); \
        printf(")\n"); \
    } \
} while (0)

#define expect_not_eq(a, b) do { \
    runner.assertions += 1; \
    if (_EQ_FN(a)((a), (b))) { \
        runner.assertions_failed += 1; \
        printf(ASSERTION_FAILED " (%s:%d): expected '%s' to not equal '%s' (both are ", __FILE__, __LINE__, #a, #b); \
        printf(_FMT(a), (a)); \
        printf(")\n"); \
    } \
} while (0)

static const char *current_describe = NULL;
static const char *_register_group = NULL;

typedef void (*TestFunc)(void);

typedef struct {
    const char *test_name;
    const char *group;

    TestFunc fn;
} registered_test;

#define _CONCAT2(a, b) a##b
#define _CONCAT(a, b) _CONCAT2(a, b)
#define _UNIQUE(prefix) _CONCAT(prefix, __COUNTER__)

#define MAX_TESTS 1024

static struct {
    size_t tests_ran;
    size_t tests_failed;

    size_t assertions;
    size_t assertions_failed;

    size_t tests_pending;
    size_t tests_skipped;

    registered_test registry[MAX_TESTS];
    size_t registry_count;
} runner;

#define pending() do { \
    runner.tests_pending += 1; \
    return; \
} while (0)

#define skip(msg) do { \
    runner.tests_skipped += 1; \
    printf("    '%s'\n", msg); \
    return; \
} while (0)

#define should(name) \
    void name(void); \
    __attribute__((constructor)) void register_##name(void) { \
        runner.registry[runner.registry_count].test_name = #name; \
        runner.registry[runner.registry_count].group = _register_group; \
        runner.registry[runner.registry_count].fn = name; \
        runner.registry_count++; \
    } \
    void name(void)

#define describe(name) \
    __attribute__((constructor)) void _UNIQUE(_set_group_)(void) { \
        _register_group = name; \
    }

#define ASSERTION_FAILED "    " BOLD_RED "assertion failed" RESET

#define refute(expr) do { \
    runner.assertions += 1; \
    if (expr) { \
        runner.assertions_failed += 1; \
        printf(ASSERTION_FAILED " (%s:%d): '%s'\n", __FILE__, __LINE__, #expr); \
    } \
} while (0)

#define expect(expr) do { \
    runner.assertions += 1; \
    if (!(expr)) { \
        runner.assertions_failed += 1; \
        printf(ASSERTION_FAILED " (%s:%d): '%s'\n", __FILE__, __LINE__, #expr); \
    } \
} while (0)

#define expect_null(expr) do { \
    const void *_val = (expr); \
    runner.assertions += 1; \
    if (_val != NULL) { \
        runner.assertions_failed += 1; \
        printf(ASSERTION_FAILED " (%s:%d): expected '%s' to be NULL\n", __FILE__, __LINE__, #expr); \
    } \
} while (0)

#define expect_not_null(expr) do { \
    const void *_val = (expr); \
    runner.assertions += 1; \
    if (_val == NULL) { \
        runner.assertions_failed += 1; \
        printf(ASSERTION_FAILED " (%s:%d): expected '%s' to not be NULL\n", __FILE__, __LINE__, #expr); \
    } \
} while (0)

static void run_all_tests() {
    for (size_t i = 0; i < runner.registry_count; i++) {
        size_t prev_assertions_failed = runner.assertions_failed;
        size_t prev_pending = runner.tests_pending;
        size_t prev_skipped = runner.tests_skipped;

        current_describe = runner.registry[i].group;
        runner.registry[i].fn();
        runner.tests_ran += 1;

        if (prev_pending != runner.tests_pending) {
            if (current_describe)
                printf("" YELLOW "  test pending " RESET "'%s'" YELLOW " > " RESET "'%s'\n\n", current_describe, runner.registry[i].test_name);
            else
                printf("" YELLOW "  test pending " RESET "'%s'\n", runner.registry[i].test_name);
        } else if (prev_skipped != runner.tests_skipped) {
            if (current_describe)
                printf("" YELLOW "  test skipped " RESET "'%s'" YELLOW " > " RESET "'%s'\n\n", current_describe, runner.registry[i].test_name);
            else
                printf("" YELLOW "  test skipped " RESET "'%s'\n\n", runner.registry[i].test_name);
        } else if (prev_assertions_failed != runner.assertions_failed) {
            runner.tests_failed += 1;
            if (current_describe)
                printf(RED "  test failed " RESET "'%s'" RED " > " RESET "'%s'\n\n", current_describe, runner.registry[i].test_name);
            else
                printf(RED "  test failed " RESET "'%s'\n\n", runner.registry[i].test_name);
        }
    }
    current_describe = NULL;
}

int test_results() {
    run_all_tests();

    size_t tests_passed = runner.tests_ran - runner.tests_failed - runner.tests_pending - runner.tests_skipped;
    size_t tests_ran = runner.tests_ran - runner.tests_pending - runner.tests_skipped;

    const char *color = (runner.tests_failed > 0) ? RED : GREEN;

    printf(
        "\n%s%zu tests, %zu passed, %zu failed (%zu pending, %zu skipped)" RESET "\n",
        color, tests_ran, tests_passed, runner.tests_failed, runner.tests_pending, runner.tests_skipped
    );

    return (runner.tests_failed > 0)? 1 : 0;
}

#endif