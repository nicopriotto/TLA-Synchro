#ifndef ABSTRACT_SYNTAX_TREE_HEADER 
#define ABSTRACT_SYNTAX_TREE_HEADER

#include "../../shared/Logger.h" 
#include <stdlib.h> 
#include <stdbool.h>

/** Initialize module's internal state. */ 
void initializeAbstractSyntaxTreeModule();

/** Shutdown module's internal state. */ 
void shutdownAbstractSyntaxTreeModule();

typedef enum ExpressionType ExpressionType; 
typedef enum RelationalOperatorType RelationalOperatorType; 
typedef enum StatementType StatementType; 
typedef enum TypeNodeType TypeNodeType; 
typedef enum FunctionIdentifierType FunctionIdentifierType; 
typedef enum ValueType ValueType;

typedef struct Constant Constant; 
typedef struct Expression Expression; 
typedef struct Statement Statement; 
typedef struct StatementList StatementList; 
typedef struct OpenStatement OpenStatement; 
typedef struct ClosedStatement ClosedStatement; 
typedef struct SimpleStatement SimpleStatement; 
typedef struct Condition Condition; 
typedef struct ArgumentList ArgumentList; 
typedef struct Program Program; 
typedef struct Declaration Declaration; 
typedef struct DeclarationList DeclarationList; 
typedef struct TypeNode TypeNode; 
typedef struct Function Function; 
typedef struct FunctionList FunctionList; 
typedef struct ForInitializer ForInitializer; 
typedef struct ForUpdate ForUpdate; 
typedef struct FunctionIdentifier FunctionIdentifier; 
typedef struct VariableDeclaration VariableDeclaration; 
typedef struct RelationalOperator RelationalOperator;
typedef struct DeclarationTail DeclarationTail;
typedef struct ParameterList ParameterList;

/**

Node types for the Abstract Syntax Tree (AST). */ 
enum ExpressionType { 
	EXPR_CONSTANT, 
	EXPR_IDENTIFIER, 
	EXPR_ADD, 
	EXPR_SUB, 
	EXPR_MUL, 
	EXPR_DIV,
	EXPR_MOD, 
	EXPR_INCREMENT, 
	EXPR_DECREMENT, 
	EXPR_PRE_INCREMENT, 
	EXPR_PRE_DECREMENT, 
	EXPR_FUNCTION_CALL 
};

enum RelationalOperatorType { 
	REL_EQUALS, 
	REL_NOT_EQUALS, 
	REL_LOWER_THAN, 
	REL_GREATER_THAN, 
	REL_LOWER_EQUALS, 
	REL_GREATER_EQUALS 
};
	
enum StatementType { 
	STMT_SIMPLE, 
	STMT_IF, 
	STMT_IF_ELSE, 
	STMT_WHILE, 
	STMT_FOR, 
	STMT_FOREVER 
};
	
enum TypeNodeType { 
	TYPE_INTEGER, 
	TYPE_STRING, 
	TYPE_FLOAT, 
	TYPE_BOOLEAN, 
	TYPE_SEM 
};

enum FunctionIdentifierType { 
	FUNC_PRINT, 
	FUNC_SLEEP, 
	FUNC_RETURN, 
	FUNC_UP, 
	FUNC_DOWN, 
	FUNC_THREAD, 
	FUNC_USER_DEFINED 
};

enum ValueType { 
	VAL_EXPRESSION, 
	VAL_CONDITION 
};



struct Constant { 
	union { 
		int integer; 
		float floatVal; 
		boolean boolean; 
		char* string; 
	}; 
	enum { 
		CONST_INTEGER, 
		CONST_FLOAT, 
		CONST_BOOLEAN, 
		CONST_STRING 
	} type; 
};

struct Expression { 
	union { 
		Constant* constant; 
		char* identifier; 
		struct { 
			Expression* leftExpression; 
			Expression* rightExpression; 
		}; struct { 
			Expression* expression; 
		}; struct { 
			char* functionName; 
			ArgumentList* arguments; 
		} functionCall; 
	}; 
	ExpressionType type; 
};

struct RelationalOperator {
    RelationalOperatorType type;
};

struct SimpleStatement { 
	union { 
		struct { 
			FunctionIdentifier* function; 
			ArgumentList* arguments; 
		} functionCall; 
		struct { 
			char* identifier;
			 	boolean isPrefix; 
			} increment; 
			struct { 
				char* identifier; 
				boolean isPrefix; 
			} decrement; 
			struct { 
				char* identifier; 
				Expression* expression; 
			} assignment; 
			VariableDeclaration* declaration; 
		}; 
	enum { 
		SIMPLE_FUNCTION_CALL, 
		SIMPLE_INCREMENT, 
		SIMPLE_DECREMENT, 
		SIMPLE_ASSIGNMENT, 
		SIMPLE_DECLARATION 
	} type; 
};

struct Statement { 
	union { 
		SimpleStatement* simpleStatement; 
		struct { 
			Expression* condition; 
			Statement* thenStatement; 
		} ifStatement; 
		struct { 
			Expression* condition; 
			Statement* thenStatement; 
			Statement* elseStatement; 
		} ifElseStatement; 
		struct { 
			Expression* condition; 
			Statement* body; 
		} whileStatement; 
		struct { 
			ForInitializer* initializer; 
			Condition* condition; 
			ForUpdate* update; 
			Statement* body; 
		} forStatement; 
		struct { 
			Statement* body; 
		} foreverStatement; 
		StatementList* blockStatement; 
	}; 
	StatementType type; 
};

struct OpenStatement { 
	union { 
		struct { 
			Expression* condition; 
			Statement* thenStatement; 
		} ifStatement; 
		struct { 
			Expression* condition; 
			Statement* thenStatement; 
			OpenStatement* elseStatement; 
		} ifElseStatement; 
		struct { 
			Expression* condition; 
			OpenStatement* body; 
		} whileStatement; 
		struct { 
			ForInitializer* initializer; 
			Condition* condition; 
			ForUpdate* update; 
			OpenStatement* body; 
		} forStatement; 
		struct { 
			OpenStatement* body; 
		} foreverStatement; 
	}; 
	enum { 
		OPEN_IF, 
		OPEN_IF_ELSE, 
		OPEN_WHILE, 
		OPEN_FOR, 
		OPEN_FOREVER 
	} type; 
};

struct ClosedStatement { 
	union { 
		SimpleStatement* simpleStatement; 
		struct { 
			Expression* condition; 
			ClosedStatement* thenStatement; 
			ClosedStatement* elseStatement; 
		} ifElseStatement; 
		struct { 
			Expression* condition; 
			StatementList* body; 
		} ifBlock; 
		struct { 
			Expression* condition; 
			ClosedStatement* body; 
		} whileStatement; 
		struct { 
			Expression* condition; 
			StatementList* body; 
		} whileBlock; 
		struct { 
			ForInitializer* initializer; 
			Condition* condition; 
			ForUpdate* update; 
			ClosedStatement* body; 
		} forStatement; 
		struct { 
			ForInitializer* initializer; 
			Condition* condition; 
			ForUpdate* update; 
			StatementList* body; 
		} forBlock; 
		struct { 
			ClosedStatement* body; 
		} foreverStatement; 
		struct { 
			StatementList* body; 
		} foreverBlock; 
	}; 
	enum { 
		CLOSED_SIMPLE, 
		CLOSED_IF_ELSE, 
		CLOSED_IF_BLOCK, 
		CLOSED_WHILE, 
		CLOSED_WHILE_BLOCK, 
		CLOSED_FOR, 
		CLOSED_FOR_BLOCK, 
		CLOSED_FOREVER, 
		CLOSED_FOREVER_BLOCK 
	} type; 
};

struct StatementList { 
	Statement* statement; 
	StatementList* next; 
};

struct Condition { 
	union { 
		struct { 
			Expression* leftValue;
			RelationalOperatorType operator; 
			Expression* rightValue; 
		}; 
		struct { 
			Condition* condition; 
		} not; 
		struct { 
			Condition* condition; 
		} parenthesis; 
		struct { 
			Condition* leftCondition; 
			Condition* rightCondition; 
		} logical; 
	}; 
	enum { 
		COND_RELATIONAL, 
		COND_NOT, 
		COND_AND, 
		COND_OR, 
		COND_PARENTHESIS, 
		COND_EMPTY 
	} type; 
};

struct TypeNode { 
	TypeNodeType type; 
};

struct ParameterList { 
	TypeNode* type; 
	char* identifier; 
	ParameterList* next; 
};

struct ArgumentList { 
	Expression* expression; 
	ArgumentList* next; 
};

struct FunctionIdentifier { 
	union { 
		char* identifier;
	}; 
	FunctionIdentifierType type; };

struct VariableDeclaration { 
	TypeNode* type; 
	char* identifier; 
	union { 
		Condition* condition; 
		Expression* expression; 
	};
};


struct ForInitializer { 
	VariableDeclaration* declaration; 
	ForInitializer* next; 
};

struct ForUpdate { 
	SimpleStatement* statement; 
	ForUpdate* next; 
};

struct Function { 
	ParameterList* parameters; 
	StatementList* body; 
};

struct FunctionList { 
	Function* function; 
	FunctionList* next; 
};

struct DeclarationTail {
    union {
        Constant* constant;
        struct {
            ParameterList* parameterList;
            StatementList* statementList;
        } function;
    };
    enum {
        DECL_CONSTANT,
        DECL_FUNCTION
    } type;
};

struct DeclarationList {
	TypeNode* type;
	char* identifier;
	DeclarationTail* declarationTail;
	DeclarationList* next;
};

struct Program { 
	DeclarationList* globalDeclarations; 
};


// Node recursive destructors.

void releaseConstant(Constant* constant); 
void releaseExpression(Expression* expression); 
void releaseStatement(Statement* statement); 
void releaseStatementList(StatementList* statementList); 
void releaseCondition(Condition* condition); 
void releaseParameterList(ParameterList* parameterList); 
void releaseArgumentList(ArgumentList* argumentList); 
void releaseProgram(Program* program); 
void releaseDeclaration(Declaration* declaration);
void releaseDeclarationList(DeclarationList* declarationList); 
void releaseTypeNode(TypeNode* typeNode); 
void releaseFunction(Function* function); 
void releaseFunctionList(FunctionList* functionList); 
void releaseOpenStatement(OpenStatement* openStatement); 
void releaseClosedStatement(ClosedStatement* closedStatement); 
void releaseSimpleStatement(SimpleStatement* simpleStatement); 
void releaseFunctionIdentifier(FunctionIdentifier* functionIdentifier); 
void releaseForInitializer(ForInitializer* forInitializer); 
void releaseForUpdate(ForUpdate* forUpdate); 
void releaseVariableDeclaration(VariableDeclaration* variableDeclaration); 
void releaseRelationalOperator(RelationalOperator* relationalOperator);
void releaseDeclarationTail(DeclarationTail* declarationTail);
#endif
