#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <papi.h>

#define ERROR_RETURN(retval) do { \
    fprintf(stderr, "Error %d (%s) at %s:%d\n", \
            (retval), PAPI_strerror(retval), __FILE__, __LINE__); \
    exit(retval); \
} while (0)

static int test_function(int size) {
    float tmp = 0.0f;

    for (int i = 1; i < size; i++) {
        tmp = (tmp + 100.0f) / i;
    }

    return (int)tmp;
}

int main(int argc, char *argv[]) {
    int retval;
    int size = 1000;
    int num;
    int EventSet = PAPI_NULL;
    PAPI_option_t options;
    long long values[4] = {0, 0, 0, 0};

    if (argc > 1) {
        size = atoi(argv[1]);
    }
    if (size < 2) {
        fprintf(stderr, "Usage: %s [size >= 2]\n", argv[0]);
        return 1;
    }

    retval = PAPI_library_init(PAPI_VER_CURRENT);
    if (retval != PAPI_VER_CURRENT) {
        fprintf(stderr, "PAPI library init error\n");
        return 1;
    }

    retval = PAPI_thread_init((unsigned long (*)(void))(pthread_self));
    if (retval != PAPI_OK) {
        ERROR_RETURN(retval);
    }

    num = PAPI_num_hwctrs();
    if (num < 0) {
        ERROR_RETURN(num);
    }

    if ((retval = PAPI_create_eventset(&EventSet)) != PAPI_OK) {
        ERROR_RETURN(retval);
    }
    if ((retval = PAPI_assign_eventset_component(EventSet, 0)) != PAPI_OK) {
        ERROR_RETURN(retval);
    }

    memset(&options, 0, sizeof(options));
    options.domain.eventset = EventSet;
    options.domain.domain = PAPI_DOM_ALL;
    if ((retval = PAPI_set_opt(PAPI_DOMAIN, &options)) != PAPI_OK) {
        ERROR_RETURN(retval);
    }

    if ((retval = PAPI_add_event(EventSet, PAPI_TOT_INS)) != PAPI_OK) ERROR_RETURN(retval);
    if ((retval = PAPI_add_event(EventSet, PAPI_TOT_CYC)) != PAPI_OK) ERROR_RETURN(retval);
    if ((retval = PAPI_add_event(EventSet, PAPI_L1_DCM)) != PAPI_OK) ERROR_RETURN(retval);
    if ((retval = PAPI_add_event(EventSet, PAPI_L2_TCM)) != PAPI_OK) ERROR_RETURN(retval);

    if ((retval = PAPI_start(EventSet)) != PAPI_OK) {
        ERROR_RETURN(retval);
    }

    test_function(size);

    if ((retval = PAPI_stop(EventSet, values)) != PAPI_OK) {
        ERROR_RETURN(retval);
    }

    printf("run,size,hwctrs,total_instructions,total_cycles,l1_dcache_misses,l2_total_cache_misses\n");
    printf("1,%d,%d,%lld,%lld,%lld,%lld\n",
           size, num, values[0], values[1], values[2], values[3]);

    PAPI_shutdown();
    return 0;
}
