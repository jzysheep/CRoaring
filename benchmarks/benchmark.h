/* benchmark.h
 */

#ifndef BENCHMARKS_INCLUDE_BENCHMARK_H_
#define BENCHMARKS_INCLUDE_BENCHMARK_H_

#include "misc/configreport.h"

#define RDTSC_START(cycles)                                                   \
    do {                                                                      \
        register unsigned cyc_high, cyc_low;                                  \
        __asm volatile(                                                       \
            "cpuid\n\t"                                                       \
            "rdtsc\n\t"                                                       \
            "mov %%edx, %0\n\t"                                               \
            "mov %%eax, %1\n\t"                                               \
            : "=r"(cyc_high), "=r"(cyc_low)::"%rax", "%rbx", "%rcx", "%rdx"); \
        (cycles) = ((uint64_t)cyc_high << 32) | cyc_low;                      \
    } while (0)

#define RDTSC_FINAL(cycles)                                                   \
    do {                                                                      \
        register unsigned cyc_high, cyc_low;                                  \
        __asm volatile(                                                       \
            "rdtscp\n\t"                                                      \
            "mov %%edx, %0\n\t"                                               \
            "mov %%eax, %1\n\t"                                               \
            "cpuid\n\t"                                                       \
            : "=r"(cyc_high), "=r"(cyc_low)::"%rax", "%rbx", "%rcx", "%rdx"); \
        (cycles) = ((uint64_t)cyc_high << 32) | cyc_low;                      \
    } while (0)

/*
 * Prints the best number of operations per cycle where
 * test is the function call, answer is the expected answer generated by
 * test, repeat is the number of times we should repeat and size is the
 * number of operations represented by test.
 */
#define BEST_TIME(test, answer, repeat, size)                         \
    do {                                                              \
        printf("%s: ", #test);                                        \
        fflush(NULL);                                                 \
        uint64_t cycles_start, cycles_final, cycles_diff;             \
        uint64_t min_diff = (uint64_t)-1;                             \
        int wrong_answer = 0;                                         \
        for (int i = 0; i < repeat; i++) {                            \
            __asm volatile("" ::: /* pretend to clobber */ "memory"); \
            RDTSC_START(cycles_start);                                \
            if (test != answer) wrong_answer = 1;                     \
            RDTSC_FINAL(cycles_final);                                \
            cycles_diff = (cycles_final - cycles_start);              \
            if (cycles_diff < min_diff) min_diff = cycles_diff;       \
        }                                                             \
        uint64_t S = (uint64_t) size;                                            \
        float cycle_per_op = (min_diff) / (float)S;                   \
        printf(" %.2f cycles per operation", cycle_per_op);           \
        if (wrong_answer) printf(" [ERROR]");                         \
        printf("\n");                                                 \
        fflush(NULL);                                                 \
    } while (0)


/*
 * This is like BEST_TIME except that ... it runs functions "test" using the
 * first parameter "base" and various parameters from "testvalues" (there
 * are nbrtestvalues), calling pre on base between tests
 */
#define BEST_TIME_PRE_ARRAY(base, test, pre,  testvalues, nbrtestvalues)        \
    do {                                                                                \
        printf("%s %s: ", #test, #pre);                                                 \
        fflush(NULL);                                                                   \
        uint64_t cycles_start, cycles_final, cycles_diff;                               \
        int sum = 0;                                                                    \
        for (size_t j = 0; j < nbrtestvalues; j++) {                                    \
            pre(base);                                                                  \
            __asm volatile("" ::: /* pretend to clobber */ "memory");                   \
            RDTSC_START(cycles_start);                                                  \
            test(base,testvalues[j]);                                                   \
            RDTSC_FINAL(cycles_final);                                                  \
            cycles_diff = (cycles_final - cycles_start);                                \
            sum += cycles_diff;                                                         \
        }                                                                               \
        uint64_t S = (uint64_t) nbrtestvalues;                                                     \
        float cycle_per_op = sum / (float)S;                                           \
        printf(" %.2f cycles per operation", cycle_per_op);                             \
        printf("\n");                                                                   \
        fflush(NULL);                                                                   \
    } while (0)

#endif /* BENCHMARKS_INCLUDE_BENCHMARK_H_ */
