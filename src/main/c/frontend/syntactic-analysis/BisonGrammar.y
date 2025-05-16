%{

#include "BisonActions.h"

%}

// You touch this, and you die.
%define api.value.union.name SemanticValue






%union {
	/** Terminals. */

	int integer;
	char* identifier;
	boolean boolean;
	float float; 
    String string;
	Token token;

	/** Non-terminals. */

	Constant * constant;
	Expression * expression;
	Factor * factor;
	Program * program;
}






/**
 * Destructors. This functions are executed after the parsing ends, so if the
 * AST must be used in the following phases of the compiler you shouldn't used
 * this approach for the AST root node ("program" non-terminal, in this
 * grammar), or it will drop the entire tree even if the parse succeeds.
 *
 * @see https://www.gnu.org/software/bison/manual/html_node/Destructor-Decl.html
 */
%destructor { releaseConstant($$); } <constant>
%destructor { releaseExpression($$); } <expression>
%destructor { releaseFactor($$); } <factor>





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
%token <token> RIGH_TPARENTHESIS

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
%token <float> FLOAT
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
%type <constant> constant
%type <expression> expression
%type <factor> factor
%type <program> program
// TODO (%type)










/**
 * Precedence and associativity.
 *
 * @see https://www.gnu.org/software/bison/manual/html_node/Precedence.html
 */
%left ADD SUB
%left MUL DIV
%right NOT
// TODO: ver temas de asociatividad y precedencia de lo que definimos











%%

// IMPORTANT: To use λ in the following grammar, use the %empty symbol.

program: expression													{ $$ = ExpressionProgramSemanticAction(currentCompilerState(), $1); }
	;

expression: expression[left] ADD expression[right]					{ $$ = ArithmeticExpressionSemanticAction($left, $right, ADDITION); }
	| expression[left] DIV expression[right]						{ $$ = ArithmeticExpressionSemanticAction($left, $right, DIVISION); }
	| expression[left] MUL expression[right]						{ $$ = ArithmeticExpressionSemanticAction($left, $right, MULTIPLICATION); }
	| expression[left] SUB expression[right]						{ $$ = ArithmeticExpressionSemanticAction($left, $right, SUBTRACTION); }
	| factor														{ $$ = FactorExpressionSemanticAction($1); }
	;

factor: OPEN_PARENTHESIS expression CLOSE_PARENTHESIS				{ $$ = ExpressionFactorSemanticAction($2); }
	| constant														{ $$ = ConstantFactorSemanticAction($1); }
	;

constant: INTEGER													{ $$ = IntegerConstantSemanticAction($1); }
	;



// NUEVO
program:
    expression {
        printf("AST generado para la expresión\n");
        // Acción para manejar el AST final
    }
;

expression:
    term {
        $$ = $1;  // Nodo raíz de la expresión es el término
    }
    | expression ADD term {
        $$ = createBinaryOpNode("+", $1, $3);  // Nodo con operador "+"
    }
    | expression SUB term {
        $$ = createBinaryOpNode("-", $1, $3);  // Nodo con operador "-"
    }
;

term:
    factor {
        $$ = $1;  // Nodo raíz del factor
    }
    | term MUL factor {
        $$ = createBinaryOpNode("*", $1, $3);  // Nodo con operador "*"
    }
    | term DIV factor {
        $$ = createBinaryOpNode("/", $1, $3);  // Nodo con operador "/"
    }
;

factor:
    INTEGER {
        $$ = createConstantNode($1);  // Nodo constante para un entero
    }
    | ID {
        $$ = createIdentifierNode($1);  // Nodo para un identificador
    }
    | OPEN_PARENTHESIS expression CLOSE_PARENTHESIS {
        $$ = $2;  // Si es una expresión entre paréntesis, usar la expresión interna
    }
;

%%


%%
