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
%type <statementList> statement_list
%type <statement> statement
%type <openStatement> open_statement
%type <closedStatement> closed_statement
%type <simpleStatement> simple_statement
%type <expression> expression
%type <condition> condition
%type <condition> condition_optional
%type <argumentList> argument_list
%type <argumentList> argument_list_not_empty
%type <typeNode> type
%type <constant> constant
%type <functionIdentifier> function_identifier
%type <forInitializer> for_initializer
%type <forInitializer> for_initializer_not_empty
%type <forUpdate> for_update
%type <forUpdate> for_update_not_empty
%type <variableDeclaration> variable_declaration
%type <relationalOperator> relational_operator
%type <declarationTail> declaration_tail
%type <declarationList> declaration_list
%type <parameterList> parameter_list

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
    : declaration_list                                                                             { $$ = ProgramSemanticAction($1, currentCompilerState()); } 
    ;

declaration_list
    : type IDENTIFIER declaration_tail declaration_list                                            { $$ = DeclarationListSemanticAction($1, $2, $3, $4); }    
    | type MAIN declaration_tail                                                                   { $$ = DeclarationListSemanticAction($1, $2, $3, NULL); }
    | %empty                                                                                       { $$ = NULL; }
    ;

declaration_tail
    : ASSIGN constant SEMICOLON                                                                       { $$ = DeclarationSemanticAction($2); }
    | LEFT_PARENTHESIS parameter_list RIGHT_PARENTHESIS LEFT_BRACE statement_list RIGHT_BRACE         { $$ = FunctionSemanticAction($2, $5); }
    ;
    
parameter_list
    : type IDENTIFIER                                           { $$ = ParameterListSemanticAction($1, $2, NULL); }
    | type IDENTIFIER COMMA parameter_list                      { $$ = ParameterListSemanticAction($1, $2, $4); }
    | %empty                                                    { $$ = NULL; }
    ;


statement_list
    : statement statement_list                                  { $$ = StatementListSemanticAction($1, $2); }
    | %empty                                                    { $$ = NULL; }
    ;

statement: open_statement                                          { $$ = OpenStatementSemanticAction($1); }
    | closed_statement                                             { $$ = ClosedStatementSemanticAction($1); }
    ;

open_statement: IF LEFT_PARENTHESIS expression RIGHT_PARENTHESIS statement{ $$ = IfOpenStatementSemanticAction($3, $5); }

    | IF LEFT_PARENTHESIS condition RIGHT_PARENTHESIS statement { $$ = IfOpenStatementSemanticAction($3, $5); }

    | IF LEFT_PARENTHESIS expression RIGHT_PARENTHESIS closed_statement ELSE open_statement
                                                                  { $$ = IfElseOpenStatementSemanticAction($3, ClosedStatementSemanticAction($5), $7); }
    | WHILE LEFT_PARENTHESIS expression RIGHT_PARENTHESIS open_statement
                                                                  { $$ = WhileOpenStatementSemanticAction($3, $5); }
    | FOR LEFT_PARENTHESIS for_initializer SEMICOLON condition_optional SEMICOLON for_update RIGHT_PARENTHESIS open_statement
                                                                  { $$ = ForOpenStatementSemanticAction($3, $5, $7, $9); }
    | FOREVER open_statement                                       { $$ = ForeverOpenStatementSemanticAction($2); }
    ;

closed_statement: simple_statement SEMICOLON                       { $$ = SimpleClosedStatementSemanticAction($1); }
    | IF LEFT_PARENTHESIS expression RIGHT_PARENTHESIS closed_statement ELSE closed_statement
                                                                  { $$ = IfElseClosedStatementSemanticAction($3, $5, $7); }
    | IF LEFT_PARENTHESIS expression RIGHT_PARENTHESIS LEFT_BRACE statement_list RIGHT_BRACE
                                                                  { $$ = IfBlockClosedStatementSemanticAction($3, $6); }
    | WHILE LEFT_PARENTHESIS expression RIGHT_PARENTHESIS closed_statement
                                                                  { $$ = WhileClosedStatementSemanticAction($3, $5); }
    | WHILE LEFT_PARENTHESIS expression RIGHT_PARENTHESIS LEFT_BRACE statement_list RIGHT_BRACE
                                                                  { $$ = WhileBlockClosedStatementSemanticAction($3, $6); }
    | FOR LEFT_PARENTHESIS for_initializer SEMICOLON condition_optional SEMICOLON for_update RIGHT_PARENTHESIS closed_statement
                                                                  { $$ = ForClosedStatementSemanticAction($3, $5, $7, $9); }
    | FOR LEFT_PARENTHESIS for_initializer SEMICOLON condition_optional SEMICOLON for_update RIGHT_PARENTHESIS LEFT_BRACE statement_list RIGHT_BRACE
                                                                  { $$ = ForBlockClosedStatementSemanticAction($3, $5, $7, $10); }
    | FOREVER closed_statement                                     { $$ = ForeverClosedStatementSemanticAction($2); }
    | FOREVER LEFT_BRACE statement_list RIGHT_BRACE                { $$ = ForeverBlockClosedStatementSemanticAction($3); }
    ;

simple_statement: function_identifier LEFT_PARENTHESIS argument_list RIGHT_PARENTHESIS
                                                                  { $$ = FunctionCallSimpleStatementSemanticAction($1, $3); }
    | IDENTIFIER INCREMENT                                         { $$ = IncrementSimpleStatementSemanticAction($1, false); }
    | INCREMENT IDENTIFIER                                         { $$ = IncrementSimpleStatementSemanticAction($2, true); }
    | IDENTIFIER DECREMENT                                         { $$ = DecrementSimpleStatementSemanticAction($1, false); }
    | DECREMENT IDENTIFIER                                         { $$ = DecrementSimpleStatementSemanticAction($2, true); }
    | IDENTIFIER ASSIGN expression                                 { $$ = AssignmentSimpleStatementSemanticAction($1, $3); }
    | variable_declaration                                         { $$ = DeclarationSimpleStatementSemanticAction($1); }
    ;

variable_declaration
: type IDENTIFIER ASSIGN condition                                 { $$ = VariableDeclarationSemanticActionCondition($1, $2, $4); }
| type IDENTIFIER ASSIGN expression                                { $$ = VariableDeclarationSemanticActionExpression($1, $2, $4); }
    ;

for_initializer: for_initializer_not_empty                         { $$ = $1; }
    | %empty                                                       { $$ = NULL; }
    ;

for_initializer_not_empty: variable_declaration                    { $$ = ForInitializerSemanticAction($1, NULL); }
    | variable_declaration COMMA for_initializer_not_empty         { $$ = ForInitializerSemanticAction($1, $3); }
    ;

for_update: for_update_not_empty                                   { $$ = $1; }
    | %empty                                                       { $$ = NULL; }
    ;

for_update_not_empty: simple_statement                             { $$ = ForUpdateSemanticAction($1, NULL); }
    | simple_statement COMMA for_update_not_empty                  { $$ = ForUpdateSemanticAction($1, $3); }
    ;

function_identifier: PRINT                                         { $$ = FunctionIdentifierSemanticAction(FUNC_PRINT, NULL); }
    | SLEEP                                                        { $$ = FunctionIdentifierSemanticAction(FUNC_SLEEP, NULL); }
    | RETURN                                                       { $$ = FunctionIdentifierSemanticAction(FUNC_RETURN, NULL); }
    | UP                                                           { $$ = FunctionIdentifierSemanticAction(FUNC_UP, NULL); }
    | DOWN                                                         { $$ = FunctionIdentifierSemanticAction(FUNC_DOWN, NULL); }
    | THREAD                                                       { $$ = FunctionIdentifierSemanticAction(FUNC_THREAD, NULL); }
    | IDENTIFIER                                                   { $$ = FunctionIdentifierSemanticAction(FUNC_USER_DEFINED, $1); }
    ;

argument_list: argument_list_not_empty                             { $$ = $1; }
    | %empty                                                       { $$ = NULL; }
    ;

argument_list_not_empty: expression                                { $$ = ArgumentListSemanticAction($1, NULL); }
    | expression COMMA argument_list_not_empty                     { $$ = ArgumentListSemanticAction($1, $3); }
    ;

condition_optional: condition                                      { $$ = $1; }
    | %empty                                                       { $$ = EmptyConditionSemanticAction(); }
    ;

condition: expression relational_operator expression               { $$ = RelationalConditionSemanticAction($1, $2, $3);}
    | NOT condition                                                { $$ = NotConditionSemanticAction($2); }
    | condition AND condition                                      { $$ = LogicalConditionSemanticAction($1, $3, COND_AND); }
    | condition OR condition                                       { $$ = LogicalConditionSemanticAction($1, $3, COND_OR); }
    | LEFT_PARENTHESIS condition RIGHT_PARENTHESIS                 { $$ = ParenthesisConditionSemanticAction($2); }
    ;

relational_operator: EQUALS                                        { $$ = RelationalOperatorSemanticAction(REL_EQUALS); }
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
    | IDENTIFIER LEFT_PARENTHESIS argument_list RIGHT_PARENTHESIS  { $$ = FunctionCallExpressionSemanticAction($1, $3); }
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
