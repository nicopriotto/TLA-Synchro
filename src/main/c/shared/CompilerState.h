#ifndef COMPILER_STATE_HEADER
#define COMPILER_STATE_HEADER

#include "Type.h"
#include "semantic-analysis/SymbolTable.h"
#include "semantic-analysis/ScopeStack.h"

/**
 * The general status of a compilation.
*/
typedef enum {
	SUCCEED = 0,
	FAILED = 1
} CompilationStatus;

/**
 * The global state of the compiler. Should transport every data structure
 * needed across the different phases of a compilation.
 */
typedef struct {
	// The root node of the AST.
	void * abstractSyntaxtTree;

	// A flag that indicates the current state of the compilation so far.
	boolean succeed;

	SymbolTable symbolTable;
	ScopeStack scopeStack;

	// The computed value of the entire program (only for the calculator).
	int value;
} CompilerState;

#endif
