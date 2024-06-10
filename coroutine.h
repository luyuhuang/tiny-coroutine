#include <stddef.h>

#ifndef __x86_64__
    #error "support x86-64 only"
#endif

enum {
    CO_STATUS_INIT,
    CO_STATUS_PENDING,
    CO_STATUS_NORMAL,
    CO_STATUS_RUNNING,
    CO_STATUS_DEAD,
};

typedef void *(*start_coroutine)(void *);
struct coroutine;

struct coroutine *co_new(start_coroutine start, size_t stack_size);
void co_free(struct coroutine *co);
int co_status(struct coroutine *co);
struct coroutine *co_curr();
int co_resume(struct coroutine *co, void *param, void **result);
int co_yield(void *result, void **param);
