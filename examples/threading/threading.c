#include "threading.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#define MS_TO_US 1000

// Optional: use these functions to add debug or error prints to your application
#define DEBUG_LOG(msg,...)
//#define DEBUG_LOG(msg,...) printf("threading: " msg "\n" , ##__VA_ARGS__)
#define ERROR_LOG(msg,...) printf("threading ERROR: " msg "\n" , ##__VA_ARGS__)

void* threadfunc(void* thread_param)
{
    
    // TODO: wait, obtain mutex, wait, release mutex as described by thread_data structure
    // hint: use a cast like the one below to obtain thread arguments from your parameter
    struct thread_data* thread_func_args = (struct thread_data *) thread_param;

    // Create a thread completely (Join in main function of thread)
    thread_func_args->thread_complete_success = true;

    int return_value;

    // Step 1: Wait for obtain mutex
    return_value = usleep(thread_func_args->wait_to_obtain_ms * MS_TO_US);
    if(return_value != 0)
    {
        ERROR_LOG("obtain mutex usleep failed\n");
        thread_func_args->thread_complete_success = false;
        return thread_param;
    }
    DEBUG_LOG("waiting to obtain mutex successful\n"); 

    // Step 2: Obtain mutex
    return_value = pthread_mutex_lock(thread_func_args->mutex);
    if(return_value != 0)
    {
        ERROR_LOG("locking mutex failed\n");
        thread_func_args->thread_complete_success = false;
        return thread_param;
    }
    DEBUG_LOG("obtaining mutex successful\n"); 

    // Step 3: Wait for release mutex
    return_value = usleep(thread_func_args->wait_to_release_ms * MS_TO_US);
    if(return_value != 0)
    {
        ERROR_LOG("Release mutex failed\n");
        thread_func_args->thread_complete_success = false;
        return thread_param;
    }
    DEBUG_LOG("Release mutex successful\n"); 

    // Step 4: Unlock mutex
    return_value = pthread_mutex_unlock(thread_func_args->mutex);
    if(return_value != 0)
    {
        ERROR_LOG("Unlocking mutex failed\n");
        thread_func_args->thread_complete_success = false;
        return thread_param;
    }
    DEBUG_LOG("obtaining mutex successful\n"); 
    return thread_param;
}


bool start_thread_obtaining_mutex(pthread_t *thread, pthread_mutex_t *mutex,int wait_to_obtain_ms, int wait_to_release_ms)
{
    /**
     * TODO: allocate memory for thread_data, setup mutex and wait arguments, pass thread_data to created thread
     * using threadfunc() as entry point.
     *
     * return true if successful.
     *
     * See implementation details in threading.h file comment block
     */

    // Step 1: Allocate memory for thread_data
    struct thread_data *thread_data_ptr = (struct thread_data*)malloc(sizeof(struct thread_data));
    if(thread_data_ptr == NULL)
    {
        ERROR_LOG("Malloc for thread_data failed");
        return false;
    }

    // Step 2: Setup mutex and wait arguments
    thread_data_ptr -> mutex = mutex;
    thread_data_ptr -> wait_to_obtain_ms = wait_to_obtain_ms;
    thread_data_ptr -> wait_to_release_ms = wait_to_release_ms;

    // Step 3: Create thread and pass thread data to created_thread
    int rc = pthread_create(thread, NULL, threadfunc, thread_data_ptr);

    if (rc != 0)
    {
        ERROR_LOG("Pthread_Create failed\n");
        free(thread_data_ptr);
        return false;
    }

    DEBUG_LOG("Start thread obtaining mutex success\n");
    
    return true;
}

