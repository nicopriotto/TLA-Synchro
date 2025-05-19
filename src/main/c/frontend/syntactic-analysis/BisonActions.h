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

/** Values */
Value* ExpressionValueSemanticAction(Expression* expression);
Value* ConditionValueSemanticAction(Condition* condition);

/** Conditions */
Condition* RelationalConditionSemanticAction(Value* leftValue, RelationalOperatorType operator, Value* rightValue);
Condition* NotConditionSemanticAction(Condition* subCondition);
Condition* LogicalConditionSemanticAction(Condition* leftCondition, Condition* rightCondition, int logicalType);
Condition* ParenthesisConditionSemanticAction(Condition* subCondition);
Condition* EmptyConditionSemanticAction(void);

/** Relational Operators */
RelationalOperator* RelationalOperatorSemanticAction(RelationalOperatorType operatorType);

/** Type nodes */
TypeNode* TypeNodeSemanticAction(TypeNodeType type);

/** Declarations */
Declaration* DeclarationSemanticAction(TypeNode* type, char* identifier, Constant* value);
DeclarationList* DeclarationListSemanticAction(Declaration* declaration, DeclarationList* nextDeclarations);

/** Variable Declarations */
VariableDeclaration* VariableDeclarationSemanticAction(TypeNode* type, char* identifier, Value* value);

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

/** Open statements (`if`, `while`, `for`, `forever`) */
OpenStatement* IfOpenStatementSemanticAction(Expression* condition, Statement* thenStatement);
OpenStatement* IfElseOpenStatementSemanticAction(Expression* condition, Statement* thenStatement, OpenStatement* elseStatement);
OpenStatement* WhileOpenStatementSemanticAction(Expression* condition, OpenStatement* body);
OpenStatement* ForOpenStatementSemanticAction(ForInitializer* initializer, Condition* condition, ForUpdate* update, OpenStatement* body);
OpenStatement* ForeverOpenStatementSemanticAction(OpenStatement* body);

/** Closed statements (terminated by newline or explicit block) */
ClosedStatement* SimpleClosedStatementSemanticAction(SimpleStatement* simple);
ClosedStatement* IfElseClosedStatementSemanticAction(Expression* condition, ClosedStatement* thenStmt, ClosedStatement* elseStmt);
ClosedStatement* IfBlockClosedStatementSemanticAction(Expression* condition, StatementList* body);
ClosedStatement* WhileClosedStatementSemanticAction(Expression* condition, ClosedStatement* body);
ClosedStatement* WhileBlockClosedStatementSemanticAction(Expression* condition, StatementList* body);
ClosedStatement* ForClosedStatementSemanticAction(ForInitializer* initializer, Condition* condition, ForUpdate* update, ClosedStatement* body);
ClosedStatement* ForBlockClosedStatementSemanticAction(ForInitializer* initializer, Condition* condition, ForUpdate* update, StatementList* body);
ClosedStatement* ForeverClosedStatementSemanticAction(ClosedStatement* body);
ClosedStatement* ForeverBlockClosedStatementSemanticAction(StatementList* body);

/** General statements & statement lists */
Statement* OpenStatementSemanticAction(OpenStatement* openStmt);
Statement* ClosedStatementSemanticAction(ClosedStatement* closedStmt);
StatementList* StatementListSemanticAction(Statement* stmt, StatementList* nextStatements);

/** Functions & program */
Function* FunctionSemanticAction(TypeNode* returnType, char* name, ParameterList* parameters, StatementList* body, boolean isMain);
FunctionList* FunctionListSemanticAction(Function* function, FunctionList* nextFunctions);
Program* ProgramSemanticAction(DeclarationList* globalDeclarations, FunctionList* functions, CompilerState* compilerState);

#endif /* BISON_ACTIONS_HEADER */
