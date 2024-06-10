#include "coroutine.h"
#include <stdio.h>

void *bar(void *param) {
    char *s = param;
    printf("co bar1: %s\n", s);
    co_yield("boy", (void**)&s);
    printf("co bar2: %s\n", s);
    return "girl";
}

void *foo(void *param) {
    char *s = param, *s1;

    printf("co foo1: %s\n", s);

    struct coroutine *co = co_new(bar, 1024 * 1024);
    char buf[128];
    snprintf(buf, sizeof(buf), "%s-%s", "foo", s);
    co_resume(co, buf, (void**)&s1);
    printf("co foo2: %s\n", s1);

    co_yield(co, (void**)&s);
    printf("co foo3: %s\n", s);

    co_free(co);
    return "cat";
}

int main() {
    struct coroutine *co = co_new(foo, 1024 * 1024);
    struct coroutine *co1;
    co_resume(co, "apple", (void**)&co1);

    char *s;
    co_resume(co1, "orange", (void**)&s);
    printf("co main1: %s. status=%d\n", s, co_status(co1));
    co_resume(co, "banana", (void**)&s);
    printf("co main2: %s. status=%d\n", s, co_status(co));
    co_free(co);
    return 0;
}
