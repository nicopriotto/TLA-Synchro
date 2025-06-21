#ifndef TYPE_CHECKING_HEADER
#define TYPE_CHECKING_HEADER

#include "../Logger.h"
#include "../Type.h"
#include "../../frontend/syntactic-analysis/AbstractSyntaxTree.h"

/** Initialize module's internal state. */
void initializeTypeCheckingModule();

/** Shutdown module's internal state. */
void shutdownTypeCheckingModule();

/* IMMEDIATE TYPE CHECKING FUNCTIONS */
boolean CheckTypeImmediate_Constant(Constant* constant);
boolean CheckTypeImmediate_Expression(Expression* expr);
boolean CheckTypeImmediate_Condition(Condition* condition);
boolean CheckTypeImmediate_Assignment(const char* identifier, Expression* expr);
boolean CheckTypeImmediate_VariableDeclaration(TypeNode* type, const char* identifier, Condition* initCondition);
boolean CheckTypeImmediate_FunctionCall(const char* functionName, ArgumentList* arguments);
boolean CheckTypeImmediate_IncrementDecrement(const char* identifier);

/* LEGACY COMPATIBILITY FUNCTIONS */
unsigned int CheckTypeConstant(Constant *c);
unsigned int CheckTypeTypeNode(TypeNode *t);
unsigned int CheckTypeRelationalOperator(RelationalOperator *r);
unsigned int CheckTypeExpression(Expression *e);
unsigned int CheckTypeCondition(Condition *c);
unsigned int CheckTypeArgumentList(ArgumentList *a);
unsigned int CheckTypeFunctionIdentifier(FunctionIdentifier *f);
unsigned int CheckTypeVariableDeclaration(VariableDeclaration *v);
unsigned int CheckTypeForInitializer(ForInitializer *fi);
unsigned int CheckTypeForUpdate(ForUpdate *fu);
unsigned int CheckTypeSimpleStatement(SimpleStatement *s);
unsigned int CheckTypeClosedStatement(ClosedStatement *c);
unsigned int CheckTypeOpenStatement(OpenStatement *o);
unsigned int CheckTypeStatement(Statement *s);
unsigned int CheckTypeStatementList(StatementList *sl);
unsigned int CheckTypeParameterList(ParameterList *pl);
unsigned int CheckTypeDeclarationTail(DeclarationTail *dt);
unsigned int CheckTypeDeclarationList(DeclarationList *dl);
unsigned int CheckTypeProgram(Program *p);

#endif
