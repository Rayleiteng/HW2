#include<stdio.h>
#include<stdlib.h>
#include<papi.h>
#include <string.h>
#include <pthread.h>

#define ERROR_RETURN(retval) { fprintf(stderr, "Error %d (%s) at %s:%d\n.\n", retval,PAPI_strerror(retval),__FILE__,__LINE__); exit(retval); }

int test_function() {
   float tmp;
   int i;

   for(i=1; i<2000; i++)
   {
      tmp=(tmp+100.0)/i;
   }
   return 0;
}

int main() {
    int num, retval;

    /* Initialize the PAPI library */
    retval = PAPI_library_init(PAPI_VER_CURRENT);

    if (retval != PAPI_VER_CURRENT) {
        fprintf(stderr, "PAPI library init error!\n");
        exit(1); 
    }

    retval = PAPI_thread_init(pthread_self);
    if (retval != PAPI_OK) {
        fprintf(stderr, "PAPI Thread library init error!\n");
        exit(1); 
    }   
    
    num = PAPI_num_hwctrs();
    if (num < 0)
        ERROR_RETURN(num);
    printf("This machine has %d counters.\n", num);


    int EventSet = PAPI_NULL;
    PAPI_option_t options;
    long long values[6];
    if ((retval = PAPI_create_eventset(&EventSet)) != PAPI_OK)
        ERROR_RETURN(retval);

    // Set the domain of this EventSet to counter user and kernel modes for this process
    if ((retval = PAPI_assign_eventset_component(EventSet, 0)) != PAPI_OK)
        ERROR_RETURN(retval);
    
    //This line is necessary to avoid problems
    memset(&options,0x0,sizeof(options));

    options.domain.eventset = EventSet;
    options.domain.domain = PAPI_DOM_ALL;
    if ((retval=PAPI_set_opt(PAPI_DOMAIN, &options)) != PAPI_OK)
        ERROR_RETURN(retval);

    if ( (retval = PAPI_add_event(EventSet, PAPI_TOT_INS)) != PAPI_OK)
        ERROR_RETURN(retval);

    // Add Total Cycles Executed event to the EventSet 
    if ( (retval = PAPI_add_event(EventSet, PAPI_TOT_CYC)) != PAPI_OK)
        ERROR_RETURN(retval);

    // Add Total L1 DCache Misses event to the EventSet 
    if ( (retval = PAPI_add_event(EventSet, PAPI_L1_DCM)) != PAPI_OK)
        ERROR_RETURN(retval);
    
    // Add Total L1 DCache Misses event to the EventSet 
    if ( (retval = PAPI_add_event(EventSet, PAPI_L1_ICM)) != PAPI_OK)
        ERROR_RETURN(retval);

    // Add Total L2 Total Cache Misses event to the EventSet 
    if ( (retval = PAPI_add_event(EventSet, PAPI_L2_TCM)) != PAPI_OK)
        ERROR_RETURN(retval); 

    //Add Request for access to shared cache line (SMP) toi the EventSet
    if ( (retval = PAPI_add_event(EventSet, PAPI_CA_SHR)) != PAPI_OK)
        ERROR_RETURN(retval); 
    
    // Start counting 
    if((retval=PAPI_start(EventSet)) != PAPI_OK)
        ERROR_RETURN(retval);    

    test_function();

    // Stop counting 
    if((retval=PAPI_stop(EventSet, values)) != PAPI_OK)
        ERROR_RETURN(retval);

    printf(" Total instructions: %lld   Total Cycles: %lld   L1 D-Cache Misses: %lld    L1 I-Cache Misses: %lld L2 Total cache misses: %lld Share requests: %lld\n", values[0],
            values[1], values[2], values[3], values[4], values[5]);

    // clean up
    PAPI_shutdown();
   
    return 0;
}
