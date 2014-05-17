#ifndef TEST_UTIL_H
#define TEST_UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _MSC_VER

#define CCALL __cdecl
#pragma section(".CRT$XCU",read)
#define INITIALIZER(f) \
   static void __cdecl f(void); \
   __declspec(allocate(".CRT$XCU")) void (__cdecl*f##_)(void) = f; \
   static void __cdecl f(void)

#elif defined(__GNUC__)

#define CCALL
#define INITIALIZER(f) \
   static void f(void) __attribute__((constructor)); \
   static void f(void)

#endif

// __attribute__((constructor)) static void test_##NAME##_init()

#define TEST(NAME) \
    static void test_##NAME(); \
    void test_util_add_test(const char *name, void (*func)()); \
    INITIALIZER(test_##NAME##_init) \
    { \
        test_util_add_test(__FILE__ "_" #NAME, &test_##NAME); \
    } \
    static void test_##NAME()

#define TEST_MAIN() \
    typedef void (*test_func)(); \
    struct test_def { \
        const char *name; \
        test_func func; \
        struct test_def *next; \
    }; \
    static struct test_def test_defs; \
    void test_util_add_test(const char *name, void (*func)()) \
    { \
        struct test_def *def = calloc(1, sizeof(*def)); \
        struct test_def *last = &test_defs; \
        while (last && last->next) last = last->next; \
        last->next = def; \
        def->name = strdup(name); \
        def->func = func; \
    } \
    int main() \
    { \
        struct test_def *def = test_defs.next; \
        while (def) \
        { \
            printf("running test %s\n", def->name); \
            def->func(); \
            def = def->next; \
        } \
        return 0; \
    }

#define ASSERT(X) \
    do { \
        if (!(X)) { \
            fprintf(stderr, "assertion %s failed at %s:%d\n", \
                    #X, __FILE__, __LINE__); \
            exit(1); \
        } \
    } while (0)

#define ASSERT_MSG(X, FMT, ...) \
    do { \
        if (!(X)) { \
            fprintf(stderr, "assertion %s failed at %s:%d (" FMT ")\n", \
                    #X, __FILE__, __LINE__, ##__VA_ARGS__); \
            exit(1); \
        } \
    } while (0)

#define ASSERT_TRUE(X) ASSERT(X)
#define ASSERT_FALSE(X) ASSERT(!(X))

#define ASSERT_EQ(EXPECTED, ACTUAL) ASSERT(((EXPECTED) == (ACTUAL)))
#define ASSERT_NEQ(EXPECTED, ACTUAL) ASSERT(((EXPECTED) != (ACTUAL)))

#define ASSERT_STREQ(EXPECTED, ACTUAL) ASSERT(strcmp((EXPECTED), (ACTUAL)) == 0)
#define ASSERT_STREQ_N(EXPECTED, ACTUAL, N) ASSERT(strncmp((EXPECTED), (ACTUAL), (N)) == 0)

#endif
