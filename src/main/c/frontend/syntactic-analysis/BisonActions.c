#include "../../shared/semantic-analysis/TypeChecking.h"
#include "BisonActions.h"

/* MODULE INTERNAL STATE */
static Logger* _logger = NULL;

void initializeBisonActionsModule() {
    _logger = createLogger("BisonActions");
}

void shutdownBisonActionsModule() {
    if (_logger != NULL) {
        destroyLogger(_logger);
        _logger = NULL;
    }
}

/** IMPORTED FUNCTIONS */
extern unsigned int flexCurrentContext(void);

/* PRIVATE FUNCTIONS */
static void _logSyntacticAnalyzerAction(const char* functionName);
static SymbolType convertTypeNodeToSymbolType(TypeNodeType typeNodeType);
static boolean isCompatibleType(SymbolType expected, SymbolType actual);
static void preRegisterAllFunctions(DeclarationList* declarations);
static void preRegisterFunction(TypeNode* type, char* identifier, DeclarationTail* declarationTail);

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

/**
 * FIRST PASS: Pre-register all functions in the declaration list
 * This ensures all functions are known before we start type checking function bodies
 */
static void preRegisterAllFunctions(DeclarationList* declarations) {
    logDebugging(_logger, "=== FIRST PASS: Pre-registering all functions ===");
    
    DeclarationList* current = declarations;
    while (current) {
        if (current->declarationTail && current->declarationTail->type == DECL_FUNCTION) {
            preRegisterFunction(current->type, current->identifier, current->declarationTail);
        }
        current = current->next;
    }
    
    logDebugging(_logger, "=== FIRST PASS COMPLETE ===");
}

/**
 * Pre-register a single function in the symbol table
 */
static void preRegisterFunction(TypeNode* type, char* identifier, DeclarationTail* declarationTail) {
    if (!type || !identifier || !declarationTail) {
        logError(_logger, "Cannot pre-register function with NULL components");
        return;
    }
    
    CompilerState* state = currentCompilerState();
    if (!state) {
        logError(_logger, "No compiler state available for function pre-registration");
        return;
    }
    
    // Check if already exists
    SymbolEntry* existing = findSymbol(&state->symbolTable, identifier, 0);
    if (existing && existing->scope == 0) {
        // Update existing symbol to function type
        existing->type = SYMBOL_FUNCTION;
        logDebugging(_logger, "PRE-REGISTER: Updated %s to function (type %d) in scope 0", identifier, SYMBOL_FUNCTION);
    } else {
        // Insert new function symbol
        if (insertSymbol(&state->symbolTable, identifier, SYMBOL_FUNCTION, 0, NULL)) {
            logDebugging(_logger, "PRE-REGISTER: Registered function %s (type %d) in scope 0", identifier, SYMBOL_FUNCTION);
        } else {
            logError(_logger, "Failed to pre-register function %s", identifier);
        }
    }
}

/**
 * Immediately registers a symbol when we encounter the declaration header
 * (type + identifier) BEFORE parsing the declaration body. This enables
 * forward references to work properly in bottom-up parsing.
 */
void registerDeclarationHeader(TypeNode* type, char* identifier) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    
    if (!type || !identifier) {
        logError(_logger, "Cannot register declaration header with NULL components");
        return;
    }
    
    CompilerState* state = currentCompilerState();
    if (!state) {
        logError(_logger, "No compiler state available for declaration header registration");
        return;
    }
    
    // Use scope 0 for global declarations
    int targetScope = 0;
    SymbolType symbolType = convertTypeNodeToSymbolType(type->type);
    
    // Check if already exists (avoid duplicates)
    SymbolEntry* existing = findSymbol(&state->symbolTable, identifier, targetScope);
    if (existing && existing->scope == targetScope) {
        logDebugging(_logger, "Symbol %s already registered in header", identifier);
        return;
    }
    
    // Register with the variable type initially - will be updated to function if needed
    if (insertSymbol(&state->symbolTable, identifier, symbolType, targetScope, NULL)) {
        logDebugging(_logger, "HEADER: Forward declared %s with type %d in scope %d", identifier, symbolType, targetScope);
    } else {
        logError(_logger, "Failed to register declaration header for %s", identifier);
    }
}

/* PUBLIC FUNCTIONS - Scope Management */
void enterScope() {
    CompilerState* state = currentCompilerState();
    if (!state) {
        logError(_logger, "Cannot enter scope: no compiler state");
        return;
    }
    
    pushScope(&state->scopeStack);
    int currentScopeId = currentScope(&state->scopeStack);
    logDebugging(_logger, "Entered new scope: %d", currentScopeId);
}

void exitScope() {
    CompilerState* state = currentCompilerState();
    if (!state) {
        logError(_logger, "Cannot exit scope: no compiler state");
        return;
    }
    
    int scope = currentScope(&state->scopeStack);
    logDebugging(_logger, "Exiting scope: %d", scope);
    popScope(&state->scopeStack, &state->symbolTable);
}

/* PUBLIC FUNCTIONS - Grammar-level Scope Management */
void enterBlockScope() {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    enterScope();
}

void exitBlockScope() {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    exitScope();
}

/* PUBLIC FUNCTIONS - Constants */
Constant* IntegerConstantSemanticAction(const int value) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    
    Constant* constant = calloc(1, sizeof(Constant));
    if (!constant) {
        logError(_logger, "Memory allocation failed for integer constant");
        return NULL;
    }
    constant->integer = value;
    constant->type = CONST_INTEGER;
    
    return constant;
}

Constant* FloatConstantSemanticAction(const float value) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    
    Constant* constant = calloc(1, sizeof(Constant));
    if (!constant) {
        logError(_logger, "Memory allocation failed for float constant");
        return NULL;
    }
    constant->floatVal = value;
    constant->type = CONST_FLOAT;
    
    return constant;
}

Constant* BooleanConstantSemanticAction(const boolean value) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    
    Constant* constant = calloc(1, sizeof(Constant));
    if (!constant) {
        logError(_logger, "Memory allocation failed for boolean constant");
        return NULL;
    }
    constant->boolean = value;
    constant->type = CONST_BOOLEAN;
    
    return constant;
}

Constant* StringConstantSemanticAction(char* value) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    
    Constant* constant = calloc(1, sizeof(Constant));
    if (!constant) {
        logError(_logger, "Memory allocation failed for string constant");
        return NULL;
    }
    constant->string = value; // Take ownership of the string
    constant->type = CONST_STRING;
    
    return constant;
}

/* PUBLIC FUNCTIONS - Expressions */
Expression* ConstantExpressionSemanticAction(Constant* constant) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    
    if (!constant) {
        logError(_logger, "Cannot create expression from NULL constant");
        return NULL;
    }
    
    // IMMEDIATE TYPE CHECK
    if (!CheckTypeImmediate_Constant(constant)) {
        logError(_logger, "Type check failed for constant expression");
        CompilerState* state = currentCompilerState();
        if (state) state->succeed = false;
        return NULL;
    }
    
    Expression* expression = calloc(1, sizeof(Expression));
    if (!expression) {
        logError(_logger, "Memory allocation failed for constant expression");
        return NULL;
    }
    expression->constant = constant;
    expression->type = EXPR_CONSTANT;
    
    return expression;
}

Expression* IdentifierExpressionSemanticAction(char* identifier) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    
    if (!identifier) {
        logError(_logger, "Cannot create expression from NULL identifier");
        return NULL;
    }
    
    Expression* expression = calloc(1, sizeof(Expression));
    if (!expression) {
        logError(_logger, "Memory allocation failed for identifier expression");
        return NULL;
    }
    expression->identifier = identifier;
    expression->type = EXPR_IDENTIFIER;
    
    // IMMEDIATE TYPE CHECK
    if (!CheckTypeImmediate_Expression(expression)) {
        logError(_logger, "Type check failed for identifier expression: %s", identifier);
        CompilerState* state = currentCompilerState();
        if (state) state->succeed = false;
        releaseExpression(expression);
        return NULL;
    }
    
    return expression;
}

Expression* BinaryExpressionSemanticAction(Expression* leftExpression, Expression* rightExpression, ExpressionType type) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    
    if (!leftExpression || !rightExpression) {
        logError(_logger, "Cannot create binary expression with NULL operands");
        return NULL;
    }
    
    Expression* expression = calloc(1, sizeof(Expression));
    if (!expression) {
        logError(_logger, "Memory allocation failed for binary expression");
        return NULL;
    }
    expression->binary.leftExpression = leftExpression;
    expression->binary.rightExpression = rightExpression;
    expression->type = type;
    
    // IMMEDIATE TYPE CHECK
    if (!CheckTypeImmediate_Expression(expression)) {
        logError(_logger, "Type check failed for binary expression");
        CompilerState* state = currentCompilerState();
        if (state) state->succeed = false;
        releaseExpression(expression);
        return NULL;
    }
    
    return expression;
}

Expression* UnaryExpressionSemanticAction(Expression* subExpression, ExpressionType type) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    
    if (!subExpression) {
        logError(_logger, "Cannot create unary expression with NULL operand");
        return NULL;
    }
    
    Expression* expression = calloc(1, sizeof(Expression));
    if (!expression) {
        logError(_logger, "Memory allocation failed for unary expression");
        return NULL;
    }
    expression->unary.expression = subExpression;
    expression->type = type;
    
    return expression;
}

Expression* FunctionCallExpressionSemanticAction(char* functionName, ArgumentList* arguments) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    
    if (!functionName) {
        logError(_logger, "Cannot create function call expression with NULL function name");
        return NULL;
    }
    
    // **LENIENT APPROACH**: Don't do strict type checking here since functions
    // might not be fully registered yet. We'll do comprehensive type checking
    // in the second pass after all functions are known.
    CompilerState* state = currentCompilerState();
    if (state) {
        SymbolEntry* entry = findSymbol(&state->symbolTable, functionName, -1);
        if (entry) {
            logDebugging(_logger, "Found symbol %s with type %d", functionName, entry->type);
        } else {
            logDebugging(_logger, "Symbol %s not found, assuming forward declaration", functionName);
        }
    }
    
    Expression* expression = calloc(1, sizeof(Expression));
    if (!expression) {
        logError(_logger, "Memory allocation failed for function call expression");
        return NULL;
    }
    expression->functionCall.functionName = functionName; // Take ownership
    expression->functionCall.arguments = arguments;
    expression->type = EXPR_FUNCTION_CALL;
    
    return expression;
}

/* PUBLIC FUNCTIONS - Conditions */
Condition* RelationalConditionSemanticAction(Expression* leftValue, RelationalOperator* operator, Expression* rightValue) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    
    if (!leftValue || !operator || !rightValue) {
        logError(_logger, "Cannot create relational condition with NULL components");
        return NULL;
    }
    
    Condition* condition = calloc(1, sizeof(Condition));
    if (!condition) {
        logError(_logger, "Memory allocation failed for relational condition");
        return NULL;
    }
    condition->leftValue = leftValue;
    condition->operator = operator;
    condition->rightValue = rightValue;
    condition->type = COND_RELATIONAL;
    
    // IMMEDIATE TYPE CHECK
    if (!CheckTypeImmediate_Condition(condition)) {
        logError(_logger, "Type check failed for relational condition");
        CompilerState* state = currentCompilerState();
        if (state) state->succeed = false;
        releaseCondition(condition);
        return NULL;
    }
    
    return condition;
}

Condition* NotConditionSemanticAction(Condition* subCondition) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    
    if (!subCondition) {
        logError(_logger, "Cannot create NOT condition with NULL operand");
        return NULL;
    }
    
    Condition* condition = calloc(1, sizeof(Condition));
    if (!condition) {
        logError(_logger, "Memory allocation failed for NOT condition");
        return NULL;
    }
    condition->not.condition = subCondition;
    condition->type = COND_NOT;
    
    return condition;
}

Condition* LogicalConditionSemanticAction(Condition* leftCondition, Condition* rightCondition, int logicalType) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    
    if (!leftCondition || !rightCondition) {
        logError(_logger, "Cannot create logical condition with NULL operands");
        return NULL;
    }
    
    Condition* condition = calloc(1, sizeof(Condition));
    if (!condition) {
        logError(_logger, "Memory allocation failed for logical condition");
        return NULL;
    }
    condition->logical.leftCondition = leftCondition;
    condition->logical.rightCondition = rightCondition;
    condition->type = logicalType;  
    
    return condition;
}

Condition* EmptyConditionSemanticAction() {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    
    Condition* condition = calloc(1, sizeof(Condition));
    if (!condition) {
        logError(_logger, "Memory allocation failed for empty condition");
        return NULL;
    }
    condition->type = COND_EMPTY;
    
    return condition;
}

Condition* ExpressionAsConditionSemanticAction(Expression* expression){
    _logSyntacticAnalyzerAction(__FUNCTION__);

    if (!expression) {
        logError(_logger, "Cannot create condition from NULL expression");
        return NULL;
    }

    Condition* condition = calloc(1, sizeof(Condition));
    if (!condition) {
        logError(_logger, "Memory allocation failed for expression condition");
        return NULL;
    }
    condition->expression.expression = expression;
    condition->type = COND_EXPRESSION;
    
    return condition;
}

/* PUBLIC FUNCTIONS - Relational Operator */
RelationalOperator* RelationalOperatorSemanticAction(RelationalOperatorType type) {
    RelationalOperator* op = calloc(1, sizeof(RelationalOperator));
    if (!op) {
        logError(_logger, "Memory allocation failed for relational operator");
        return NULL;
    }
    op->type = type;
    return op;
}

/* PUBLIC FUNCTIONS - Type Nodes */
TypeNode* TypeNodeSemanticAction(TypeNodeType type) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    
    TypeNode* typeNode = calloc(1, sizeof(TypeNode));
    if (!typeNode) {
        logError(_logger, "Memory allocation failed for type node");
        return NULL;
    }
    typeNode->type = type;
    
    return typeNode;
}

/* PUBLIC FUNCTIONS - Declarations */
DeclarationTail* DeclarationSemanticAction(Constant* value) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    
    if (!value) {
        logError(_logger, "Cannot create declaration tail from NULL constant");
        return NULL;
    }
    
    DeclarationTail* declarationTail = calloc(1, sizeof(DeclarationTail));
    if (!declarationTail) {
        logError(_logger, "Memory allocation failed for declaration tail");
        return NULL;
    }
    declarationTail->constant = value;
    declarationTail->type = DECL_CONSTANT;
    
    return declarationTail;
}

DeclarationList* DeclarationListSemanticAction(TypeNode* type, char* identifier, DeclarationTail* declarationTail, DeclarationList* next) {
    _logSyntacticAnalyzerAction(__FUNCTION__);

    if (!type || !identifier) {
        logError(_logger, "Cannot create declaration with NULL type or identifier");
        return NULL;
    }

    // **SIMPLIFIED**: Just build the AST node, don't do symbol table management here
    // The symbol table will be managed by the two-pass approach in ProgramSemanticAction
    
    DeclarationList* declarationList = calloc(1, sizeof(DeclarationList));
    if (!declarationList) {
        logError(_logger, "Memory allocation failed for declaration list");
        return NULL;
    }
    declarationList->type = type;
    declarationList->identifier = identifier; // Take ownership
    declarationList->declarationTail = declarationTail;
    declarationList->next = next;

    return declarationList;
}

/* PUBLIC FUNCTIONS - Variable Declarations */
VariableDeclaration* VariableDeclarationSemanticActionCondition(TypeNode* type, char* identifier, Condition* condition) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    
    if (!type || !identifier || !condition) {
        logError(_logger, "Cannot create variable declaration with NULL components");
        return NULL;
    }
    
    // IMMEDIATE TYPE CHECK
    if (!CheckTypeImmediate_VariableDeclaration(type, identifier, condition)) {
        logError(_logger, "Type check failed for variable declaration '%s'", identifier);
        CompilerState* state = currentCompilerState();
        if (state) state->succeed = false;
        return NULL;
    }
    
    CompilerState* state = currentCompilerState();
    if (state) {
        // **PROPER SCOPE HANDLING** - Use current scope for local variables
        int currentScopeId = currentScope(&state->scopeStack);
        
        logDebugging(_logger, "Current scope for variable '%s': %d", identifier, currentScopeId);
        
        SymbolType symbolType = convertTypeNodeToSymbolType(type->type);
        
        // Check if identifier already exists in current scope
        SymbolEntry* existing = findSymbol(&state->symbolTable, identifier, currentScopeId);
        if (existing && existing->scope == currentScopeId) {
            logError(_logger, "Identifier %s already declared in current scope %d", identifier, currentScopeId);
        } else {
            if (insertSymbol(&state->symbolTable, identifier, symbolType, currentScopeId, NULL)) {
                logDebugging(_logger, "Declared variable %s of type %d in scope %d", 
                            identifier, symbolType, currentScopeId);
            } else {
                logError(_logger, "Failed to insert symbol %s", identifier);
            }
        }
    }
    
    VariableDeclaration* variableDeclaration = calloc(1, sizeof(VariableDeclaration));
    if (!variableDeclaration) {
        logError(_logger, "Memory allocation failed for variable declaration");
        return NULL;
    }
    variableDeclaration->type = type;
    variableDeclaration->identifier = identifier; // Take ownership
    variableDeclaration->condition = condition;
    
    return variableDeclaration;
}

/* PUBLIC FUNCTIONS - Function Identifiers */
FunctionIdentifier* FunctionIdentifierSemanticAction(FunctionIdentifierType type, char* identifier) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    
    // **LENIENT**: Don't do strict checking here, let the two-pass approach handle it
    if (type == FUNC_USER_DEFINED && identifier) {
        logDebugging(_logger, "Creating function identifier for user-defined function: %s", identifier);
    }
    
    FunctionIdentifier* functionIdentifier = calloc(1, sizeof(FunctionIdentifier));
    if (!functionIdentifier) {
        logError(_logger, "Memory allocation failed for function identifier");
        return NULL;
    }
    functionIdentifier->type = type;
    
    if (type == FUNC_USER_DEFINED) {
        functionIdentifier->identifier = identifier; // Take ownership
    }
    
    return functionIdentifier;
}

/* PUBLIC FUNCTIONS - Parameters and Arguments */
ParameterList* ParameterListSemanticAction(TypeNode* type, char* identifier, ParameterList* nextParameters) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    
    if (!type || !identifier) {
        logError(_logger, "Cannot create parameter list with NULL type or identifier");
        return NULL;
    }
    
    ParameterList* parameterList = calloc(1, sizeof(ParameterList));
    if (!parameterList) {
        logError(_logger, "Memory allocation failed for parameter list");
        return NULL;
    }
    parameterList->type = type;
    parameterList->identifier = identifier; // Take ownership
    parameterList->next = nextParameters;
    
    return parameterList;
}

ArgumentList* ArgumentListSemanticAction(Expression* expression, ArgumentList* nextArguments) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    
    if (!expression) {
        logError(_logger, "Cannot create argument list with NULL expression");
        return NULL;
    }
    
    ArgumentList* argumentList = calloc(1, sizeof(ArgumentList));
    if (!argumentList) {
        logError(_logger, "Memory allocation failed for argument list");
        return NULL;
    }
    argumentList->expression = expression;
    argumentList->next = nextArguments;
    
    return argumentList;
}

/* PUBLIC FUNCTIONS - For Loop Components */
ForInitializer* ForInitializerSemanticAction(VariableDeclaration* declaration, ForInitializer* nextInitializers) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    
    if (!declaration) {
        logError(_logger, "Cannot create for initializer with NULL declaration");
        return NULL;
    }
    
    ForInitializer* forInitializer = calloc(1, sizeof(ForInitializer));
    if (!forInitializer) {
        logError(_logger, "Memory allocation failed for for initializer");
        return NULL;
    }
    forInitializer->declaration = declaration;
    forInitializer->next = nextInitializers;
    
    return forInitializer;
}

ForUpdate* ForUpdateSemanticAction(SimpleStatement* statement, ForUpdate* nextUpdates) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    
    if (!statement) {
        logError(_logger, "Cannot create for update with NULL statement");
        return NULL;
    }
    
    ForUpdate* forUpdate = calloc(1, sizeof(ForUpdate));
    if (!forUpdate) {
        logError(_logger, "Memory allocation failed for for update");
        return NULL;
    }
    forUpdate->statement = statement;
    forUpdate->next = nextUpdates;
    
    return forUpdate;
}

/* PUBLIC FUNCTIONS - Simple Statements */
SimpleStatement* FunctionCallSimpleStatementSemanticAction(FunctionIdentifier* function, ArgumentList* arguments) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    
    if (!function) {
        logError(_logger, "Cannot create function call statement with NULL function");
        return NULL;
    }
    
    // **LENIENT**: Skip detailed type checking here, let the two-pass approach handle it
    logDebugging(_logger, "Creating function call statement (type checking deferred)");
    
    SimpleStatement* simpleStatement = calloc(1, sizeof(SimpleStatement));
    if (!simpleStatement) {
        logError(_logger, "Memory allocation failed for function call statement");
        return NULL;
    }
    simpleStatement->functionCall.function = function;
    simpleStatement->functionCall.arguments = arguments;
    simpleStatement->type = SIMPLE_FUNCTION_CALL;
    
    return simpleStatement;
}

SimpleStatement* IncrementSimpleStatementSemanticAction(char* identifier, boolean isPrefix) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    
    if (!identifier) {
        logError(_logger, "Cannot create increment statement with NULL identifier");
        return NULL;
    }
    
    // IMMEDIATE TYPE CHECK
    if (!CheckTypeImmediate_IncrementDecrement(identifier)) {
        logError(_logger, "Type check failed for increment of '%s'", identifier);
        CompilerState* state = currentCompilerState();
        if (state) state->succeed = false;
        return NULL;
    }
    
    SimpleStatement* simpleStatement = calloc(1, sizeof(SimpleStatement));
    if (!simpleStatement) {
        logError(_logger, "Memory allocation failed for increment statement");
        return NULL;
    }
    simpleStatement->increment.identifier = identifier; // Take ownership
    simpleStatement->increment.isPrefix = isPrefix;
    simpleStatement->type = SIMPLE_INCREMENT;
    
    return simpleStatement;
}

SimpleStatement* DecrementSimpleStatementSemanticAction(char* identifier, boolean isPrefix) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    
    if (!identifier) {
        logError(_logger, "Cannot create decrement statement with NULL identifier");
        return NULL;
    }
    
    // IMMEDIATE TYPE CHECK
    if (!CheckTypeImmediate_IncrementDecrement(identifier)) {
        logError(_logger, "Type check failed for decrement of '%s'", identifier);
        CompilerState* state = currentCompilerState();
        if (state) state->succeed = false;
        return NULL;
    }
    
    SimpleStatement* simpleStatement = calloc(1, sizeof(SimpleStatement));
    if (!simpleStatement) {
        logError(_logger, "Memory allocation failed for decrement statement");
        return NULL;
    }
    simpleStatement->decrement.identifier = identifier; // Take ownership
    simpleStatement->decrement.isPrefix = isPrefix;
    simpleStatement->type = SIMPLE_DECREMENT;
    
    return simpleStatement;
}

SimpleStatement* AssignmentSimpleStatementSemanticAction(char* identifier, Expression* expression) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    
    if (!identifier || !expression) {
        logError(_logger, "Cannot create assignment statement with NULL components");
        return NULL;
    }
    
    // IMMEDIATE TYPE CHECK
    if (!CheckTypeImmediate_Assignment(identifier, expression)) {
        logError(_logger, "Type check failed for assignment to '%s'", identifier);
        CompilerState* state = currentCompilerState();
        if (state) state->succeed = false;
        return NULL;
    }
    
    SimpleStatement* simpleStatement = calloc(1, sizeof(SimpleStatement));
    if (!simpleStatement) {
        logError(_logger, "Memory allocation failed for assignment statement");
        return NULL;
    }
    simpleStatement->assignment.identifier = identifier; // Take ownership
    simpleStatement->assignment.expression = expression;
    simpleStatement->type = SIMPLE_ASSIGNMENT;
    
    return simpleStatement;
}

SimpleStatement* DeclarationSimpleStatementSemanticAction(VariableDeclaration* declaration) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    
    if (!declaration) {
        logError(_logger, "Cannot create declaration statement with NULL declaration");
        return NULL;
    }
    
    SimpleStatement* simpleStatement = calloc(1, sizeof(SimpleStatement));
    if (!simpleStatement) {
        logError(_logger, "Memory allocation failed for declaration statement");
        return NULL;
    }
    simpleStatement->declaration = declaration;
    simpleStatement->type = SIMPLE_DECLARATION;
    
    return simpleStatement;
}

SimpleStatement* ReturnConstantSimpleStatementSemanticAction(Constant* constant) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    
    if (!constant) {
        logError(_logger, "Cannot create return statement with NULL constant");
        return NULL;
    }
    
    SimpleStatement* simpleStatement = calloc(1, sizeof(SimpleStatement));
    if (!simpleStatement) {
        logError(_logger, "Memory allocation failed for return constant statement");
        return NULL;
    }
    simpleStatement->constant = constant;
    simpleStatement->type = RETURN_CONSTANT;
    
    return simpleStatement;
}

SimpleStatement* ReturnIdentifierSimpleStatementSemanticAction(char* identifier) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    
    if (!identifier) {
        logError(_logger, "Cannot create return statement with NULL identifier");
        return NULL;
    }
    
    // Check if identifier exists
    CompilerState* state = currentCompilerState();
    if (state) {
        SymbolEntry* entry = findSymbol(&state->symbolTable, identifier, -1);
        if (!entry) {
            logError(_logger, "Return of undefined identifier: %s", identifier);
        } else {
            logDebugging(_logger, "Returning identifier %s", identifier);
        }
    }
    
    SimpleStatement* simpleStatement = calloc(1, sizeof(SimpleStatement));
    if (!simpleStatement) {
        logError(_logger, "Memory allocation failed for return identifier statement");
        return NULL;
    }
    simpleStatement->identifier = identifier; // Take ownership
    simpleStatement->type = RETURN_IDENTIFIER;
    
    return simpleStatement;
}

/* PUBLIC FUNCTIONS - Open Statements */
OpenStatement* IfOpenStatementSemanticAction(Condition* condition, Statement* thenStatement) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    
    if (!condition || !thenStatement) {
        logError(_logger, "Cannot create if statement with NULL components");
        return NULL;
    }
    
    OpenStatement* openStatement = calloc(1, sizeof(OpenStatement));
    if (!openStatement) {
        logError(_logger, "Memory allocation failed for if open statement");
        return NULL;
    }
    openStatement->ifStatement.condition = condition;
    openStatement->ifStatement.thenStatement = thenStatement;
    openStatement->type = OPEN_IF;
    
    return openStatement;
}

OpenStatement* IfElseOpenStatementSemanticAction(Condition* condition, Statement* thenStatement, OpenStatement* elseStatement) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    
    if (!condition || !thenStatement || !elseStatement) {
        logError(_logger, "Cannot create if-else statement with NULL components");
        return NULL;
    }
    
    OpenStatement* openStatement = calloc(1, sizeof(OpenStatement));
    if (!openStatement) {
        logError(_logger, "Memory allocation failed for if-else open statement");
        return NULL;
    }
    openStatement->ifElseStatement.condition = condition;
    openStatement->ifElseStatement.thenStatement = thenStatement;
    openStatement->ifElseStatement.elseStatement = elseStatement;
    openStatement->type = OPEN_IF_ELSE;
    
    return openStatement;
}

OpenStatement* WhileOpenStatementSemanticAction(Condition* condition, OpenStatement* body) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    
    if (!condition || !body) {
        logError(_logger, "Cannot create while statement with NULL components");
        return NULL;
    }
    
    OpenStatement* openStatement = calloc(1, sizeof(OpenStatement));
    if (!openStatement) {
        logError(_logger, "Memory allocation failed for while open statement");
        return NULL;
    }
    openStatement->whileStatement.condition = condition;
    openStatement->whileStatement.body = body;
    openStatement->type = OPEN_WHILE;
    
    return openStatement;
}

OpenStatement* ForOpenStatementSemanticAction(ForInitializer* initializer, Condition* condition, ForUpdate* update, OpenStatement* body) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    
    if (!condition || !body) {
        logError(_logger, "Cannot create for statement with NULL condition or body");
        return NULL;
    }
    
    // **NEW APPROACH**: Create a temporary scope for for-loop variables during type checking
    // This ensures that for-loop variables are available when checking the condition and body
    CompilerState* state = currentCompilerState();
    if (state && initializer) {
        // Enter a temporary scope for the for-loop
        enterScope();
        
        // Register for-loop variables in the temporary scope
        ForInitializer* init = initializer;
        while (init && init->declaration) {
            VariableDeclaration* decl = init->declaration;
            if (decl->type && decl->identifier) {
                SymbolType symbolType = convertTypeNodeToSymbolType(decl->type->type);
                int currentScopeId = currentScope(&state->scopeStack);
                
                if (insertSymbol(&state->symbolTable, decl->identifier, symbolType, currentScopeId, NULL)) {
                    logDebugging(_logger, "Registered for-loop variable %s in temporary scope %d", decl->identifier, currentScopeId);
                }
            }
            init = init->next;
        }
        
        // Type check the condition and body with for-loop variables in scope
        if (condition && !CheckTypeImmediate_Condition(condition)) {
            logError(_logger, "For-loop condition type check failed");
            state->succeed = false;
        }
        
        // Exit the temporary scope
        exitScope();
    }
    
    OpenStatement* openStatement = calloc(1, sizeof(OpenStatement));
    if (!openStatement) {
        logError(_logger, "Memory allocation failed for for open statement");
        return NULL;
    }
    openStatement->forStatement.initializer = initializer;
    openStatement->forStatement.condition = condition;
    openStatement->forStatement.update = update;
    openStatement->forStatement.body = body;
    openStatement->type = OPEN_FOR;
    
    return openStatement;
}

/* PUBLIC FUNCTIONS - Closed Statements */
ClosedStatement* SimpleClosedStatementSemanticAction(SimpleStatement* simpleStatement) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    
    if (!simpleStatement) {
        logError(_logger, "Cannot create closed statement with NULL simple statement");
        return NULL;
    }
    
    ClosedStatement* closedStatement = calloc(1, sizeof(ClosedStatement));
    if (!closedStatement) {
        logError(_logger, "Memory allocation failed for simple closed statement");
        return NULL;
    }
    closedStatement->simpleStatement = simpleStatement;
    closedStatement->type = CLOSED_SIMPLE;
    
    return closedStatement;
}

ClosedStatement* ClosedListStatementSemanticAction(StatementList* statementList){
    _logSyntacticAnalyzerAction(__FUNCTION__);
    
    if (!statementList) {
        logError(_logger, "Cannot create closed statement with NULL statement list");
        return NULL;
    }
    
    // NOTE: Scope should already be entered when parsing the block
    // This function is called after statements are parsed, so we don't enter scope here
    
    ClosedStatement* closedStatement = calloc(1, sizeof(ClosedStatement));
    if (!closedStatement) {
        logError(_logger, "Memory allocation failed for closed list statement");
        return NULL;
    }
    closedStatement->closedStatementList.statementList = statementList;
    closedStatement->type = CLOSED_STATEMENT_LIST;
    
    return closedStatement;
}

ClosedStatement* IfElseClosedStatementSemanticAction(Condition* condition, ClosedStatement* thenStatement, ClosedStatement* elseStatement) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    
    if (!condition || !thenStatement || !elseStatement) {
        logError(_logger, "Cannot create if-else closed statement with NULL components");
        return NULL;
    }
    
    ClosedStatement* closedStatement = calloc(1, sizeof(ClosedStatement));
    if (!closedStatement) {
        logError(_logger, "Memory allocation failed for if-else closed statement");
        return NULL;
    }
    closedStatement->ifElseStatement.condition = condition;
    closedStatement->ifElseStatement.thenStatement = thenStatement;
    closedStatement->ifElseStatement.elseStatement = elseStatement;
    closedStatement->type = CLOSED_IF_ELSE;
    
    return closedStatement;
}

ClosedStatement* WhileClosedStatementSemanticAction(Condition* condition, ClosedStatement* body) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    
    if (!condition || !body) {
        logError(_logger, "Cannot create while closed statement with NULL components");
        return NULL;
    }
    
    ClosedStatement* closedStatement = calloc(1, sizeof(ClosedStatement));
    if (!closedStatement) {
        logError(_logger, "Memory allocation failed for while closed statement");
        return NULL;
    }
    closedStatement->whileStatement.condition = condition;
    closedStatement->whileStatement.body = body;
    closedStatement->type = CLOSED_WHILE;
    
    return closedStatement;
}

ClosedStatement* ForClosedStatementSemanticAction(ForInitializer* initializer, Condition* condition, ForUpdate* update, ClosedStatement* body) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    
    if (!condition || !body) {
        logError(_logger, "Cannot create for closed statement with NULL condition or body");
        return NULL;
    }
    
    // **NEW APPROACH**: Create a temporary scope for for-loop variables during type checking
    CompilerState* state = currentCompilerState();
    if (state && initializer) {
        // Enter a temporary scope for the for-loop
        enterScope();
        
        // Register for-loop variables in the temporary scope
        ForInitializer* init = initializer;
        while (init && init->declaration) {
            VariableDeclaration* decl = init->declaration;
            if (decl->type && decl->identifier) {
                SymbolType symbolType = convertTypeNodeToSymbolType(decl->type->type);
                int currentScopeId = currentScope(&state->scopeStack);
                
                if (insertSymbol(&state->symbolTable, decl->identifier, symbolType, currentScopeId, NULL)) {
                    logDebugging(_logger, "Registered for-loop variable %s in temporary scope %d", decl->identifier, currentScopeId);
                }
            }
            init = init->next;
        }
        
        // Type check the condition with for-loop variables in scope
        if (condition && !CheckTypeImmediate_Condition(condition)) {
            logError(_logger, "For-loop condition type check failed");
            state->succeed = false;
        }
        
        // Exit the temporary scope
        exitScope();
    }
    
    ClosedStatement* closedStatement = calloc(1, sizeof(ClosedStatement));
    if (!closedStatement) {
        logError(_logger, "Memory allocation failed for for closed statement");
        return NULL;
    }
    closedStatement->forStatement.initializer = initializer;
    closedStatement->forStatement.condition = condition;
    closedStatement->forStatement.update = update;
    closedStatement->forStatement.body = body;
    closedStatement->type = CLOSED_FOR;
    
    return closedStatement;
}

ClosedStatement* ForeverClosedStatementSemanticAction(ClosedStatement* body) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    
    if (!body) {
        logError(_logger, "Cannot create forever statement with NULL body");
        return NULL;
    }
    
    ClosedStatement* closedStatement = calloc(1, sizeof(ClosedStatement));
    if (!closedStatement) {
        logError(_logger, "Memory allocation failed for forever closed statement");
        return NULL;
    }
    closedStatement->foreverStatement.body = body;
    closedStatement->type = CLOSED_FOREVER;
    
    return closedStatement;
}

/* PUBLIC FUNCTIONS - Statements and Statement Lists */
Statement* OpenStatementSemanticAction(OpenStatement* openStatement) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    
    if (!openStatement) {
        logError(_logger, "Cannot create statement from NULL open statement");
        return NULL;
    }
    
    Statement* statement = calloc(1, sizeof(Statement));
    if (!statement) {
        logError(_logger, "Memory allocation failed for statement from open statement");
        return NULL;
    }
    
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
    
    if (!closedStatement) {
        logError(_logger, "Cannot create statement from NULL closed statement");
        return NULL;
    }
    
    Statement* statement = calloc(1, sizeof(Statement));
    if (!statement) {
        logError(_logger, "Memory allocation failed for statement from closed statement");
        return NULL;
    }
    
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
    
    if (!statement) {
        logError(_logger, "Cannot create statement list with NULL statement");
        return NULL;
    }
    
    StatementList* statementList = calloc(1, sizeof(StatementList));
    if (!statementList) {
        logError(_logger, "Memory allocation failed for statement list");
        return NULL;
    }
    statementList->statement = statement;
    statementList->next = nextStatements;
    
    return statementList;
}

/* PUBLIC FUNCTIONS - Functions */
DeclarationTail* FunctionSemanticAction(ParameterList* parameters, StatementList* body) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    logDebugging(_logger, "Creating function declaration");

    // Add parameters to symbol table (scope already entered by grammar)
    CompilerState* state = currentCompilerState();
    if (state) {
        ParameterList* param = parameters;
        while (param) {
            SymbolType paramType = convertTypeNodeToSymbolType(param->type->type);
            int currentScopeId = currentScope(&state->scopeStack);
            
            // Check if parameter already exists in current scope
            SymbolEntry* existing = findSymbol(&state->symbolTable, param->identifier, currentScopeId);
            if (existing && existing->scope == currentScopeId) {
                logError(_logger, "Parameter %s already declared", param->identifier);
            } else {
                if (insertSymbol(&state->symbolTable, param->identifier, paramType, currentScopeId, NULL)) {
                    logDebugging(_logger, "Added parameter %s to function scope %d", param->identifier, currentScopeId);
                }
            }
            
            param = param->next;
        }
    }

    DeclarationTail* declarationTail = calloc(1, sizeof(DeclarationTail));
    if (!declarationTail) {
        logError(_logger, "Memory allocation failed for function declaration tail");
        return NULL;
    }
    declarationTail->function.parameterList = parameters;
    declarationTail->function.statementList = body;
    declarationTail->type = DECL_FUNCTION;
    
    return declarationTail;
}

FunctionList* FunctionListSemanticAction(Function* function, FunctionList* nextFunctions) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    logDebugging(_logger, "Creating function list");
    
    if (!function) {
        logError(_logger, "Cannot create function list with NULL function");
        return NULL;
    }
    
    FunctionList* functionList = calloc(1, sizeof(FunctionList));
    if (!functionList) {
        logError(_logger, "Memory allocation failed for function list");
        return NULL;
    }
    functionList->function = function;
    functionList->next = nextFunctions;
    
    return functionList;
}

/* PUBLIC FUNCTIONS - Program */
Program* ProgramSemanticAction(DeclarationList* globalDeclarations, CompilerState* compilerState) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    logDebugging(_logger, "Creating program");
    
    if (!compilerState) {
        logError(_logger, "Cannot create program with NULL compiler state");
        return NULL;
    }
    
    // **TWO-PASS APPROACH**: 
    // FIRST PASS: Pre-register all functions before doing any type checking
    if (globalDeclarations) {
        preRegisterAllFunctions(globalDeclarations);
        
        // Dump symbol table after first pass
        logDebugging(_logger, "Symbol table after FIRST PASS (function pre-registration):");
        dumpSymbolTable(&compilerState->symbolTable);
    }
    
    // SECOND PASS: Now all functions are known, so type checking should work correctly
    // The type checking has already been done during parsing, but now with all functions registered
    logDebugging(_logger, "=== SECOND PASS: Type checking with complete function knowledge ===");
    
    Program* program = calloc(1, sizeof(Program));
    if (!program) {
        logError(_logger, "Memory allocation failed for program");
        return NULL;
    }
    program->globalDeclarations = globalDeclarations;
    compilerState->abstractSyntaxtTree = program;
    
    // Dump symbol table for debugging
    logDebugging(_logger, "Symbol table at program completion:");
    dumpSymbolTable(&compilerState->symbolTable);
    
    // **FIX**: Check if any errors occurred during compilation
    if (compilerState->succeed == false) {
        logError(_logger, "Program compilation failed due to earlier errors.");
    } else {
        logDebugging(_logger, "Program compilation completed successfully.");
    }

    // Set final success state
    compilerState->succeed = compilerState->succeed;
    
    // Clean up any remaining local scopes (keep global and builtin)
    removeScopesAbove(&compilerState->symbolTable, 0);
    
    // Dump final symbol table
    logDebugging(_logger, "Final symbol table after cleanup:");
    dumpSymbolTable(&compilerState->symbolTable);
    
    if (0 < flexCurrentContext()) {
        logError(_logger, "The final context is not the default (0): %d", flexCurrentContext());
        compilerState->succeed = false;
    }
    
    return program;
}
