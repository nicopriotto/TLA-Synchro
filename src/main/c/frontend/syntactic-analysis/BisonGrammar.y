%{

#include "BisonActions.h"

%}

// You touch this, and you die.
%define api.value.union.name SemanticValue

%union {
    int integer;
    char* identifier;
    boolean boolean;
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
    Function* function;
    FunctionList* functionList;
    Constant* constant;
    OpenStatement* openStatement;
    ClosedStatement* closedStatement;
    SimpleStatement* simpleStatement;
    FunctionIdentifier* functionIdentifier;
    ForInitializer* forInitializer;
    ForUpdate* forUpdate;
    VariableDeclaration* variableDeclaration;
    RelationalOperator* relationalOperator;
    Value* value;
}


/**
 * Destructors. This functions are executed after the parsing ends, so if the
 * AST must be used in the following phases of the compiler you shouldn't used
 * this approach for the AST root node ("program" non-terminal, in this
 * grammar), or it will drop the entire tree even if the parse succeeds.
 *
 * @see https://www.gnu.org/software/bison/manual/html_node/Destructor-Decl.html
 */
%destructor { releaseExpression($$); } <expression>
%destructor { releaseStatement($$); } <statement>
%destructor { releaseStatementList($$); } <statementList>
%destructor { releaseCondition($$); } <condition>
%destructor { releaseParameterList($$); } <parameterList>
%destructor { releaseArgumentList($$); } <argumentList>
%destructor { releaseDeclaration($$); } <declaration>
%destructor { releaseDeclarationList($$); } <declarationList>
%destructor { releaseTypeNode($$); } <typeNode>
%destructor { releaseFunction($$); } <function>
%destructor { releaseFunctionList($$); } <functionList>
%destructor { releaseConstant($$); } <constant>
%destructor { releaseOpenStatement($$); } <openStatement>
%destructor { releaseClosedStatement($$); } <closedStatement>
%destructor { releaseSimpleStatement($$); } <simpleStatement>
%destructor { releaseFunctionIdentifier($$); } <functionIdentifier>
%destructor { releaseForInitializer($$); } <forInitializer>
%destructor { releaseForUpdate($$); } <forUpdate>
%destructor { releaseVariableDeclaration($$); } <variableDeclaration>
%destructor { releaseValue($$); } <value>

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


/** Non-terminals. **/
%type <program> program
%type <declarationList> globalDeclarations
%type <constant> constant
%type <functionList> functions
%type <function> functionBody
%type <parameterList> parameterList
%type <typeNode> type
%type <statementList> statementList
%type <statement> statement
%type <openStatement> openStatement
%type <closedStatement> closedStatement
%type <simpleStatement> simpleStatement
%type <functionIdentifier> functionIdentifier
%type <forInitializer> forInitializer
%type <forUpdate> forUpdate
%type <variableDeclaration> variableDeclaration
%type <value> value
%type <argumentList> argumentList
%type <condition> condition
%type <relationalOperator> relationalOperator
%type <expression> expression


/**
 * Precedence and associativity - ONLY for expressions, NO artificial precedences
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

// Global declarations should always be at the beginning of the program
program
    : globalDeclarations functions         { $$ = ProgramSemanticAction($1, $2, compilerState); }
    ;

globalDeclarations
    : type IDENTIFIER EQUALS constant NEWLINE globalDeclarations    
        { 
            Declaration* declaration = DeclarationSemanticAction($1, $2, $4);
            $$ = DeclarationListSemanticAction(declaration, $6);
        }
    | %empty                               { $$ = NULL; }
    ;

constant
    : INTEGER                              { $$ = IntegerConstantSemanticAction($1); }
    | STRING                               { $$ = StringConstantSemanticAction($1); }
    | BOOLEAN                              { $$ = BooleanConstantSemanticAction($1); }
    | FLOAT                                { $$ = FloatConstantSemanticAction($1); }
    ;

// Main function should be the last function in the program
functions
    : FUNCTION IDENTIFIER functionBody functions    
        { 
            Function* function = FunctionSemanticAction($2, $3->parameters, $3->body, false);
            $$ = FunctionListSemanticAction(function, $4);
        }
    | FUNCTION MAIN functionBody           
        { 
            Function* function = FunctionSemanticAction(strdup("main"), $3->parameters, $3->body, true);
            $$ = FunctionListSemanticAction(function, NULL);
        }
    ;

functionBody
    : LEFT_PARENTHESIS parameterList RIGHT_PARENTHESIS LEFT_BRACE statementList RIGHT_BRACE
        { 
            $$ = FunctionSemanticAction(NULL, $2, $5, false);
        }
    ;

parameterList
    : type IDENTIFIER COMMA parameterList  { $$ = ParameterListSemanticAction($1, $2, $4); }
    | type IDENTIFIER                      { $$ = ParameterListSemanticAction($1, $2, NULL); }
    | %empty                               { $$ = NULL; }
    ;

type
    : INTEGER_DECLARATION                  { $$ = TypeNodeSemanticAction(TYPE_INTEGER); }
    | STRING_DECLARATION                   { $$ = TypeNodeSemanticAction(TYPE_STRING); }
    | FLOAT_DECLARATION                    { $$ = TypeNodeSemanticAction(TYPE_FLOAT); }
    | BOOLEAN_DECLARATION                  { $$ = TypeNodeSemanticAction(TYPE_BOOLEAN); }
    | SEM_DECLARATION                      { $$ = TypeNodeSemanticAction(TYPE_SEM); }
    ;

statementList
    : statement statementList              { $$ = StatementListSemanticAction($1, $2); }
    | %empty                               { $$ = NULL; }
    ;

// Solution to the dangling else problem
// See: https://en.wikipedia.org/wiki/Dangling_else 
statement
    : openStatement                        { $$ = OpenStatementSemanticAction($1); }
    | closedStatement                      { $$ = ClosedStatementSemanticAction($1); }
    ;

// Open statements - those that end with an open (unmatched) statement
openStatement
    : IF LEFT_PARENTHESIS expression RIGHT_PARENTHESIS statement
        { $$ = IfOpenStatementSemanticAction($3, $5); }
    | IF LEFT_PARENTHESIS expression RIGHT_PARENTHESIS closedStatement ELSE openStatement
        { 
            Statement* thenStmt = ClosedStatementSemanticAction($5);
            $$ = IfElseOpenStatementSemanticAction($3, thenStmt, $7);
        }
    | WHILE LEFT_PARENTHESIS expression RIGHT_PARENTHESIS openStatement
        { $$ = WhileOpenStatementSemanticAction($3, $5); }
    | FOR LEFT_PARENTHESIS forInitializer SEMICOLON condition SEMICOLON forUpdate RIGHT_PARENTHESIS openStatement
        { $$ = ForOpenStatementSemanticAction($3, $5, $7, $9); }
    | FOREVER openStatement
        { $$ = ForeverOpenStatementSemanticAction($2); }
    ;

// Closed statements - those that are completely matched and closed
closedStatement
    : simpleStatement NEWLINE
        { $$ = SimpleClosedStatementSemanticAction($1); }
    | IF LEFT_PARENTHESIS expression RIGHT_PARENTHESIS closedStatement ELSE closedStatement
        { $$ = IfElseClosedStatementSemanticAction($3, $5, $7); }
    | IF LEFT_PARENTHESIS expression RIGHT_PARENTHESIS LEFT_BRACE statementList RIGHT_BRACE
        { $$ = IfBlockClosedStatementSemanticAction($3, $6); }
    | WHILE LEFT_PARENTHESIS expression RIGHT_PARENTHESIS closedStatement
        { $$ = WhileClosedStatementSemanticAction($3, $5); }
    | WHILE LEFT_PARENTHESIS expression RIGHT_PARENTHESIS LEFT_BRACE statementList RIGHT_BRACE
        { $$ = WhileBlockClosedStatementSemanticAction($3, $6); }
    | FOR LEFT_PARENTHESIS forInitializer SEMICOLON condition SEMICOLON forUpdate RIGHT_PARENTHESIS closedStatement
        { $$ = ForClosedStatementSemanticAction($3, $5, $7, $9); }
    | FOR LEFT_PARENTHESIS forInitializer SEMICOLON condition SEMICOLON forUpdate RIGHT_PARENTHESIS LEFT_BRACE statementList RIGHT_BRACE
        { $$ = ForBlockClosedStatementSemanticAction($3, $5, $7, $10); }
    | FOREVER closedStatement
        { $$ = ForeverClosedStatementSemanticAction($2); }
    | FOREVER LEFT_BRACE statementList RIGHT_BRACE
        { $$ = ForeverBlockClosedStatementSemanticAction($3); }
    ;

simpleStatement
    : functionIdentifier LEFT_PARENTHESIS argumentList RIGHT_PARENTHESIS
        { $$ = FunctionCallSimpleStatementSemanticAction($1, $3); }
    | INCREMENT IDENTIFIER
        { $$ = IncrementSimpleStatementSemanticAction($2, true); }
    | DECREMENT IDENTIFIER
        { $$ = DecrementSimpleStatementSemanticAction($2, true); }
    | IDENTIFIER INCREMENT
        { $$ = IncrementSimpleStatementSemanticAction($1, false); }
    | IDENTIFIER DECREMENT
        { $$ = DecrementSimpleStatementSemanticAction($1, false); }
    | variableDeclaration
        { $$ = DeclarationSimpleStatementSemanticAction($1); }
    | IDENTIFIER EQUALS expression
        { $$ = AssignmentSimpleStatementSemanticAction($1, $3); }
    ;

functionIdentifier
    : PRINT                                { $$ = FunctionIdentifierSemanticAction(FUNC_PRINT, NULL); }
    | SLEEP                                { $$ = FunctionIdentifierSemanticAction(FUNC_SLEEP, NULL); }
    | RETURN                               { $$ = FunctionIdentifierSemanticAction(FUNC_RETURN, NULL); }
    | UP                                   { $$ = FunctionIdentifierSemanticAction(FUNC_UP, NULL); }
    | DOWN                                 { $$ = FunctionIdentifierSemanticAction(FUNC_DOWN, NULL); }
    | THREAD                               { $$ = FunctionIdentifierSemanticAction(FUNC_THREAD, NULL); }
    | IDENTIFIER                           { $$ = FunctionIdentifierSemanticAction(FUNC_USER_DEFINED, $1); }
    ;
    
forInitializer
    : variableDeclaration COMMA forInitializer  { $$ = ForInitializerSemanticAction($1, $3); }
    | variableDeclaration                       { $$ = ForInitializerSemanticAction($1, NULL); }
    | %empty                                    { $$ = NULL; }
    ;

forUpdate
    : simpleStatement                      { $$ = ForUpdateSemanticAction($1, NULL); }
    | simpleStatement COMMA forUpdate      { $$ = ForUpdateSemanticAction($1, $3); }
    | %empty                               { $$ = NULL; }
    ;

variableDeclaration
    : type IDENTIFIER EQUALS value         { $$ = VariableDeclarationSemanticAction($1, $2, $4); }
    ;

value
    : expression                           { $$ = ExpressionValueSemanticAction($1); }
    | condition                            { $$ = ConditionValueSemanticAction($1); }
    ;

argumentList
    : expression COMMA argumentList        { $$ = ArgumentListSemanticAction($1, $3); }
    | expression                           { $$ = ArgumentListSemanticAction($1, NULL); }
    | %empty                               { $$ = NULL; }
    ;

condition
    : value relationalOperator value       
        { 
            RelationalOperatorType opType;
            switch ($2->type) {
                case EQUALS: opType = REL_EQUALS; break;
                case NOT_EQUALS: opType = REL_NOT_EQUALS; break;
                case LOWER_THAN: opType = REL_LOWER_THAN; break;
                case GREATER_THAN: opType = REL_GREATER_THAN; break;
                case LOWER_EQUALS: opType = REL_LOWER_EQUALS; break;
                case GREATER_EQUALS: opType = REL_GREATER_EQUALS; break;
            }
            $$ = RelationalConditionSemanticAction($1, opType, $3);
        }
    | NOT condition                        { $$ = NotConditionSemanticAction($2); }
    | condition AND condition              { $$ = LogicalConditionSemanticAction($1, $3, COND_AND); }
    | condition OR condition               { $$ = LogicalConditionSemanticAction($1, $3, COND_OR); }
    | LEFT_PARENTHESIS condition RIGHT_PARENTHESIS  { $$ = ParenthesisConditionSemanticAction($2); }
    | %empty                               { $$ = EmptyConditionSemanticAction(); }
    ;

relationalOperator
    : EQUALS                               { $$ = &$1; }
    | NOT_EQUALS                           { $$ = &$1; }
    | LOWER_THAN                           { $$ = &$1; }
    | GREATER_THAN                         { $$ = &$1; }
    | LOWER_EQUALS                         { $$ = &$1; }
    | GREATER_EQUALS                       { $$ = &$1; }
    ;

expression
    : constant                             { $$ = ConstantExpressionSemanticAction($1); }
    | IDENTIFIER                           { $$ = IdentifierExpressionSemanticAction($1); }
    | expression ADD expression            { $$ = BinaryExpressionSemanticAction($1, $3, EXPR_ADD); }
    | expression SUB expression            { $$ = BinaryExpressionSemanticAction($1, $3, EXPR_SUB); }
    | expression MUL expression            { $$ = BinaryExpressionSemanticAction($1, $3, EXPR_MUL); }
    | expression DIV expression            { $$ = BinaryExpressionSemanticAction($1, $3, EXPR_DIV); }
    | expression MOD expression            { $$ = BinaryExpressionSemanticAction($1, $3, EXPR_MOD); }
    | expression INCREMENT                 { $$ = UnaryExpressionSemanticAction($1, EXPR_INCREMENT); }
    | expression DECREMENT                 { $$ = UnaryExpressionSemanticAction($1, EXPR_DECREMENT); }
    | INCREMENT expression                 { $$ = UnaryExpressionSemanticAction($2, EXPR_PRE_INCREMENT); }
    | DECREMENT expression                 { $$ = UnaryExpressionSemanticAction($2, EXPR_PRE_DECREMENT); }
    | IDENTIFIER LEFT_PARENTHESIS argumentList RIGHT_PARENTHESIS
        { $$ = FunctionCallExpressionSemanticAction($1, $3); }
    ;

%%