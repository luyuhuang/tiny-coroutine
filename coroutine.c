#include "coroutine.h"
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

static __thread struct co_env *g_co_env = NULL;
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
    co->start = start;
    co->param = co->result = NULL;

    return co;
}

void co_free(struct coroutine *co) {
    free(co->stack);
    free(co);
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
    co->ctx.regs[CO_RSP] = sp;
    co->ctx.regs[CO_RAX] = co_entrance;
    co->ctx.regs[CO_RDI] = co;
}

static struct co_env *co_env_new() {
    struct co_env *env = malloc(sizeof(struct co_env));
    env->stack[0] = co_new(NULL, 0);
    env->stack_top = 1;
    return env;
}

static void co_swap(struct coroutine *curr, struct coroutine *next) {
    if (curr->status == CO_STATUS_RUNNING) {
        curr->status = CO_STATUS_PENDING;
    }
    next->status = CO_STATUS_RUNNING;
    co_ctx_swap(&curr->ctx, &next->ctx);
}

int co_resume(struct coroutine *co, void *param, void **result) {
    if (!g_co_env) {
        g_co_env = co_env_new();
    }

    switch (co->status) {
        case CO_STATUS_INIT:
            co_ctx_make(co);
            break;
        case CO_STATUS_DEAD:
        case CO_STATUS_RUNNING:
            return -1;
    }

    struct coroutine *curr = g_co_env->stack[g_co_env->stack_top-1];
    g_co_env->stack[g_co_env->stack_top++] = co;
    co->param = param;
    co_swap(curr, co);
    *result = co->result;

    return 0;
}

int co_yield(void *result, void **param) {
    if (!g_co_env) {
        g_co_env = co_env_new();
    }

    if (g_co_env->stack_top == 1) {
        return -1;
    }

    struct coroutine *curr = g_co_env->stack[--g_co_env->stack_top];
    struct coroutine *prev = g_co_env->stack[g_co_env->stack_top-1];

    curr->result = result;
    co_swap(curr, prev);
    *param = curr->param;

    return 0;
}

