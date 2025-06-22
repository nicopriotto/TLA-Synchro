#include "ScopeStack.h"
#include "SymbolTable.h"
#include <stdlib.h>
#include <stdio.h>

#define INITIAL_CAP 20
#define GLOBAL_SCOPE_ID 0 

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

void initScopeStack(ScopeStack *stk) {
    stk->levels = malloc(sizeof(int) * INITIAL_CAP);
    if (!stk->levels) {
        fprintf(stderr, "OOM\n");
        exit(EXIT_FAILURE);
    }    
    stk->size = 0;
    stk->capacity = INITIAL_CAP;
    stk->nextScopeId = 1;
}

void pushScope(ScopeStack *stk) {
    if (!stk || !stk->levels) return;
    ensureCapacity(stk);
    stk->levels[stk->size++] = stk->nextScopeId++;
}

void popScope(ScopeStack *stk, SymbolTable *st) {
    if (!stk || stk->size == 0) return;
    int scopeId = stk->levels[--stk->size];
}

int currentScope(const ScopeStack *stk) {
    if (!stk || stk->size == 0) return GLOBAL_SCOPE_ID; 
    return stk->levels[stk->size - 1];
}

int isGlobalScope(const ScopeStack *stack) {
    return currentScope(stack) == GLOBAL_SCOPE_ID;
}

void freeScopeStack(ScopeStack *stk) {
    if (!stk) return;
    if (stk->levels) {
        free(stk->levels);
        stk->levels = NULL;
    }
    stk->size = stk->capacity = 0;
    stk->nextScopeId = 1;
}
