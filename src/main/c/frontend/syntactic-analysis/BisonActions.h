#ifndef BISON_ACTIONS_HEADER
#define BISON_ACTIONS_HEADER

#include "../../shared/CompilerState.h"
#include "../../shared/Logger.h"
#include "../../shared/Type.h"
#include "AbstractSyntaxTree.h"
#include "SyntacticAnalyzer.h"
#include <stdlib.h>

/** Initialize and teardown module state. */
void initializeBisonActionsModule(void);
void shutdownBisonActionsModule(void);

/** Constants */
Constant* IntegerConstantSemanticAction(const int value);
Constant* FloatConstantSemanticAction(const float value);
Constant* BooleanConstantSemanticAction(const boolean value);
Constant* StringConstantSemanticAction(char* value);

/** Expressions */
Expression* ConstantExpressionSemanticAction(Constant* constant);
Expression* IdentifierExpressionSemanticAction(char* identifier);
Expression* BinaryExpressionSemanticAction(Expression* leftExpression, Expression* rightExpression, ExpressionType type);
Expression* UnaryExpressionSemanticAction(Expression* subExpression, ExpressionType type);
Expression* FunctionCallExpressionSemanticAction(char* functionName, ArgumentList* arguments);

/** Conditions */
Condition* RelationalConditionSemanticAction(Expression* leftValue, RelationalOperator* operator, Expression* rightValue);
Condition* NotConditionSemanticAction(Condition* subCondition);
Condition* LogicalConditionSemanticAction(Condition* leftCondition, Condition* rightCondition, int logicalType);
Condition* EmptyConditionSemanticAction(void);
Condition* ExpressionAsConditionSemanticAction(Expression* expression);

/** Relational Operators */
RelationalOperator* RelationalOperatorSemanticAction(RelationalOperatorType operatorType);

/** Type nodes */
TypeNode* TypeNodeSemanticAction(TypeNodeType type);

/** Declarations */
DeclarationTail* DeclarationSemanticAction(Constant* value);
DeclarationList* DeclarationListSemanticAction(TypeNode* type, char* identifier, DeclarationTail* declarationTail, DeclarationList* next);

/** Variable Declarations */
VariableDeclaration* VariableDeclarationSemanticActionCondition(TypeNode* type, char* identifier, Condition* value);



/** Function identifiers */
FunctionIdentifier* FunctionIdentifierSemanticAction(FunctionIdentifierType type, char* identifier);

/** Parameter & argument lists */
ParameterList* ParameterListSemanticAction(TypeNode* type, char* identifier, ParameterList* nextParameters);
ArgumentList* ArgumentListSemanticAction(Expression* expression, ArgumentList* nextArguments);

/** For‐loop components */
ForInitializer* ForInitializerSemanticAction(VariableDeclaration* declaration, ForInitializer* nextInitializers);
ForUpdate* ForUpdateSemanticAction(SimpleStatement* statement, ForUpdate* nextUpdates);

/** Simple statements */
SimpleStatement* FunctionCallSimpleStatementSemanticAction(FunctionIdentifier* function, ArgumentList* arguments);
SimpleStatement* IncrementSimpleStatementSemanticAction(char* identifier, boolean isPrefix);
SimpleStatement* DecrementSimpleStatementSemanticAction(char* identifier, boolean isPrefix);
SimpleStatement* AssignmentSimpleStatementSemanticAction(char* identifier, Expression* expression);
SimpleStatement* DeclarationSimpleStatementSemanticAction(VariableDeclaration* declaration);
SimpleStatement* ReturnConstantSimpleStatementSemanticAction(Constant* constant);
SimpleStatement* ReturnIdentifierSimpleStatementSemanticAction(char* identifier);

/** Open statements (`if`, `while`, `for`, `forever`) */
OpenStatement* IfOpenStatementSemanticAction(Condition* condition, Statement* thenStatement);
OpenStatement* IfElseOpenStatementSemanticAction(Condition* condition, Statement* thenStatement, OpenStatement* elseStatement);
OpenStatement* WhileOpenStatementSemanticAction(Condition* condition, OpenStatement* body);
OpenStatement* ForOpenStatementSemanticAction(ForInitializer* initializer, Condition* condition, ForUpdate* update, OpenStatement* body);

/** Closed statements (terminated by newline or explicit block) */
ClosedStatement* SimpleClosedStatementSemanticAction(SimpleStatement* simple);
ClosedStatement* ClosedListStatementSemanticAction(StatementList* statementList);
ClosedStatement* IfElseClosedStatementSemanticAction(Condition* condition, ClosedStatement* thenStmt, ClosedStatement* elseStmt);
ClosedStatement* IfElseBracesClosedStatementSemanticAction(Condition* condition, ClosedStatement* thenStmt, StatementList* elseStmt);
ClosedStatement* IfBlockClosedStatementSemanticAction(Condition* condition, StatementList* body);
ClosedStatement* IfElseBlockClosedStatementSemanticAction(Condition* condition, StatementList* body, StatementList* elseBody);
ClosedStatement* WhileClosedStatementSemanticAction(Condition* condition, ClosedStatement* body);
ClosedStatement* ForClosedStatementSemanticAction(ForInitializer* initializer, Condition* condition, ForUpdate* update, ClosedStatement* body);
ClosedStatement* ForeverClosedStatementSemanticAction(ClosedStatement* body);

/** General statements & statement lists */
Statement* OpenStatementSemanticAction(OpenStatement* openStmt);
Statement* ClosedStatementSemanticAction(ClosedStatement* closedStmt);
StatementList* StatementListSemanticAction(Statement* stmt, StatementList* nextStatements);

/** Functions & program */ 
DeclarationTail* FunctionSemanticAction(ParameterList* parameters, StatementList* body);
FunctionList* FunctionListSemanticAction(Function* function, FunctionList* nextFunctions);
Program* ProgramSemanticAction(DeclarationList* globalDeclarations, CompilerState* compilerState);

#endif /* BISON_ACTIONS_HEADER */
