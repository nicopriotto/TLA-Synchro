#include "TypeChecking.h"
#include "ScopeStack.h"
#include "../CompilerState.h"
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
        _logger = NULL;
    }
}

/* PRIVATE FUNCTIONS */
static void _logTypeCheckingAction(const char* functionName);
static SymbolEntry* lookupSymbolInScope(const char* identifier);
static boolean isNumericType(SymbolType type);
static boolean isCompatibleAssignment(SymbolType target, SymbolType source);
static SymbolType getExpressionType(Expression* expr);
static SymbolType getConstantType(Constant* constant);

static void _logTypeCheckingAction(const char* functionName) {
    if (_logger) {
        logDebugging(_logger, "TYPE_CHECK: %s", functionName);
    }
}

/**
 * Looks up a symbol starting from current scope and going down to global/builtin scopes
 */
static SymbolEntry* lookupSymbolInScope(const char* identifier) {
    if (!identifier) return NULL;
    
    CompilerState* state = currentCompilerState();
    if (!state) return NULL;
    
    // Search from current scope down to builtin scope (-1)
    int currentScopeId = currentScope(&state->scopeStack);
    
    // First try current scope and all parent scopes
    for (int scope = currentScopeId; scope >= -1; scope--) {
        SymbolEntry* entry = findSymbol(&state->symbolTable, identifier, scope);
        if (entry && entry->scope == scope) {
            if (_logger) {
                logDebugging(_logger, "Found symbol '%s' in scope %d (type: %d)", 
                           identifier, scope, entry->type);
            }
            return entry;
        }
    }
    
    if (_logger) {
        logError(_logger, "Symbol '%s' not found in any accessible scope", identifier);
    }
    return NULL;
}

static boolean isNumericType(SymbolType type) {
    return type == SYMBOL_INTEGER || type == SYMBOL_FLOAT;
}

static boolean isCompatibleAssignment(SymbolType target, SymbolType source) {
    if (target == source) return true;
    
    // Allow int <-> float conversions
    if ((target == SYMBOL_INTEGER && source == SYMBOL_FLOAT) ||
        (target == SYMBOL_FLOAT && source == SYMBOL_INTEGER)) {
        return true;
    }
    
    return false;
}

static SymbolType getConstantType(Constant* constant) {
    if (!constant) return (SymbolType)-1;
    
    switch (constant->type) {
        case CONST_INTEGER: return SYMBOL_INTEGER;
        case CONST_FLOAT: return SYMBOL_FLOAT;
        case CONST_BOOLEAN: return SYMBOL_BOOLEAN;
        case CONST_STRING: return SYMBOL_STRING;
        default: return (SymbolType)-1;
    }
}

static SymbolType getExpressionType(Expression* expr) {
    if (!expr) return (SymbolType)-1;
    
    switch (expr->type) {
        case EXPR_CONSTANT:
            return getConstantType(expr->constant);
            
        case EXPR_IDENTIFIER: {
            SymbolEntry* entry = lookupSymbolInScope(expr->identifier);
            return entry ? entry->type : (SymbolType)-1;
        }
        
        case EXPR_ADD:
        case EXPR_SUB:
        case EXPR_MUL:
        case EXPR_DIV:
        case EXPR_MOD: {
            SymbolType leftType = getExpressionType(expr->binary.leftExpression);
            SymbolType rightType = getExpressionType(expr->binary.rightExpression);
            
            if (!isNumericType(leftType) || !isNumericType(rightType)) {
                return (SymbolType)-1;
            }
            
            // If either operand is float, result is float
            if (leftType == SYMBOL_FLOAT || rightType == SYMBOL_FLOAT) {
                return SYMBOL_FLOAT;
            }
            return SYMBOL_INTEGER;
        }
        
        case EXPR_INCREMENT:
        case EXPR_DECREMENT:
        case EXPR_PRE_INCREMENT:
        case EXPR_PRE_DECREMENT:
            return getExpressionType(expr->unary.expression);
            
        case EXPR_FUNCTION_CALL: {
            // Built-in functions return int, user functions return int by default
            return SYMBOL_INTEGER;
        }
        
        default:
            return (SymbolType)-1;
    }
}

/* PUBLIC IMMEDIATE TYPE CHECKING FUNCTIONS */

boolean CheckTypeImmediate_Constant(Constant* constant) {
    _logTypeCheckingAction(__FUNCTION__);
    
    if (!constant) {
        if (_logger) logError(_logger, "NULL constant in type check");
        return false;
    }
    
    // Constants are always valid by themselves
    return true;
}

boolean CheckTypeImmediate_Expression(Expression* expr) {
    _logTypeCheckingAction(__FUNCTION__);
    
    if (!expr) {
        if (_logger) logError(_logger, "NULL expression in type check");
        return false;
    }
    
    SymbolType exprType = getExpressionType(expr);
    if (exprType == (SymbolType)-1) {
        if (_logger) logError(_logger, "Cannot determine type for expression");
        return false;
    }
    
    // Additional checks based on expression type
    switch (expr->type) {
        case EXPR_IDENTIFIER: {
            SymbolEntry* entry = lookupSymbolInScope(expr->identifier);
            if (!entry) {
                if (_logger) logError(_logger, "Undefined identifier: %s", expr->identifier);
                return false;
            }
            if (entry->type == SYMBOL_FUNCTION) {
                if (_logger) logError(_logger, "Cannot use function '%s' as value", expr->identifier);
                return false;
            }
            break;
        }
        
        case EXPR_FUNCTION_CALL: {
            SymbolEntry* entry = lookupSymbolInScope(expr->functionCall.functionName);
            if (!entry) {
                if (_logger) logError(_logger, "Undefined function: %s", expr->functionCall.functionName);
                return false;
            }
            if (entry->type != SYMBOL_FUNCTION) {
                if (_logger) logError(_logger, "Identifier '%s' is not a function", expr->functionCall.functionName);
                return false;
            }
            
            // Check arguments
            ArgumentList* arg = expr->functionCall.arguments;
            while (arg) {
                if (!CheckTypeImmediate_Expression(arg->expression)) {
                    return false;
                }
                arg = arg->next;
            }
            break;
        }
        
        case EXPR_ADD:
        case EXPR_SUB:
        case EXPR_MUL:
        case EXPR_DIV:
        case EXPR_MOD: {
            if (!CheckTypeImmediate_Expression(expr->binary.leftExpression) ||
                !CheckTypeImmediate_Expression(expr->binary.rightExpression)) {
                return false;
            }
            
            SymbolType leftType = getExpressionType(expr->binary.leftExpression);
            SymbolType rightType = getExpressionType(expr->binary.rightExpression);
            
            if (!isNumericType(leftType) || !isNumericType(rightType)) {
                if (_logger) logError(_logger, "Arithmetic operation requires numeric operands");
                return false;
            }
            break;
        }
        
        case EXPR_INCREMENT:
        case EXPR_DECREMENT:
        case EXPR_PRE_INCREMENT:
        case EXPR_PRE_DECREMENT: {
            if (!CheckTypeImmediate_Expression(expr->unary.expression)) {
                return false;
            }
            
            SymbolType operandType = getExpressionType(expr->unary.expression);
            if (!isNumericType(operandType)) {
                if (_logger) logError(_logger, "Increment/decrement requires numeric operand");
                return false;
            }
            break;
        }
    }
    
    return true;
}

boolean CheckTypeImmediate_Condition(Condition* condition) {
    _logTypeCheckingAction(__FUNCTION__);
    
    if (!condition) {
        if (_logger) logError(_logger, "NULL condition in type check");
        return false;
    }
    
    switch (condition->type) {
        case COND_RELATIONAL: {
            if (!CheckTypeImmediate_Expression(condition->leftValue) ||
                !CheckTypeImmediate_Expression(condition->rightValue)) {
                return false;
            }
            
            SymbolType leftType = getExpressionType(condition->leftValue);
            SymbolType rightType = getExpressionType(condition->rightValue);
            
            if (!isCompatibleAssignment(leftType, rightType) && 
                !isCompatibleAssignment(rightType, leftType)) {
                if (_logger) logError(_logger, "Type mismatch in relational condition");
                return false;
            }
            break;
        }
        
        case COND_AND:
        case COND_OR:
            return CheckTypeImmediate_Condition(condition->logical.leftCondition) &&
                   CheckTypeImmediate_Condition(condition->logical.rightCondition);
                   
        case COND_NOT:
            return CheckTypeImmediate_Condition(condition->not.condition);
            
        case COND_EXPRESSION:
            return CheckTypeImmediate_Expression(condition->expression.expression);
            
        case COND_EMPTY:
            return true;
            
        default:
            if (_logger) logError(_logger, "Unknown condition type: %d", condition->type);
            return false;
    }
    
    return true;
}

boolean CheckTypeImmediate_Assignment(const char* identifier, Expression* expr) {
    _logTypeCheckingAction(__FUNCTION__);
    
    if (!identifier || !expr) {
        if (_logger) logError(_logger, "NULL components in assignment type check");
        return false;
    }
    
    // Check if target variable exists
    SymbolEntry* target = lookupSymbolInScope(identifier);
    if (!target) {
        if (_logger) logError(_logger, "Assignment to undefined variable: %s", identifier);
        return false;
    }
    
    if (target->type == SYMBOL_FUNCTION) {
        if (_logger) logError(_logger, "Cannot assign to function: %s", identifier);
        return false;
    }
    
    // Check expression type
    if (!CheckTypeImmediate_Expression(expr)) {
        return false;
    }
    
    SymbolType exprType = getExpressionType(expr);
    if (!isCompatibleAssignment(target->type, exprType)) {
        if (_logger) logError(_logger, "Type mismatch in assignment to '%s': expected %d, got %d", 
                   identifier, target->type, exprType);
        return false;
    }
    
    return true;
}

boolean CheckTypeImmediate_VariableDeclaration(TypeNode* type, const char* identifier, Condition* initCondition) {
    _logTypeCheckingAction(__FUNCTION__);
    
    if (!type || !identifier) {
        if (_logger) logError(_logger, "NULL components in variable declaration type check");
        return false;
    }
    
    // Check initialization condition if present
    if (initCondition && !CheckTypeImmediate_Condition(initCondition)) {
        return false;
    }
    
    // If there's an initialization, check type compatibility
    if (initCondition && initCondition->type == COND_EXPRESSION) {
        SymbolType declaredType;
        switch (type->type) {
            case TYPE_INTEGER: declaredType = SYMBOL_INTEGER; break;
            case TYPE_FLOAT: declaredType = SYMBOL_FLOAT; break;
            case TYPE_BOOLEAN: declaredType = SYMBOL_BOOLEAN; break;
            case TYPE_STRING: declaredType = SYMBOL_STRING; break;
            case TYPE_SEM: declaredType = SYMBOL_SEMAPHORE; break;
            default: 
                if (_logger) logError(_logger, "Unknown type in declaration");
                return false;
        }
        
        SymbolType initType = getExpressionType(initCondition->expression.expression);
        if (!isCompatibleAssignment(declaredType, initType)) {
            if (_logger) logError(_logger, "Type mismatch in variable '%s' initialization: declared %d, got %d", 
                       identifier, declaredType, initType);
            return false;
        }
    }
    
    return true;
}

boolean CheckTypeImmediate_FunctionCall(const char* functionName, ArgumentList* arguments) {
    _logTypeCheckingAction(__FUNCTION__);
    
    if (!functionName) {
        if (_logger) logError(_logger, "NULL function name in function call type check");
        return false;
    }
    
    // Check if function exists
    SymbolEntry* func = lookupSymbolInScope(functionName);
    if (!func) {
        if (_logger) logError(_logger, "Undefined function: %s", functionName);
        return false;
    }
    
    if (func->type != SYMBOL_FUNCTION) {
        if (_logger) logError(_logger, "Identifier '%s' is not a function", functionName);
        return false;
    }
    
    // Check all arguments
    ArgumentList* arg = arguments;
    int argCount = 0;
    while (arg) {
        argCount++;
        if (!CheckTypeImmediate_Expression(arg->expression)) {
            if (_logger) logError(_logger, "Type error in argument %d of function '%s'", argCount, functionName);
            return false;
        }
        arg = arg->next;
    }
    
    return true;
}

boolean CheckTypeImmediate_IncrementDecrement(const char* identifier) {
    _logTypeCheckingAction(__FUNCTION__);
    
    if (!identifier) {
        if (_logger) logError(_logger, "NULL identifier in increment/decrement type check");
        return false;
    }
    
    SymbolEntry* entry = lookupSymbolInScope(identifier);
    if (!entry) {
        if (_logger) logError(_logger, "Undefined variable in increment/decrement: %s", identifier);
        return false;
    }
    
    if (!isNumericType(entry->type)) {
        if (_logger) logError(_logger, "Cannot increment/decrement non-numeric variable: %s", identifier);
        return false;
    }
    
    return true;
}

/* LEGACY COMPATIBILITY FUNCTIONS - These now call the immediate versions */
unsigned int CheckTypeConstant(Constant *c) {
    return CheckTypeImmediate_Constant(c) ? 1 : 0;
}

unsigned int CheckTypeExpression(Expression *e) {
    return CheckTypeImmediate_Expression(e) ? 1 : 0;
}

unsigned int CheckTypeCondition(Condition *c) {
    return CheckTypeImmediate_Condition(c) ? 1 : 0;
}

// Simplified versions of other functions for compatibility
unsigned int CheckTypeArgumentList(ArgumentList *a) {
    while (a) {
        if (!CheckTypeImmediate_Expression(a->expression)) {
            return 0;
        }
        a = a->next;
    }
    return 1;
}

unsigned int CheckTypeFunctionIdentifier(FunctionIdentifier *f) {
    if (!f) return 0;
    
    if (f->type == FUNC_USER_DEFINED) {
        SymbolEntry* entry = lookupSymbolInScope(f->identifier);
        return (entry && entry->type == SYMBOL_FUNCTION) ? 1 : 0;
    }
    
    return 1; // Built-in functions are always valid
}

unsigned int CheckTypeVariableDeclaration(VariableDeclaration *v) {
    if (!v) return 0;
    return CheckTypeImmediate_VariableDeclaration(v->type, v->identifier, v->condition) ? 1 : 0;
}

unsigned int CheckTypeForInitializer(ForInitializer *fi) {
    while (fi) {
        if (!CheckTypeVariableDeclaration(fi->declaration)) {
            return 0;
        }
        fi = fi->next;
    }
    return 1;
}

unsigned int CheckTypeForUpdate(ForUpdate *fu) {
    // For updates are simple statements, we'll check them when they're created
    return 1;
}

unsigned int CheckTypeStatementList(StatementList *sl) {
    // Statements are checked when created, so this is just a placeholder
    return 1;
}
