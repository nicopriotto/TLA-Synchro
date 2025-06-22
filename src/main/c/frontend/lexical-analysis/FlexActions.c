#include "FlexActions.h"

/* MODULE INTERNAL STATE */

static Logger* _logger = NULL;
static boolean _logIgnoredLexemes = true;

extern int yylineno;

void initializeFlexActionsModule() {
    _logIgnoredLexemes = getBooleanOrDefault("LOG_IGNORED_LEXEMES", _logIgnoredLexemes);
    _logger = createLogger("FlexActions");
}

void shutdownFlexActionsModule() {
    if (_logger != NULL) {
        destroyLogger(_logger);
    }
}

/* PRIVATE FUNCTIONS */

static void _logLexicalAnalyzerContext(const char* functionName, LexicalAnalyzerContext* lexicalAnalyzerContext);

/**
 * Logs a lexical-analyzer context in DEBUGGING level.
 */
static void _logLexicalAnalyzerContext(const char* functionName, LexicalAnalyzerContext* lexicalAnalyzerContext) {
    char* escapedLexeme = escape(lexicalAnalyzerContext->lexeme);
    logDebugging(_logger, "%s: %s (context = %d, length = %d, line = %d)",
        functionName,
        escapedLexeme,
        lexicalAnalyzerContext->currentContext,
        lexicalAnalyzerContext->length,
        lexicalAnalyzerContext->line);
    free(escapedLexeme);
}

/* PUBLIC FUNCTIONS */

void BeginMultilineCommentLexemeAction(LexicalAnalyzerContext* lexicalAnalyzerContext) {
    if (_logIgnoredLexemes) {
        _logLexicalAnalyzerContext(__FUNCTION__, lexicalAnalyzerContext);
    }
    destroyLexicalAnalyzerContext(lexicalAnalyzerContext);
}

void EndMultilineCommentLexemeAction(LexicalAnalyzerContext* lexicalAnalyzerContext) {
    if (_logIgnoredLexemes) {
        _logLexicalAnalyzerContext(__FUNCTION__, lexicalAnalyzerContext);
    }
    destroyLexicalAnalyzerContext(lexicalAnalyzerContext);
}

void IgnoredLexemeAction(LexicalAnalyzerContext* lexicalAnalyzerContext) {
    if (_logIgnoredLexemes) {
        _logLexicalAnalyzerContext(__FUNCTION__, lexicalAnalyzerContext);
    }
    destroyLexicalAnalyzerContext(lexicalAnalyzerContext);
}

Token ArithmeticOperatorLexemeAction(LexicalAnalyzerContext* lexicalAnalyzerContext, Token token) {
    _logLexicalAnalyzerContext(__FUNCTION__, lexicalAnalyzerContext);
    lexicalAnalyzerContext->semanticValue->token = token;
    destroyLexicalAnalyzerContext(lexicalAnalyzerContext);
    return token;
}

Token SymbolLexemeAction(LexicalAnalyzerContext* lexicalAnalyzerContext, Token token) {
    _logLexicalAnalyzerContext(__FUNCTION__, lexicalAnalyzerContext);
    lexicalAnalyzerContext->semanticValue->token = token;
    destroyLexicalAnalyzerContext(lexicalAnalyzerContext);
    return token;
}

Token RelationalOperatorLexemeAction(LexicalAnalyzerContext* lexicalAnalyzerContext, Token token) {
    _logLexicalAnalyzerContext(__FUNCTION__, lexicalAnalyzerContext);
    lexicalAnalyzerContext->semanticValue->token = token;
    destroyLexicalAnalyzerContext(lexicalAnalyzerContext);
    return token;
}

Token KeywordLexemeAction(LexicalAnalyzerContext* lexicalAnalyzerContext, Token token) {
    _logLexicalAnalyzerContext(__FUNCTION__, lexicalAnalyzerContext);
    lexicalAnalyzerContext->semanticValue->token = token;
    destroyLexicalAnalyzerContext(lexicalAnalyzerContext);
    return token;
}

Token BooleanLexemeAction(LexicalAnalyzerContext* lexicalAnalyzerContext) {
    _logLexicalAnalyzerContext(__FUNCTION__, lexicalAnalyzerContext);

    if (strcmp(lexicalAnalyzerContext->lexeme, "true") == 0) {
        lexicalAnalyzerContext->semanticValue->boolean = true;
    } else {
        lexicalAnalyzerContext->semanticValue->boolean = false;
    }

    destroyLexicalAnalyzerContext(lexicalAnalyzerContext);
    return BOOLEAN;
}

Token StringLexemeAction(LexicalAnalyzerContext* lexicalAnalyzerContext) {
    _logLexicalAnalyzerContext(__FUNCTION__, lexicalAnalyzerContext);
    
    size_t len = lexicalAnalyzerContext->length;
    char* rawString = lexicalAnalyzerContext->lexeme;

    char* unquoted = (char*)calloc(len - 1, sizeof(char));
    if (unquoted != NULL) {
        strncpy(unquoted, rawString + 1, len - 2);
        unquoted[len - 2] = '\0';
        lexicalAnalyzerContext->semanticValue->string = unquoted;
    }

    destroyLexicalAnalyzerContext(lexicalAnalyzerContext);
    return STRING;
}

Token IntegerLexemeAction(LexicalAnalyzerContext* lexicalAnalyzerContext) {
    _logLexicalAnalyzerContext(__FUNCTION__, lexicalAnalyzerContext);
    lexicalAnalyzerContext->semanticValue->integer = atoi(lexicalAnalyzerContext->lexeme);
    destroyLexicalAnalyzerContext(lexicalAnalyzerContext);
    return INTEGER;
}

Token FloatLexemeAction(LexicalAnalyzerContext* lexicalAnalyzerContext) {
    _logLexicalAnalyzerContext(__FUNCTION__, lexicalAnalyzerContext);
    lexicalAnalyzerContext->semanticValue->floatVal = atof(lexicalAnalyzerContext->lexeme);
    destroyLexicalAnalyzerContext(lexicalAnalyzerContext);
    return FLOAT;
}

Token IdentifierLexemeAction(LexicalAnalyzerContext* lexicalAnalyzerContext) {
    _logLexicalAnalyzerContext(__FUNCTION__, lexicalAnalyzerContext);
    
    char* id = strdup(lexicalAnalyzerContext->lexeme);
    lexicalAnalyzerContext->semanticValue->identifier = id;

    destroyLexicalAnalyzerContext(lexicalAnalyzerContext);
    return IDENTIFIER;
}

Token NewlineLexemeAction(LexicalAnalyzerContext* lexicalAnalyzerContext) {
    _logLexicalAnalyzerContext(__FUNCTION__, lexicalAnalyzerContext);
    yylineno++;
    destroyLexicalAnalyzerContext(lexicalAnalyzerContext);
    return NEWLINE;
}

Token UnknownLexemeAction(LexicalAnalyzerContext* lexicalAnalyzerContext) {
    _logLexicalAnalyzerContext(__FUNCTION__, lexicalAnalyzerContext);
    destroyLexicalAnalyzerContext(lexicalAnalyzerContext);
    return UNKNOWN;
}