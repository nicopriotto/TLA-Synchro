#ifndef COMPILER_STATE_H
#define COMPILER_STATE_H

#include "semantic-analysis/SymbolTable.h"
#include "semantic-analysis/ScopeStack.h"
#include "Type.h"

typedef enum {
    SUCCEED = 0,    
    FAILED = 1      
} CompilationStatus;

typedef struct {
    void* abstractSyntaxtTree;
    boolean succeed;
    SymbolTable symbolTable;
    ScopeStack scopeStack;
    int value;
} CompilerState;

CompilerState* currentCompilerState(void);

#endif
