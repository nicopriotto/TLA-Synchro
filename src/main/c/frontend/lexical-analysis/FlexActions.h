#ifndef FLEX_ACTIONS_HEADER
#define FLEX_ACTIONS_HEADER

#include "../../shared/Environment.h"
#include "../../shared/Logger.h"
#include "../../shared/String.h"
#include "../../shared/Type.h"
#include "../syntactic-analysis/AbstractSyntaxTree.h"
#include "../syntactic-analysis/BisonParser.h"
#include "LexicalAnalyzerContext.h"
#include <stdio.h>
#include <stdlib.h>

/** Initialize module's internal state. */
void initializeFlexActionsModule();

/** Shutdown module's internal state. */
void shutdownFlexActionsModule();

/**
 * Flex lexeme processing actions.
 */

// Comments
void BeginMultilineCommentLexemeAction(LexicalAnalyzerContext * lexicalAnalyzerContext);
void EndMultilineCommentLexemeAction(LexicalAnalyzerContext * lexicalAnalyzerContext);
void IgnoredLexemeAction(LexicalAnalyzerContext * lexicalAnalyzerContext);

// Arithmetic Operators
Token ArithmeticOperatorLexemeAction(LexicalAnalyzerContext * lexicalAnalyzerContext, Token token);

// Relational Operators
Token RelationalOperatorLexemeAction(LexicalAnalyzerContext * lexicalAnalyzerContext, Token token);

// Symbols
Token SymbolLexemeAction(LexicalAnalyzerContext * lexicalAnalyzerContext, Token token);

// Literals
Token IntegerLexemeAction(LexicalAnalyzerContext * lexicalAnalyzerContext);
Token StringLexemeAction(LexicalAnalyzerContext * lexicalAnalyzerContext);
Token BooleanLexemeAction(LexicalAnalyzerContext * lexicalAnalyzerContext);
Token FloatLexemeAction(LexicalAnalyzerContext * lexicalAnalyzerContext);

// Keywords
Token KeywordLexemeAction(LexicalAnalyzerContext * lexicalAnalyzerContext, Token token);

// Identifiers
Token IdentifierLexemeAction(LexicalAnalyzerContext * lexicalAnalyzerContext);

// Newline
Token NewlineLexemeAction(LexicalAnalyzerContext * lexicalAnalyzerContext);

// Unknown Token
Token UnknownLexemeAction(LexicalAnalyzerContext * lexicalAnalyzerContext);

#endif
