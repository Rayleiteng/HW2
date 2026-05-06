#include <stdio.h>
#include <stdlib.h>
#include <papi.h>
#include <string.h>
#include <pthread.h>
#include <omp.h>

#define ERROR_RETURN(retval) { fprintf(stderr, "Error %d (%s) at %s:%d\n", retval, PAPI_strerror(retval), __FILE__, __LINE__); exit(retval); }

int test_function(int size) {
    float tmp = 0.0;
    int i;
    for(i=1; i<size; i++) {
        tmp = (tmp + 100.0) / i;
    }
    return 0;
}

int main(int argc, char *argv[]) {
    int retval;
    int size = 100000000; 
    int num_threads = 2;  

    if (argc > 1) num_threads = atoi(argv[1]);
    if (argc > 2) size = atoi(argv[2]);

    retval = PAPI_library_init(PAPI_VER_CURRENT);
    if (retval != PAPI_VER_CURRENT) {
        fprintf(stderr, "PAPI library init error!\n");
        exit(1); 
    }

    retval = PAPI_thread_init((unsigned long (*)(void))(pthread_self));
    if (retval != PAPI_OK) {
        fprintf(stderr, "PAPI Thread library init error!\n");
        exit(1); 
    }   

    omp_set_num_threads(num_threads);

    // 用于汇总所有线程的数据
    long long total_values[4] = {0, 0, 0, 0};

    #pragma omp parallel
    {
        int EventSet = PAPI_NULL;
        long long values[4] = {0, 0, 0, 0}; 
        int tid = omp_get_thread_num();     

        if ((retval = PAPI_create_eventset(&EventSet)) != PAPI_OK) ERROR_RETURN(retval);
        if ((retval = PAPI_assign_eventset_component(EventSet, 0)) != PAPI_OK) ERROR_RETURN(retval);

        if ((retval = PAPI_add_event(EventSet, PAPI_TOT_INS)) != PAPI_OK) ERROR_RETURN(retval); 
        if ((retval = PAPI_add_event(EventSet, PAPI_TOT_CYC)) != PAPI_OK) ERROR_RETURN(retval); 
        if ((retval = PAPI_add_event(EventSet, PAPI_L1_DCM)) != PAPI_OK)  ERROR_RETURN(retval); 
        if ((retval = PAPI_add_event(EventSet, PAPI_L2_TCM)) != PAPI_OK)  ERROR_RETURN(retval); 

        if((retval = PAPI_start(EventSet)) != PAPI_OK) ERROR_RETURN(retval);    

        test_function(size);

        if((retval = PAPI_stop(EventSet, values)) != PAPI_OK) ERROR_RETURN(retval);

        #pragma omp critical
        {
            // 打印每个线程的数据
            printf("  Thread %2d | Instr: %10lld | Cycles: %10lld | L1D_Miss: %8lld | L2_Miss: %8lld\n", 
                   tid, values[0], values[1], values[2], values[3]);
            
            // 累加到总和中
            total_values[0] += values[0];
            total_values[1] += values[1];
            total_values[2] += values[2];
            total_values[3] += values[3];
        }
    } // 并行区域结束

    // 打印汇总数据（填表主要看这个）
    printf("  [TOTAL]   | Instr: %10lld | Cycles: %10lld | L1D_Miss: %8lld | L2_Miss: %8lld\n", 
           total_values[0], total_values[1], total_values[2], total_values[3]);

    PAPI_shutdown();
   
    return 0;
}