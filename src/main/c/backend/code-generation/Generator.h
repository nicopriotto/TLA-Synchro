#ifndef GENERATOR_HEADER
#define GENERATOR_HEADER

#include "../../frontend/syntactic-analysis/AbstractSyntaxTree.h"
#include "../../shared/CompilerState.h"
#include "../../shared/Logger.h"
#include "../../shared/String.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

/** Initialize module's internal state. */
void initializeGeneratorModule();

/** Shutdown module's internal state. */
void shutdownGeneratorModule();

/**
 * Generates the final C code output using the current compiler state.
 */
void generate(CompilerState * compilerState);

#endif
