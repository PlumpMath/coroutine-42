#include <stdio.h>
#include "coroutine.h"

typedef struct {
    coroutine* c;
    int max_x,max_y;
    int x,y;
} iter;

void iterate(void* p) {
    iter* it = (iter*)p;
    int x,y;
    for(x = 0; x < it->max_x; x++) {
        for(y = 0; y < it->max_y; y++) {
            it->x = x;
            it->y = y;
            yield(it->c);
        }
    }
}

#define N 1024
int main() {
    coroutine c;
    int stack[N];
    iter it = {&c,5,5};
    start(&c,&iterate,&it,stack + N);
    while(next(&c)) {
        printf("%d %d\n",it.x,it.y);
    }
}
