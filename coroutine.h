#include <stddef.h>

#ifndef __x86_64__
    #error "support x86-64 only"
#endif

enum co_regs {
    CO_R15,
    CO_R14,
    CO_R13,
    CO_R12,
    CO_R9,
    CO_R8,
    CO_RBP,
    CO_RDI,
    CO_RSI,
    CO_RAX,
    CO_RDX,
    CO_RCX,
    CO_RBX,
    CO_RSP,
};

enum co_status {
    CO_STATUS_INIT,
    CO_STATUS_PENDING,
    CO_STATUS_RUNNING,
    CO_STATUS_DEAD,
};

struct co_context {
    void *regs[14];
};

typedef void *(*start_coroutine)(void *);

struct coroutine {
    struct co_context ctx;
    char *stack;
    size_t stack_size;
    int status;
    start_coroutine start;
    void *param;
    void *result;
};

struct co_env {
    struct coroutine *stack[128];
    size_t stack_top;
};


struct coroutine *co_new(start_coroutine start, size_t stack_size);
void co_free(struct coroutine *co);
int co_resume(struct coroutine *co, void *param, void **result);
int co_yield(void *result, void **param);
