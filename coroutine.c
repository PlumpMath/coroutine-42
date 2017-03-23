#include "coroutine.h"

enum {WORKING=1, DONE};

void yield(coroutine* c) {
    if(!setjmp(c->callee_context)) {
        longjmp(c->caller_context,WORKING);
    }
}

int next(coroutine* c) {
    int ret = setjmp(c->caller_context);
    if(!ret) {
        longjmp(c->callee_context,1);
    } else {
        return ret == WORKING;
    }
}

typedef struct {
    coroutine* c;
    func f;
    void* arg;
    void* old_sp;
    void* old_fp;
} start_params;

#define get_sp(p) \
    asm volatile("movq %%rsp, %0" : "=r"(p));
#define get_fp(p) \
    asm volatile("movq %%rbp, %0" : "=r"(p));
#define set_sp(p) \
    asm volatile("movq %0, %%rsp" : : "r"(p));
#define set_fp(p) \
    asm volatile("movq %0, %%rbp" : : "r"(p));

enum {FRAME_SZ = 5};

void start(coroutine* c,func f,void* arg,void* sp) {
    start_params* p = ((start_params*)sp) - 1;
    //save params before stack switching
    p->c = c;
    p->f = f;
    p->arg = arg;
    get_sp(p->old_sp);
    get_fp(p->old_fp);

    set_sp(p - FRAME_SZ);
    set_fp(p);
    get_fp(p);
    
    //read params from p 
    if(!setjmp(p->c->callee_context)) {
        set_sp(p->old_sp);
        set_fp(p->old_fp);
        return;
    }
    (*p->f)(p->arg);
    longjmp(p->c->caller_context,DONE);
}
