#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <omp.h>
#include <papi.h>

#define MAX_EVENTS 4

#define ERROR_RETURN(retval) do { \
    fprintf(stderr, "Error %d (%s) at %s:%d\n", \
            (retval), PAPI_strerror(retval), __FILE__, __LINE__); \
    exit(retval); \
} while (0)

typedef struct {
    const char *name;
    int count;
    int events[MAX_EVENTS];
} event_group_t;

static int test_function(int size) {
    float tmp = 0.0f;

    for (int i = 1; i < size; i++) {
        tmp = (tmp + 100.0f) / i;
    }

    return (int)tmp;
}

static event_group_t get_group(const char *name) {
    if (strcmp(name, "cache") == 0) {
        event_group_t group = {
            "cache",
            2,
            {PAPI_L1_DCM, PAPI_L2_TCM}
        };
        return group;
    }

    if (strcmp(name, "icache") == 0) {
        event_group_t group = {
            "icache",
            1,
            {PAPI_L1_ICM}
        };
        return group;
    }

    if (strcmp(name, "stalls") == 0) {
        event_group_t group = {
            "stalls",
            2,
            {PAPI_RES_STL, PAPI_STL_ICY}
        };
        return group;
    }

    if (strcmp(name, "branch") == 0) {
        event_group_t group = {
            "branch",
            1,
            {PAPI_BR_PRC}
        };
        return group;
    }

    if (strcmp(name, "base") == 0) {
        event_group_t group = {
            "base",
            2,
            {PAPI_TOT_INS, PAPI_TOT_CYC}
        };
        return group;
    }

    fprintf(stderr, "Unknown group '%s'. Use base, cache, icache, stalls, or branch.\n", name);
    exit(1);
}

static void print_result(
    const char *group_name,
    int requested_threads,
    int actual_threads,
    int size,
    double elapsed,
    long long totals[MAX_EVENTS]
) {
    printf("group,threads,actual_threads,size,elapsed_seconds,instructions,cycles,l1_i_misses,l1_d_misses,l2_misses,resource_stalls,no_issue_cycles,branch_correct\n");

    if (strcmp(group_name, "base") == 0) {
        printf("%s,%d,%d,%d,%.9f,%lld,%lld,,,,,,\n",
               group_name, requested_threads, actual_threads, size, elapsed,
               totals[0], totals[1]);
    } else if (strcmp(group_name, "cache") == 0) {
        printf("%s,%d,%d,%d,%.9f,,,,%lld,%lld,,,\n",
               group_name, requested_threads, actual_threads, size, elapsed,
               totals[0], totals[1]);
    } else if (strcmp(group_name, "icache") == 0) {
        printf("%s,%d,%d,%d,%.9f,,,%lld,,,,,\n",
               group_name, requested_threads, actual_threads, size, elapsed,
               totals[0]);
    } else if (strcmp(group_name, "stalls") == 0) {
        printf("%s,%d,%d,%d,%.9f,,,,,,%lld,%lld,\n",
               group_name, requested_threads, actual_threads, size, elapsed,
               totals[0], totals[1]);
    } else {
        printf("%s,%d,%d,%d,%.9f,,,,,,,,%lld\n",
               group_name, requested_threads, actual_threads, size, elapsed,
               totals[0]);
    }
}

int main(int argc, char *argv[]) {
    int retval;
    int requested_threads = 1;
    int size = 100000000;
    const char *group_name = "cache";
    event_group_t group;
    long long totals[MAX_EVENTS] = {0, 0, 0, 0};
    int actual_threads = 0;

    if (argc > 1) {
        group_name = argv[1];
    }
    if (argc > 2) {
        requested_threads = atoi(argv[2]);
    }
    if (argc > 3) {
        size = atoi(argv[3]);
    }
    if (requested_threads < 1 || size < 2) {
        fprintf(stderr, "Usage: %s [base|cache|icache|stalls|branch] [threads >= 1] [size >= 2]\n", argv[0]);
        return 1;
    }

    group = get_group(group_name);

    retval = PAPI_library_init(PAPI_VER_CURRENT);
    if (retval != PAPI_VER_CURRENT) {
        fprintf(stderr, "PAPI library init error\n");
        return 1;
    }

    retval = PAPI_thread_init((unsigned long (*)(void))(pthread_self));
    if (retval != PAPI_OK) {
        ERROR_RETURN(retval);
    }

    omp_set_num_threads(requested_threads);

    double start = omp_get_wtime();

    #pragma omp parallel
    {
        int thread_retval;
        int EventSet = PAPI_NULL;
        long long values[MAX_EVENTS] = {0, 0, 0, 0};

        #pragma omp single
        {
            actual_threads = omp_get_num_threads();
        }

        thread_retval = PAPI_create_eventset(&EventSet);
        if (thread_retval != PAPI_OK) ERROR_RETURN(thread_retval);

        thread_retval = PAPI_assign_eventset_component(EventSet, 0);
        if (thread_retval != PAPI_OK) ERROR_RETURN(thread_retval);

        for (int i = 0; i < group.count; i++) {
            thread_retval = PAPI_add_event(EventSet, group.events[i]);
            if (thread_retval != PAPI_OK) ERROR_RETURN(thread_retval);
        }

        #pragma omp barrier

        thread_retval = PAPI_start(EventSet);
        if (thread_retval != PAPI_OK) ERROR_RETURN(thread_retval);

        test_function(size);

        thread_retval = PAPI_stop(EventSet, values);
        if (thread_retval != PAPI_OK) ERROR_RETURN(thread_retval);

        #pragma omp critical
        {
            for (int i = 0; i < group.count; i++) {
                totals[i] += values[i];
            }
        }

        PAPI_cleanup_eventset(EventSet);
        PAPI_destroy_eventset(&EventSet);
    }

    double elapsed = omp_get_wtime() - start;

    print_result(group.name, requested_threads, actual_threads, size, elapsed, totals);

    PAPI_shutdown();
    return 0;
}
