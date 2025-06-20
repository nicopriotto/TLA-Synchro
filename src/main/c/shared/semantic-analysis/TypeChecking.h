#ifndef TYPE_CHECKING_HEADER
#define TYPE_CHECKING_HEADER

#include "../../frontend/syntactic-analysis/AbstractSyntaxTree.h"
#include "SymbolTable.h"
#include "../../frontend/syntactic-analysis/SyntacticAnalyzer.h"

#include "../Logger.h"
#include "../CompilerState.h"

void initializeTypeCheckingModule();
void shutdownTypeCheckingModule();

unsigned int CheckTypeProgram(Program *program);
unsigned int CheckTypeStatement(Statement *statement);
unsigned int CheckTypeStatementList(StatementList *statementList);
unsigned int CheckTypeExpression(Expression *expression);
unsigned int CheckTypeCondition(Condition *condition);
unsigned int CheckTypeArgumentList(ArgumentList *argumentList);
unsigned int CheckTypeDeclarationList(DeclarationList *declarationList);
unsigned int CheckTypeTypeNode(TypeNode *typeNode);
unsigned int CheckTypeConstant(Constant *constant);
unsigned int CheckTypeOpenStatement(OpenStatement *openStatement);
unsigned int CheckTypeClosedStatement(ClosedStatement *closedStatement);
unsigned int CheckTypeSimpleStatement(SimpleStatement *simpleStatement);
unsigned int CheckTypeFunctionIdentifier(FunctionIdentifier *functionIdentifier);
unsigned int CheckTypeForInitializer(ForInitializer *forInitializer);
unsigned int CheckTypeForUpdate(ForUpdate *forUpdate);
unsigned int CheckTypeVariableDeclaration(VariableDeclaration *variableDeclaration);
unsigned int CheckTypeRelationalOperator(RelationalOperator *relationalOperator);
unsigned int CheckTypeDeclarationTail(DeclarationTail *declarationTail);
unsigned int CheckTypeParameterList(ParameterList *parameterList);

#endif
