%{

#include "FlexActions.h"

%}

/**
 * Enable start condition manipulation functions.
 */

%option stack
%option noyywrap
%option yylineno

/**
 * Flex contexts (a.k.a. start conditions).
 *
 * @see https://westes.github.io/flex/manual/Start-Conditions.html
 */
%x MULTILINE_COMMENT

/**
 * Reusable patterns.
 *
 * @see https://westes.github.io/flex/manual/Matching.html
 * @see https://westes.github.io/flex/manual/Patterns.html
 */

%%

"/*"                                { BEGIN(MULTILINE_COMMENT); BeginMultilineCommentLexemeAction(createLexicalAnalyzerContext()); }
<MULTILINE_COMMENT>"*/"             { EndMultilineCommentLexemeAction(createLexicalAnalyzerContext()); BEGIN(INITIAL); }
<MULTILINE_COMMENT>[[:space:]]+     { IgnoredLexemeAction(createLexicalAnalyzerContext()); }
<MULTILINE_COMMENT>[^*]+            { IgnoredLexemeAction(createLexicalAnalyzerContext()); }
<MULTILINE_COMMENT>.                { IgnoredLexemeAction(createLexicalAnalyzerContext()); }

"//".*                              { IgnoredLexemeAction(createLexicalAnalyzerContext()); }

"-"                                 { return ArithmeticOperatorLexemeAction(createLexicalAnalyzerContext(), SUB); }
"+"                                 { return ArithmeticOperatorLexemeAction(createLexicalAnalyzerContext(), ADD); }
"*"                                 { return ArithmeticOperatorLexemeAction(createLexicalAnalyzerContext(), MUL); }
"/"                                 { return ArithmeticOperatorLexemeAction(createLexicalAnalyzerContext(), DIV); }
"%"                                 { return ArithmeticOperatorLexemeAction(createLexicalAnalyzerContext(), MOD); }
"++"                                { return ArithmeticOperatorLexemeAction(createLexicalAnalyzerContext(), INCREMENT); }
"--"                                { return ArithmeticOperatorLexemeAction(createLexicalAnalyzerContext(), DECREMENT); }

"{"                                 { return SymbolLexemeAction(createLexicalAnalyzerContext(), LEFT_BRACE); }
"}"                                 { return SymbolLexemeAction(createLexicalAnalyzerContext(), RIGHT_BRACE); }
"("                                 { return SymbolLexemeAction(createLexicalAnalyzerContext(), LEFT_PARENTHESIS); }
")"                                 { return SymbolLexemeAction(createLexicalAnalyzerContext(), RIGHT_PARENTHESIS); }
","                                 { return SymbolLexemeAction(createLexicalAnalyzerContext(), COMMA); }
";"                                 { return SymbolLexemeAction(createLexicalAnalyzerContext(), SEMICOLON); }
"="                                 { return SymbolLexemeAction(createLexicalAnalyzerContext(), ASSIGN); }

"=="                                { return RelationalOperatorLexemeAction(createLexicalAnalyzerContext(), EQUALS); }
"!="                                { return RelationalOperatorLexemeAction(createLexicalAnalyzerContext(), NOT_EQUALS); }
"<="                                { return RelationalOperatorLexemeAction(createLexicalAnalyzerContext(), LOWER_EQUALS); }
">="                                { return RelationalOperatorLexemeAction(createLexicalAnalyzerContext(), GREATER_EQUALS); }
"<"                                 { return RelationalOperatorLexemeAction(createLexicalAnalyzerContext(), LOWER_THAN); }
">"                                 { return RelationalOperatorLexemeAction(createLexicalAnalyzerContext(), GREATER_THAN); }
(and|&&)                            { return RelationalOperatorLexemeAction(createLexicalAnalyzerContext(), AND); }
(or|\|\|)                           { return RelationalOperatorLexemeAction(createLexicalAnalyzerContext(), OR); }
(not|!)                             { return RelationalOperatorLexemeAction(createLexicalAnalyzerContext(), NOT); }

"thread"                            { return KeywordLexemeAction(createLexicalAnalyzerContext(), THREAD); }
"main"                              { return KeywordLexemeAction(createLexicalAnalyzerContext(), MAIN); }
"up"                                { return KeywordLexemeAction(createLexicalAnalyzerContext(), UP); }
"down"                              { return KeywordLexemeAction(createLexicalAnalyzerContext(), DOWN); }

"if"                                { return KeywordLexemeAction(createLexicalAnalyzerContext(), IF); }
"else"                              { return KeywordLexemeAction(createLexicalAnalyzerContext(), ELSE); }
"while"                             { return KeywordLexemeAction(createLexicalAnalyzerContext(), WHILE); }
"for"                               { return KeywordLexemeAction(createLexicalAnalyzerContext(), FOR); }
"forever"                           { return KeywordLexemeAction(createLexicalAnalyzerContext(), FOREVER); }

"print"                             { return KeywordLexemeAction(createLexicalAnalyzerContext(), PRINT); }
"sleep"                             { return KeywordLexemeAction(createLexicalAnalyzerContext(), SLEEP); }
"return"                            { return KeywordLexemeAction(createLexicalAnalyzerContext(), RETURN); }

"int"                               { return KeywordLexemeAction(createLexicalAnalyzerContext(), INTEGER_TYPE); }
"float"                             { return KeywordLexemeAction(createLexicalAnalyzerContext(), FLOAT_TYPE); }
"boolean"                           { return KeywordLexemeAction(createLexicalAnalyzerContext(), BOOLEAN_TYPE); }
"string"                            { return KeywordLexemeAction(createLexicalAnalyzerContext(), STRING_TYPE); }
"sem"                               { return KeywordLexemeAction(createLexicalAnalyzerContext(), SEM_TYPE); }

"true"                              { return BooleanLexemeAction(createLexicalAnalyzerContext()); }
"false"                             { return BooleanLexemeAction(createLexicalAnalyzerContext()); }
\"[^\"]*\"                          { return StringLexemeAction(createLexicalAnalyzerContext()); }
[0-9]+                              { return IntegerLexemeAction(createLexicalAnalyzerContext()); }
[0-9]+\.[0-9]+                      { return FloatLexemeAction(createLexicalAnalyzerContext()); }

[a-zA-Z_][a-zA-Z0-9_]*              { return IdentifierLexemeAction(createLexicalAnalyzerContext()); }
\n                                  { return NewlineLexemeAction(createLexicalAnalyzerContext()); }

[[:space:]]+                        { IgnoredLexemeAction(createLexicalAnalyzerContext()); }

.                                   { return UnknownLexemeAction(createLexicalAnalyzerContext()); }

%%

#include "FlexExport.h"