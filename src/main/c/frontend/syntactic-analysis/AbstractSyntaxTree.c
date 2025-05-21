#include "AbstractSyntaxTree.h"
#include <string.h>

/* MODULE INTERNAL STATE */
static Logger* _logger = NULL;

void initializeAbstractSyntaxTreeModule() {
    _logger = createLogger("AbstractSyntaxTree");
    logInformation(_logger, "Abstract Syntax Tree module initialized");
}

void shutdownAbstractSyntaxTreeModule() {
    if (_logger != NULL) {
        logInformation(_logger, "Abstract Syntax Tree module shutting down");
        destroyLogger(_logger);
        _logger = NULL;
    }
}

#define SAFE_FREE(ptr) do { if (ptr) { free(ptr); ptr = NULL; } } while (0)

/* HELPER FUNCTIONS */

static char* duplicateString(const char* str) {
    if (!str) return NULL;
    size_t len = strlen(str);
    char* result = (char*)calloc(len + 1, sizeof(char));
    if (!result) {
        logError(_logger, "Memory allocation failed for string duplication");
        return NULL;
    }
    strcpy(result, str);
    return result;
}

/* NODE CREATION FUNCTIONS */

Constant* createConstantInteger(int value) {
    Constant* constant = (Constant*)calloc(1, sizeof(Constant));
    if (!constant) {
        logError(_logger, "Memory allocation failed for integer constant");
        return NULL;
    }
    constant->type = CONST_INTEGER;
    constant->integer = value;
    return constant;
}

Constant* createConstantFloat(float value) {
    Constant* constant = (Constant*)calloc(1, sizeof(Constant));
    if (!constant) {
        logError(_logger, "Memory allocation failed for float constant");
        return NULL;
    }
    constant->type = CONST_FLOAT;
    constant->floatVal = value;
    return constant;
}

Constant* createConstantBoolean(bool value) {
    Constant* constant = (Constant*)calloc(1, sizeof(Constant));
    if (!constant) {
        logError(_logger, "Memory allocation failed for boolean constant");
        return NULL;
    }
    constant->type = CONST_BOOLEAN;
    constant->boolean = value;
    return constant;
}

Constant* createConstantString(const char* value) {
    Constant* constant = (Constant*)calloc(1, sizeof(Constant));
    if (!constant) {
        logError(_logger, "Memory allocation failed for string constant");
        return NULL;
    }
    constant->type = CONST_STRING;
    constant->string = duplicateString(value);
    if (!constant->string && value) {
        logError(_logger, "Memory allocation failed for string value");
        free(constant);
        return NULL;
    }
    return constant;
}

Expression* createExpressionConstant(Constant* constant) {
    if (!constant) return NULL;
    
    Expression* expr = (Expression*)calloc(1, sizeof(Expression));
    if (!expr) {
        logError(_logger, "Memory allocation failed for constant expression");
        return NULL;
    }
    expr->type = EXPR_CONSTANT;
    expr->constant = constant;
    return expr;
}

Expression* createExpressionIdentifier(const char* identifier) {
    if (!identifier) return NULL;
    
    Expression* expr = (Expression*)calloc(1, sizeof(Expression));
    if (!expr) {
        logError(_logger, "Memory allocation failed for identifier expression");
        return NULL;
    }
    expr->type = EXPR_IDENTIFIER;
    expr->identifier = duplicateString(identifier);
    if (!expr->identifier) {
        logError(_logger, "Memory allocation failed for identifier string");
        free(expr);
        return NULL;
    }
    return expr;
}

Expression* createExpressionBinary(ExpressionType type, Expression* left, Expression* right) {
    if (!left || !right) return NULL;
    if (type != EXPR_ADD && type != EXPR_SUB && type != EXPR_MUL && 
        type != EXPR_DIV && type != EXPR_MOD) {
        logError(_logger, "Invalid binary expression type");
        return NULL;
    }
    
    Expression* expr = (Expression*)calloc(1, sizeof(Expression));
    if (!expr) {
        logError(_logger, "Memory allocation failed for binary expression");
        return NULL;
    }
    expr->type = type;
    expr->binary.leftExpression = left;
    expr->binary.rightExpression = right;
    return expr;
}

Expression* createExpressionUnary(ExpressionType type, Expression* expr) {
    if (!expr) return NULL;
    if (type != EXPR_INCREMENT && type != EXPR_DECREMENT && 
        type != EXPR_PRE_INCREMENT && type != EXPR_PRE_DECREMENT) {
        logError(_logger, "Invalid unary expression type");
        return NULL;
    }
    
    Expression* result = (Expression*)calloc(1, sizeof(Expression));
    if (!result) {
        logError(_logger, "Memory allocation failed for unary expression");
        return NULL;
    }
    result->type = type;
    result->unary.expression = expr;
    return result;
}


Expression* createExpressionFunctionCall(const char* name, ArgumentList* args) {
    if (!name) return NULL;
    
    Expression* expr = (Expression*)calloc(1, sizeof(Expression));
    if (!expr) {
        logError(_logger, "Memory allocation failed for function call expression");
        return NULL;
    }
    expr->type = EXPR_FUNCTION_CALL;
    expr->functionCall.functionName = duplicateString(name);
    if (!expr->functionCall.functionName) {
        logError(_logger, "Memory allocation failed for function name");
        free(expr);
        return NULL;
    }
    expr->functionCall.arguments = args;
    return expr;
}

/* NODE DESTRUCTION FUNCTIONS */

void releaseConstant(Constant* constant) {
    if (!constant) return;
    
    if (constant->type == CONST_STRING && constant->string) {
        SAFE_FREE(constant->string);
    }
    SAFE_FREE(constant);
}

void releaseExpression(Expression* expression) {
    if (!expression) return;
    
    logDebugging(_logger, "Releasing expression of type %d", expression->type);
    
    switch (expression->type) {
        case EXPR_CONSTANT:
            releaseConstant(expression->constant);
            break;
        case EXPR_IDENTIFIER:
            SAFE_FREE(expression->identifier);
            break;
        case EXPR_ADD: case EXPR_SUB: case EXPR_MUL:
        case EXPR_DIV: case EXPR_MOD:
            releaseExpression(expression->binary.leftExpression);
            releaseExpression(expression->binary.rightExpression);
            break;
        case EXPR_INCREMENT: case EXPR_DECREMENT:
        case EXPR_PRE_INCREMENT: case EXPR_PRE_DECREMENT:
            releaseExpression(expression->unary.expression);
            break;
        case EXPR_FUNCTION_CALL:
            SAFE_FREE(expression->functionCall.functionName);
            releaseArgumentList(expression->functionCall.arguments);
            break;
    }
    SAFE_FREE(expression);
}

void releaseRelationalOperator(RelationalOperator* op) {
    if (!op) return;
    
    logDebugging(_logger, "Releasing relational operator of type %d", op->type);
    SAFE_FREE(op);
}

void releaseSimpleStatement(SimpleStatement* ss) {
    if (!ss) return;
    
    switch(ss->type) {
        case SIMPLE_FUNCTION_CALL:
            releaseFunctionIdentifier(ss->functionCall.function);
            releaseArgumentList(ss->functionCall.arguments);
            break;
        case SIMPLE_INCREMENT:
            SAFE_FREE(ss->increment.identifier);
            break;
        case SIMPLE_DECREMENT:
            SAFE_FREE(ss->decrement.identifier);
            break;
        case SIMPLE_ASSIGNMENT:
            SAFE_FREE(ss->assignment.identifier);
            releaseExpression(ss->assignment.expression);
            break;
        case SIMPLE_DECLARATION:
            releaseVariableDeclaration(ss->declaration);
            break;
    }
    SAFE_FREE(ss);
}

void releaseOpenStatement(OpenStatement* os) {
    if (!os) return;
    
    logDebugging(_logger, "Releasing open statement of type %d", os->type);
    
    switch (os->type) {
        case OPEN_IF:
            releaseExpression(os->ifStatement.condition);
            releaseStatement(os->ifStatement.thenStatement);
            break;
        case OPEN_IF_ELSE:
            releaseExpression(os->ifElseStatement.condition);
            releaseStatement(os->ifElseStatement.thenStatement);
            releaseOpenStatement(os->ifElseStatement.elseStatement);
            break;
        case OPEN_WHILE:
            releaseExpression(os->whileStatement.condition);
            releaseOpenStatement(os->whileStatement.body);
            break;
        case OPEN_FOR:
            releaseForInitializer(os->forStatement.initializer);
            releaseCondition(os->forStatement.condition);
            releaseForUpdate(os->forStatement.update);
            releaseOpenStatement(os->forStatement.body);
            break;
        case OPEN_FOREVER:
            releaseOpenStatement(os->foreverStatement.body);
            break;
    }
    SAFE_FREE(os);
}

void releaseClosedStatement(ClosedStatement* cs) {
    if (!cs) return;
    
    logDebugging(_logger, "Releasing closed statement of type %d", cs->type);
    
    switch (cs->type) {
        case CLOSED_SIMPLE:
            releaseSimpleStatement(cs->simpleStatement);
            break;
        case CLOSED_IF_ELSE:
            releaseExpression(cs->ifElseStatement.condition);
            releaseClosedStatement(cs->ifElseStatement.thenStatement);
            releaseClosedStatement(cs->ifElseStatement.elseStatement);
            break;
        case CLOSED_IF_BLOCK:
            releaseExpression(cs->ifBlock.condition);
            releaseStatementList(cs->ifBlock.body);
            break;
        case CLOSED_WHILE:
            releaseExpression(cs->whileStatement.condition);
            releaseClosedStatement(cs->whileStatement.body);
            break;
        case CLOSED_WHILE_BLOCK:
            releaseExpression(cs->whileBlock.condition);
            releaseStatementList(cs->whileBlock.body);
            break;
        case CLOSED_FOR:
            releaseForInitializer(cs->forStatement.initializer);
            releaseCondition(cs->forStatement.condition);
            releaseForUpdate(cs->forStatement.update);
            releaseClosedStatement(cs->forStatement.body);
            break;
        case CLOSED_FOR_BLOCK:
            releaseForInitializer(cs->forBlock.initializer);
            releaseCondition(cs->forBlock.condition);
            releaseForUpdate(cs->forBlock.update);
            releaseStatementList(cs->forBlock.body);
            break;
        case CLOSED_FOREVER:
            releaseClosedStatement(cs->foreverStatement.body);
            break;
        case CLOSED_FOREVER_BLOCK:
            releaseStatementList(cs->foreverBlock.body);
            break;
    }
    SAFE_FREE(cs);
}

void releaseStatement(Statement* stmt) {
    if (!stmt) return;
    
    logDebugging(_logger, "Releasing statement of type %d", stmt->type);
    
    switch (stmt->type) {
        case STMT_SIMPLE:
            releaseSimpleStatement(stmt->simpleStatement);
            break;
        case STMT_IF:
            releaseExpression(stmt->ifStatement.condition);
            releaseStatement(stmt->ifStatement.thenStatement);
            break;
        case STMT_IF_ELSE:
            releaseExpression(stmt->ifElseStatement.condition);
            releaseStatement(stmt->ifElseStatement.thenStatement);
            releaseStatement(stmt->ifElseStatement.elseStatement);
            break;
        case STMT_WHILE:
            releaseExpression(stmt->whileStatement.condition);
            releaseStatement(stmt->whileStatement.body);
            break;
        case STMT_FOR:
            releaseForInitializer(stmt->forStatement.initializer);
            releaseCondition(stmt->forStatement.condition);
            releaseForUpdate(stmt->forStatement.update);
            releaseStatement(stmt->forStatement.body);
            break;
        case STMT_FOREVER:
            releaseStatement(stmt->foreverStatement.body);
            break;
    }
    SAFE_FREE(stmt);
}

void releaseStatementList(StatementList* sl) {
    if (!sl) return;
    
    // Use an iterative approach to avoid stack overflow for large lists
    StatementList* current = sl;
    StatementList* next = NULL;

    
    while (current) {
        next = current->next;
        releaseStatement(current->statement);
        SAFE_FREE(current);
        current = next;
    }
}

void releaseCondition(Condition* cond) {
    if (!cond) return;
    
    logDebugging(_logger, "Releasing condition of type %d", cond->type);
    
    switch (cond->type) {
        case COND_RELATIONAL:
            releaseExpression(cond->leftValue);
            releaseExpression(cond->rightValue);
            releaseRelationalOperator(cond->operator);
            break;
        case COND_NOT:
            releaseCondition(cond->not.condition);
            break;
        case COND_PARENTHESIS:
            releaseCondition(cond->parenthesis.condition);
            break;
        case COND_AND: case COND_OR:
            releaseCondition(cond->logical.leftCondition);
            releaseCondition(cond->logical.rightCondition);
            break;
        case COND_EMPTY:
            break;
    }
    SAFE_FREE(cond);
}

void releaseParameterList(ParameterList* pl) {
    if (!pl) return;
    
    // Use an iterative approach to avoid stack overflow for large lists
    ParameterList* current = pl;
    ParameterList* next = NULL;
    
    while (current) {
        next = current->next;
        releaseTypeNode(current->type);
        SAFE_FREE(current->identifier);
        SAFE_FREE(current);
        current = next;
    }
}

void releaseArgumentList(ArgumentList* al) {
    if (!al) return;
    
    // Use an iterative approach to avoid stack overflow for large lists
    ArgumentList* current = al;
    ArgumentList* next = NULL;
    
    while (current) {
        next = current->next;
        releaseExpression(current->expression);
        SAFE_FREE(current);
        current = next;
    }
}

void releaseDeclarationTail(DeclarationTail* dt) {
    if (!dt) return;
    
    logDebugging(_logger, "Releasing declaration tail of type %d", dt->type);
    
    switch (dt->type) {
        case DECL_CONSTANT:
            releaseConstant(dt->constant);
            break;
        case DECL_FUNCTION:
            releaseParameterList(dt->function.parameterList);
            releaseStatementList(dt->function.statementList);
            break;
    }
    SAFE_FREE(dt);
}

void releaseDeclarationList(DeclarationList* dl) {
    if (!dl) return;
    
    // Use an iterative approach to avoid stack overflow for large lists
    DeclarationList* current = dl;
    DeclarationList* next = NULL;
    
    while (current) {
        next = current->next;
        releaseTypeNode(current->type);
        logDebugging(_logger, "THIS IS THE IDENTIFIER!! %s", current->identifier != NULL ? current->identifier : "NULL");
        SAFE_FREE(current->identifier);
        releaseDeclarationTail(current->declarationTail);
        SAFE_FREE(current);
        current = next;
    }
}

void releaseTypeNode(TypeNode* tn) {
    if (!tn) return;
    
    logDebugging(_logger, "Releasing type node of type %d", tn->type);
    SAFE_FREE(tn);
}

void releaseFunction(Function* fn) {
    if (!fn) return;
    
    logDebugging(_logger, "Releasing function");
    releaseParameterList(fn->parameters);
    releaseStatementList(fn->body);
    SAFE_FREE(fn);
}

void releaseFunctionList(FunctionList* fl) {
    if (!fl) return;
    
    // Use an iterative approach to avoid stack overflow for large lists
    FunctionList* current = fl;
    FunctionList* next = NULL;
    
    while (current) {
        next = current->next;
        releaseFunction(current->function);
        SAFE_FREE(current);
        current = next;
    }
}

void releaseFunctionIdentifier(FunctionIdentifier* fi) {
    if (!fi) return;
    
    logDebugging(_logger, "Releasing function identifier of type %d", fi->type);
    if (fi->type == FUNC_USER_DEFINED) {
        SAFE_FREE(fi->identifier);
    }
    SAFE_FREE(fi);
}

void releaseForInitializer(ForInitializer* fi) {
    if (!fi) return;
    
    // Use an iterative approach to avoid stack overflow for large lists
    ForInitializer* current = fi;
    ForInitializer* next = NULL;
    
    while (current) {
        next = current->next;
        releaseVariableDeclaration(current->declaration);
        SAFE_FREE(current);
        current = next;
    }
}

void releaseForUpdate(ForUpdate* fu) {
    if (!fu) return;
    
    // Use an iterative approach to avoid stack overflow for large lists
    ForUpdate* current = fu;
    ForUpdate* next = NULL;
    
    while (current) {
        next = current->next;
        releaseSimpleStatement(current->statement);
        SAFE_FREE(current);
        current = next;
    }
}

void releaseVariableDeclaration(VariableDeclaration* vd) {
    if (!vd) return;
    
    logDebugging(_logger, "Releasing variable declaration");
    releaseTypeNode(vd->type);
    SAFE_FREE(vd->identifier);
    
    switch(vd->uniontype){
        case CONDITION:
            releaseCondition(vd->condition);
            break;
        case EXPRESSION:
            releaseExpression(vd->expression);
            break;
    }
    
    SAFE_FREE(vd);
}

void releaseProgram(Program* p) {
    if (!p) return;
    logDebugging(_logger, "Releasing program");
    releaseDeclarationList(p->globalDeclarations);
    SAFE_FREE(p);
}