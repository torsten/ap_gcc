/*
 * 2009 - rtti.de
 *
 * compile me like this:
 * $ g++ -o profile_test profile_test.cpp -finstrument-functions -g
 *
 * to see symbol names for function pointers use:
 * $ nm profile_test
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>

#include <stdarg.h>
#include <stdlib.h>

#include <pthread.h>

/******************************************************************************/

// max stack depth the profiling can work with
#define MAX_STACK_DEPTH 1024

/******************************************************************************/

#define NO_PROFILE  __attribute__   ((no_instrument_function))
#define NAKED       __attribute__   ((naked)) // does not work on x86
#define NO_INLINE   __attribute__   ((noinline))

/******************************************************************************/

// prevent c++ name mangling
extern "C" {
    void __cyg_profile_func_enter(void*, void*) NO_PROFILE NO_INLINE;
    void __cyg_profile_func_exit(void*, void*) NO_PROFILE NO_INLINE;
    void print_total_calls() NO_PROFILE;
}

/******************************************************************************/

/*
    FIXME only single threaded at this point
    TODO pthread_getspecific -> thread specific storage for the stuff below
                                maybe we wanna use boost thread stuff here?
*/

// number of profiled function calls
uint g_calls = 0;

// current (profiled) call stack depth
uint g_level = 0;

// list of function execution times
timeval g_exec_times[MAX_STACK_DEPTH];

// call ids
uint g_call_ids[MAX_STACK_DEPTH];

/******************************************************************************/

// called right after function entry
void __cyg_profile_func_enter(void *p_fn, void *p_call_site) {

    // // the asm way
    // 
    // uint* frame_ptr;
    // uint* this_ptr
    // 
    // // copy the base pointer value from register EBP into frame_ptr
    // asm volatile("movl %%ebp, %0"
    //     : "=r" (frame_ptr)
    //     : /* no input */
    //     : /* no registers changed */);
    // 
    // // jump to the previous functions frame ptr, goto parameter area, read this
    // this_ptr = (uint*)*(((uint*)(*frame_ptr)) + 2);


    // the gcc way - damn, just found this way...
    
    // get id of current thread
    pthread_t thread_id = pthread_self();

    // frame ptr of the previous function (1 level down)
    void* frame_ptr = __builtin_frame_address(1);
    
    // return adress of the previous function (1 level down)
    void* return_ptr = __builtin_return_address(1);

    // first parameter, if object this ptr
    void* this_ptr = (void*)*((uint*)frame_ptr + 2);

    // get the current time
    gettimeofday(&g_exec_times[g_level], 0);
    double timestamp =  g_exec_times[g_level].tv_sec + 
                        (g_exec_times[g_level].tv_usec / 1000000.0);

    g_call_ids[g_level] = g_calls;

    printf("\n>> #######################################\n");
    printf(">> # THREAD ID       : %u\n", (uint)thread_id);
    printf(">> # CALL ID         : %u\n", g_call_ids[g_level]);
    printf(">> # CALL LEVEL      : %u\n", g_level);
    printf(">> # FUNCTION ADDRESS: 0x%08X\n", p_fn);
    printf(">> # RETURN ADDRESS  : 0x%08X\n", return_ptr);
    printf(">> # THIS POINTER    : 0x%08X (if in member func)\n", this_ptr);
    printf(">> # TIMESTAMP       : %f\n", timestamp);
    printf(">> # FLAG            : ENTRY\n\n");

    g_calls++;
    g_level++;
}

// called right before function exit
void __cyg_profile_func_exit(void *p_fn, void *p_call_site) {

    g_level--;

    // get id of current thread
    pthread_t thread_id = pthread_self();

    // frame ptr of the previous function (1 level down)
    void* frame_ptr = __builtin_frame_address(1);
    
    // return adress of the previous function (1 level down)
    void* return_ptr = __builtin_return_address(1);

    // first parameter, if object this ptr
    void* this_ptr = (void*)*((uint*)frame_ptr + 2);

    // get the current time
    timeval t;
    gettimeofday(&t, 0);
    double timestamp =  g_exec_times[g_level].tv_sec + 
                        (g_exec_times[g_level].tv_usec / 1000000.0);
    double timestamp2 = t.tv_sec + (t.tv_usec / 1000000.0);

    printf("\n<< #########################################\n");
    printf("<< # THREAD ID       : %u\n", (uint)thread_id);
    printf("<< # CALL ID         : %u\n", g_call_ids[g_level]);
    printf("<< # CALL LEVEL      : %u\n", g_level);
    printf("<< # FUNCTION ADDRESS: 0x%08X\n", p_fn);
    printf("<< # RETURN ADDRESS  : 0x%08X\n", return_ptr);
    printf("<< # THIS POINTER    : 0x%08X (if in member func)\n", this_ptr);
    printf("<< # EXEC DURATION   : %f seconds\n", timestamp2 - timestamp);
    printf("<< # FLAG            : EXIT\n\n");

}

void print_total_calls() {
    printf("\n\n   # TOTAL CALLS: %d\n", g_calls);
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

// a function that is not profiled
void baz() NO_PROFILE;
void baz() {
    printf("i am in the baz() function...\n");
}

// profiled functions
void bar() {
    printf("i am in the bar() function...\n");
}

void foo() {
    printf("i am in the foo() function...\n");
    bar();
}

// class with profiled ctor and dtor
class klass
{
public:
    klass () {
        printf("klass ctor (this: 0x%X)\n", this);
    };
    virtual ~klass () {
        printf("klass dtor\n");
    }
    void foo() {
        printf("klass foo\n");
    }
    void foo(int a) {
        a++;
        printf("klass foo\n");
    }
};

int main (int argc, char const *argv[]) NO_PROFILE;
int main (int argc, char const *argv[]) {

    atexit(print_total_calls);

    for(size_t i = 0; i < 3; ++i) {
        foo();
    }
    
    klass k;
    k.foo();
    k.foo(0xDEADBEEF);
    
    klass k2;

    baz();

    return 0;
}
