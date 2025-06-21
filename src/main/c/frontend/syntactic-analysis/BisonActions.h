#ifndef BISON_ACTIONS_H
#define BISON_ACTIONS_H

#include "../../shared/CompilerState.h"
#include "../../shared/Logger.h"
#include "../../shared/Type.h"
#include "AbstractSyntaxTree.h"

/** Initialize module. */
void initializeBisonActionsModule();

/** Shutdown module. */
void shutdownBisonActionsModule();

/**
 * This function is called when the parser encounters a declaration header
 * (type + identifier) BEFORE parsing the declaration tail. This allows
 * forward references to work properly.
 */
void registerDeclarationHeader(TypeNode* type, char* identifier);

/* Scope management functions */
void enterScope();
void exitScope();

/* Grammar-level scope management functions */
void enterBlockScope();
void exitBlockScope();

/* Constant semantic actions. */
Constant* IntegerConstantSemanticAction(const int value);
Constant* FloatConstantSemanticAction(const float value);
Constant* BooleanConstantSemanticAction(const boolean value);
Constant* StringConstantSemanticAction(char* value);

/* Expression semantic actions. */
Expression* ConstantExpressionSemanticAction(Constant* constant);
Expression* IdentifierExpressionSemanticAction(char* identifier);
Expression* BinaryExpressionSemanticAction(Expression* leftExpression, Expression* rightExpression, ExpressionType type);
Expression* UnaryExpressionSemanticAction(Expression* subExpression, ExpressionType type);
Expression* FunctionCallExpressionSemanticAction(char* functionName, ArgumentList* arguments);

/* Condition semantic actions. */
Condition* RelationalConditionSemanticAction(Expression* leftValue, RelationalOperator* operator, Expression* rightValue);
Condition* NotConditionSemanticAction(Condition* subCondition);
Condition* LogicalConditionSemanticAction(Condition* leftCondition, Condition* rightCondition, int logicalType);
Condition* EmptyConditionSemanticAction();
Condition* ExpressionAsConditionSemanticAction(Expression* expression);

/* Relational operator semantic actions. */
RelationalOperator* RelationalOperatorSemanticAction(RelationalOperatorType type);

/* Type node semantic actions. */
TypeNode* TypeNodeSemanticAction(TypeNodeType type);

/* Declaration semantic actions. */
DeclarationTail* DeclarationSemanticAction(Constant* value);
DeclarationList* DeclarationListSemanticAction(TypeNode* type, char* identifier, DeclarationTail* declarationTail, DeclarationList* next);

/* Variable declaration semantic actions. */
VariableDeclaration* VariableDeclarationSemanticActionCondition(TypeNode* type, char* identifier, Condition* condition);

/* Function identifier semantic actions. */
FunctionIdentifier* FunctionIdentifierSemanticAction(FunctionIdentifierType type, char* identifier);

/* Parameter and argument semantic actions. */
ParameterList* ParameterListSemanticAction(TypeNode* type, char* identifier, ParameterList* nextParameters);
ArgumentList* ArgumentListSemanticAction(Expression* expression, ArgumentList* nextArguments);

/* For loop semantic actions. */
ForInitializer* ForInitializerSemanticAction(VariableDeclaration* declaration, ForInitializer* nextInitializers);
ForUpdate* ForUpdateSemanticAction(SimpleStatement* statement, ForUpdate* nextUpdates);

/* Simple statement semantic actions. */
SimpleStatement* FunctionCallSimpleStatementSemanticAction(FunctionIdentifier* function, ArgumentList* arguments);
SimpleStatement* IncrementSimpleStatementSemanticAction(char* identifier, boolean isPrefix);
SimpleStatement* DecrementSimpleStatementSemanticAction(char* identifier, boolean isPrefix);
SimpleStatement* AssignmentSimpleStatementSemanticAction(char* identifier, Expression* expression);
SimpleStatement* DeclarationSimpleStatementSemanticAction(VariableDeclaration* declaration);
SimpleStatement* ReturnConstantSimpleStatementSemanticAction(Constant* constant);
SimpleStatement* ReturnIdentifierSimpleStatementSemanticAction(char* identifier);

/* Open statement semantic actions. */
OpenStatement* IfOpenStatementSemanticAction(Condition* condition, Statement* thenStatement);
OpenStatement* IfElseOpenStatementSemanticAction(Condition* condition, Statement* thenStatement, OpenStatement* elseStatement);
OpenStatement* WhileOpenStatementSemanticAction(Condition* condition, OpenStatement* body);
OpenStatement* ForOpenStatementSemanticAction(ForInitializer* initializer, Condition* condition, ForUpdate* update, OpenStatement* body);

/* Closed statement semantic actions. */
ClosedStatement* SimpleClosedStatementSemanticAction(SimpleStatement* simpleStatement);
ClosedStatement* ClosedListStatementSemanticAction(StatementList* statementList);
ClosedStatement* IfElseClosedStatementSemanticAction(Condition* condition, ClosedStatement* thenStatement, ClosedStatement* elseStatement);
ClosedStatement* WhileClosedStatementSemanticAction(Condition* condition, ClosedStatement* body);
ClosedStatement* ForClosedStatementSemanticAction(ForInitializer* initializer, Condition* condition, ForUpdate* update, ClosedStatement* body);
ClosedStatement* ForeverClosedStatementSemanticAction(ClosedStatement* body);

/* Statement semantic actions. */
Statement* OpenStatementSemanticAction(OpenStatement* openStatement);
Statement* ClosedStatementSemanticAction(ClosedStatement* closedStatement);
StatementList* StatementListSemanticAction(Statement* statement, StatementList* nextStatements);

/* Function semantic actions. */
DeclarationTail* FunctionSemanticAction(ParameterList* parameters, StatementList* body);
FunctionList* FunctionListSemanticAction(Function* function, FunctionList* nextFunctions);

/* Program semantic action. */
Program* ProgramSemanticAction(DeclarationList* globalDeclarations, CompilerState* compilerState);
static void updateSymbolToFunction(const char* identifier);

#endif
