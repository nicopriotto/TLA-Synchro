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

#define SAFE_FREE(ptr) do { free(ptr); ptr = NULL; } while (0)

/** PUBLIC FUNCTIONS */

void releaseConstant(Constant* constant) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (constant != NULL) {
		if (constant->type == CONST_STRING) {
			SAFE_FREE(constant->string);
		}
		SAFE_FREE(constant);
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
				SAFE_FREE(expression->identifier);
				break;
			case EXPR_ADD: case EXPR_SUB: case EXPR_MUL:
			case EXPR_DIV: case EXPR_MOD:
				releaseExpression(expression->leftExpression);
				releaseExpression(expression->rightExpression);
				break;
			case EXPR_INCREMENT: case EXPR_DECREMENT:
			case EXPR_PRE_INCREMENT: case EXPR_PRE_DECREMENT:
				releaseExpression(expression->expression);
				break;
			case EXPR_FUNCTION_CALL:
				SAFE_FREE(expression->functionCall.functionName);
				releaseArgumentList(expression->functionCall.arguments);
				break;
		}
		SAFE_FREE(expression);
	}
}

void releaseRelationalOperator(RelationalOperator* op) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	SAFE_FREE(op);
}

void releaseSimpleStatement(SimpleStatement* ss) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (ss != NULL) {
		switch (ss->type) {
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
}

void releaseOpenStatement(OpenStatement* os) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (os != NULL) {
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
}

void releaseClosedStatement(ClosedStatement* cs) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (cs != NULL) {
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
}

void releaseStatement(Statement* stmt) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (stmt != NULL) {
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
}

void releaseStatementList(StatementList* sl) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (sl != NULL) {
		releaseStatement(sl->statement);
		releaseStatementList(sl->next);
		SAFE_FREE(sl);
	}
}

void releaseCondition(Condition* cond) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (cond != NULL) {
		switch (cond->type) {
			case COND_RELATIONAL:
				releaseExpression(cond->leftValue);
				releaseExpression(cond->rightValue);
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
}

void releaseParameterList(ParameterList* pl) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (pl != NULL) {
		releaseTypeNode(pl->type);
		SAFE_FREE(pl->identifier);
		releaseParameterList(pl->next);
		SAFE_FREE(pl);
	}
}

void releaseArgumentList(ArgumentList* al) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (al != NULL) {
		releaseExpression(al->expression);
		releaseArgumentList(al->next);
		SAFE_FREE(al);
	}
}

void releaseDeclarationTail(DeclarationTail* dt) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (dt != NULL) {
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
}

void releaseDeclarationList(DeclarationList* dl) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (dl != NULL) {
		DeclarationList* next = dl->next;
		releaseTypeNode(dl->type);
		releaseDeclarationTail(dl->declarationTail);
		SAFE_FREE(dl->identifier); // CHECK IT!!!!
		SAFE_FREE(dl);
		releaseDeclarationList(next);
	}
}

void releaseTypeNode(TypeNode* tn) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	SAFE_FREE(tn);
}

void releaseFunction(Function* fn) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (fn != NULL) {
		releaseParameterList(fn->parameters);
		releaseStatementList(fn->body);
		SAFE_FREE(fn);
	}
}

void releaseFunctionList(FunctionList* fl) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (fl != NULL) {
		releaseFunction(fl->function);
		releaseFunctionList(fl->next);
		SAFE_FREE(fl);
	}
}

void releaseFunctionIdentifier(FunctionIdentifier* fi) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (fi != NULL) {
		if (fi->type == FUNC_USER_DEFINED) {
			SAFE_FREE(fi->identifier);
		}
		SAFE_FREE(fi);
	}
}

void releaseForInitializer(ForInitializer* fi) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (fi != NULL) {
		releaseVariableDeclaration(fi->declaration);
		releaseForInitializer(fi->next);
		SAFE_FREE(fi);
	}
}

void releaseForUpdate(ForUpdate* fu) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (fu != NULL) {
		releaseSimpleStatement(fu->statement);
		releaseForUpdate(fu->next);
		SAFE_FREE(fu);
	}
}

void releaseVariableDeclaration(VariableDeclaration* vd) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (vd != NULL) {
		releaseTypeNode(vd->type);
		SAFE_FREE(vd->identifier);
		releaseCondition(vd->condition);
		releaseExpression(vd->expression);
		SAFE_FREE(vd);
	}
}

void releaseProgram(Program* p) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (p != NULL) {
		releaseDeclarationList(p->globalDeclarations);
		SAFE_FREE(p);
	}
}