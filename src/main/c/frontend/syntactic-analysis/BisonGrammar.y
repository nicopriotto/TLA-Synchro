%{

#include "BisonActions.h"

%}

// You touch this, and you die.
%define api.value.union.name SemanticValue

%union {
    int integer;
    char* identifier;
    bool boolean;
    float floatVal;
    char* string;
    Token token;

    Expression* expression;
    Statement* statement;
    StatementList* statementList;
    Condition* condition;
    ParameterList* parameterList;
    ArgumentList* argumentList;
    Program* program;
    Declaration* declaration;
    DeclarationList* declarationList;
    TypeNode* typeNode;
}


/**
 * Destructors. This functions are executed after the parsing ends, so if the
 * AST must be used in the following phases of the compiler you shouldn't used
 * this approach for the AST root node ("program" non-terminal, in this
 * grammar), or it will drop the entire tree even if the parse succeeds.
 *
 * @see https://www.gnu.org/software/bison/manual/html_node/Destructor-Decl.html
 */
 // TODO: Hacer
%destructor { releaseExpression($$); } <expression>


/** Terminals. */

%token <token> SUB
%token <token> ADD
%token <token> DIV
%token <token> MUL
%token <token> MOD
%token <token> INCREMENT
%token <token> DECREMENT

%token <token> LEFT_BRACE
%token <token> RIGHT_BRACE
%token <token> LEFT_PARENTHESIS
%token <token> RIGHT_PARENTHESIS

%token <token> COMMA
%token <token> SEMICOLON

%token <token> EQUALS
%token <token> NOT_EQUALS
%token <token> LOWER_EQUALS
%token <token> GREATER_EQUALS
%token <token> LOWER_THAN
%token <token> GREATER_THAN
%token <token> AND
%token <token> OR
%token <token> NOT

%token <token> THREAD
%token <token> FUNCTION
%token <token> MAIN
%token <token> UP
%token <token> DOWN

%token <token> IF
%token <token> ELSE 
%token <token> WHILE
%token <token> FOR
%token <token> FOREVER

%token <token> PRINT
%token <token> SLEEP
%token <token> RETURN

%token <integer> INTEGER
%token <floatVal> FLOAT
%token <boolean> BOOLEAN
%token <string> STRING

%token <token> INTEGER_DECLARATION
%token <token> FLOAT_DECLARATION
%token <token> BOOLEAN_DECLARATION
%token <token> STRING_DECLARATION
%token <token> SEM_DECLARATION

%token <identifier> IDENTIFIER
%token <token> NEWLINE

%token <token> UNKNOWN


/** Non-terminals. **/ // tipos de dato para los no terminales generados por bison (ni idea comment de mr miz)
%type <program> program
%type <declarationList> globalDeclarations
%type <declaration> globalDeclaration
%type <statementList> functions
%type <statement> functionBody
%type <parameterList> parameterList
%type <typeNode> type
%type <statementList> statementList
%type <statement> statement
%type <statement> ifStatement 
%type <statement> elseIfChain
%type <statement> statementOrBlock
%type <statement> forInitializer
%type <statementList> forUpdate
%type <statement> variableDeclaration
%type <expression> value
%type <argumentList> argumentList
%type <condition> condition
%type <token> relationalOperator
%type <expression> expression




/**
 * Precedence and associativity.
 *
 * @see https://www.gnu.org/software/bison/manual/html_node/Precedence.html
 */

%left OR
%left AND
%nonassoc EQUALS NOT_EQUALS
%nonassoc LOWER_THAN GREATER_THAN LOWER_EQUALS GREATER_EQUALS
%right NOT
%left ADD SUB
%left MUL DIV MOD
%right INCREMENT DECREMENT

// Grammar

%%

program
    : globalDeclarations functions
    ;

globalDeclarations
    : globalDeclaration globalDeclarations
    | %empty
    ;

globalDeclaration
    : INTEGER_DECLARATION IDENTIFIER EQUALS INTEGER
    | STRING_DECLARATION IDENTIFIER EQUALS STRING
    | BOOLEAN_DECLARATION IDENTIFIER EQUALS BOOLEAN
    | FLOAT_DECLARATION IDENTIFIER EQUALS FLOAT
    | SEM_DECLARATION IDENTIFIER EQUALS INTEGER
    ;

functions
    : FUNCTION IDENTIFIER functionBody
    | FUNCTION MAIN functionBody
    ;

functionBody
    : LEFT_PARENTHESIS parameterList RIGHT_PARENTHESIS LEFT_BRACE statementList RIGHT_BRACE
    ;

parameterList
    : type IDENTIFIER COMMA parameterList
    | type IDENTIFIER
    | %empty
    ;

type
    : INTEGER_DECLARATION
    | STRING_DECLARATION
    | FLOAT_DECLARATION
    | BOOLEAN_DECLARATION
    | SEM_DECLARATION
    ;

statementList
    : statement statementList
    | %empty
    ;

statement
    : variableDeclaration NEWLINE
    | ifStatement
    | WHILE LEFT_PARENTHESIS condition RIGHT_PARENTHESIS statementOrBlock
    | FOREVER statementOrBlock
    | FOR LEFT_PARENTHESIS forInitializer SEMICOLON condition SEMICOLON forUpdate RIGHT_PARENTHESIS statementOrBlock
    | PRINT expression NEWLINE
    | SLEEP INTEGER NEWLINE
    | RETURN expression NEWLINE
    | THREAD IDENTIFIER LEFT_PARENTHESIS argumentList RIGHT_PARENTHESIS NEWLINE
    | UP IDENTIFIER NEWLINE
    | DOWN IDENTIFIER NEWLINE
    | INCREMENT IDENTIFIER NEWLINE     
    | DECREMENT IDENTIFIER NEWLINE     
    | IDENTIFIER INCREMENT NEWLINE    
    | IDENTIFIER DECREMENT NEWLINE    
    ;

ifStatement
    : IF LEFT_PARENTHESIS condition RIGHT_PARENTHESIS statementOrBlock elseIfChain
    ;

elseIfChain
    : ELSE IF LEFT_PARENTHESIS condition RIGHT_PARENTHESIS statementOrBlock elseIfChain
    | ELSE statementOrBlock
    | %empty
    ;

statementOrBlock
    : LEFT_BRACE statementList RIGHT_BRACE
    | statement
    ;

forInitializer
    : variableDeclaration COMMA forInitializer
    | variableDeclaration
    | %empty
    ;

forUpdate
    : statement
    | %empty
    ;

variableDeclaration
    : INTEGER_DECLARATION IDENTIFIER EQUALS expression
    | STRING_DECLARATION IDENTIFIER EQUALS expression
    | BOOLEAN_DECLARATION IDENTIFIER EQUALS expression
    | BOOLEAN_DECLARATION IDENTIFIER EQUALS condition
    | FLOAT_DECLARATION IDENTIFIER EQUALS expression
    | SEM_DECLARATION IDENTIFIER EQUALS expression
    ;

value
    : INTEGER
    | STRING
    | BOOLEAN
    | FLOAT
    | IDENTIFIER
    ;

argumentList
    : expression COMMA argumentList
    | expression
    | %empty
    ;

condition
    : value relationalOperator value
    | NOT condition
    | condition AND condition
    | condition OR condition
    | LEFT_PARENTHESIS condition RIGHT_PARENTHESIS
    | %empty 
    ;

relationalOperator
    : EQUALS
    | NOT_EQUALS
    | LOWER_THAN
    | GREATER_THAN
    | LOWER_EQUALS
    | GREATER_EQUALS
    ;

expression
    : value
    | expression ADD expression
    | expression SUB expression
    | expression MUL expression
    | expression DIV expression
    | expression MOD expression
    | expression INCREMENT     
    | expression DECREMENT     
    | INCREMENT expression     
    | DECREMENT expression     
    | IDENTIFIER LEFT_PARENTHESIS argumentList RIGHT_PARENTHESIS
    ;

%%


%%
