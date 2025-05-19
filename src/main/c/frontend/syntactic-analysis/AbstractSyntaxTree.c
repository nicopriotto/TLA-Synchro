#include "AbstractSyntaxTree.h"

/* MODULE INTERNAL STATE */

static Logger * _logger = NULL;

void initializeAbstractSyntaxTreeModule() {
	_logger = createLogger("AbstractSyntxTree");
}

void shutdownAbstractSyntaxTreeModule() {
	if (_logger != NULL) {
		destroyLogger(_logger);
	}
}

/** PUBLIC FUNCTIONS */

void releaseConstant(Constant* constant) {
    logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
    if (constant != NULL) {
        if (constant->type == CONST_STRING && constant->string != NULL) {
            free(constant->string);
        }
        free(constant);
    }
}

void releaseExpression(Expression* expression) {
    logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
    if (expression != NULL) {
        switch (expression->type) {
            case EXPR_CONSTANT:
                releaseConstant(expression->constant);
                break;
            case EXPR_IDENTIFIER:
                if (expression->identifier != NULL) {
                    free(expression->identifier);
                }
                break;
            case EXPR_ADD:
            case EXPR_SUB:
            case EXPR_MUL:
            case EXPR_DIV:
            case EXPR_MOD:
                releaseExpression(expression->leftExpression);
                releaseExpression(expression->rightExpression);
                break;
            case EXPR_INCREMENT:
            case EXPR_DECREMENT:
            case EXPR_PRE_INCREMENT:
            case EXPR_PRE_DECREMENT:
                releaseExpression(expression->expression);
                break;
            case EXPR_FUNCTION_CALL:
                if (expression->functionCall.functionName != NULL) {
                    free(expression->functionCall.functionName);
                }
                releaseArgumentList(expression->functionCall.arguments);
                break;
        }
        free(expression);
    }
}

void releaseValue(Value* value) {
    logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
    if (value != NULL) {
        switch (value->type) {
            case VAL_EXPRESSION:
                releaseExpression(value->expression);
                break;
            case VAL_CONDITION:
                releaseCondition(value->condition);
                break;
        }
        free(value);
    }
}

void releaseSimpleStatement(SimpleStatement* simpleStatement) {
    logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
    if (simpleStatement != NULL) {
        switch (simpleStatement->type) {
            case SIMPLE_FUNCTION_CALL:
                releaseFunctionIdentifier(simpleStatement->functionCall.function);
                releaseArgumentList(simpleStatement->functionCall.arguments);
                break;
            case SIMPLE_INCREMENT:
                if (simpleStatement->increment.identifier != NULL) {
                    free(simpleStatement->increment.identifier);
                }
                break;
            case SIMPLE_DECREMENT:
                if (simpleStatement->decrement.identifier != NULL) {
                    free(simpleStatement->decrement.identifier);
                }
                break;
            case SIMPLE_ASSIGNMENT:
                if (simpleStatement->assignment.identifier != NULL) {
                    free(simpleStatement->assignment.identifier);
                }
                releaseExpression(simpleStatement->assignment.expression);
                break;
            case SIMPLE_DECLARATION:
                releaseVariableDeclaration(simpleStatement->declaration);
                break;
        }
        free(simpleStatement);
    }
}

void releaseOpenStatement(OpenStatement* openStatement) {
    logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
    if (openStatement != NULL) {
        switch (openStatement->type) {
            case OPEN_IF:
                releaseExpression(openStatement->ifStatement.condition);
                releaseStatement(openStatement->ifStatement.thenStatement);
                break;
            case OPEN_IF_ELSE:
                releaseExpression(openStatement->ifElseStatement.condition);
                releaseStatement(openStatement->ifElseStatement.thenStatement);
                releaseOpenStatement(openStatement->ifElseStatement.elseStatement);
                break;
            case OPEN_WHILE:
                releaseExpression(openStatement->whileStatement.condition);
                releaseOpenStatement(openStatement->whileStatement.body);
                break;
            case OPEN_FOR:
                releaseForInitializer(openStatement->forStatement.initializer);
                releaseCondition(openStatement->forStatement.condition);
                releaseForUpdate(openStatement->forStatement.update);
                releaseOpenStatement(openStatement->forStatement.body);
                break;
            case OPEN_FOREVER:
                releaseOpenStatement(openStatement->foreverStatement.body);
                break;
        }
        free(openStatement);
    }
}

void releaseClosedStatement(ClosedStatement* closedStatement) {
    logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
    if (closedStatement != NULL) {
        switch (closedStatement->type) {
            case CLOSED_SIMPLE:
                releaseSimpleStatement(closedStatement->simpleStatement);
                break;
            case CLOSED_IF_ELSE:
                releaseExpression(closedStatement->ifElseStatement.condition);
                releaseClosedStatement(closedStatement->ifElseStatement.thenStatement);
                releaseClosedStatement(closedStatement->ifElseStatement.elseStatement);
                break;
            case CLOSED_IF_BLOCK:
                releaseExpression(closedStatement->ifBlock.condition);
                releaseStatementList(closedStatement->ifBlock.body);
                break;
            case CLOSED_WHILE:
                releaseExpression(closedStatement->whileStatement.condition);
                releaseClosedStatement(closedStatement->whileStatement.body);
                break;
            case CLOSED_WHILE_BLOCK:
                releaseExpression(closedStatement->whileBlock.condition);
                releaseStatementList(closedStatement->whileBlock.body);
                break;
            case CLOSED_FOR:
                releaseForInitializer(closedStatement->forStatement.initializer);
                releaseCondition(closedStatement->forStatement.condition);
                releaseForUpdate(closedStatement->forStatement.update);
                releaseClosedStatement(closedStatement->forStatement.body);
                break;
            case CLOSED_FOR_BLOCK:
                releaseForInitializer(closedStatement->forBlock.initializer);
                releaseCondition(closedStatement->forBlock.condition);
                releaseForUpdate(closedStatement->forBlock.update);
                releaseStatementList(closedStatement->forBlock.body);
                break;
            case CLOSED_FOREVER:
                releaseClosedStatement(closedStatement->foreverStatement.body);
                break;
            case CLOSED_FOREVER_BLOCK:
                releaseStatementList(closedStatement->foreverBlock.body);
                break;
        }
        free(closedStatement);
    }
}

void releaseStatement(Statement* statement) {
    logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
    if (statement != NULL) {
        switch (statement->type) {
            case STMT_SIMPLE:
                releaseSimpleStatement(statement->simpleStatement);
                break;
            case STMT_IF:
                releaseExpression(statement->ifStatement.condition);
                releaseStatement(statement->ifStatement.thenStatement);
                break;
            case STMT_IF_ELSE:
                releaseExpression(statement->ifElseStatement.condition);
                releaseStatement(statement->ifElseStatement.thenStatement);
                releaseStatement(statement->ifElseStatement.elseStatement);
                break;
            case STMT_WHILE:
                releaseExpression(statement->whileStatement.condition);
                releaseStatement(statement->whileStatement.body);
                break;
            case STMT_FOR:
                releaseForInitializer(statement->forStatement.initializer);
                releaseCondition(statement->forStatement.condition);
                releaseForUpdate(statement->forStatement.update);
                releaseStatement(statement->forStatement.body);
                break;
            case STMT_FOREVER:
                releaseStatement(statement->foreverStatement.body);
                break;
        }
        free(statement);
    }
}

void releaseStatementList(StatementList* statementList) {
    logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
    if (statementList != NULL) {
        releaseStatement(statementList->statement);
        releaseStatementList(statementList->next);
        free(statementList);
    }
}

void releaseCondition(Condition* condition) {
    logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
    if (condition != NULL) {
        switch (condition->type) {
            case COND_RELATIONAL:
                releaseValue(condition->leftValue);
                releaseValue(condition->rightValue);
                break;
            case COND_NOT:
				releaseCondition(condition->not.condition);
                break;
            case COND_PARENTHESIS:
                releaseCondition(condition->parenthesis.condition);
                break;
            case COND_AND:
            case COND_OR:
                releaseCondition(condition->logical.leftCondition);
                releaseCondition(condition->logical.rightCondition);
                break;
            case COND_EMPTY:
                // Nothing to release
                break;
        }
        free(condition);
    }
}

void releaseParameterList(ParameterList* parameterList) {
    logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
    if (parameterList != NULL) {
        releaseTypeNode(parameterList->type);
        if (parameterList->identifier != NULL) {
            free(parameterList->identifier);
        }
        releaseParameterList(parameterList->next);
        free(parameterList);
    }
}

void releaseArgumentList(ArgumentList* argumentList) {
    logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
    if (argumentList != NULL) {
        releaseExpression(argumentList->expression);
        releaseArgumentList(argumentList->next);
        free(argumentList);
    }
}

void releaseDeclaration(Declaration* declaration) {
    logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
    if (declaration != NULL) {
        releaseTypeNode(declaration->type);
        if (declaration->identifier != NULL) {
            free(declaration->identifier);
        }
        releaseConstant(declaration->value);
        free(declaration);
    }
}

void releaseDeclarationList(DeclarationList* declarationList) {
    logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
    if (declarationList != NULL) {
        releaseDeclaration(declarationList->declaration);
        releaseDeclarationList(declarationList->next);
        free(declarationList);
    }
}

void releaseTypeNode(TypeNode* typeNode) {
    logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
    if (typeNode != NULL) {
        free(typeNode);
    }
}

void releaseFunction(Function* function) {
    logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
    if (function != NULL) {
        if (function->name != NULL) {
            free(function->name);
        }
        releaseParameterList(function->parameters);
        releaseStatementList(function->body);
        free(function);
    }
}

void releaseFunctionList(FunctionList* functionList) {
    logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
    if (functionList != NULL) {
        releaseFunction(functionList->function);
        releaseFunctionList(functionList->next);
        free(functionList);
    }
}

void releaseFunctionIdentifier(FunctionIdentifier* functionIdentifier) {
    logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
    if (functionIdentifier != NULL) {
        if (functionIdentifier->type == FUNC_USER_DEFINED && functionIdentifier->identifier != NULL) {
            free(functionIdentifier->identifier);
        }
        free(functionIdentifier);
    }
}

void releaseForInitializer(ForInitializer* forInitializer) {
    logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
    if (forInitializer != NULL) {
        releaseVariableDeclaration(forInitializer->declaration);
        releaseForInitializer(forInitializer->next);
        free(forInitializer);
    }
}

void releaseForUpdate(ForUpdate* forUpdate) {
    logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
    if (forUpdate != NULL) {
        releaseSimpleStatement(forUpdate->statement);
        releaseForUpdate(forUpdate->next);
        free(forUpdate);
    }
}

void releaseVariableDeclaration(VariableDeclaration* variableDeclaration) {
    logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
    if (variableDeclaration != NULL) {
        releaseTypeNode(variableDeclaration->type);
        if (variableDeclaration->identifier != NULL) {
            free(variableDeclaration->identifier);
        }
        releaseValue(variableDeclaration->value);
        free(variableDeclaration);
    }
}

void releaseProgram(Program* program) {
    logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
    if (program != NULL) {
        releaseDeclarationList(program->globalDeclarations);
        releaseFunctionList(program->functions);
        free(program);
    }
}