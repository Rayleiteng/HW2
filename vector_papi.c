#include <omp.h>
#include <papi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_EVENTS 2

typedef struct {
    const char *name;
    int count;
    int events[MAX_EVENTS];
} event_group_t;

static event_group_t get_group(const char *name) {
    if (strcmp(name, "base") == 0) {
        event_group_t group = {"base", 2, {PAPI_TOT_INS, PAPI_TOT_CYC}};
        return group;
    }
    if (strcmp(name, "cache") == 0) {
        event_group_t group = {"cache", 2, {PAPI_L1_DCM, PAPI_L2_TCM}};
        return group;
    }
    if (strcmp(name, "icache") == 0) {
        event_group_t group = {"icache", 1, {PAPI_L1_ICM}};
        return group;
    }
    if (strcmp(name, "vecsp") == 0) {
        event_group_t group = {"vecsp", 1, {PAPI_VEC_SP}};
        return group;
    }
    if (strcmp(name, "vecdp") == 0) {
        event_group_t group = {"vecdp", 1, {PAPI_VEC_DP}};
        return group;
    }

    fprintf(stderr, "Unknown group '%s'\n", name);
    exit(2);
}

static void add_event_or_exit(int EventSet, int event) {
    int retval = PAPI_add_event(EventSet, event);
    if (retval != PAPI_OK) {
        fprintf(stderr, "PAPI_add_event failed: %s\n", PAPI_strerror(retval));
        exit(2);
    }
}

int main(int argc, char *argv[]) {
    const char *group_name = "base";
    int threads = 1;
    long n = 1000000;
    int actual_threads = 0;
    long long totals[MAX_EVENTS] = {0, 0};

    if (argc > 1) group_name = argv[1];
    if (argc > 2) threads = atoi(argv[2]);
    if (argc > 3) n = atol(argv[3]);
    if (threads < 1 || n < 1) {
        fprintf(stderr, "Usage: %s [base|cache|icache|vecsp|vecdp] [threads] [size]\n", argv[0]);
        return 1;
    }

    double *a = malloc((size_t)n * sizeof(double));
    double *b = malloc((size_t)n * sizeof(double));
    double *c = malloc((size_t)n * sizeof(double));
    if (!a || !b || !c) {
        fprintf(stderr, "Memory allocation failed\n");
        free(a);
        free(b);
        free(c);
        return 1;
    }

    for (long i = 0; i < n; i++) {
        b[i] = (double)i;
        c[i] = 2.0 * (double)i;
    }

    event_group_t group = get_group(group_name);

    int retval = PAPI_library_init(PAPI_VER_CURRENT);
    if (retval != PAPI_VER_CURRENT) {
        fprintf(stderr, "PAPI_library_init failed\n");
        return 1;
    }
    retval = PAPI_thread_init((unsigned long (*)(void))(omp_get_thread_num));
    if (retval != PAPI_OK) {
        fprintf(stderr, "PAPI_thread_init failed: %s\n", PAPI_strerror(retval));
        return 1;
    }

    omp_set_num_threads(threads);
    double start = omp_get_wtime();

    #pragma omp parallel
    {
        int EventSet = PAPI_NULL;
        long long values[MAX_EVENTS] = {0, 0};

        #pragma omp single
        actual_threads = omp_get_num_threads();

        int thread_retval = PAPI_create_eventset(&EventSet);
        if (thread_retval != PAPI_OK) {
            fprintf(stderr, "PAPI_create_eventset failed: %s\n", PAPI_strerror(thread_retval));
            exit(1);
        }

        for (int e = 0; e < group.count; e++) {
            add_event_or_exit(EventSet, group.events[e]);
        }

        #pragma omp barrier

        thread_retval = PAPI_start(EventSet);
        if (thread_retval != PAPI_OK) {
            fprintf(stderr, "PAPI_start failed: %s\n", PAPI_strerror(thread_retval));
            exit(1);
        }

        #pragma omp for
        for (long i = 0; i < n; i++) {
            a[i] = b[i] + c[i];
        }

        thread_retval = PAPI_stop(EventSet, values);
        if (thread_retval != PAPI_OK) {
            fprintf(stderr, "PAPI_stop failed: %s\n", PAPI_strerror(thread_retval));
            exit(1);
        }

        #pragma omp critical
        {
            for (int e = 0; e < group.count; e++) {
                totals[e] += values[e];
            }
        }

        PAPI_cleanup_eventset(EventSet);
        PAPI_destroy_eventset(&EventSet);
    }

    double elapsed = omp_get_wtime() - start;

    printf("app,group,threads,actual_threads,size,elapsed_seconds,instructions,cycles,l1_i_misses,l1_d_misses,l2_misses,vec_sp,vec_dp\n");
    if (strcmp(group.name, "base") == 0) {
        printf("vector,%s,%d,%d,%ld,%.9f,%lld,%lld,,,,,\n",
               group.name, threads, actual_threads, n, elapsed, totals[0], totals[1]);
    } else if (strcmp(group.name, "cache") == 0) {
        printf("vector,%s,%d,%d,%ld,%.9f,,,,%lld,%lld,,\n",
               group.name, threads, actual_threads, n, elapsed, totals[0], totals[1]);
    } else if (strcmp(group.name, "icache") == 0) {
        printf("vector,%s,%d,%d,%ld,%.9f,,,%lld,,,,\n",
               group.name, threads, actual_threads, n, elapsed, totals[0]);
    } else if (strcmp(group.name, "vecsp") == 0) {
        printf("vector,%s,%d,%d,%ld,%.9f,,,,,,%lld,\n",
               group.name, threads, actual_threads, n, elapsed, totals[0]);
    } else {
        printf("vector,%s,%d,%d,%ld,%.9f,,,,,,,%lld\n",
               group.name, threads, actual_threads, n, elapsed, totals[0]);
    }

    free(a);
    free(b);
    free(c);
    PAPI_shutdown();
    return 0;
}
