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
}

/** IMPORTED FUNCTIONS */
extern unsigned int flexCurrentContext(void);

/* PRIVATE FUNCTIONS */
static void _logSyntacticAnalyzerAction(const char* functionName);

/**
 * Logs a syntactic-analyzer action in DEBUGGING level.
 */
static void _logSyntacticAnalyzerAction(const char* functionName) {
    logDebugging(_logger, "%s", functionName);
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
    
    Expression* expression = calloc(1, sizeof(Expression));
    expression->identifier = identifier;
    expression->type = EXPR_IDENTIFIER;
    
    return expression;
}

Expression* BinaryExpressionSemanticAction(Expression* leftExpression, Expression* rightExpression, ExpressionType type) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    
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

Condition* ParenthesisConditionSemanticAction(Condition* subCondition) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    
    Condition* condition = calloc(1, sizeof(Condition));
    condition->parenthesis.condition = subCondition;
    condition->type = COND_PARENTHESIS;
    
    return condition;
}

Condition* EmptyConditionSemanticAction() {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    
    Condition* condition = calloc(1, sizeof(Condition));
    condition->type = COND_EMPTY;
    
    return condition;
}

/* PUBLIC FUNCTIONS - Relational Operator */

RelationalOperator* RelationalOperatorSemanticAction(RelationalOperatorType type) {
    RelationalOperator* op = malloc(sizeof(RelationalOperator));
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
    
    VariableDeclaration* variableDeclaration = calloc(1, sizeof(VariableDeclaration));
    variableDeclaration->type = type;
    variableDeclaration->identifier = identifier;
    variableDeclaration->condition = condition;
    variableDeclaration->uniontype = CONDITION;
    
    return variableDeclaration;
}

VariableDeclaration* VariableDeclarationSemanticActionExpression(TypeNode* type, char* identifier, Expression* expression) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    
    VariableDeclaration* variableDeclaration = calloc(1, sizeof(VariableDeclaration));
    variableDeclaration->type = type;
    variableDeclaration->identifier = identifier;
    variableDeclaration->expression = expression;
    variableDeclaration->uniontype = EXPRESSION;

    return variableDeclaration;
}

/* PUBLIC FUNCTIONS - Function Identifiers */
FunctionIdentifier* FunctionIdentifierSemanticAction(FunctionIdentifierType type, char* identifier) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    
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
    
    SimpleStatement* simpleStatement = calloc(1, sizeof(SimpleStatement));
    simpleStatement->functionCall.function = function;
    simpleStatement->functionCall.arguments = arguments;
    simpleStatement->type = SIMPLE_FUNCTION_CALL;
    
    return simpleStatement;
}

SimpleStatement* IncrementSimpleStatementSemanticAction(char* identifier, boolean isPrefix) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    
    SimpleStatement* simpleStatement = calloc(1, sizeof(SimpleStatement));
    simpleStatement->increment.identifier = identifier;
    simpleStatement->increment.isPrefix = isPrefix;
    simpleStatement->type = SIMPLE_INCREMENT;
    
    return simpleStatement;
}

SimpleStatement* DecrementSimpleStatementSemanticAction(char* identifier, boolean isPrefix) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    
    SimpleStatement* simpleStatement = calloc(1, sizeof(SimpleStatement));
    simpleStatement->decrement.identifier = identifier;
    simpleStatement->decrement.isPrefix = isPrefix;
    simpleStatement->type = SIMPLE_DECREMENT;
    
    return simpleStatement;
}

SimpleStatement* AssignmentSimpleStatementSemanticAction(char* identifier, Expression* expression) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    
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

/* PUBLIC FUNCTIONS - Open Statements */
OpenStatement* IfOpenStatementSemanticAction(Expression* condition, Statement* thenStatement) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    
    OpenStatement* openStatement = calloc(1, sizeof(OpenStatement));
    openStatement->ifStatement.condition = condition;
    openStatement->ifStatement.thenStatement = thenStatement;
    openStatement->type = OPEN_IF;
    
    return openStatement;
}

OpenStatement* IfElseOpenStatementSemanticAction(Expression* condition, Statement* thenStatement, OpenStatement* elseStatement) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    
    OpenStatement* openStatement = calloc(1, sizeof(OpenStatement));
    openStatement->ifElseStatement.condition = condition;
    openStatement->ifElseStatement.thenStatement = thenStatement;
    openStatement->ifElseStatement.elseStatement = elseStatement;
    openStatement->type = OPEN_IF_ELSE;
    
    return openStatement;
}

OpenStatement* WhileOpenStatementSemanticAction(Expression* condition, OpenStatement* body) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    
    OpenStatement* openStatement = calloc(1, sizeof(OpenStatement));
    openStatement->whileStatement.condition = condition;
    openStatement->whileStatement.body = body;
    openStatement->type = OPEN_WHILE;
    
    return openStatement;
}

OpenStatement* ForOpenStatementSemanticAction(ForInitializer* initializer, Condition* condition, ForUpdate* update, OpenStatement* body) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    
    OpenStatement* openStatement = calloc(1, sizeof(OpenStatement));
    openStatement->forStatement.initializer = initializer;
    openStatement->forStatement.condition = condition;
    openStatement->forStatement.update = update;
    openStatement->forStatement.body = body;
    openStatement->type = OPEN_FOR;
    
    return openStatement;
}

OpenStatement* ForeverOpenStatementSemanticAction(OpenStatement* body) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    
    OpenStatement* openStatement = calloc(1, sizeof(OpenStatement));
    openStatement->foreverStatement.body = body;
    openStatement->type = OPEN_FOREVER;
    
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

ClosedStatement* IfElseClosedStatementSemanticAction(Expression* condition, ClosedStatement* thenStatement, ClosedStatement* elseStatement) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    
    ClosedStatement* closedStatement = calloc(1, sizeof(ClosedStatement));
    closedStatement->ifElseStatement.condition = condition;
    closedStatement->ifElseStatement.thenStatement = thenStatement;
    closedStatement->ifElseStatement.elseStatement = elseStatement;
    closedStatement->type = CLOSED_IF_ELSE;
    
    return closedStatement;
}

ClosedStatement* IfBlockClosedStatementSemanticAction(Expression* condition, StatementList* body) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    
    ClosedStatement* closedStatement = calloc(1, sizeof(ClosedStatement));
    closedStatement->ifBlock.condition = condition;
    closedStatement->ifBlock.body = body;
    closedStatement->type = CLOSED_IF_BLOCK;
    
    return closedStatement;
}

ClosedStatement* WhileClosedStatementSemanticAction(Expression* condition, ClosedStatement* body) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    
    ClosedStatement* closedStatement = calloc(1, sizeof(ClosedStatement));
    closedStatement->whileStatement.condition = condition;
    closedStatement->whileStatement.body = body;
    closedStatement->type = CLOSED_WHILE;
    
    return closedStatement;
}

ClosedStatement* WhileBlockClosedStatementSemanticAction(Expression* condition, StatementList* body) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    
    ClosedStatement* closedStatement = calloc(1, sizeof(ClosedStatement));
    closedStatement->whileBlock.condition = condition;
    closedStatement->whileBlock.body = body;
    closedStatement->type = CLOSED_WHILE_BLOCK;
    
    return closedStatement;
}

ClosedStatement* ForClosedStatementSemanticAction(ForInitializer* initializer, Condition* condition, ForUpdate* update, ClosedStatement* body) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    
    ClosedStatement* closedStatement = calloc(1, sizeof(ClosedStatement));
    closedStatement->forStatement.initializer = initializer;
    closedStatement->forStatement.condition = condition;
    closedStatement->forStatement.update = update;
    closedStatement->forStatement.body = body;
    closedStatement->type = CLOSED_FOR;
    
    return closedStatement;
}

ClosedStatement* ForBlockClosedStatementSemanticAction(ForInitializer* initializer, Condition* condition, ForUpdate* update, StatementList* body) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    
    ClosedStatement* closedStatement = calloc(1, sizeof(ClosedStatement));
    closedStatement->forBlock.initializer = initializer;
    closedStatement->forBlock.condition = condition;
    closedStatement->forBlock.update = update;
    closedStatement->forBlock.body = body;
    closedStatement->type = CLOSED_FOR_BLOCK;
    
    return closedStatement;
}

ClosedStatement* ForeverClosedStatementSemanticAction(ClosedStatement* body) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    
    ClosedStatement* closedStatement = calloc(1, sizeof(ClosedStatement));
    closedStatement->foreverStatement.body = body;
    closedStatement->type = CLOSED_FOREVER;
    
    return closedStatement;
}

ClosedStatement* ForeverBlockClosedStatementSemanticAction(StatementList* body) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    
    ClosedStatement* closedStatement = calloc(1, sizeof(ClosedStatement));
    closedStatement->foreverBlock.body = body;
    closedStatement->type = CLOSED_FOREVER_BLOCK;
    
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
        case OPEN_FOREVER:
            statement->type = STMT_FOREVER;
            statement->foreverStatement.body = OpenStatementSemanticAction(openStatement->foreverStatement.body);
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
        case CLOSED_IF_BLOCK:
            statement->type = STMT_IF;
            statement->ifStatement.condition = closedStatement->ifBlock.condition;
            statement->ifStatement.thenStatement = calloc(1, sizeof(Statement));
            statement->ifStatement.thenStatement->type = STMT_SIMPLE; 
            statement->ifStatement.thenStatement->blockStatement = closedStatement->ifBlock.body;
            break;
        case CLOSED_WHILE:
            statement->type = STMT_WHILE;
            statement->whileStatement.condition = closedStatement->whileStatement.condition;
            statement->whileStatement.body = ClosedStatementSemanticAction(closedStatement->whileStatement.body);
            break;
        case CLOSED_WHILE_BLOCK:
            statement->type = STMT_WHILE;
            statement->whileStatement.condition = closedStatement->whileBlock.condition;
            statement->whileStatement.body = calloc(1, sizeof(Statement));
            statement->whileStatement.body->type = STMT_SIMPLE; 
            statement->whileStatement.body->blockStatement = closedStatement->whileBlock.body;
            break;
        case CLOSED_FOR:
            statement->type = STMT_FOR;
            statement->forStatement.initializer = closedStatement->forStatement.initializer;
            statement->forStatement.condition = closedStatement->forStatement.condition;
            statement->forStatement.update = closedStatement->forStatement.update;
            statement->forStatement.body = ClosedStatementSemanticAction(closedStatement->forStatement.body);
            break;
        case CLOSED_FOR_BLOCK:
            statement->type = STMT_FOR;
            statement->forStatement.initializer = closedStatement->forBlock.initializer;
            statement->forStatement.condition = closedStatement->forBlock.condition;
            statement->forStatement.update = closedStatement->forBlock.update;
            statement->forStatement.body = calloc(1, sizeof(Statement));
            statement->forStatement.body->type = STMT_SIMPLE; 
            statement->forStatement.body->blockStatement = closedStatement->forBlock.body;
            break;
        case CLOSED_FOREVER:
            statement->type = STMT_FOREVER;
            statement->foreverStatement.body = ClosedStatementSemanticAction(closedStatement->foreverStatement.body);
            break;
        case CLOSED_FOREVER_BLOCK:
            statement->type = STMT_FOREVER;
            statement->foreverStatement.body = calloc(1, sizeof(Statement));
            statement->foreverStatement.body->type = STMT_SIMPLE; 
            statement->foreverStatement.body->blockStatement = closedStatement->foreverBlock.body;
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
    logInformation(_logger, "exec FunctionListSemanticAction");

    DeclarationTail* declarationTail = calloc(1, sizeof(DeclarationTail));
    declarationTail->function.parameterList = parameters;
    declarationTail->function.statementList = body;
    declarationTail->type = DECL_FUNCTION;
    
    return declarationTail;
}

FunctionList* FunctionListSemanticAction(Function* function, FunctionList* nextFunctions) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    logInformation(_logger, "exec FunctionListSemanticAction");
    
    FunctionList* functionList = calloc(1, sizeof(FunctionList));
    functionList->function = function;
    functionList->next = nextFunctions;
    
    return functionList;
}

/* PUBLIC FUNCTIONS - Program */
Program* ProgramSemanticAction(DeclarationList* globalDeclarations, CompilerState* compilerState) {
    _logSyntacticAnalyzerAction(__FUNCTION__);
    logInformation(_logger, "exec ProgramSemanticAction");
    
    Program* program = calloc(1, sizeof(Program));
    program->globalDeclarations = globalDeclarations;
    compilerState->abstractSyntaxtTree = program;
    
    if (0 < flexCurrentContext()) {
        logError(_logger, "The final context is not the default (0): %d", flexCurrentContext());
        compilerState->succeed = false;
    } else {
        compilerState->succeed = true;
    }
    
    return program;
}
