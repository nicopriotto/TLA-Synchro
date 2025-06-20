#include "BisonActions.h"

/* MODULE INTERNAL STATE */
static Logger* _logger = NULL;

void initializeBisonActionsModule() {
    _logger = createLogger("BisonActions");
}

void shutdownBisonActionsModule() {
    if (_logger != NULL) {
        destroyLogger(_logger);
    }
    // freeSymbolTable(&currentCompilerState()->symbolTable);
    // freeScopeStack(&currentCompilerState()->scopeStack);
}

/** IMPORTED FUNCTIONS */
extern unsigned int flexCurrentContext(void);

/* PRIVATE FUNCTIONS */
static void _logSyntacticAnalyzerAction(const char* functionName);
static SymbolType convertTypeNodeToSymbolType(TypeNodeType typeNodeType);
static boolean isCompatibleType(SymbolType expected, SymbolType actual);

/**
 * Logs a syntactic-analyzer action in DEBUGGING level.
 */
static void _logSyntacticAnalyzerAction(const char* functionName) {
    logDebugging(_logger, "%s", functionName);
}

/**
 * Converts TypeNodeType to SymbolType
 */
static SymbolType convertTypeNodeToSymbolType(TypeNodeType typeNodeType) {
    switch (typeNodeType) {
        case TYPE_INTEGER: return SYMBOL_INTEGER;
        case TYPE_FLOAT: return SYMBOL_FLOAT;
        case TYPE_BOOLEAN: return SYMBOL_BOOLEAN;
        case TYPE_STRING: return SYMBOL_STRING;
        case TYPE_SEM: return SYMBOL_SEMAPHORE;
        default: return SYMBOL_INTEGER; // Default fallback
    }
}

/**
 * Checks if two types are compatible
 */
static boolean isCompatibleType(SymbolType expected, SymbolType actual) {
    if (expected == actual) return true;
    // Add type coercion rules if needed
    if ((expected == SYMBOL_FLOAT && actual == SYMBOL_INTEGER) ||
        (expected == SYMBOL_INTEGER && actual == SYMBOL_FLOAT)) {
        return true;
    }
    return false;
}

/* PUBLIC FUNCTIONS - Scope Management */
void enterScope() {
    pushScope(&currentCompilerState()->scopeStack);
    logDebugging(_logger, "Entered new scope: %d", currentScope(&currentCompilerState()->scopeStack));
}

void exitScope() {
    int scope = currentScope(&currentCompilerState()->scopeStack);
    logDebugging(_logger, "Exiting scope: %d", scope);
    popScope(&currentCompilerState()->scopeStack, &currentCompilerState()->symbolTable);
}

/* PUBLIC FUNCTIONS - Constants */
Constant* IntegerConstantSemanticAction(const int value) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    
    Constant* constant = calloc(1, sizeof(Constant));
    constant->integer = value;
    constant->type = CONST_INTEGER;
    
    return constant;
}

Constant* FloatConstantSemanticAction(const float value) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    
    Constant* constant = calloc(1, sizeof(Constant));
    constant->floatVal = value;
    constant->type = CONST_FLOAT;
    
    return constant;
}

Constant* BooleanConstantSemanticAction(const boolean value) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    
    Constant* constant = calloc(1, sizeof(Constant));
    constant->boolean = value;
    constant->type = CONST_BOOLEAN;
    
    return constant;
}

Constant* StringConstantSemanticAction(char* value) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    
    Constant* constant = calloc(1, sizeof(Constant));
    constant->string = value;
    constant->type = CONST_STRING;
    
    return constant;
}

/* PUBLIC FUNCTIONS - Expressions */
Expression* ConstantExpressionSemanticAction(Constant* constant) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    
    Expression* expression = calloc(1, sizeof(Expression));
    expression->constant = constant;
    expression->type = EXPR_CONSTANT;
    
    return expression;
}

Expression* IdentifierExpressionSemanticAction(char* identifier) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    
    // Check if identifier exists in symbol table
    SymbolEntry* entry = findSymbol(&currentCompilerState()->symbolTable, identifier, -1); // -1 for any scope
    if (!entry) {
        logError(_logger, "Undefined identifier: %s", identifier);
        // Continue parsing but mark error
    } else {
        logDebugging(_logger, "Found identifier %s in scope %d", identifier, entry->scope);
    }
    
    Expression* expression = calloc(1, sizeof(Expression));
    expression->identifier = identifier;
    expression->type = EXPR_IDENTIFIER;
    
    return expression;
}

Expression* BinaryExpressionSemanticAction(Expression* leftExpression, Expression* rightExpression, ExpressionType type) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    
    // TODO: Add type checking for binary operations
    
    Expression* expression = calloc(1, sizeof(Expression));
    expression->binary.leftExpression = leftExpression;
    expression->binary.rightExpression = rightExpression;
    expression->type = type;
    
    return expression;
}

Expression* UnaryExpressionSemanticAction(Expression* subExpression, ExpressionType type) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    
    Expression* expression = calloc(1, sizeof(Expression));
    expression->unary.expression = subExpression;
    expression->type = type;
    
    return expression;
}

Expression* FunctionCallExpressionSemanticAction(char* functionName, ArgumentList* arguments) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    
    // Check if function exists in symbol table
    SymbolEntry* entry = findSymbol(&currentCompilerState()->symbolTable, functionName, -1);
    if (!entry) {
        logError(_logger, "Undefined function: %s", functionName);
    } else if (entry->type != SYMBOL_FUNCTION) {
        logError(_logger, "Identifier %s is not a function", functionName);
    } else {
        logDebugging(_logger, "Found function %s", functionName);
        // TODO: Check argument count and types
    }
    
    Expression* expression = calloc(1, sizeof(Expression));
    expression->functionCall.functionName = functionName;
    expression->functionCall.arguments = arguments;
    expression->type = EXPR_FUNCTION_CALL;
    
    return expression;
}

/* PUBLIC FUNCTIONS - Conditions */
Condition* RelationalConditionSemanticAction(Expression* leftValue, RelationalOperator* operator, Expression* rightValue) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    
    Condition* condition = calloc(1, sizeof(Condition));
    condition->leftValue = leftValue;
    condition->operator = operator;
    condition->rightValue = rightValue;
    condition->type = COND_RELATIONAL;
    
    return condition;
}

Condition* NotConditionSemanticAction(Condition* subCondition) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    
    Condition* condition = calloc(1, sizeof(Condition));
    condition->not.condition = subCondition;
    condition->type = COND_NOT;
    
    return condition;
}

Condition* LogicalConditionSemanticAction(Condition* leftCondition, Condition* rightCondition, int logicalType) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    
    Condition* condition = calloc(1, sizeof(Condition));
    condition->logical.leftCondition = leftCondition;
    condition->logical.rightCondition = rightCondition;
    condition->type = logicalType;  
    
    return condition;
}

Condition* EmptyConditionSemanticAction() {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    
    Condition* condition = calloc(1, sizeof(Condition));
    condition->type = COND_EMPTY;
    
    return condition;
}

Condition* ExpressionAsConditionSemanticAction(Expression* expression){
    _logSyntacticAnalyzerAction(__FUNCTION__);

    Condition* condition = calloc(1, sizeof(Condition));
    condition->expression.expression = expression;
    condition->type = COND_EXPRESSION;
    
    return condition;
}

/* PUBLIC FUNCTIONS - Relational Operator */
RelationalOperator* RelationalOperatorSemanticAction(RelationalOperatorType type) {
    RelationalOperator* op = calloc(1, sizeof(RelationalOperator));
    if (!op) return NULL;  
    op->type = type;
    return op;
}

/* PUBLIC FUNCTIONS - Type Nodes */
TypeNode* TypeNodeSemanticAction(TypeNodeType type) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    
    TypeNode* typeNode = calloc(1, sizeof(TypeNode));
    typeNode->type = type;
    
    return typeNode;
}

/* PUBLIC FUNCTIONS - Declarations */
DeclarationTail* DeclarationSemanticAction(Constant* value) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    
    DeclarationTail* declarationTail = calloc(1, sizeof(DeclarationTail));
    declarationTail->constant = value;
    declarationTail->type = DECL_CONSTANT;
    
    return declarationTail;
}

DeclarationList* DeclarationListSemanticAction(TypeNode* type, char* identifier, DeclarationTail* declarationTail, DeclarationList* next) {
    _logSyntacticAnalyzerAction(__FUNCTION__);

    int currentScopeId = currentScope(&currentCompilerState()->scopeStack);
    SymbolType symbolType = convertTypeNodeToSymbolType(type->type);
    
    // Handle different declaration types
    if (declarationTail) {
        if (declarationTail->type == DECL_FUNCTION) {
            // Function declaration
            SymbolEntry* existing = findSymbol(&currentCompilerState()->symbolTable, identifier, currentScopeId);
            if (existing) {
                logError(_logger, "Function %s already declared in current scope", identifier);
            } else {
                if (insertSymbol(&currentCompilerState()->symbolTable, identifier, SYMBOL_FUNCTION, currentScopeId, &symbolType)) {
                    logDebugging(_logger, "Declared function %s with return type %d", identifier, symbolType);
                }
            }
        } else if (declarationTail->type == DECL_CONSTANT) {
            // Variable declaration with initialization
            SymbolEntry* existing = findSymbol(&currentCompilerState()->symbolTable, identifier, currentScopeId);
            if (existing) {
                logError(_logger, "Variable %s already declared in current scope", identifier);
            } else {
                void* initValue = NULL;
                switch (declarationTail->constant->type) {
                    case CONST_INTEGER:
                        initValue = &declarationTail->constant->integer;
                        break;
                    case CONST_FLOAT:
                        initValue = &declarationTail->constant->floatVal;
                        break;
                    case CONST_BOOLEAN:
                        initValue = &declarationTail->constant->boolean;
                        break;
                    case CONST_STRING:
                        initValue = declarationTail->constant->string;
                        break;
                }
                
                if (insertSymbol(&currentCompilerState()->symbolTable, identifier, symbolType, currentScopeId, initValue)) {
                    logDebugging(_logger, "Declared and initialized variable %s", identifier);
                }
            }
        }
    } else {
        // Simple variable declaration without initialization
        SymbolEntry* existing = findSymbol(&currentCompilerState()->symbolTable, identifier, currentScopeId);
        if (existing) {
            logError(_logger, "Variable %s already declared in current scope", identifier);
        } else {
            void* defaultValue = NULL;
            int intVal = 0;
            float floatVal = 0.0f;
            boolean boolVal = false;
            char* stringVal = "";
            
            switch (symbolType) {
                case SYMBOL_INTEGER:
                case SYMBOL_SEMAPHORE:
                    defaultValue = &intVal;
                    break;
                case SYMBOL_FLOAT:
                    defaultValue = &floatVal;
                    break;
                case SYMBOL_BOOLEAN:
                    defaultValue = &boolVal;
                    break;
                case SYMBOL_STRING:
                    defaultValue = stringVal;
                    break;
                case SYMBOL_FUNCTION:
                    break;
            }
            
            if (insertSymbol(&currentCompilerState()->symbolTable, identifier, symbolType, currentScopeId, defaultValue)) {
                logDebugging(_logger, "Declared variable %s of type %d", identifier, symbolType);
            }
        }
    }

    DeclarationList* declarationList = calloc(1, sizeof(DeclarationList));
    declarationList->type = type;
    declarationList->identifier = identifier;
    declarationList->declarationTail = declarationTail;
    declarationList->next = next;

    return declarationList;
}

/* PUBLIC FUNCTIONS - Variable Declarations */
VariableDeclaration* VariableDeclarationSemanticActionCondition(TypeNode* type, char* identifier, Condition* condition) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    
    int currentScopeId = currentScope(&currentCompilerState()->scopeStack);
    SymbolType symbolType = convertTypeNodeToSymbolType(type->type);
    
    // Check if identifier already exists in current scope
    SymbolEntry* existing = findSymbol(&currentCompilerState()->symbolTable, identifier, currentScopeId);
    if (existing) {
        logError(_logger, "Identifier %s already declared in current scope", identifier);
    } else {
        // Insert symbol with default value
        void* defaultValue = NULL;
        int intVal = 0;
        float floatVal = 0.0f;
        boolean boolVal = false;
        char* stringVal = "";
        
        switch (symbolType) {
            case SYMBOL_INTEGER:
            case SYMBOL_SEMAPHORE:
                defaultValue = &intVal;
                break;
            case SYMBOL_FLOAT:
                defaultValue = &floatVal;
                break;
            case SYMBOL_BOOLEAN:
                defaultValue = &boolVal;
                break;
            case SYMBOL_STRING:
                defaultValue = stringVal;
                break;
            case SYMBOL_FUNCTION:
                // Functions handled separately
                break;
        }
        
        if (insertSymbol(&currentCompilerState()->symbolTable, identifier, symbolType, currentScopeId, defaultValue)) {
            logDebugging(_logger, "Declared variable %s of type %d in scope %d", 
                        identifier, symbolType, currentScopeId);
        } else {
            logError(_logger, "Failed to insert symbol %s", identifier);
        }
    }
    
    VariableDeclaration* variableDeclaration = calloc(1, sizeof(VariableDeclaration));
    variableDeclaration->type = type;
    variableDeclaration->identifier = identifier;
    variableDeclaration->condition = condition;
    
    return variableDeclaration;
}

/* PUBLIC FUNCTIONS - Function Identifiers */
FunctionIdentifier* FunctionIdentifierSemanticAction(FunctionIdentifierType type, char* identifier) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    
    if (type == FUNC_USER_DEFINED && identifier) {
        // Check if function exists in symbol table
        SymbolEntry* entry = findSymbol(&currentCompilerState()->symbolTable, identifier, -1);
        if (!entry) {
            logError(_logger, "Undefined function: %s", identifier);
        } else if (entry->type != SYMBOL_FUNCTION) {
            logError(_logger, "Identifier %s is not a function", identifier);
        } else {
            logDebugging(_logger, "Found user-defined function %s", identifier);
        }
    }
    
    FunctionIdentifier* functionIdentifier = calloc(1, sizeof(FunctionIdentifier));
    functionIdentifier->type = type;
    
    if (type == FUNC_USER_DEFINED) {
        functionIdentifier->identifier = identifier;
    }
    
    return functionIdentifier;
}

/* PUBLIC FUNCTIONS - Parameters and Arguments */
ParameterList* ParameterListSemanticAction(TypeNode* type, char* identifier, ParameterList* nextParameters) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    
    ParameterList* parameterList = calloc(1, sizeof(ParameterList));
    parameterList->type = type;
    parameterList->identifier = identifier;
    parameterList->next = nextParameters;
    
    return parameterList;
}

ArgumentList* ArgumentListSemanticAction(Expression* expression, ArgumentList* nextArguments) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    
    ArgumentList* argumentList = calloc(1, sizeof(ArgumentList));
    argumentList->expression = expression;
    argumentList->next = nextArguments;
    
    return argumentList;
}

/* PUBLIC FUNCTIONS - For Loop Components */
ForInitializer* ForInitializerSemanticAction(VariableDeclaration* declaration, ForInitializer* nextInitializers) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    
    ForInitializer* forInitializer = calloc(1, sizeof(ForInitializer));
    forInitializer->declaration = declaration;
    forInitializer->next = nextInitializers;
    
    return forInitializer;
}

ForUpdate* ForUpdateSemanticAction(SimpleStatement* statement, ForUpdate* nextUpdates) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    
    ForUpdate* forUpdate = calloc(1, sizeof(ForUpdate));
    forUpdate->statement = statement;
    forUpdate->next = nextUpdates;
    
    return forUpdate;
}

/* PUBLIC FUNCTIONS - Simple Statements */
SimpleStatement* FunctionCallSimpleStatementSemanticAction(FunctionIdentifier* function, ArgumentList* arguments) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    
    if (function->type == FUNC_USER_DEFINED && function->identifier) {
        // Check if function exists in symbol table
        SymbolEntry* entry = findSymbol(&currentCompilerState()->symbolTable, function->identifier, -1);
        if (!entry) {
            logError(_logger, "Undefined function: %s", function->identifier);
        } else if (entry->type != SYMBOL_FUNCTION) {
            logError(_logger, "Identifier %s is not a function", function->identifier);
        } else {
            logDebugging(_logger, "Calling function %s", function->identifier);
            // TODO: Check argument count and types
        }
    }
    
    SimpleStatement* simpleStatement = calloc(1, sizeof(SimpleStatement));
    simpleStatement->functionCall.function = function;
    simpleStatement->functionCall.arguments = arguments;
    simpleStatement->type = SIMPLE_FUNCTION_CALL;
    
    return simpleStatement;
}

SimpleStatement* IncrementSimpleStatementSemanticAction(char* identifier, boolean isPrefix) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    
    // Check if identifier exists and is numeric
    SymbolEntry* entry = findSymbol(&currentCompilerState()->symbolTable, identifier, -1);
    if (!entry) {
        logError(_logger, "Increment of undefined identifier: %s", identifier);
    } else if (entry->type != SYMBOL_INTEGER && entry->type != SYMBOL_FLOAT) {
        logError(_logger, "Cannot increment non-numeric identifier: %s", identifier);
    } else {
        logDebugging(_logger, "Incrementing %s", identifier);
    }
    
    SimpleStatement* simpleStatement = calloc(1, sizeof(SimpleStatement));
    simpleStatement->increment.identifier = identifier;
    simpleStatement->increment.isPrefix = isPrefix;
    simpleStatement->type = SIMPLE_INCREMENT;
    
    return simpleStatement;
}

SimpleStatement* DecrementSimpleStatementSemanticAction(char* identifier, boolean isPrefix) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    
    // Check if identifier exists and is numeric
    SymbolEntry* entry = findSymbol(&currentCompilerState()->symbolTable, identifier, -1);
    if (!entry) {
        logError(_logger, "Decrement of undefined identifier: %s", identifier);
    } else if (entry->type != SYMBOL_INTEGER && entry->type != SYMBOL_FLOAT) {
        logError(_logger, "Cannot decrement non-numeric identifier: %s", identifier);
    } else {
        logDebugging(_logger, "Decrementing %s", identifier);
    }
    
    SimpleStatement* simpleStatement = calloc(1, sizeof(SimpleStatement));
    simpleStatement->decrement.identifier = identifier;
    simpleStatement->decrement.isPrefix = isPrefix;
    simpleStatement->type = SIMPLE_DECREMENT;
    
    return simpleStatement;
}

SimpleStatement* AssignmentSimpleStatementSemanticAction(char* identifier, Expression* expression) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    
    // Check if identifier exists
    SymbolEntry* entry = findSymbol(&currentCompilerState()->symbolTable, identifier, -1);
    if (!entry) {
        logError(_logger, "Assignment to undefined identifier: %s", identifier);
    } else {
        logDebugging(_logger, "Assignment to %s (type: %d)", identifier, entry->type);
        // TODO: Check type compatibility between identifier and expression
    }
    
    SimpleStatement* simpleStatement = calloc(1, sizeof(SimpleStatement));
    simpleStatement->assignment.identifier = identifier;
    simpleStatement->assignment.expression = expression;
    simpleStatement->type = SIMPLE_ASSIGNMENT;
    
    return simpleStatement;
}

SimpleStatement* DeclarationSimpleStatementSemanticAction(VariableDeclaration* declaration) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    
    SimpleStatement* simpleStatement = calloc(1, sizeof(SimpleStatement));
    simpleStatement->declaration = declaration;
    simpleStatement->type = SIMPLE_DECLARATION;
    
    return simpleStatement;
}

SimpleStatement* ReturnConstantSimpleStatementSemanticAction(Constant* constant) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    
    SimpleStatement* simpleStatement = calloc(1, sizeof(SimpleStatement));
    simpleStatement->constant = constant;
    simpleStatement->type = RETURN_CONSTANT;
    
    return simpleStatement;
}

SimpleStatement* ReturnIdentifierSimpleStatementSemanticAction(char* identifier) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    
    // Check if identifier exists
    SymbolEntry* entry = findSymbol(&currentCompilerState()->symbolTable, identifier, -1);
    if (!entry) {
        logError(_logger, "Return of undefined identifier: %s", identifier);
    } else {
        logDebugging(_logger, "Returning identifier %s", identifier);
    }
    
    SimpleStatement* simpleStatement = calloc(1, sizeof(SimpleStatement));
    simpleStatement->identifier = identifier;
    simpleStatement->type = RETURN_IDENTIFIER;
    
    return simpleStatement;
}

/* PUBLIC FUNCTIONS - Open Statements */
OpenStatement* IfOpenStatementSemanticAction(Condition* condition, Statement* thenStatement) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    
    OpenStatement* openStatement = calloc(1, sizeof(OpenStatement));
    openStatement->ifStatement.condition = condition;
    openStatement->ifStatement.thenStatement = thenStatement;
    openStatement->type = OPEN_IF;
    
    return openStatement;
}

OpenStatement* IfElseOpenStatementSemanticAction(Condition* condition, Statement* thenStatement, OpenStatement* elseStatement) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    
    OpenStatement* openStatement = calloc(1, sizeof(OpenStatement));
    openStatement->ifElseStatement.condition = condition;
    openStatement->ifElseStatement.thenStatement = thenStatement;
    openStatement->ifElseStatement.elseStatement = elseStatement;
    openStatement->type = OPEN_IF_ELSE;
    
    return openStatement;
}

OpenStatement* WhileOpenStatementSemanticAction(Condition* condition, OpenStatement* body) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    
    OpenStatement* openStatement = calloc(1, sizeof(OpenStatement));
    openStatement->whileStatement.condition = condition;
    openStatement->whileStatement.body = body;
    openStatement->type = OPEN_WHILE;
    
    return openStatement;
}

OpenStatement* ForOpenStatementSemanticAction(ForInitializer* initializer, Condition* condition, ForUpdate* update, OpenStatement* body) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    
    // Enter new scope for for-loop
    enterScope();
    
    OpenStatement* openStatement = calloc(1, sizeof(OpenStatement));
    openStatement->forStatement.initializer = initializer;
    openStatement->forStatement.condition = condition;
    openStatement->forStatement.update = update;
    openStatement->forStatement.body = body;
    openStatement->type = OPEN_FOR;
    
    // Exit for-loop scope
    exitScope();
    
    return openStatement;
}

/* PUBLIC FUNCTIONS - Closed Statements */
ClosedStatement* SimpleClosedStatementSemanticAction(SimpleStatement* simpleStatement) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    
    ClosedStatement* closedStatement = calloc(1, sizeof(ClosedStatement));
    closedStatement->simpleStatement = simpleStatement;
    closedStatement->type = CLOSED_SIMPLE;
    
    return closedStatement;
}

ClosedStatement* ClosedListStatementSemanticAction(StatementList* statementList){
    _logSyntacticAnalyzerAction(__FUNCTION__);
    
    // Enter new scope for block
    enterScope();
    
    ClosedStatement* closedStatement = calloc(1, sizeof(ClosedStatement));
    closedStatement->closedStatementList.statementList = statementList;
    closedStatement->type = CLOSED_STATEMENT_LIST;
    
    // Exit block scope
    exitScope();
    
    return closedStatement;
}

ClosedStatement* IfElseClosedStatementSemanticAction(Condition* condition, ClosedStatement* thenStatement, ClosedStatement* elseStatement) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    
    ClosedStatement* closedStatement = calloc(1, sizeof(ClosedStatement));
    closedStatement->ifElseStatement.condition = condition;
    closedStatement->ifElseStatement.thenStatement = thenStatement;
    closedStatement->ifElseStatement.elseStatement = elseStatement;
    closedStatement->type = CLOSED_IF_ELSE;
    
    return closedStatement;
}

ClosedStatement* WhileClosedStatementSemanticAction(Condition* condition, ClosedStatement* body) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    
    ClosedStatement* closedStatement = calloc(1, sizeof(ClosedStatement));
    closedStatement->whileStatement.condition = condition;
    closedStatement->whileStatement.body = body;
    closedStatement->type = CLOSED_WHILE;
    
    return closedStatement;
}

ClosedStatement* ForClosedStatementSemanticAction(ForInitializer* initializer, Condition* condition, ForUpdate* update, ClosedStatement* body) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    
    // Enter new scope for for-loop
    enterScope();
    
    ClosedStatement* closedStatement = calloc(1, sizeof(ClosedStatement));
    closedStatement->forStatement.initializer = initializer;
    closedStatement->forStatement.condition = condition;
    closedStatement->forStatement.update = update;
    closedStatement->forStatement.body = body;
    closedStatement->type = CLOSED_FOR;
    
    // Exit for-loop scope
    exitScope();
    
    return closedStatement;
}

ClosedStatement* ForeverClosedStatementSemanticAction(ClosedStatement* body) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    
    ClosedStatement* closedStatement = calloc(1, sizeof(ClosedStatement));
    closedStatement->foreverStatement.body = body;
    closedStatement->type = CLOSED_FOREVER;
    
    return closedStatement;
}

/* PUBLIC FUNCTIONS - Statements and Statement Lists */
Statement* OpenStatementSemanticAction(OpenStatement* openStatement) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    
    Statement* statement = calloc(1, sizeof(Statement));
    
    switch (openStatement->type) {
        case OPEN_IF:
            statement->type = STMT_IF;
            statement->ifStatement.condition = openStatement->ifStatement.condition;
            statement->ifStatement.thenStatement = openStatement->ifStatement.thenStatement;
            break;
        case OPEN_IF_ELSE:
            statement->type = STMT_IF_ELSE;
            statement->ifElseStatement.condition = openStatement->ifElseStatement.condition;
            statement->ifElseStatement.thenStatement = openStatement->ifElseStatement.thenStatement;
            statement->ifElseStatement.elseStatement = OpenStatementSemanticAction(openStatement->ifElseStatement.elseStatement);
            break;
        case OPEN_WHILE:
            statement->type = STMT_WHILE;
            statement->whileStatement.condition = openStatement->whileStatement.condition;
            statement->whileStatement.body = OpenStatementSemanticAction(openStatement->whileStatement.body);
            break;
        case OPEN_FOR:
            statement->type = STMT_FOR;
            statement->forStatement.initializer = openStatement->forStatement.initializer;
            statement->forStatement.condition = openStatement->forStatement.condition;
            statement->forStatement.update = openStatement->forStatement.update;
            statement->forStatement.body = OpenStatementSemanticAction(openStatement->forStatement.body);
            break;
    }
    
    free(openStatement); 
    
    return statement;
}

Statement* ClosedStatementSemanticAction(ClosedStatement* closedStatement) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    
    Statement* statement = calloc(1, sizeof(Statement));
    
    switch (closedStatement->type) {
        case CLOSED_SIMPLE:
            statement->type = STMT_SIMPLE;
            statement->simpleStatement = closedStatement->simpleStatement;
            break;
        case CLOSED_IF_ELSE:
            statement->type = STMT_IF_ELSE;
            statement->ifElseStatement.condition = closedStatement->ifElseStatement.condition;
            statement->ifElseStatement.thenStatement = ClosedStatementSemanticAction(closedStatement->ifElseStatement.thenStatement);
            statement->ifElseStatement.elseStatement = ClosedStatementSemanticAction(closedStatement->ifElseStatement.elseStatement);
            break;
        case CLOSED_WHILE:
            statement->type = STMT_WHILE;
            statement->whileStatement.condition = closedStatement->whileStatement.condition;
            statement->whileStatement.body = ClosedStatementSemanticAction(closedStatement->whileStatement.body);
            break;
        case CLOSED_FOR:
            statement->type = STMT_FOR;
            statement->forStatement.initializer = closedStatement->forStatement.initializer;
            statement->forStatement.condition = closedStatement->forStatement.condition;
            statement->forStatement.update = closedStatement->forStatement.update;
            statement->forStatement.body = ClosedStatementSemanticAction(closedStatement->forStatement.body);
            break;
        case CLOSED_FOREVER:
            statement->type = STMT_FOREVER;
            statement->foreverStatement.body = ClosedStatementSemanticAction(closedStatement->foreverStatement.body);
            break;
        case CLOSED_STATEMENT_LIST:
            statement->type = STMT_BLOCK;
            statement->blockStatement = closedStatement->closedStatementList.statementList;
            break;
    }
    
    free(closedStatement); 
    
    return statement;
}

StatementList* StatementListSemanticAction(Statement* statement, StatementList* nextStatements) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    
    StatementList* statementList = calloc(1, sizeof(StatementList));
    statementList->statement = statement;
    statementList->next = nextStatements;
    
    return statementList;
}

/* PUBLIC FUNCTIONS - Functions */
DeclarationTail* FunctionSemanticAction(ParameterList* parameters, StatementList* body) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    logDebugging(_logger, "exec FunctionSemanticAction");

    // Enter function scope for parameters and body
    enterScope();
    
    // Add parameters to symbol table
    ParameterList* param = parameters;
    while (param) {
        SymbolType paramType = convertTypeNodeToSymbolType(param->type->type);
        int currentScopeId = currentScope(&currentCompilerState()->scopeStack);
        
        // Check if parameter already exists in current scope
        SymbolEntry* existing = findSymbol(&currentCompilerState()->symbolTable, param->identifier, currentScopeId);
        if (existing) {
            logError(_logger, "Parameter %s already declared", param->identifier);
        } else {
            void* defaultValue = NULL;
            int intVal = 0;
            float floatVal = 0.0f;
            boolean boolVal = false;
            char* stringVal = "";
            
            switch (paramType) {
                case SYMBOL_INTEGER:
                case SYMBOL_SEMAPHORE:
                    defaultValue = &intVal;
                    break;
                case SYMBOL_FLOAT:
                    defaultValue = &floatVal;
                    break;
                case SYMBOL_BOOLEAN:
                    defaultValue = &boolVal;
                    break;
                case SYMBOL_STRING:
                    defaultValue = stringVal;
                    break;
                case SYMBOL_FUNCTION:
                    break;
            }
            
            if (insertSymbol(&currentCompilerState()->symbolTable, param->identifier, paramType, currentScopeId, defaultValue)) {
                logDebugging(_logger, "Added parameter %s to function scope", param->identifier);
            }
        }
        
        param = param->next;
    }

    DeclarationTail* declarationTail = calloc(1, sizeof(DeclarationTail));
    declarationTail->function.parameterList = parameters;
    declarationTail->function.statementList = body;
    declarationTail->type = DECL_FUNCTION;
    
    // Exit function scope
    exitScope();
    
    return declarationTail;
}

FunctionList* FunctionListSemanticAction(Function* function, FunctionList* nextFunctions) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    logDebugging(_logger, "exec FunctionListSemanticAction");
    
    FunctionList* functionList = calloc(1, sizeof(FunctionList));
    functionList->function = function;
    functionList->next = nextFunctions;
    
    return functionList;
}

/* PUBLIC FUNCTIONS - Program */
Program* ProgramSemanticAction(DeclarationList* globalDeclarations, CompilerState* compilerState) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    logDebugging(_logger, "exec ProgramSemanticAction");
    
    Program* program = calloc(1, sizeof(Program));
    program->globalDeclarations = globalDeclarations;
    compilerState->abstractSyntaxtTree = program;
    
    // Dump symbol table for debugging
    logDebugging(_logger, "Final symbol table:");
    dumpSymbolTable(&currentCompilerState()->symbolTable);
    
    if (0 < flexCurrentContext()) {
        logError(_logger, "The final context is not the default (0): %d", flexCurrentContext());
        compilerState->succeed = false;
    } else {
        compilerState->succeed = true;
    }

    if(!CheckTypeProgram(program)) {
        logError(_logger, "Type checking failed for the program.");
        compilerState->succeed = false;
    }
    
    return program;
}