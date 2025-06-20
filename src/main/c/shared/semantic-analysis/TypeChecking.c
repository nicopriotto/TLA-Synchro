#include "TypeChecking.h"
#include "ScopeStack.h"
#include <string.h>
#include <math.h>

/* MODULE INTERNAL STATE */
static Logger* _logger = NULL;

void initializeTypeCheckingModule() {
    _logger = createLogger("TypeChecking");
}

void shutdownTypeCheckingModule() {
    if (_logger != NULL) {
        destroyLogger(_logger);
    }
}

/* PRIVATE FUNCTIONS */
static void _logTypeCheckingAction(const char* functionName);
static SymbolEntry  *lookupSymbol(const char *id);
static int           variableExists(const char *id);
static int           functionExists(const char *id);
static TypeNodeType  mapSymbolType(SymbolType t);
static TypeNodeType  inferExpressionType(Expression *e);
static TypeNodeType  inferConditionType(Condition *c);
static int           typesCompatible(TypeNodeType a, TypeNodeType b);

static void _logTypeCheckingAction(const char* functionName) {
    if (_logger) {
        logDebugging(_logger, "%s", functionName);
    }
}

static SymbolEntry *lookupSymbol(const char *id)
{
    _logTypeCheckingAction(__FUNCTION__);
    
    if (!id) {
        if (_logger) logError(_logger, "lookupSymbol called with NULL identifier");
        return NULL;
    }

    if (!&currentCompilerState()->scopeStack) {
        if (_logger) logDebugging(_logger, "No scope stack, searching in global scope for: %s", id);
        return findSymbol(&currentCompilerState()->symbolTable, id, -1);
    }

    int lvl = currentScope(&currentCompilerState()->scopeStack);
    if (_logger) logDebugging(_logger, "Looking up symbol '%s' starting from scope level %d", id, lvl);

    //NUEVO PARA PROBAR
    // if (lvl == -1) {
    //     return findSymbol(&currentCompilerState()->symbolTable, id, -1);
    // }

    for (int s = lvl; s >= -1; --s) {
        SymbolEntry *e = findSymbol(&currentCompilerState()->symbolTable, id, s);
        if (e) {
            if (_logger) logDebugging(_logger, "Found symbol '%s' in scope %d with type %d", id, s, e->type);
            return e;
        }
    }
    
    if (_logger) logDebugging(_logger, "Symbol '%s' not found in any scope", id);
    return NULL;
}

static int variableExists(const char *id)
{
    _logTypeCheckingAction(__FUNCTION__);
    
    SymbolEntry *e = lookupSymbol(id);
    int exists = e && e->type != SYMBOL_FUNCTION;
    
    if (_logger) {
        if (exists) {
            logDebugging(_logger, "Variable '%s' exists with type %d", id, e->type);
        } else {
            logError(_logger, "Variable '%s' does not exist or is not a variable", id);
        }
    }
    
    return exists;
}

static int functionExists(const char *id)
{
    _logTypeCheckingAction(__FUNCTION__);
    
    SymbolEntry *e = lookupSymbol(id);
    int exists = e && e->type == SYMBOL_FUNCTION;
    
    if (_logger) {
        if (exists) {
            logDebugging(_logger, "Function '%s' exists", id);
        } else {
            logError(_logger, "Function '%s' does not exist or is not a function", id);
        }
    }
    
    return exists;
}

static TypeNodeType mapSymbolType(SymbolType t)
{
    _logTypeCheckingAction(__FUNCTION__);
    
    TypeNodeType result;
    switch (t) {
        case SYMBOL_INTEGER:   result = TYPE_INTEGER; break;
        case SYMBOL_STRING:    result = TYPE_STRING; break;
        case SYMBOL_FLOAT:     result = TYPE_FLOAT; break;
        case SYMBOL_BOOLEAN:   result = TYPE_BOOLEAN; break;
        case SYMBOL_SEMAPHORE: result = TYPE_SEM; break;
        default:               result = (TypeNodeType)-1; break;
    }
    
    if (_logger) {
        if (result != (TypeNodeType)-1) {
            logDebugging(_logger, "Mapped SymbolType %d to TypeNodeType %d", t, result);
        } else {
            logError(_logger, "Failed to map SymbolType %d to TypeNodeType", t);
        }
    }
    
    return result;
}

static TypeNodeType constantToType(Constant *c)
{
    _logTypeCheckingAction(__FUNCTION__);
    
    if (!c) {
        if (_logger) logError(_logger, "constantToType called with NULL constant");
        return (TypeNodeType)-1;
    }
    
    TypeNodeType result;
    switch (c->type) {
        case CONST_INTEGER:  result = TYPE_INTEGER; break;
        case CONST_FLOAT:    result = TYPE_FLOAT; break;
        case CONST_BOOLEAN:  result = TYPE_BOOLEAN; break;
        case CONST_STRING:   result = TYPE_STRING; break;
        default:             result = (TypeNodeType)-1; break;
    }
    
    if (_logger) {
        if (result != (TypeNodeType)-1) {
            logDebugging(_logger, "Constant type %d mapped to TypeNodeType %d", c->type, result);
        } else {
            logError(_logger, "Unknown constant type %d", c->type);
        }
    }
    
    return result;
}

static TypeNodeType inferExpressionType(Expression *e)
{
    _logTypeCheckingAction(__FUNCTION__);
    
    if (!e) {
        if (_logger) logError(_logger, "inferExpressionType called with NULL expression");
        return (TypeNodeType)-1;
    }

    TypeNodeType result = (TypeNodeType)-1;
    
    switch (e->type) {
        case EXPR_CONSTANT:
            if (_logger) logDebugging(_logger, "Inferring type for constant expression");
            result = constantToType(e->constant);
            break;

        case EXPR_IDENTIFIER: {
            if (_logger) logDebugging(_logger, "Inferring type for identifier: %s", e->identifier);
            SymbolEntry *se = lookupSymbol(e->identifier);
            result = se ? mapSymbolType(se->type) : (TypeNodeType)-1;
            break;
        }

        case EXPR_ADD: 
        case EXPR_SUB:
        case EXPR_MUL: 
        case EXPR_DIV: 
        case EXPR_MOD: {
            if (_logger) logDebugging(_logger, "Inferring type for binary arithmetic expression (type %d)", e->type);
            TypeNodeType l = inferExpressionType(e->binary.leftExpression);
            TypeNodeType r = inferExpressionType(e->binary.rightExpression);
            
            if (_logger) logDebugging(_logger, "Left operand type: %d, Right operand type: %d", l, r);
            
            if (l == r && (l == TYPE_INTEGER || l == TYPE_FLOAT)) {
                result = l;
                if (_logger) logDebugging(_logger, "Binary operation result type: %d", result);
            } else {
                if (_logger) logError(_logger, "Type mismatch in binary operation: %d vs %d", l, r);
            }
            break;
        }

        case EXPR_INCREMENT: 
        case EXPR_DECREMENT:
        case EXPR_PRE_INCREMENT: 
        case EXPR_PRE_DECREMENT:
            if (_logger) logDebugging(_logger, "Inferring type for unary increment/decrement expression");
            result = inferExpressionType(e->unary.expression);
            break;

        case EXPR_FUNCTION_CALL: {
            const char *fname = e->functionCall.functionName;
            if (_logger) logDebugging(_logger, "Inferring type for function call: %s", fname ? fname : "NULL");
            
            if (!fname) {
                if (_logger) logError(_logger, "Function call with NULL function name");
                return (TypeNodeType)-1;
            }
            
            SymbolEntry *se = lookupSymbol(fname);
            if (!se) {
                if (_logger) logWarning(_logger, "Function '%s' not found, assuming INTEGER return type", fname);
                return TYPE_INTEGER;
            }
            if (se->type != SYMBOL_FUNCTION) {
                if (_logger) logError(_logger, "Identifier '%s' is not a function (type: %d)", fname, se->type);
                return (TypeNodeType)-1;
            }
            result = mapSymbolType(se->data.functionType);
            if (_logger) logDebugging(_logger, "Function '%s' return type: %d", fname, result);
            break;
        }
        
        default:
            if (_logger) logError(_logger, "Unknown expression type: %d", e->type);
            break;
    }
    
    if (_logger && result == (TypeNodeType)-1) {
        logError(_logger, "Failed to infer expression type for expression type %d", e->type);
    }
    
    return result;
}

static TypeNodeType inferConditionType(Condition *c)
{
    _logTypeCheckingAction(__FUNCTION__);
    
    if (!c) {
        if (_logger) logError(_logger, "inferConditionType called with NULL condition");
        return (TypeNodeType)-1;
    }
    
    TypeNodeType result = (TypeNodeType)-1;
    
    switch (c->type) {
        case COND_RELATIONAL:
        case COND_AND:
        case COND_OR:
        case COND_NOT:
        case COND_EMPTY:
            if (_logger) logDebugging(_logger, "Condition type %d always returns BOOLEAN", c->type);
            result = TYPE_BOOLEAN;
            break;
        case COND_EXPRESSION:
            if (_logger) logDebugging(_logger, "Inferring type for expression condition");
            result = inferExpressionType(c->expression.expression);
            break;
        default:
            if (_logger) logError(_logger, "Unknown condition type: %d", c->type);
            break;
    }
    
    return result;
}

static int typesCompatible(TypeNodeType a, TypeNodeType b)
{
    _logTypeCheckingAction(__FUNCTION__);
    
    int compatible = a == b && a != (TypeNodeType)-1;
    
    if (_logger) {
        if (compatible) {
            logDebugging(_logger, "Types %d and %d are compatible", a, b);
        } else {
            logError(_logger, "Types %d and %d are not compatible", a, b);
        }
    }
    
    return compatible;
}

unsigned int CheckTypeConstant(Constant *c)
{
    _logTypeCheckingAction(__FUNCTION__);
    
    unsigned int result = c != NULL;
    
    if (_logger) {
        if (result) {
            logDebugging(_logger, "Constant type check passed");
        } else {
            logError(_logger, "Constant type check failed - NULL constant");
        }
    }
    
    return result;
}

unsigned int CheckTypeTypeNode(TypeNode *t)
{
    _logTypeCheckingAction(__FUNCTION__);
    
    unsigned int result = t != NULL;
    
    if (_logger) {
        if (result) {
            logDebugging(_logger, "TypeNode check passed for type %d", t->type);
        } else {
            logError(_logger, "TypeNode check failed - NULL TypeNode");
        }
    }
    
    return result;
}

unsigned int CheckTypeRelationalOperator(RelationalOperator *r)
{
    _logTypeCheckingAction(__FUNCTION__);
    
    unsigned int result = r != NULL;
    
    if (_logger) {
        if (result) {
            logDebugging(_logger, "RelationalOperator check passed");
        } else {
            logError(_logger, "RelationalOperator check failed - NULL operator");
        }
    }
    
    return result;
}

unsigned int CheckTypeExpression(Expression *e)
{
    _logTypeCheckingAction(__FUNCTION__);
    
    TypeNodeType type = inferExpressionType(e);
    unsigned int result = type != (TypeNodeType)-1;
    
    if (_logger) {
        if (result) {
            logDebugging(_logger, "Expression type check passed with type %d", type);
        } else {
            logError(_logger, "Expression type check failed - could not infer type");
        }
    }
    
    return result;
}

unsigned int CheckTypeCondition(Condition *c)
{
    _logTypeCheckingAction(__FUNCTION__);
    
    TypeNodeType type = inferConditionType(c);
    unsigned int result = type == TYPE_BOOLEAN;
    
    if (_logger) {
        if (result) {
            logDebugging(_logger, "Condition type check passed");
        } else {
            logError(_logger, "Condition type check failed - expected BOOLEAN, got %d", type);
        }
    }
    
    return result;
}

unsigned int CheckTypeArgumentList(ArgumentList *a)
{
    _logTypeCheckingAction(__FUNCTION__);
    
    int argCount = 0;
    while (a) {
        argCount++;
        if (_logger) logDebugging(_logger, "Checking argument %d", argCount);
        
        if (!CheckTypeExpression(a->expression)) {
            if (_logger) logError(_logger, "Argument %d type check failed", argCount);
            return 0;
        }
        a = a->next;
    }
    
    if (_logger) logDebugging(_logger, "All %d arguments passed type checking", argCount);
    return 1;
}

unsigned int CheckTypeFunctionIdentifier(FunctionIdentifier *f)
{
    _logTypeCheckingAction(__FUNCTION__);
    
    if (!f) {
        if (_logger) logError(_logger, "FunctionIdentifier check failed - NULL identifier");
        return 0;
    }
    
    unsigned int result;
    if (f->type == FUNC_USER_DEFINED) {
        result = functionExists(f->identifier);
        if (_logger && !result) {
            logError(_logger, "User-defined function '%s' does not exist", f->identifier);
        }
    } else {
        result = 1;
        if (_logger) logDebugging(_logger, "Built-in function identifier check passed");
    }
    
    return result;
}

unsigned int CheckTypeVariableDeclaration(VariableDeclaration *v)
{
    _logTypeCheckingAction(__FUNCTION__);

    if (!v || !v->type || !v->identifier || !v->condition) {
        if (_logger) logError(_logger, "VariableDeclaration check failed - NULL components");
        return 0;
    }

    if (_logger) logDebugging(_logger, "Checking variable declaration for '%s'", v->identifier);

    TypeNodeType lhs = v->type->type;
    TypeNodeType rhs = inferConditionType(v->condition);

    if (_logger) logDebugging(_logger, "Variable '%s' - declared type: %d, condition type: %d", v->identifier, lhs, rhs);

    unsigned int result = typesCompatible(lhs, rhs);
    if (_logger && !result) {
        logError(_logger, "Type mismatch in variable declaration '%s': %d vs %d", v->identifier, lhs, rhs);
    }

    return result;
}

unsigned int CheckTypeForInitializer(ForInitializer *fi)
{
    _logTypeCheckingAction(__FUNCTION__);
    
    int initCount = 0;
    while (fi) {
        initCount++;
        if (_logger) logDebugging(_logger, "Checking for-loop initializer %d", initCount);
        
        if (!CheckTypeVariableDeclaration(fi->declaration)) {
            if (_logger) logError(_logger, "For-loop initializer %d failed type check", initCount);
            return 0;
        }
        fi = fi->next;
    }
    
    if (_logger) logDebugging(_logger, "All %d for-loop initializers passed type checking", initCount);
    return 1;
}

unsigned int CheckTypeSimpleStatement(SimpleStatement *s);

unsigned int CheckTypeForUpdate(ForUpdate *fu)
{
    _logTypeCheckingAction(__FUNCTION__);
    
    int updateCount = 0;
    while (fu) {
        updateCount++;
        if (_logger) logDebugging(_logger, "Checking for-loop update %d", updateCount);
        
        if (!CheckTypeSimpleStatement(fu->statement)) {
            if (_logger) logError(_logger, "For-loop update %d failed type check", updateCount);
            return 0;
        }
        fu = fu->next;
    }
    
    if (_logger) logDebugging(_logger, "All %d for-loop updates passed type checking", updateCount);
    return 1;
}

unsigned int CheckTypeSimpleStatement(SimpleStatement *s)
{
    _logTypeCheckingAction(__FUNCTION__);
    
    if (!s) {
        if (_logger) logError(_logger, "SimpleStatement check failed - NULL statement");
        return 0;
    }

    unsigned int result = 0;
    
    switch (s->type) {
        case SIMPLE_FUNCTION_CALL:
            if (_logger) logDebugging(_logger, "Checking function call statement");
            result = CheckTypeFunctionIdentifier(s->functionCall.function) &&
                   CheckTypeArgumentList(s->functionCall.arguments);
            break;

        case SIMPLE_INCREMENT:
        case SIMPLE_DECREMENT:
            if (_logger) logDebugging(_logger, "Checking increment/decrement statement for '%s'", s->increment.identifier);
            result = variableExists(s->increment.identifier);
            break;

        case SIMPLE_ASSIGNMENT: {
            if (_logger) logDebugging(_logger, "Checking assignment statement for '%s'", s->assignment.identifier);
            SymbolEntry *se = lookupSymbol(s->assignment.identifier);
            if (!se || se->type == SYMBOL_FUNCTION) {
                if (_logger) logError(_logger, "Assignment target '%s' is not a valid variable", s->assignment.identifier);
                result = 0;
            } else {
                TypeNodeType lhs = mapSymbolType(se->type);
                TypeNodeType rhs = inferExpressionType(s->assignment.expression);
                if (_logger) logDebugging(_logger, "Assignment '%s' - target type: %d, expression type: %d", s->assignment.identifier, lhs, rhs);
                result = typesCompatible(lhs, rhs);
            }
            break;
        }
        
        case SIMPLE_DECLARATION:
            if (_logger) logDebugging(_logger, "Checking declaration statement");
            result = CheckTypeVariableDeclaration(s->declaration);
            break;

        case RETURN_CONSTANT:
            if (_logger) logDebugging(_logger, "Checking return constant statement");
            result = CheckTypeConstant(s->constant);
            break;

        case RETURN_IDENTIFIER:
            if (_logger) logDebugging(_logger, "Checking return identifier statement for '%s'", s->identifier);
            result = variableExists(s->identifier);
            break;
            
        default:
            if (_logger) logError(_logger, "Unknown SimpleStatement type: %d", s->type);
            break;
    }
    
    if (_logger) {
        if (result) {
            logDebugging(_logger, "SimpleStatement type check passed");
        } else {
            logError(_logger, "SimpleStatement type check failed for type %d", s->type);
        }
    }
    
    return result;
}

unsigned int CheckTypeClosedStatement(ClosedStatement *c)
{
    _logTypeCheckingAction(__FUNCTION__);
    
    if (!c) {
        if (_logger) logError(_logger, "ClosedStatement check failed - NULL statement");
        return 0;
    }
    
    unsigned int result = 0;
    
    switch (c->type) {
        case CLOSED_SIMPLE:
            if (_logger) logDebugging(_logger, "Checking closed simple statement");
            result = CheckTypeSimpleStatement(c->simpleStatement);
            break;
        case CLOSED_IF_ELSE:
            if (_logger) logDebugging(_logger, "Checking closed if-else statement");
            result = CheckTypeCondition(c->ifElseStatement.condition) &&
                   CheckTypeClosedStatement(c->ifElseStatement.thenStatement) &&
                   CheckTypeClosedStatement(c->ifElseStatement.elseStatement);
            break;
        case CLOSED_WHILE:
            if (_logger) logDebugging(_logger, "Checking closed while statement");
            result = CheckTypeCondition(c->whileStatement.condition) &&
                   CheckTypeClosedStatement(c->whileStatement.body);
            break;
        case CLOSED_FOR:
            if (_logger) logDebugging(_logger, "Checking closed for statement");
            result = CheckTypeForInitializer(c->forStatement.initializer) &&
                   CheckTypeCondition(c->forStatement.condition) &&
                   CheckTypeForUpdate(c->forStatement.update) &&
                   CheckTypeClosedStatement(c->forStatement.body);
            break;
        case CLOSED_FOREVER:
            if (_logger) logDebugging(_logger, "Checking closed forever statement");
            result = CheckTypeClosedStatement(c->foreverStatement.body);
            break;
        case CLOSED_STATEMENT_LIST:
            if (_logger) logDebugging(_logger, "Checking closed statement list");
            result = CheckTypeStatementList(c->closedStatementList.statementList);
            break;
        default:
            if (_logger) logError(_logger, "Unknown ClosedStatement type: %d", c->type);
            break;
    }
    
    if (_logger && !result) {
        logError(_logger, "ClosedStatement type check failed for type %d", c->type);
    }
    
    return result;
}

unsigned int CheckTypeOpenStatement(OpenStatement *o)
{
    _logTypeCheckingAction(__FUNCTION__);
    
    if (!o) {
        if (_logger) logError(_logger, "OpenStatement check failed - NULL statement");
        return 0;
    }
    
    unsigned int result = 0;
    
    switch (o->type) {
        case OPEN_IF:
            if (_logger) logDebugging(_logger, "Checking open if statement");
            result = CheckTypeCondition(o->ifStatement.condition) &&
                   CheckTypeStatement(o->ifStatement.thenStatement);
            break;
        case OPEN_IF_ELSE:
            if (_logger) logDebugging(_logger, "Checking open if-else statement");
            result = CheckTypeCondition(o->ifElseStatement.condition) &&
                   CheckTypeStatement(o->ifElseStatement.thenStatement) &&
                   CheckTypeOpenStatement(o->ifElseStatement.elseStatement);
            break;
        case OPEN_WHILE:
            if (_logger) logDebugging(_logger, "Checking open while statement");
            result = CheckTypeCondition(o->whileStatement.condition) &&
                   CheckTypeOpenStatement(o->whileStatement.body);
            break;
        case OPEN_FOR:
            if (_logger) logDebugging(_logger, "Checking open for statement");
            result = CheckTypeForInitializer(o->forStatement.initializer) &&
                   CheckTypeCondition(o->forStatement.condition) &&
                   CheckTypeForUpdate(o->forStatement.update) &&
                   CheckTypeOpenStatement(o->forStatement.body);
            break;
        default:
            if (_logger) logError(_logger, "Unknown OpenStatement type: %d", o->type);
            break;
    }
    
    if (_logger && !result) {
        logError(_logger, "OpenStatement type check failed for type %d", o->type);
    }
    
    return result;
}

unsigned int CheckTypeStatement(Statement *s)
{
    _logTypeCheckingAction(__FUNCTION__);
    
    if (!s) {
        if (_logger) logError(_logger, "Statement check failed - NULL statement");
        return 0;
    }
    
    unsigned int result = 0;
    
    switch (s->type) {
        case STMT_SIMPLE:
            if (_logger) logDebugging(_logger, "Checking simple statement");
            result = CheckTypeSimpleStatement(s->simpleStatement);
            break;
        case STMT_IF:
            if (_logger) logDebugging(_logger, "Checking if statement");
            result = CheckTypeCondition(s->ifStatement.condition) &&
                   CheckTypeStatement(s->ifStatement.thenStatement);
            break;
        case STMT_IF_ELSE:
            if (_logger) logDebugging(_logger, "Checking if-else statement");
            result = CheckTypeCondition(s->ifElseStatement.condition) &&
                   CheckTypeStatement(s->ifElseStatement.thenStatement) &&
                   CheckTypeStatement(s->ifElseStatement.elseStatement);
            break;
        case STMT_WHILE:
            if (_logger) logDebugging(_logger, "Checking while statement");
            result = CheckTypeCondition(s->whileStatement.condition) &&
                   CheckTypeStatement(s->whileStatement.body);
            break;
        case STMT_FOR:
            if (_logger) logDebugging(_logger, "Checking for statement");
            result = CheckTypeForInitializer(s->forStatement.initializer) &&
                   CheckTypeCondition(s->forStatement.condition) &&
                   CheckTypeForUpdate(s->forStatement.update) &&
                   CheckTypeStatement(s->forStatement.body);
            break;
        case STMT_FOREVER:
            if (_logger) logDebugging(_logger, "Checking forever statement");
            result = CheckTypeStatement(s->foreverStatement.body);
            break;
        case STMT_BLOCK:
            if (_logger) logDebugging(_logger, "Checking block statement");
            result = CheckTypeStatementList(s->blockStatement);
            break;
        default:
            if (_logger) logError(_logger, "Unknown Statement type: %d", s->type);
            break;
    }
    
    if (_logger && !result) {
        logError(_logger, "Statement type check failed for type %d", s->type);
    }
    
    return result;
}

unsigned int CheckTypeStatementList(StatementList *sl)
{
    _logTypeCheckingAction(__FUNCTION__);
    
    int stmtCount = 0;
    while (sl) {
        stmtCount++;
        if (_logger) logDebugging(_logger, "Checking statement %d in list", stmtCount);
        
        if (!CheckTypeStatement(sl->statement)) {
            if (_logger) logError(_logger, "Statement %d in list failed type check", stmtCount);
            return 0;
        }
        sl = sl->next;
    }
    
    if (_logger) logDebugging(_logger, "All %d statements in list passed type checking", stmtCount);
    return 1;
}

unsigned int CheckTypeParameterList(ParameterList *pl)
{
    _logTypeCheckingAction(__FUNCTION__);
    
    int paramCount = 0;
    while (pl) {
        paramCount++;
        if (_logger) logDebugging(_logger, "Checking parameter %d: %s", paramCount, pl->identifier);
        
        if (!CheckTypeTypeNode(pl->type)) {
            if (_logger) logError(_logger, "Parameter %d (%s) has invalid type", paramCount, pl->identifier);
            return 0;
        }
        pl = pl->next;
    }
    
    if (_logger) logDebugging(_logger, "All %d parameters passed type checking", paramCount);
    return 1;
}

unsigned int CheckTypeDeclarationTail(DeclarationTail *dt)
{
    _logTypeCheckingAction(__FUNCTION__);
    
    if (!dt) {
        if (_logger) logError(_logger, "DeclarationTail check failed - NULL declaration tail");
        return 0;
    }
    
    unsigned int result = 0;
    
    switch (dt->type) {
        case DECL_CONSTANT:
            if (_logger) logDebugging(_logger, "Checking constant declaration tail");
            result = CheckTypeConstant(dt->constant);
            break;
        case DECL_FUNCTION:
            if (_logger) logDebugging(_logger, "Checking function declaration tail");
            result = CheckTypeParameterList(dt->function.parameterList) &&
                   CheckTypeStatementList(dt->function.statementList);
            break;
        default:
            if (_logger) logError(_logger, "Unknown DeclarationTail type: %d", dt->type);
            break;
    }
    
    if (_logger && !result) {
        logError(_logger, "DeclarationTail type check failed for type %d", dt->type);
    }
    
    return result;
}

unsigned int CheckTypeDeclarationList(DeclarationList *dl)
{
    _logTypeCheckingAction(__FUNCTION__);

    int declCount = 0;
    while (dl) {
        declCount++;
        if (_logger) logDebugging(_logger, "Checking declaration %d: %s", declCount, dl->identifier);

        if (!CheckTypeTypeNode(dl->type)) {
            if (_logger) logError(_logger, "Declaration %d (%s) has invalid type", declCount, dl->identifier);
            return 0;
        }

        if (!CheckTypeDeclarationTail(dl->declarationTail)) {
            if (_logger) logError(_logger, "Declaration %d (%s) tail failed type check", declCount, dl->identifier);
            return 0;
        }

        dl = dl->next;
    }

    if (_logger) logDebugging(_logger, "All %d declarations passed type checking", declCount);
    return 1;
}

unsigned int CheckTypeProgram(Program *p)
{
    _logTypeCheckingAction(__FUNCTION__);
    
    if (!p) {
        if (_logger) logError(_logger, "Program type check failed - NULL program");
        return 0;
    }
    
    if (_logger) logDebugging(_logger, "Starting program type checking");
    
    unsigned int result = CheckTypeDeclarationList(p->globalDeclarations);
    
    if (_logger) {
        if (result) {
            logDebugging(_logger, "Program type checking completed successfully");
        } else {
            logError(_logger, "Program type checking failed");
        }
    }
    
    return result;
}
