#include "coroutine.h"
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

enum {
    CO_R15 = 0,
    CO_R14,
    CO_R13,
    CO_R12,
    CO_R9,
    CO_R8,
    CO_RBP,
    CO_RDI,
    CO_RSI,
    CO_RDX,
    CO_RCX,
    CO_RBX,
    CO_RSP,
};

struct co_context {
    void *regs[13];
};

struct coroutine {
    struct co_context ctx;
    char *stack;
    size_t stack_size;
    int status;
    struct coroutine *prev;
    start_coroutine start;
    void *param;
    void *result;
};

static __thread struct coroutine *g_curr_co = NULL;
extern void co_ctx_swap(struct co_context *curr, struct co_context *next);

struct coroutine *co_new(start_coroutine start, size_t stack_size) {
    struct coroutine *co = malloc(sizeof(struct coroutine));
    memset(&co->ctx, 0, sizeof(co->ctx));
    if (stack_size) {
        co->stack = malloc(stack_size);
    } else {
        co->stack = NULL;
    }
    co->stack_size = stack_size;
    co->status = CO_STATUS_INIT;
    co->prev = NULL;
    co->start = start;
    co->param = co->result = NULL;

    return co;
}

void co_free(struct coroutine *co) {
    free(co->stack);
    free(co);
}

int co_status(struct coroutine *co) {
    return co->status;
}

static void co_entrance(struct coroutine *co) {
    void *result = co->start(co->param);
    co->status = CO_STATUS_DEAD;
    co_yield(result, NULL);
    // never run here
}

static void co_ctx_make(struct coroutine *co) {
    char *sp = co->stack + co->stack_size - sizeof(void*);
    sp = (char*)((intptr_t)sp & -16LL);
    *(void**)sp = (void*)co_entrance;
    co->ctx.regs[CO_RSP] = sp;
    co->ctx.regs[CO_RDI] = co;
}

static void check_init() {
    if (!g_curr_co) {
        g_curr_co = co_new(NULL, 0);
        g_curr_co->status = CO_STATUS_RUNNING;
    }
}

struct coroutine *co_curr() {
    check_init();
    return g_curr_co;
}

int co_resume(struct coroutine *next, void *param, void **result) {
    check_init();

    switch (next->status) {
        case CO_STATUS_INIT:
            co_ctx_make(next);
        case CO_STATUS_PENDING:
            break;
        default:
            return -1;
    }

    struct coroutine *curr = g_curr_co;
    g_curr_co = next;
    next->prev = curr;
    next->param = param;
    curr->status = CO_STATUS_NORMAL;
    next->status = CO_STATUS_RUNNING;
    co_ctx_swap(&curr->ctx, &next->ctx);
    if (result) {
        *result = next->result;
    }

    return 0;
}

int co_yield(void *result, void **param) {
    check_init();

    struct coroutine *curr = g_curr_co;
    struct coroutine *prev = curr->prev;

    if (!prev) {
        return -1;
    }

    g_curr_co = prev;
    curr->result = result;
    if (curr->status != CO_STATUS_DEAD) {
        curr->status = CO_STATUS_PENDING;
    }
    prev->status = CO_STATUS_RUNNING;
    co_ctx_swap(&curr->ctx, &prev->ctx);
    if (param) {
        *param = curr->param;
    }

    return 0;
}

