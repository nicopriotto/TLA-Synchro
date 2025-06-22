#include "Generator.h"
#include "../domain-specific/SynchronizationRuntime.h"

/* MODULE INTERNAL STATE */

const char _indentationCharacter = ' ';
const char _indentationSize = 4;
static Logger * _logger = NULL;
static FILE* _outputFile = NULL;

void initializeGeneratorModule() {
    _logger = createLogger("Generator");
    _outputFile = fopen("output.c", "w");
    if (_outputFile == NULL) {
        logError(_logger, "Cannot open output file");
        exit(1);
    }
}

void shutdownGeneratorModule() {
    if (_outputFile != NULL) {
        fclose(_outputFile);
        _outputFile = NULL;
    }
    if (_logger != NULL) {
        destroyLogger(_logger);
    }
}

/** PRIVATE FUNCTIONS */

static void _generateIncludes();
static void _generateProgram(Program * program);
static void _generateDeclarationList(DeclarationList * declarationList);
static void _generateDeclaration(TypeNode* type, char* identifier, DeclarationTail* declarationTail);
static void _generateVariableDeclaration(TypeNode* type, char* identifier, Constant* constant);
static void _generateFunctionDeclaration(TypeNode* type, char* identifier, ParameterList* parameterList, StatementList* statementList);
static void _generateParameterList(ParameterList * paramList);
static void _generateStatementList(StatementList * stmtList);
static void _generateStatement(Statement * stmt);
static void _generateSimpleStatement(SimpleStatement * simpleStmt);
static void _generateCondition(Condition * condition);
static void _generateExpression(Expression * expr);
static void _generateArgumentList(ArgumentList * argList);
static void _generateRelationalOperator(RelationalOperator * relOp);
static void _generateForInitializer(ForInitializer * forInit);
static void _generateForUpdate(ForUpdate * forUpdate);
static char * _indentation(const unsigned int level);
static void _output(const unsigned int indentationLevel, const char * const format, ...);
static const char* _getTypeName(TypeNode* type);
static const char* _getBuiltinFunctionName(const char* funcName);
static void _generateMainFunction(DeclarationList * declarationList);
static DeclarationList* _findMainFunction(DeclarationList * declarationList);

/**
 * Generates the necessary includes for the C output
 */
static void _generateIncludes() {
    _output(0, "#include <stdio.h>\n");
    _output(0, "#include <stdlib.h>\n");
    _output(0, "#include <unistd.h>\n");
    _output(0, "#include <semaphore.h>\n");
    _output(0, "#include <pthread.h>\n");
    _output(0, "#include <string.h>\n");
    _output(0, "#include <stdbool.h>\n");
    _output(0, "#include <stdint.h>\n\n");
}

/**
 * Gets the C type name from TypeNode
 */
static const char* _getTypeName(TypeNode* type) {
    if (type == NULL) return "int";
    
    switch (type->type) {
        case TYPE_INTEGER: return "int";
        case TYPE_STRING: return "char*";
        case TYPE_FLOAT: return "float";
        case TYPE_BOOLEAN: return "bool";
        case TYPE_SEM: return "sem_t*";
        default: return "int";
    }
}

/**
 * Maps built-in function names to C equivalents
 */
static const char* _getBuiltinFunctionName(const char* funcName) {
    if (funcName == NULL) return "_unknown_function";
    if (strcmp(funcName, "print") == 0) return "_synchro_print";
    if (strcmp(funcName, "sleep") == 0) return "_synchro_sleep";
    if (strcmp(funcName, "up") == 0) return "_synchro_up";
    if (strcmp(funcName, "down") == 0) return "_synchro_down";
    if (strcmp(funcName, "thread") == 0) return "_synchro_thread";
    return funcName;
}

/**
 * Finds the main function in the declaration list
 */
static DeclarationList* _findMainFunction(DeclarationList * declarationList) {
    while (declarationList != NULL) {
        if (declarationList->identifier != NULL && 
            strcmp(declarationList->identifier, "main") == 0 &&
            declarationList->declarationTail != NULL &&
            declarationList->declarationTail->type == DECL_FUNCTION) {
            return declarationList;
        }
        declarationList = declarationList->next;
    }
    return NULL;
}

/**
 * Generates the main function that calls user's main
 */
static void _generateMainFunction(DeclarationList * declarationList) {
    DeclarationList* mainFunc = _findMainFunction(declarationList);
    
    _output(0, "int main() {\n");
    _output(1, "_synchro_runtime_init();\n\n");
    
    DeclarationList* current = declarationList;
    while (current != NULL) {
        if (current->type && current->type->type == TYPE_SEM && 
            current->declarationTail && current->declarationTail->type == DECL_CONSTANT) {
            _output(1, "sem_init(%s, 0, %s_init_value);\n", current->identifier, current->identifier);
        }
        current = current->next;
    }
    _output(0, "\n");
    
    if (mainFunc != NULL) {
        _output(1, "// Call user's main function\n");
        _output(1, "_userMain(NULL);\n");
    } else {
        _output(1, "// No main function found\n");
        _output(1, "printf(\"No main function to execute\\n\");\n");
    }
    
    _output(1, "\n_synchro_runtime_cleanup();\n");
    _output(1, "return 0;\n");
    _output(0, "}\n");
}

/**
 * Generates the output of the entire program
 */
static void _generateProgram(Program * program) {
    logDebugging(_logger, "Generating program...");
    
    _generateIncludes();
    generateRuntimeInitialization(_outputFile);
    generateBuiltinFunctionDeclarations(_outputFile);
    generateThreadSupport(_outputFile);
    generateBuiltinFunctionImplementations(_outputFile);
    generateRuntimeCleanup(_outputFile);
    
    if (program->globalDeclarations != NULL) {
        DeclarationList* current = program->globalDeclarations;
        while (current != NULL) {
            if (current->identifier != NULL && strcmp(current->identifier, "main") != 0) {
                _generateDeclaration(current->type, current->identifier, current->declarationTail);
            }
            current = current->next;
        }
    }
    
    DeclarationList* mainFunc = _findMainFunction(program->globalDeclarations);
    if (mainFunc != NULL) {
        _generateDeclaration(mainFunc->type, mainFunc->identifier, mainFunc->declarationTail);
    }
    
    _generateMainFunction(program->globalDeclarations);
    
    logDebugging(_logger, "Code generation completed.");
}

/**
 * Generates a list of declarations
 */
static void _generateDeclarationList(DeclarationList * declarationList) {
    if (declarationList == NULL) return;
    
    _generateDeclaration(declarationList->type, declarationList->identifier, declarationList->declarationTail);
    
    if (declarationList->next != NULL) {
        _generateDeclarationList(declarationList->next);
    }
}

/**
 * Generates a single declaration
 */
static void _generateDeclaration(TypeNode* type, char* identifier, DeclarationTail* declarationTail) {
    if (declarationTail == NULL) {
        _output(0, "%s %s;\n", _getTypeName(type), identifier);
        return;
    }
    
    switch (declarationTail->type) {
        case DECL_CONSTANT:
            _generateVariableDeclaration(type, identifier, declarationTail->constant);
            break;
        case DECL_FUNCTION:
            _generateFunctionDeclaration(type, identifier, 
                declarationTail->function.parameterList, 
                declarationTail->function.statementList);
            break;
    }
}

/**
 * Generates a variable declaration with initialization
 */
static void _generateVariableDeclaration(TypeNode* type, char* identifier, Constant* constant) {
    if (type->type == TYPE_SEM) {
        int initValue = 0;
        if (constant != NULL && constant->type == CONST_INTEGER) {
            initValue = constant->integer;
        }
        _output(0, "sem_t %s_sem;\n", identifier);
        _output(0, "sem_t* %s = &%s_sem;\n", identifier, identifier);
        _output(0, "int %s_init_value = %d;\n", identifier, initValue);
    } else {
        _output(0, "%s %s", _getTypeName(type), identifier);
        
        if (constant != NULL) {
            _output(0, " = ");
            switch (constant->type) {
                case CONST_INTEGER:
                    _output(0, "%d", constant->integer);
                    break;
                case CONST_FLOAT:
                    _output(0, "%f", constant->floatVal);
                    break;
                case CONST_BOOLEAN:
                    _output(0, "%s", constant->boolean ? "true" : "false");
                    break;
                case CONST_STRING:
                    _output(0, "\"%s\"", constant->string);
                    break;
            }
        }
        
        _output(0, ";\n");
    }
}

/**
 * Generates a function declaration
 */
static void _generateFunctionDeclaration(TypeNode* type, char* identifier, ParameterList* parameterList, StatementList* statementList) {
    const char* functionName = identifier;
    if (identifier != NULL && strcmp(identifier, "main") == 0) {
        functionName = "_userMain";
        _output(0, "void* %s(", functionName);
    } else {
        _output(0, "void* %s(", functionName);
    }
    
    if (parameterList != NULL) {
        _generateParameterList(parameterList);
    } else {
        _output(0, "void* unused");
    }
    
    _output(0, ") {\n");
    
    if (statementList != NULL) {
        _generateStatementList(statementList);
    }
    
    _output(1, "return NULL;\n");
    _output(0, "}\n\n");
}

/**
 * Generates a parameter list
 */
static void _generateParameterList(ParameterList * paramList) {
    if (paramList == NULL) return;
    
    _output(0, "%s %s", _getTypeName(paramList->type), paramList->identifier);
    
    if (paramList->next != NULL) {
        _output(0, ", ");
        _generateParameterList(paramList->next);
    }
}

/**
 * Generates a list of statements
 */
static void _generateStatementList(StatementList * stmtList) {
    if (stmtList == NULL) return;
    
    _generateStatement(stmtList->statement);
    
    if (stmtList->next != NULL) {
        _generateStatementList(stmtList->next);
    }
}

/**
 * Generates for loop initializer
 */
static void _generateForInitializer(ForInitializer * forInit) {
    if (forInit == NULL) return;
    
    if (forInit->declaration) {
        _output(0, "%s %s", _getTypeName(forInit->declaration->type), 
            forInit->declaration->identifier);
        
        if (forInit->declaration->condition && 
            forInit->declaration->condition->type == COND_EXPRESSION) {
            _output(0, " = ");
            _generateExpression(forInit->declaration->condition->expression.expression);
        }
    }
    
    if (forInit->next != NULL) {
        _output(0, ", ");
        _generateForInitializer(forInit->next);
    }
}

/**
 * Generates for loop update
 */
static void _generateForUpdate(ForUpdate * forUpdate) {
    if (forUpdate == NULL) return;
    
    if (forUpdate->statement) {
        switch (forUpdate->statement->type) {
            case SIMPLE_INCREMENT:
                if (forUpdate->statement->increment.isPrefix) {
                    _output(0, "++%s", forUpdate->statement->increment.identifier);
                } else {
                    _output(0, "%s++", forUpdate->statement->increment.identifier);
                }
                break;
            case SIMPLE_DECREMENT:
                if (forUpdate->statement->decrement.isPrefix) {
                    _output(0, "--%s", forUpdate->statement->decrement.identifier);
                } else {
                    _output(0, "%s--", forUpdate->statement->decrement.identifier);
                }
                break;
            case SIMPLE_ASSIGNMENT:
                _output(0, "%s = ", forUpdate->statement->assignment.identifier);
                _generateExpression(forUpdate->statement->assignment.expression);
                break;
            default:
                break;
        }
    }
    
    if (forUpdate->next != NULL) {
        _output(0, ", ");
        _generateForUpdate(forUpdate->next);
    }
}

/**
 * Generates a single statement
 */
static void _generateStatement(Statement * stmt) {
    if (stmt == NULL) return;
    
    switch (stmt->type) {
        case STMT_SIMPLE:
            _generateSimpleStatement(stmt->simpleStatement);
            break;
        case STMT_IF:
            _output(1, "if (");
            _generateCondition(stmt->ifStatement.condition);
            _output(0, ") {\n");
            _generateStatement(stmt->ifStatement.thenStatement);
            _output(1, "}\n");
            break;
        case STMT_IF_ELSE:
            _output(1, "if (");
            _generateCondition(stmt->ifElseStatement.condition);
            _output(0, ") {\n");
            _generateStatement(stmt->ifElseStatement.thenStatement);
            _output(1, "} else {\n");
            _generateStatement(stmt->ifElseStatement.elseStatement);
            _output(1, "}\n");
            break;
        case STMT_WHILE:
            _output(1, "while (");
            _generateCondition(stmt->whileStatement.condition);
            _output(0, ") {\n");
            _generateStatement(stmt->whileStatement.body);
            _output(1, "}\n");
            break;
        case STMT_FOR:
            _output(1, "for (");
            
            if (stmt->forStatement.initializer != NULL) {
                _generateForInitializer(stmt->forStatement.initializer);
            }
            _output(0, "; ");
            
            if (stmt->forStatement.condition != NULL) {
                _generateCondition(stmt->forStatement.condition);
            }
            _output(0, "; ");
            
            if (stmt->forStatement.update != NULL) {
                _generateForUpdate(stmt->forStatement.update);
            }
            
            _output(0, ") {\n");
            _generateStatement(stmt->forStatement.body);
            _output(1, "}\n");
            break;
        case STMT_FOREVER:
            _output(1, "while (1) {\n");
            _generateStatement(stmt->foreverStatement.body);
            _output(1, "}\n");
            break;
        case STMT_BLOCK:
            _generateStatementList(stmt->blockStatement);
            break;
    }
}

/**
 * Generates a simple statement
 */
static void _generateSimpleStatement(SimpleStatement * simpleStmt) {
    if (simpleStmt == NULL) return;
    
    switch (simpleStmt->type) {
        case SIMPLE_FUNCTION_CALL:
            if (simpleStmt->functionCall.function != NULL && simpleStmt->functionCall.function->identifier != NULL) {
                _output(1, "%s(", _getBuiltinFunctionName(simpleStmt->functionCall.function->identifier));
                if (simpleStmt->functionCall.arguments != NULL) {
                    _generateArgumentList(simpleStmt->functionCall.arguments);
                }
                _output(0, ");\n");
            } else {
                _output(1, "/* NULL function call */;\n");
            }
            break;
        case SIMPLE_INCREMENT:
            if (simpleStmt->increment.isPrefix) {
                _output(1, "++%s;\n", simpleStmt->increment.identifier);
            } else {
                _output(1, "%s++;\n", simpleStmt->increment.identifier);
            }
            break;
        case SIMPLE_DECREMENT:
            if (simpleStmt->decrement.isPrefix) {
                _output(1, "--%s;\n", simpleStmt->decrement.identifier);
            } else {
                _output(1, "%s--;\n", simpleStmt->decrement.identifier);
            }
            break;
        case SIMPLE_ASSIGNMENT:
            _output(1, "%s = ", simpleStmt->assignment.identifier);
            _generateExpression(simpleStmt->assignment.expression);
            _output(0, ";\n");
            break;
        case SIMPLE_DECLARATION: {
            if (simpleStmt->declaration) {
                _output(1, "%s %s", _getTypeName(simpleStmt->declaration->type), 
                    simpleStmt->declaration->identifier);
                
                if (simpleStmt->declaration->condition && 
                    simpleStmt->declaration->condition->type == COND_EXPRESSION) {
                    _output(0, " = ");
                    
                    _generateExpression(simpleStmt->declaration->condition->expression.expression);
                }
                
                _output(0, ";\n");
            } else {
                _output(1, "/* NULL declaration */;\n");
            }
            break;
        }
        case RETURN_CONSTANT:
            _output(1, "return ");
            switch (simpleStmt->constant->type) {
                case CONST_INTEGER:
                    _output(0, "%d", simpleStmt->constant->integer);
                    break;
                case CONST_FLOAT:
                    _output(0, "%f", simpleStmt->constant->floatVal);
                    break;
                case CONST_BOOLEAN:
                    _output(0, "%s", simpleStmt->constant->boolean ? "true" : "false");
                    break;
                case CONST_STRING:
                    _output(0, "\"%s\"", simpleStmt->constant->string);
                    break;
            }
            _output(0, ";\n");
            break;
        case RETURN_IDENTIFIER:
            _output(1, "return %s;\n", simpleStmt->identifier);
            break;
    }
}

/**
 * Generates a condition
 */
static void _generateCondition(Condition * condition) {
    if (condition == NULL) return;
    
    switch (condition->type) {
        case COND_RELATIONAL:
            _generateExpression(condition->leftValue);
            _output(0, " ");
            _generateRelationalOperator(condition->operator);
            _output(0, " ");
            _generateExpression(condition->rightValue);
            break;
        case COND_NOT:
            _output(0, "!(");
            _generateCondition(condition->not.condition);
            _output(0, ")");
            break;
        case COND_AND:
            _output(0, "(");
            _generateCondition(condition->logical.leftCondition);
            _output(0, " && ");
            _generateCondition(condition->logical.rightCondition);
            _output(0, ")");
            break;
        case COND_OR:
            _output(0, "(");
            _generateCondition(condition->logical.leftCondition);
            _output(0, " || ");
            _generateCondition(condition->logical.rightCondition);
            _output(0, ")");
            break;
        case COND_EMPTY:
            _output(0, "1");
            break;
        case COND_EXPRESSION:
            _generateExpression(condition->expression.expression);
            break;
    }
}

/**
 * Generates a relational operator
 */
static void _generateRelationalOperator(RelationalOperator * relOp) {
    if (relOp == NULL) return;
    
    switch (relOp->type) {
        case REL_EQUALS: _output(0, "=="); break;
        case REL_NOT_EQUALS: _output(0, "!="); break;
        case REL_LOWER_THAN: _output(0, "<"); break;
        case REL_GREATER_THAN: _output(0, ">"); break;
        case REL_LOWER_EQUALS: _output(0, "<="); break;
        case REL_GREATER_EQUALS: _output(0, ">="); break;
    }
}

/**
 * Generates an expression
 */
static void _generateExpression(Expression * expr) {
    if (expr == NULL) return;
    
    switch (expr->type) {
        case EXPR_CONSTANT:
            switch (expr->constant->type) {
                case CONST_INTEGER:
                    _output(0, "%d", expr->constant->integer);
                    break;
                case CONST_FLOAT:
                    _output(0, "%f", expr->constant->floatVal);
                    break;
                case CONST_BOOLEAN:
                    _output(0, "%s", expr->constant->boolean ? "true" : "false");
                    break;
                case CONST_STRING:
                    _output(0, "\"%s\"", expr->constant->string);
                    break;
            }
            break;
        case EXPR_IDENTIFIER:
            _output(0, "%s", expr->identifier);
            break;
        case EXPR_ADD:
            _output(0, "(");
            _generateExpression(expr->binary.leftExpression);
            _output(0, " + ");
            _generateExpression(expr->binary.rightExpression);
            _output(0, ")");
            break;
        case EXPR_SUB:
            _output(0, "(");
            _generateExpression(expr->binary.leftExpression);
            _output(0, " - ");
            _generateExpression(expr->binary.rightExpression);
            _output(0, ")");
            break;
        case EXPR_MUL:
            _output(0, "(");
            _generateExpression(expr->binary.leftExpression);
            _output(0, " * ");
            _generateExpression(expr->binary.rightExpression);
            _output(0, ")");
            break;
        case EXPR_DIV:
            _output(0, "(");
            _generateExpression(expr->binary.leftExpression);
            _output(0, " / ");
            _generateExpression(expr->binary.rightExpression);
            _output(0, ")");
            break;
        case EXPR_MOD:
            _output(0, "(");
            _generateExpression(expr->binary.leftExpression);
            _output(0, " %% ");
            _generateExpression(expr->binary.rightExpression);
            _output(0, ")");
            break;
        case EXPR_INCREMENT:
            _generateExpression(expr->unary.expression);
            _output(0, "++");
            break;
        case EXPR_DECREMENT:
            _generateExpression(expr->unary.expression);
            _output(0, "--");
            break;
        case EXPR_PRE_INCREMENT:
            _output(0, "++");
            _generateExpression(expr->unary.expression);
            break;
        case EXPR_PRE_DECREMENT:
            _output(0, "--");
            _generateExpression(expr->unary.expression);
            break;
        case EXPR_FUNCTION_CALL:
            if (expr->functionCall.functionName != NULL) {
                const char* builtinName = _getBuiltinFunctionName(expr->functionCall.functionName);
                if (strcmp(builtinName, expr->functionCall.functionName) == 0) {
                    _output(0, "%s(", expr->functionCall.functionName);
                } else {
                    _output(0, "%s(", builtinName);
                }
                
                if (expr->functionCall.arguments != NULL) {
                    _generateArgumentList(expr->functionCall.arguments);
                }
                _output(0, ")");
            } else {
                _output(0, "unknown_function()");
            }
            break;
    }
}

/**
 * Generates an argument list
 */
static void _generateArgumentList(ArgumentList * argList) {
    if (argList == NULL) return;
    
    _generateExpression(argList->expression);
    
    if (argList->next != NULL) {
        _output(0, ", ");
        _generateArgumentList(argList->next);
    }
}

/**
 * Generates an indentation string for the specified level.
 */
static char * _indentation(const unsigned int level) {
    return indentation(_indentationCharacter, level, _indentationSize);
}

/**
 * Outputs a formatted string to the output file.
 */
static void _output(const unsigned int indentationLevel, const char * const format, ...) {
    va_list arguments;
    va_start(arguments, format);
    char * indentationStr = _indentation(indentationLevel);
    char * effectiveFormat = concatenate(2, indentationStr, format);
    vfprintf(_outputFile, effectiveFormat, arguments);
    fflush(_outputFile);
    free(effectiveFormat);
    free(indentationStr);
    va_end(arguments);
}

/** PUBLIC FUNCTIONS */

void generate(CompilerState * compilerState) {
    logDebugging(_logger, "Generating C code...");
    
    if (compilerState->abstractSyntaxtTree != NULL) {
        _generateProgram(compilerState->abstractSyntaxtTree);
    }
    
    logDebugging(_logger, "Code generation completed.");
}
