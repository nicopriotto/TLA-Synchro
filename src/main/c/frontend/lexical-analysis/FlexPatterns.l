%{

#include "FlexActions.h"

%}

/**
 * Enable start condition manipulation functions.
 */

%option stack
%option noyywrap

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

"/*"								{ BEGIN(MULTILINE_COMMENT); BeginMultilineCommentLexemeAction(createLexicalAnalyzerContext()); }
<MULTILINE_COMMENT>"*/"				{ EndMultilineCommentLexemeAction(createLexicalAnalyzerContext()); BEGIN(INITIAL); }
<MULTILINE_COMMENT>[[:space:]]+		{ IgnoredLexemeAction(createLexicalAnalyzerContext()); }
<MULTILINE_COMMENT>[^*]+			{ IgnoredLexemeAction(createLexicalAnalyzerContext()); }
<MULTILINE_COMMENT>.				{ IgnoredLexemeAction(createLexicalAnalyzerContext()); }


"//".*                              { IgnoredLexemeAction(createLexicalAnalyzerContext()); }

"-"									{ return ArithmeticOperatorLexemeAction(createLexicalAnalyzerContext(), SUB); }
"+"									{ return ArithmeticOperatorLexemeAction(createLexicalAnalyzerContext(), ADD); }
"*"									{ return ArithmeticOperatorLexemeAction(createLexicalAnalyzerContext(), MUL); }
"/"									{ return ArithmeticOperatorLexemeAction(createLexicalAnalyzerContext(), DIV); }
"%"                                 { return ArithmeticOperatorLexemeAction(createLexicalAnalyzerContext(), MOD); }
"++"                                { return ArithmeticOperatorLexemeAction(createLexicalAnalyzerContext(), INCREMENT); }
"--"                                { return ArithmeticOperatorLexemeAction(createLexicalAnalyzerContext(), DECREMENT); }

"{"                                 { return SymbolLexemeAction(createLexicalAnalyzerContext(), LEFTBRACE); }
"}"                                 { return SymbolLexemeAction(createLexicalAnalyzerContext(), RIGHTBRACE); }
"("                                 { return SymbolLexemeAction(createLexicalAnalyzerContext(), LEFTPARENTHESIS); }
")"                                 { return SymbolLexemeAction(createLexicalAnalyzerContext(), RIGHTPARENTHESIS); }

","                                 { return SymbolLexemeAction(createLexicalAnalyzerContext(), COMMA); }
";"                                 { return SymbolLexemeAction(createLexicalAnalyzerContext(), SEMICOLON); }

"="                                 { return RelationalOperatorLexemeAction(createLexicalAnalyzerContext(), EQ); }
"!="                                { return RelationalOperatorLexemeAction(createLexicalAnalyzerContext(), NEQ); }
"<="                                { return RelationalOperatorLexemeAction(createLexicalAnalyzerContext(), LEQ); }
">="                                { return RelationalOperatorLexemeAction(createLexicalAnalyzerContext(), GEQ); }
"<"                                 { return RelationalOperatorLexemeAction(createLexicalAnalyzerContext(), LT); }
">"                                 { return RelationalOperatorLexemeAction(createLexicalAnalyzerContext(), GT); }
(and|&&)                            { return RelationalOperatorLexemeAction(createLexicalAnalyzerContext(), AND); }
(or|\|\|)                           { return RelationalOperatorLexemeAction(createLexicalAnalyzerContext(), OR); }
(not|!)                             { return RelationalOperatorLexemeAction(createLexicalAnalyzerContext(), NOT); }

"sem"                               { return KeywordLexemeAction(createLexicalAnalyzerContext(), SEM); }
"thread"                            { return KeywordLexemeAction(createLexicalAnalyzerContext(), THREAD); }
"function"                          { return KeywordLexemeAction(createLexicalAnalyzerContext(), FUNCTION); }
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

"int"                               { return KeywordLexemeAction(createLexicalAnalyzerContext(), INT); }
"float"                             { return KeywordLexemeAction(createLexicalAnalyzerContext(), FLOAT); }
"bool"                              { return KeywordLexemeAction(createLexicalAnalyzerContext(), BOOL); }
"string"                            { return KeywordLexemeAction(createLexicalAnalyzerContext(), STRING); }

"true"                              { return BooleanLexemeAction(createLexicalAnalyzerContext(), TRUE); }
"false"                             { return BooleanLexemeAction(createLexicalAnalyzerContext(), FALSE); }

\"[^\"]*\"                          { return StringLexemeAction(createLexicalAnalyzerContext()); }
[a-zA-Z_][a-zA-Z0-9_]*              { return IdentifierLexemeAction(createLexicalAnalyzerContext()); }
\n                                  { return NewlineLexemeAction(createLexicalAnalyzerContext()); }
[0-9]+\.[0-9]+                      { return FloatLexemeAction(createLexicalAnalyzerContext()); }
[0-9]+                              { return IntegerLexemeAction(createLexicalAnalyzerContext()); }

[[:space:]]+						{ IgnoredLexemeAction(createLexicalAnalyzerContext()); }

.									{ return UnknownLexemeAction(createLexicalAnalyzerContext()); }

%%

#include "FlexExport.h"