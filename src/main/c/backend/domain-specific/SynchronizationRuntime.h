#ifndef SYNCHRONIZATION_RUNTIME_HEADER
#define SYNCHRONIZATION_RUNTIME_HEADER

#include "../../shared/CompilerState.h"
#include "../../shared/Logger.h"
#include "../../frontend/syntactic-analysis/AbstractSyntaxTree.h"
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>

typedef struct {
    boolean succeed;
    int value;
} ComputationResult;

/** Initialize module's internal state. */
void initializeSynchronizationRuntimeModule();

/** Shutdown module's internal state. */
void shutdownSynchronizationRuntimeModule();

ComputationResult computeProgram(Program * program);

// Runtime support functions for generated code
void generateRuntimeInitialization(FILE* output);
void generateBuiltinFunctionDeclarations(FILE* output);
void generateThreadSupport(FILE* output);
void generateBuiltinFunctionImplementations(FILE* output);
void generateRuntimeCleanup(FILE* output);

// Runtime functions that will be available in generated code
void synchro_runtime_init();
void synchro_runtime_cleanup();
void synchro_print(int value);
void synchro_sleep(int seconds);
void synchro_up();
void synchro_down();
void* synchro_thread(void* (*func)(void*), void* arg);

#endif
