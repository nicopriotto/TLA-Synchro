#include "SynchronizationRuntime.h"
#include <unistd.h>
#include <stdint.h>

/* MODULE INTERNAL STATE */

static Logger * _logger = NULL;
static pthread_mutex_t _sync_mutex;
static bool _runtime_initialized = false;

void initializeSynchronizationRuntimeModule() {
    _logger = createLogger("SynchronizationRuntime");
}

void shutdownSynchronizationRuntimeModule() {
    if (_logger != NULL) {
        destroyLogger(_logger);
    }
}

/** PRIVATE FUNCTIONS */

static ComputationResult _validateProgram(Program * program);
static boolean _validateDeclarationList(DeclarationList * declarationList);
static boolean _validateStatementList(StatementList * statementList);
static boolean _validateStatement(Statement * statement);

/**
 * Validates the entire program structure
 */
static ComputationResult _validateProgram(Program * program) {
    ComputationResult result = { .succeed = false, .value = 0 };
    
    if (program == NULL) {
        logError(_logger, "Program is NULL");
        return result;
    }
    
    logDebugging(_logger, "Validating program structure...");
    
    if (program->globalDeclarations != NULL) {
        if (!_validateDeclarationList(program->globalDeclarations)) {
            logError(_logger, "Invalid global declarations");
            return result;
        }
    }
    
    result.succeed = true;
    result.value = 1;
    
    logDebugging(_logger, "Program validation completed successfully");
    return result;
}

/**
 * Validates a declaration list
 */
static boolean _validateDeclarationList(DeclarationList * declarationList) {
    if (declarationList == NULL) return true;
    
    if (declarationList->identifier == NULL) {
        logError(_logger, "Declaration has NULL identifier");
        return false;
    }
    
    if (declarationList->declarationTail != NULL) {
        switch (declarationList->declarationTail->type) {
            case DECL_FUNCTION:
                if (declarationList->declarationTail->function.statementList != NULL) {
                    if (!_validateStatementList(declarationList->declarationTail->function.statementList)) {
                        return false;
                    }
                }
                break;
            case DECL_CONSTANT:
                break;
        }
    }
    
    if (declarationList->next != NULL) {
        return _validateDeclarationList(declarationList->next);
    }
    
    return true;
}

/**
 * Validates a statement list
 */
static boolean _validateStatementList(StatementList * statementList) {
    if (statementList == NULL) return true;
    
    if (!_validateStatement(statementList->statement)) {
        return false;
    }
    
    if (statementList->next != NULL) {
        return _validateStatementList(statementList->next);
    }
    
    return true;
}

/**
 * Validates a single statement
 */
static boolean _validateStatement(Statement * statement) {
    if (statement == NULL) return true;
    
    switch (statement->type) {
        case STMT_SIMPLE:
            return true;
        case STMT_IF:
            return _validateStatement(statement->ifStatement.thenStatement);
        case STMT_IF_ELSE:
            return _validateStatement(statement->ifElseStatement.thenStatement) &&
                   _validateStatement(statement->ifElseStatement.elseStatement);
        case STMT_WHILE:
            return _validateStatement(statement->whileStatement.body);
        case STMT_FOR:
            return _validateStatement(statement->forStatement.body);
        case STMT_FOREVER:
            return _validateStatement(statement->foreverStatement.body);
        case STMT_BLOCK:
            return _validateStatementList(statement->blockStatement);
        default:
            logError(_logger, "Unknown statement type: %d", statement->type);
            return false;
    }
}

/** PUBLIC FUNCTIONS */

ComputationResult computeProgram(Program * program) {
    logDebugging(_logger, "Computing/validating synchronization program...");
    
    ComputationResult result = _validateProgram(program);
    
    if (result.succeed) {
        logDebugging(_logger, "Program computation completed successfully");
    } else {
        logError(_logger, "Program computation failed");
    }
    
    return result;
}

void generateRuntimeInitialization(FILE* output) {
    fprintf(output, "// Runtime globals\n");
    fprintf(output, "pthread_mutex_t sync_mutex;\n");
    fprintf(output, "bool runtime_initialized = false;\n\n");
}

void generateBuiltinFunctionDeclarations(FILE* output) {
    fprintf(output, "// Built-in function declarations\n");
    fprintf(output, "void _synchro_runtime_init();\n");
    fprintf(output, "void _synchro_runtime_cleanup();\n");
    fprintf(output, "void _synchro_print(char* value);\n");
    fprintf(output, "void _synchro_sleep(int seconds);\n");
    fprintf(output, "void _synchro_up(pthread_mutex_t* mutex);\n");
    fprintf(output, "void _synchro_down(pthread_mutex_t* mutex);\n");
    fprintf(output, "void* _synchro_thread(void* (*func)(void*), int thread_count);\n\n");
}

void generateThreadSupport(FILE* output) {
    fprintf(output, "// Thread support structure\n");
    fprintf(output, "typedef struct {\n");
    fprintf(output, "    void* (*func)(void*);\n");
    fprintf(output, "    void* arg;\n");
    fprintf(output, "} thread_args_t;\n\n");
    
    fprintf(output, "void* thread_wrapper(void* data) {\n");
    fprintf(output, "    thread_args_t* args = (thread_args_t*)data;\n");
    fprintf(output, "    void* result = args->func(args->arg);\n");
    fprintf(output, "    free(args);\n");
    fprintf(output, "    return result;\n");
    fprintf(output, "}\n\n");
}

void generateBuiltinFunctionImplementations(FILE* output) {
    fprintf(output, "// Built-in function implementations\n");
    
    fprintf(output, "void _synchro_runtime_init() {\n");
    fprintf(output, "    if (!runtime_initialized) {\n");
    fprintf(output, "        pthread_mutex_init(&sync_mutex, NULL);\n");
    fprintf(output, "        runtime_initialized = true;\n");
    fprintf(output, "        printf(\"Synchronization runtime initialized\\n\");\n");
    fprintf(output, "    }\n");
    fprintf(output, "}\n\n");
    
    fprintf(output, "void _synchro_runtime_cleanup() {\n");
    fprintf(output, "    if (runtime_initialized) {\n");
    fprintf(output, "        pthread_mutex_destroy(&sync_mutex);\n");
    fprintf(output, "        runtime_initialized = false;\n");
    fprintf(output, "        printf(\"Synchronization runtime cleaned up\\n\");\n");
    fprintf(output, "    }\n");
    fprintf(output, "}\n\n");
    
    fprintf(output, "void _synchro_print(char* value) {\n");
    fprintf(output, "    pthread_mutex_lock(&sync_mutex);\n");
    fprintf(output, "    printf(\"[PRINT] %%s\\n\", value);\n");
    fprintf(output, "    fflush(stdout);\n");
    fprintf(output, "    pthread_mutex_unlock(&sync_mutex);\n");
    fprintf(output, "}\n\n");
    
    fprintf(output, "void _synchro_sleep(int seconds) {\n");
    fprintf(output, "    printf(\"[SLEEP] Sleeping for %%d seconds\\n\", seconds);\n");
    fprintf(output, "    sleep(seconds);\n");
    fprintf(output, "    printf(\"[SLEEP] Woke up after %%d seconds\\n\", seconds);\n");
    fprintf(output, "}\n\n");
    
    fprintf(output, "void _synchro_up(pthread_mutex_t* mutex) {\n");
    fprintf(output, "    printf(\"[UP] Releasing mutex\\n\");\n");
    fprintf(output, "    pthread_mutex_unlock(mutex);\n");
    fprintf(output, "}\n\n");
    
    fprintf(output, "void _synchro_down(pthread_mutex_t* mutex) {\n");
    fprintf(output, "    printf(\"[DOWN] Acquiring mutex\\n\");\n");
    fprintf(output, "    pthread_mutex_lock(mutex);\n");
    fprintf(output, "    printf(\"[DOWN] Mutex acquired\\n\");\n");
    fprintf(output, "}\n\n");
    
    fprintf(output, "void* _synchro_thread(void* (*func)(void*), int thread_count) {\n");
    fprintf(output, "    pthread_t* threads = malloc(thread_count * sizeof(pthread_t));\n");
    fprintf(output, "    if (threads == NULL) {\n");
    fprintf(output, "        printf(\"[ERROR] Failed to allocate memory for threads\\n\");\n");
    fprintf(output, "        return NULL;\n");
    fprintf(output, "    }\n");
    fprintf(output, "    \n");
    fprintf(output, "    printf(\"[THREAD] Creating %%d threads\\n\", thread_count);\n");
    fprintf(output, "    \n");
    fprintf(output, "    for (int i = 0; i < thread_count; i++) {\n");
    fprintf(output, "        if (pthread_create(&threads[i], NULL, func, NULL) != 0) {\n");
    fprintf(output, "            printf(\"[ERROR] Failed to create thread %%d\\n\", i);\n");
    fprintf(output, "            free(threads);\n");
    fprintf(output, "            return NULL;\n");
    fprintf(output, "        }\n");
    fprintf(output, "        pthread_detach(threads[i]);\n");
    fprintf(output, "        printf(\"[THREAD] Thread %%d created and detached\\n\", i);\n");
    fprintf(output, "    }\n");
    fprintf(output, "    \n");
    fprintf(output, "    free(threads);\n");
    fprintf(output, "    return (void*)(intptr_t)thread_count; // Return number of threads created\n");
    fprintf(output, "}\n\n");
}

void generateRuntimeCleanup(FILE* output) {
}

void synchro_runtime_init() {
    if (!_runtime_initialized) {
        pthread_mutex_init(&_sync_mutex, NULL);
        _runtime_initialized = true;
    }
}

void synchro_runtime_cleanup() {
    if (_runtime_initialized) {
        pthread_mutex_destroy(&_sync_mutex);
        _runtime_initialized = false;
    }
}

void synchro_print(int value) {
    pthread_mutex_lock(&_sync_mutex);
    printf("[PRINT] %d\n", value);
    fflush(stdout);
    pthread_mutex_unlock(&_sync_mutex);
}

void synchro_sleep(int seconds) {
    printf("[SLEEP] Sleeping for %d seconds\n", seconds);
    sleep(seconds);
    printf("[SLEEP] Woke up after %d seconds\n", seconds);
}

void synchro_up() {
    printf("[UP] Releasing mutex\n");
    pthread_mutex_unlock(&_sync_mutex);
}

void synchro_down() {
    printf("[DOWN] Acquiring mutex\n");
    pthread_mutex_lock(&_sync_mutex);
    printf("[DOWN] Mutex acquired\n");
}

void* synchro_thread(void* (*func)(void*), void* arg) {
    pthread_t thread;
    
    if (pthread_create(&thread, NULL, func, arg) != 0) {
        return NULL;
    }
    
    pthread_detach(thread);
    return NULL;
}
