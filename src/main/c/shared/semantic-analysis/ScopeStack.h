#ifndef SCOPE_STACK_H
#define SCOPE_STACK_H
#include "SymbolTable.h"

typedef struct {
    int *levels;
    int size;
    int capacity;
    int nextScopeId;
} ScopeStack;

void initScopeStack(ScopeStack *stack);
void pushScope(ScopeStack *stack);
void popScope(ScopeStack *stack, SymbolTable *st);
int currentScope(const ScopeStack *stack);
void freeScopeStack(ScopeStack *stack);

#endif