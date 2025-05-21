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
    ArgumentList* argumentList;
    Program* program;
    DeclarationList* declarationList;
    TypeNode* typeNode;
    Constant* constant;
    OpenStatement* openStatement;
    ClosedStatement* closedStatement;
    SimpleStatement* simpleStatement;
    FunctionIdentifier* functionIdentifier;
    ForInitializer* forInitializer;
    ForUpdate* forUpdate;
    VariableDeclaration* variableDeclaration;
    RelationalOperator* relationalOperator;
    DeclarationTail* declarationTail;
    ParameterList* parameterList;
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
%destructor { releaseDeclarationList($$); } <declarationList>
%destructor { releaseDeclarationTail($$); } <declarationTail>
%destructor { releaseTypeNode($$); } <typeNode>
%destructor { releaseConstant($$); } <constant>
%destructor { releaseOpenStatement($$); } <openStatement>
%destructor { releaseClosedStatement($$); } <closedStatement>
%destructor { releaseSimpleStatement($$); } <simpleStatement>
%destructor { releaseFunctionIdentifier($$); } <functionIdentifier>
%destructor { releaseForInitializer($$); } <forInitializer>
%destructor { releaseForUpdate($$); } <forUpdate>
%destructor { releaseVariableDeclaration($$); } <variableDeclaration>
%destructor { releaseRelationalOperator($$); } <relationalOperator>


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
%token <token> SEMICOLON
%token <token> COMMA
%token <token> ASSIGN

%token <token> EQUALS
%token <token> NOT_EQUALS
%token <token> LOWER_THAN
%token <token> GREATER_THAN
%token <token> LOWER_EQUALS
%token <token> GREATER_EQUALS

%token <token> AND
%token <token> OR
%token <token> NOT

%token <token> IF
%token <token> ELSE
%token <token> WHILE
%token <token> FOR
%token <token> FOREVER

%token <token> PRINT
%token <token> SLEEP
%token <token> RETURN
%token <token> UP
%token <token> DOWN
%token <token> THREAD

%token <token> INTEGER_TYPE
%token <token> STRING_TYPE
%token <token> FLOAT_TYPE
%token <token> BOOLEAN_TYPE
%token <token> SEM_TYPE

%token <identifier> MAIN

%token <integer> INTEGER
%token <floatVal> FLOAT
%token <boolean> BOOLEAN
%token <string> STRING
%token <identifier> IDENTIFIER

%token <token> NEWLINE

%token <token> UNKNOWN

/** Non-terminals. */

%type <program> program
%type <statementList> statementList
%type <statement> statement
%type <openStatement> openStatement
%type <closedStatement> closedStatement
%type <simpleStatement> simpleStatement
%type <expression> expression
%type <condition> condition
%type <condition> conditionOptional
%type <argumentList> argumentList
%type <argumentList> argumentListNotEmpty
%type <typeNode> type
%type <constant> constant
%type <functionIdentifier> functionIdentifier
%type <forInitializer> forInitializer
%type <forInitializer> forInitializerNotEmpty
%type <forUpdate> forUpdate
%type <forUpdate> forUpdateNotEmpty
%type <variableDeclaration> variableDeclaration
%type <relationalOperator> relationalOperator
%type <declarationTail> declarationTail
%type <declarationList> declarationList
%type <parameterList> parameterList

/**
 * Precedence and associativity.
 *
 * @see https://www.gnu.org/software/bison/manual/html_node/Precedence.html
 */
%left OR
%left AND
%left EQUALS NOT_EQUALS
%left LOWER_THAN GREATER_THAN LOWER_EQUALS GREATER_EQUALS
%left ADD SUB
%left MUL DIV MOD
%right NOT
%right INCREMENT DECREMENT

%%

// IMPORTANT: To use λ in the following grammar, use the %empty symbol.

program
    : declarationList                                                                             { $$ = ProgramSemanticAction($1, currentCompilerState()); } 
    ;

declarationList
    : type IDENTIFIER declarationTail declarationList                                            { $$ = DeclarationListSemanticAction($1, $2, $3, $4); }    
    | type MAIN declarationTail                                                                   { $$ = DeclarationListSemanticAction($1, $2, $3, NULL); }
    | %empty                                                                                       { $$ = NULL; }
    ;

declarationTail
    : ASSIGN constant SEMICOLON                                                                       { $$ = DeclarationSemanticAction($2); }
    | LEFT_PARENTHESIS parameterList RIGHT_PARENTHESIS LEFT_BRACE statementList RIGHT_BRACE         { $$ = FunctionSemanticAction($2, $5); }
    ;
    
parameterList
    : type IDENTIFIER                                           { $$ = ParameterListSemanticAction($1, $2, NULL); }
    | type IDENTIFIER COMMA parameterList                      { $$ = ParameterListSemanticAction($1, $2, $4); }
    | %empty                                                    { $$ = NULL; }
    ;


statementList
    : statement statementList                                  { $$ = StatementListSemanticAction($1, $2); }
    | %empty                                                    { $$ = NULL; }
    ;

statement: openStatement                                          { $$ = OpenStatementSemanticAction($1); }
    | closedStatement                                             { $$ = ClosedStatementSemanticAction($1); }
    ;

openStatement: IF LEFT_PARENTHESIS condition RIGHT_PARENTHESIS statement{ $$ = IfOpenStatementSemanticAction($3, $5); }

    | IF LEFT_PARENTHESIS condition RIGHT_PARENTHESIS closedStatement ELSE openStatement
                                                                  { $$ = IfElseOpenStatementSemanticAction($3, ClosedStatementSemanticAction($5), $7); }
    | WHILE LEFT_PARENTHESIS condition RIGHT_PARENTHESIS openStatement
                                                                  { $$ = WhileOpenStatementSemanticAction($3, $5); }
    | FOR LEFT_PARENTHESIS forInitializer SEMICOLON conditionOptional SEMICOLON forUpdate RIGHT_PARENTHESIS openStatement
                                                                  { $$ = ForOpenStatementSemanticAction($3, $5, $7, $9); }
    | FOREVER openStatement                                       { $$ = ForeverOpenStatementSemanticAction($2); }
    ;

closedStatement: simpleStatement SEMICOLON                       { $$ = SimpleClosedStatementSemanticAction($1); }
    | IF LEFT_PARENTHESIS condition RIGHT_PARENTHESIS closedStatement ELSE closedStatement
                                                                  { $$ = IfElseClosedStatementSemanticAction($3, $5, $7); }
    | IF LEFT_PARENTHESIS condition RIGHT_PARENTHESIS LEFT_BRACE statementList RIGHT_BRACE
                                                                  { $$ = IfBlockClosedStatementSemanticAction($3, $6); }
    | WHILE LEFT_PARENTHESIS condition RIGHT_PARENTHESIS closedStatement
                                                                  { $$ = WhileClosedStatementSemanticAction($3, $5); }
    | WHILE LEFT_PARENTHESIS condition RIGHT_PARENTHESIS LEFT_BRACE statementList RIGHT_BRACE
                                                                  { $$ = WhileBlockClosedStatementSemanticAction($3, $6); }
    | FOR LEFT_PARENTHESIS forInitializer SEMICOLON conditionOptional SEMICOLON forUpdate RIGHT_PARENTHESIS closedStatement
                                                                  { $$ = ForClosedStatementSemanticAction($3, $5, $7, $9); }
    | FOR LEFT_PARENTHESIS forInitializer SEMICOLON conditionOptional SEMICOLON forUpdate RIGHT_PARENTHESIS LEFT_BRACE statementList RIGHT_BRACE
                                                                  { $$ = ForBlockClosedStatementSemanticAction($3, $5, $7, $10); }
    | FOREVER closedStatement                                     { $$ = ForeverClosedStatementSemanticAction($2); }
    | FOREVER LEFT_BRACE statementList RIGHT_BRACE                { $$ = ForeverBlockClosedStatementSemanticAction($3); }
    ;

simpleStatement
    : functionIdentifier LEFT_PARENTHESIS argumentList RIGHT_PARENTHESIS  { $$ = FunctionCallSimpleStatementSemanticAction($1, $3); }
    | IDENTIFIER INCREMENT                                         { $$ = IncrementSimpleStatementSemanticAction($1, false); }
    | INCREMENT IDENTIFIER                                         { $$ = IncrementSimpleStatementSemanticAction($2, true); }
    | IDENTIFIER DECREMENT                                         { $$ = DecrementSimpleStatementSemanticAction($1, false); }
    | DECREMENT IDENTIFIER                                         { $$ = DecrementSimpleStatementSemanticAction($2, true); }
    | IDENTIFIER ASSIGN expression                                 { $$ = AssignmentSimpleStatementSemanticAction($1, $3); }
    | variableDeclaration                                         { $$ = DeclarationSimpleStatementSemanticAction($1); }
    | RETURN constant                                              { $$ = ReturnConstantSimpleStatementSemanticAction($2); }
    | RETURN IDENTIFIER                                            { $$ = ReturnIdentifierSimpleStatementSemanticAction($2); }
    ;

variableDeclaration
    : type IDENTIFIER ASSIGN condition                                 { $$ = VariableDeclarationSemanticActionCondition($1, $2, $4); }
/* | type IDENTIFIER ASSIGN expression                                { $$ = VariableDeclarationSemanticActionExpression($1, $2, $4); } */
    ;

forInitializer: forInitializerNotEmpty                         { $$ = $1; }
    | %empty                                                       { $$ = NULL; }
    ;

forInitializerNotEmpty: variableDeclaration                    { $$ = ForInitializerSemanticAction($1, NULL); }
    | variableDeclaration COMMA forInitializerNotEmpty         { $$ = ForInitializerSemanticAction($1, $3); }
    ;

forUpdate: forUpdateNotEmpty                                   { $$ = $1; }
    | %empty                                                       { $$ = NULL; }
    ;

forUpdateNotEmpty: simpleStatement                             { $$ = ForUpdateSemanticAction($1, NULL); }
    | simpleStatement COMMA forUpdateNotEmpty                  { $$ = ForUpdateSemanticAction($1, $3); };

functionIdentifier: PRINT                                         { $$ = FunctionIdentifierSemanticAction(FUNC_PRINT, NULL); }
    | SLEEP                                                        { $$ = FunctionIdentifierSemanticAction(FUNC_SLEEP, NULL); }
    | UP                                                           { $$ = FunctionIdentifierSemanticAction(FUNC_UP, NULL); }
    | DOWN                                                         { $$ = FunctionIdentifierSemanticAction(FUNC_DOWN, NULL); }
    | THREAD                                                       { $$ = FunctionIdentifierSemanticAction(FUNC_THREAD, NULL); }
    | IDENTIFIER                                                   { $$ = FunctionIdentifierSemanticAction(FUNC_USER_DEFINED, $1); }
    ;

argumentList
    : argumentListNotEmpty                                      { $$ = $1; }
    | %empty                                                       { $$ = NULL; }
    ;

argumentListNotEmpty: expression                                { $$ = ArgumentListSemanticAction($1, NULL); }
    | expression COMMA argumentListNotEmpty                     { $$ = ArgumentListSemanticAction($1, $3); }
    ;

conditionOptional: condition                                      { $$ = $1; }
    | %empty                                                       { $$ = EmptyConditionSemanticAction(); }
    ;

condition: expression relationalOperator expression               { $$ = RelationalConditionSemanticAction($1, $2, $3);}
    | NOT condition                                                { $$ = NotConditionSemanticAction($2); }
    | condition AND condition                                      { $$ = LogicalConditionSemanticAction($1, $3, COND_AND); }
    | condition OR condition                                       { $$ = LogicalConditionSemanticAction($1, $3, COND_OR); }
    /* | LEFT_PARENTHESIS condition RIGHT_PARENTHESIS                 { $$ = ParenthesisConditionSemanticAction($2); } */
    /* | constant                                                     { $$ = ConstantConditionSemanticAction($1); } */
    | expression                                                   { $$ = ExpressionAsConditionSemanticAction($1); }
    ;

relationalOperator: EQUALS                                        { $$ = RelationalOperatorSemanticAction(REL_EQUALS); }
    | NOT_EQUALS                                                   { $$ = RelationalOperatorSemanticAction(REL_NOT_EQUALS); }
    | LOWER_THAN                                                   { $$ = RelationalOperatorSemanticAction(REL_LOWER_THAN); }
    | GREATER_THAN                                                 { $$ = RelationalOperatorSemanticAction(REL_GREATER_THAN); }
    | LOWER_EQUALS                                                 { $$ = RelationalOperatorSemanticAction(REL_LOWER_EQUALS); }
    | GREATER_EQUALS                                               { $$ = RelationalOperatorSemanticAction(REL_GREATER_EQUALS); }
    ;

expression: expression ADD expression                              { $$ = BinaryExpressionSemanticAction($1, $3, EXPR_ADD); }
    | expression SUB expression                                    { $$ = BinaryExpressionSemanticAction($1, $3, EXPR_SUB); }
    | expression MUL expression                                    { $$ = BinaryExpressionSemanticAction($1, $3, EXPR_MUL); }
    | expression DIV expression                                    { $$ = BinaryExpressionSemanticAction($1, $3, EXPR_DIV); }
    | expression MOD expression                                    { $$ = BinaryExpressionSemanticAction($1, $3, EXPR_MOD); }
    | IDENTIFIER INCREMENT                                         { $$ = UnaryExpressionSemanticAction(IdentifierExpressionSemanticAction($1), EXPR_INCREMENT); }
    | INCREMENT IDENTIFIER                                         { $$ = UnaryExpressionSemanticAction(IdentifierExpressionSemanticAction($2), EXPR_PRE_INCREMENT); }
    | IDENTIFIER DECREMENT                                         { $$ = UnaryExpressionSemanticAction(IdentifierExpressionSemanticAction($1), EXPR_DECREMENT); }
    | DECREMENT IDENTIFIER                                         { $$ = UnaryExpressionSemanticAction(IdentifierExpressionSemanticAction($2), EXPR_PRE_DECREMENT); }
    | LEFT_PARENTHESIS expression RIGHT_PARENTHESIS                { $$ = $2; }
    | IDENTIFIER LEFT_PARENTHESIS argumentList RIGHT_PARENTHESIS  { $$ = FunctionCallExpressionSemanticAction($1, $3); }
    | constant                                                     { $$ = ConstantExpressionSemanticAction($1); } 
    | IDENTIFIER                                                   { $$ = IdentifierExpressionSemanticAction($1); }
    ;

constant: INTEGER                                         { $$ = IntegerConstantSemanticAction($1); }
    | FLOAT                                               { $$ = FloatConstantSemanticAction($1); }
    | BOOLEAN                                             { $$ = BooleanConstantSemanticAction($1); }
    | STRING                                              { $$ = StringConstantSemanticAction($1); }
    ;

type: INTEGER_TYPE                                                 { $$ = TypeNodeSemanticAction(TYPE_INTEGER); }
    | STRING_TYPE                                                  { $$ = TypeNodeSemanticAction(TYPE_STRING); }
    | FLOAT_TYPE                                                   { $$ = TypeNodeSemanticAction(TYPE_FLOAT); }
    | BOOLEAN_TYPE                                                 { $$ = TypeNodeSemanticAction(TYPE_BOOLEAN); }
    | SEM_TYPE                                                     { $$ = TypeNodeSemanticAction(TYPE_SEM); }
    ;

%%