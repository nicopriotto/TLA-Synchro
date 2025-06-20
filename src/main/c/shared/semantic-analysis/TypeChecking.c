#include "TypeChecking.h"
#include "ScopeStack.h"
#include <string.h>
#include <math.h>

extern SymbolTable *symbolTable;   
extern ScopeStack  *scopeStack;    

static SymbolEntry  *lookupSymbol(const char *id);
static int           variableExists(const char *id);
static int           functionExists(const char *id);
static TypeNodeType  mapSymbolType(SymbolType t);
static TypeNodeType  inferExpressionType(Expression *e);
static TypeNodeType  inferConditionType(Condition *c);
static int           typesCompatible(TypeNodeType a, TypeNodeType b);

static SymbolEntry *lookupSymbol(const char *id)
{
    if (!id) return NULL;

    if (!scopeStack)
        return findSymbol(symbolTable, id, -1);

    int lvl = currentScope(scopeStack);
    for (int s = lvl; s >= 0; --s) {
        SymbolEntry *e = findSymbol(symbolTable, id, s);
        if (e) return e;
    }
    return NULL;
}

static int variableExists(const char *id)
{
    SymbolEntry *e = lookupSymbol(id);
    return e && e->type != FUNCTION;
}

static int functionExists(const char *id)
{
    SymbolEntry *e = lookupSymbol(id);
    return e && e->type == FUNCTION;
}

static TypeNodeType mapSymbolType(SymbolType t)
{
    switch (t) {
        case INTEGER:   return TYPE_INTEGER;
        case STRING:    return TYPE_STRING;
        case FLOAT:     return TYPE_FLOAT;
        case BOOLEAN:   return TYPE_BOOLEAN;
        case SEMAPHORE: return TYPE_SEM;
        default:        return (TypeNodeType)-1;
    }
}

static TypeNodeType constantToType(Constant *c)
{
    switch (c->type) {
        case CONST_INTEGER:  return TYPE_INTEGER;
        case CONST_FLOAT:    return TYPE_FLOAT;
        case CONST_BOOLEAN:  return TYPE_BOOLEAN;
        case CONST_STRING:   return TYPE_STRING;
        default:             return (TypeNodeType)-1;
    }
}

static TypeNodeType inferExpressionType(Expression *e)
{
    if (!e) return (TypeNodeType)-1;

    switch (e->type) {
        case EXPR_CONSTANT:
            return constantToType(e->constant);

        case EXPR_IDENTIFIER: {
            SymbolEntry *se = lookupSymbol(e->identifier);
            return se ? mapSymbolType(se->type) : (TypeNodeType)-1;
        }

        case EXPR_ADD: 
        case EXPR_SUB:
        case EXPR_MUL: 
        case EXPR_DIV: 
        case EXPR_MOD: {
            TypeNodeType l = inferExpressionType(e->binary.leftExpression);
            TypeNodeType r = inferExpressionType(e->binary.rightExpression);
            if (l == r && (l == TYPE_INTEGER || l == TYPE_FLOAT)) return l;
            return (TypeNodeType)-1;
        }

        case EXPR_INCREMENT: 
        case EXPR_DECREMENT:
        case EXPR_PRE_INCREMENT: 
        case EXPR_PRE_DECREMENT:
            return inferExpressionType(e->unary.expression);

        case EXPR_FUNCTION_CALL: {
            const char *fname = e->functionCall.functionName;
            if (!fname) return (TypeNodeType)-1;
            SymbolEntry *se = lookupSymbol(fname);
            if (!se) {
                return TYPE_INTEGER;
            }
            if (se->type != FUNCTION) return (TypeNodeType)-1;
            return mapSymbolType(se->data.functionType);
        }
    }
    return (TypeNodeType)-1;
}

static TypeNodeType inferConditionType(Condition *c)
{
    if (!c) return (TypeNodeType)-1;
    switch (c->type) {
        case COND_RELATIONAL:
        case COND_AND:
        case COND_OR:
        case COND_NOT:
        case COND_EMPTY:
            return TYPE_BOOLEAN;
        case COND_EXPRESSION:
            return inferExpressionType(c->expression.expression);
    }
    return (TypeNodeType)-1;
}

static int typesCompatible(TypeNodeType a, TypeNodeType b)
{
    return a == b && a != (TypeNodeType)-1;
}

unsigned int CheckTypeConstant(Constant *c)             { return c != NULL; }
unsigned int CheckTypeTypeNode(TypeNode *t)             { return t != NULL; }
unsigned int CheckTypeRelationalOperator(RelationalOperator *r) { return r != NULL; }
unsigned int CheckTypeExpression(Expression *e)         { return inferExpressionType(e) != (TypeNodeType)-1; }
unsigned int CheckTypeCondition(Condition *c)           { return inferConditionType(c) == TYPE_BOOLEAN; }

unsigned int CheckTypeArgumentList(ArgumentList *a)
{
    while (a) {
        if (!CheckTypeExpression(a->expression)) return 0;
        a = a->next;
    }
    return 1;
}

unsigned int CheckTypeFunctionIdentifier(FunctionIdentifier *f)
{
    if (!f) return 0;
    if (f->type == FUNC_USER_DEFINED) return functionExists(f->identifier);
    return 1;
}

unsigned int CheckTypeVariableDeclaration(VariableDeclaration *v)
{
    if (!v || !v->type || !v->identifier || !v->condition) return 0;

    int cur = scopeStack ? currentScope(scopeStack) : 0;
    if (findSymbol(symbolTable, v->identifier, cur)) return 0;
    TypeNodeType lhs = v->type->type;
    TypeNodeType rhs = inferConditionType(v->condition);
    return typesCompatible(lhs, rhs);
}

unsigned int CheckTypeForInitializer(ForInitializer *fi)
{
    while (fi) {
        if (!CheckTypeVariableDeclaration(fi->declaration)) return 0;
        fi = fi->next;
    }
    return 1;
}

unsigned int CheckTypeSimpleStatement(SimpleStatement *s);
unsigned int CheckTypeForUpdate(ForUpdate *fu)
{
    while (fu) {
        if (!CheckTypeSimpleStatement(fu->statement)) return 0;
        fu = fu->next;
    }
    return 1;
}

unsigned int CheckTypeSimpleStatement(SimpleStatement *s)
{
    if (!s) return 0;

    switch (s->type) {
        case SIMPLE_FUNCTION_CALL:
            return CheckTypeFunctionIdentifier(s->functionCall.function) &&
                   CheckTypeArgumentList(s->functionCall.arguments);

        case SIMPLE_INCREMENT:
        case SIMPLE_DECREMENT:
            return variableExists(s->increment.identifier);

        case SIMPLE_ASSIGNMENT: {
            SymbolEntry *se = lookupSymbol(s->assignment.identifier);
            if (!se || se->type == FUNCTION) return 0;
            TypeNodeType lhs = mapSymbolType(se->type);
            TypeNodeType rhs = inferExpressionType(s->assignment.expression);
            return typesCompatible(lhs, rhs);
        }
        case SIMPLE_DECLARATION:
            return CheckTypeVariableDeclaration(s->declaration);

        case RETURN_CONSTANT:
            return CheckTypeConstant(s->constant);

        case RETURN_IDENTIFIER:
            return variableExists(s->identifier);
    }
    return 0;
}

unsigned int CheckTypeClosedStatement(ClosedStatement *c)
{
    if (!c) return 0;
    switch (c->type) {
        case CLOSED_SIMPLE:
            return CheckTypeSimpleStatement(c->simpleStatement);
        case CLOSED_IF_ELSE:
            return CheckTypeCondition(c->ifElseStatement.condition) &&
                   CheckTypeClosedStatement(c->ifElseStatement.thenStatement) &&
                   CheckTypeClosedStatement(c->ifElseStatement.elseStatement);
        case CLOSED_WHILE:
            return CheckTypeCondition(c->whileStatement.condition) &&
                   CheckTypeClosedStatement(c->whileStatement.body);
        case CLOSED_FOR:
            return CheckTypeForInitializer(c->forStatement.initializer) &&
                   CheckTypeCondition(c->forStatement.condition) &&
                   CheckTypeForUpdate(c->forStatement.update) &&
                   CheckTypeClosedStatement(c->forStatement.body);
        case CLOSED_FOREVER:
            return CheckTypeClosedStatement(c->foreverStatement.body);
        case CLOSED_STATEMENT_LIST:
            return CheckTypeStatementList(c->closedStatementList.statementList);
    }
    return 0;
}

unsigned int CheckTypeOpenStatement(OpenStatement *o)
{
    if (!o) return 0;
    switch (o->type) {
        case OPEN_IF:
            return CheckTypeCondition(o->ifStatement.condition) &&
                   CheckTypeStatement(o->ifStatement.thenStatement);
        case OPEN_IF_ELSE:
            return CheckTypeCondition(o->ifElseStatement.condition) &&
                   CheckTypeStatement(o->ifElseStatement.thenStatement) &&
                   CheckTypeOpenStatement(o->ifElseStatement.elseStatement);
        case OPEN_WHILE:
            return CheckTypeCondition(o->whileStatement.condition) &&
                   CheckTypeOpenStatement(o->whileStatement.body);
        case OPEN_FOR:
            return CheckTypeForInitializer(o->forStatement.initializer) &&
                   CheckTypeCondition(o->forStatement.condition) &&
                   CheckTypeForUpdate(o->forStatement.update) &&
                   CheckTypeOpenStatement(o->forStatement.body);
    }
    return 0;
}

unsigned int CheckTypeStatement(Statement *s)
{
    if (!s) return 0;
    switch (s->type) {
        case STMT_SIMPLE:
            return CheckTypeSimpleStatement(s->simpleStatement);
        case STMT_IF:
            return CheckTypeCondition(s->ifStatement.condition) &&
                   CheckTypeStatement(s->ifStatement.thenStatement);
        case STMT_IF_ELSE:
            return CheckTypeCondition(s->ifElseStatement.condition) &&
                   CheckTypeStatement(s->ifElseStatement.thenStatement) &&
                   CheckTypeStatement(s->ifElseStatement.elseStatement);
        case STMT_WHILE:
            return CheckTypeCondition(s->whileStatement.condition) &&
                   CheckTypeStatement(s->whileStatement.body);
        case STMT_FOR:
            return CheckTypeForInitializer(s->forStatement.initializer) &&
                   CheckTypeCondition(s->forStatement.condition) &&
                   CheckTypeForUpdate(s->forStatement.update) &&
                   CheckTypeStatement(s->forStatement.body);
        case STMT_FOREVER:
            return CheckTypeStatement(s->foreverStatement.body);
        case STMT_BLOCK:
            return CheckTypeStatementList(s->blockStatement);
    }
    return 0;
}

unsigned int CheckTypeStatementList(StatementList *sl)
{
    while (sl) {
        if (!CheckTypeStatement(sl->statement)) return 0;
        sl = sl->next;
    }
    return 1;
}

unsigned int CheckTypeParameterList(ParameterList *pl)
{
    while (pl) {
        if (!CheckTypeTypeNode(pl->type)) return 0;
        pl = pl->next;
    }
    return 1;
}

unsigned int CheckTypeDeclarationTail(DeclarationTail *dt)
{
    if (!dt) return 0;
    switch (dt->type) {
        case DECL_CONSTANT:
            return CheckTypeConstant(dt->constant);
        case DECL_FUNCTION:
            return CheckTypeParameterList(dt->function.parameterList) &&
                   CheckTypeStatementList(dt->function.statementList);
    }
    return 0;
}

unsigned int CheckTypeDeclarationList(DeclarationList *dl)
{
    while (dl) {
        if (!CheckTypeTypeNode(dl->type)) return 0;
        int cur = scopeStack ? currentScope(scopeStack) : 0;
        if (findSymbol(symbolTable, dl->identifier, cur)) return 0;
        if (!CheckTypeDeclarationTail(dl->declarationTail)) return 0;
        dl = dl->next;
    }
    return 1;
}

unsigned int CheckTypeProgram(Program *p)
{
    return p && CheckTypeDeclarationList(p->globalDeclarations);
}
