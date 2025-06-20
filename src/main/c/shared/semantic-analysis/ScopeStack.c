#include "ScopeStack.h"
#include <stdlib.h>
#include <stdio.h>

#define INITIAL_CAP 20

static void ensureCapacity(ScopeStack *stk) {
    if (stk->size == stk->capacity) {
        stk->capacity *= 2;
        stk->levels = realloc(stk->levels, sizeof(int) * stk->capacity);
        if (!stk->levels) {
            fprintf(stderr, "OOM\n");
            exit(EXIT_FAILURE);
        }
    }
}

ScopeStack initScopeStack() {
    ScopeStack* stk = calloc(sizeof(ScopeStack), 1);
    stk->levels = malloc(sizeof(int) * INITIAL_CAP);
    if (!stk->levels) {
        fprintf(stderr, "OOM\n");
        exit(EXIT_FAILURE);
    }    
    stk->size = 0;
    stk->capacity = INITIAL_CAP;
    stk->nextScopeId = 1;
    return *stk;
}

void pushScope(ScopeStack *stk) {
    ensureCapacity(stk);
    stk->levels[stk->size++] = stk->nextScopeId++;
}

void popScope(ScopeStack *stk, SymbolTable *st) {
    if (stk->size == 0) return;
    int scopeId = stk->levels[--stk->size];
    removeScopeSymbols(st, scopeId);
}

int currentScope(const ScopeStack *stk) {
    return stk->size == 0 ? -1 : stk->levels[stk->size - 1];
}

void freeScopeStack(ScopeStack *stk) {
    free(stk->levels);
    stk->levels = NULL;
    stk->size = stk->capacity = 0;
    stk->nextScopeId = 1;
}