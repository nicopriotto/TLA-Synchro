#ifndef COMPILER_STATE_H
#define COMPILER_STATE_H

#include "semantic-analysis/SymbolTable.h"
#include "semantic-analysis/ScopeStack.h"
#include "Type.h"

// Compilation status enum - CRITICAL: SUCCEED must be 0, FAILED must be non-zero
typedef enum {
    SUCCEED = 0,    // Success (Unix convention)
    FAILED = 1      // Failure (Unix convention)
} CompilationStatus;

typedef struct {
    void* abstractSyntaxtTree;
    boolean succeed;
    SymbolTable symbolTable;
    ScopeStack scopeStack;
    int value;
} CompilerState;

// Global state access function
CompilerState* currentCompilerState(void);

#endif
