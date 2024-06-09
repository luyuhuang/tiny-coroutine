#include "coroutine.h"
#include <stdio.h>

void *foo(void *param) {
    char *s = param;
    printf("co foo1: %s\n", s);
    co_yield("dog", (void**)&s);
    printf("co foo2: %s\n", s);
    return "cat";
}

int main() {
    struct coroutine *co = co_new(foo, 1024 * 1024);
    char *s;
    co_resume(co, "apple", (void**)&s);
    printf("co main1: %s. status=%d\n", s, co->status);
    co_resume(co, "banana", (void**)&s);
    printf("co main2: %s. status=%d\n", s, co->status);
    co_free(co);
    return 0;
}
