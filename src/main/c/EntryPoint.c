// #include "backend/code-generation/Generator.h"
// #include "backend/domain-specific/Calculator.h"
#include "frontend/lexical-analysis/FlexActions.h"
#include "frontend/syntactic-analysis/AbstractSyntaxTree.h"
#include "frontend/syntactic-analysis/BisonActions.h"
#include "frontend/syntactic-analysis/SyntacticAnalyzer.h"
#include "shared/CompilerState.h"
#include "shared/Environment.h"
#include "shared/Logger.h"
#include "shared/String.h"

/**
 * The main entry-point of the entire application. If you use "strtok" to
 * parse anything inside this project instead of using Flex and Bison, I will
 * find you, and I will kill you (Bryan Mills; "Taken", 2008).
 */
const int main(const int count, const char ** arguments) {

	Logger * logger = createLogger("EntryPoint");

	logDebugging(logger, "\nINITIALIZING MODULES!!!!\n");
	initializeFlexActionsModule();
	initializeBisonActionsModule();
	initializeSyntacticAnalyzerModule();
	initializeAbstractSyntaxTreeModule();
	initializeSymbolTableModule();
	initializeTypeCheckingModule();
	logDebugging(logger, "\nFINISHEDDD INITIALIZING MODULES!!!!\n");
	
	logDebugging(logger, "\n GENERATING COMPILER STATE!\n");
	CompilerState compilerState = {
		.abstractSyntaxtTree = NULL,
		.symbolTable = NULL,
		.scopeStack = initScopeStack(),
		.succeed = false,
		.value = 0
	};

	logDebugging(logger, "\n COMPILER STATE GENERATED ALL GOOD\n");

	logDebugging(logger, "\n CALLING PARSE PARSE PARSEEE\n");
	const SyntacticAnalysisStatus syntacticAnalysisStatus = parse(&compilerState);
	logDebugging(logger, "\n PARSED!! LETS PUSH SCOPE NOW BROTHER\n");
  logDebugging(logger, "\nEL PUNTERO ES: %p\n", &currentCompilerState()->scopeStack);

    insertSymbol(&(compilerState.symbolTable), "print", SYMBOL_FUNCTION, -1, NULL);
    insertSymbol(&(compilerState.symbolTable), "sleep", SYMBOL_FUNCTION, -1, NULL);
    insertSymbol(&(compilerState.symbolTable), "up", SYMBOL_FUNCTION, -1, NULL);
    insertSymbol(&(compilerState.symbolTable), "down", SYMBOL_FUNCTION, -1, NULL);
    insertSymbol(&(compilerState.symbolTable), "thread", SYMBOL_FUNCTION, -1, NULL);
	
	// pushScope(&currentCompilerState()->scopeStack);
		// logDebugging(logger, "\n PUSH DONE\n");
	
	// initializeCalculatorModule();
	// initializeGeneratorModule();

	// Logs the arguments of the application.
	for (int k = 0; k < count; ++k) {
		logDebugging(logger, "Argument %d: \"%s\"", k, arguments[k]);
	}

	// Begin compilation process.
	CompilationStatus compilationStatus = SUCCEED;
	Program * program = compilerState.abstractSyntaxtTree;	
	if (syntacticAnalysisStatus == ACCEPT) {
		// ----------------------------------------------------------------------------------------
		// Beginning of the Backend... ------------------------------------------------------------
		// logDebugging(logger, "Computing expression value...");
		// ComputationResult computationResult = computeExpression(program->expression);
		// if (computationResult.succeed) {
		// 	compilerState.value = computationResult.value;
		// 	generate(&compilerState);
		// }
		// else {
		// 	logError(logger, "The computation phase rejects the input program.");
		// 	compilationStatus = FAILED;
		// }
		// ...end of the Backend. -----------------------------------------------------------------
		// ----------------------------------------------------------------------------------------
	}
	else {
		logError(logger, "The syntactic-analysis phase rejects the input program.");
		compilationStatus = FAILED;
	}
	logDebugging(logger, "Releasing AST resources...");
	releaseProgram(program);
	logDebugging(logger, "Releasing modules resources...");
	// shutdownGeneratorModule();
	// shutdownCalculatorModule();
	// shutdownAbstractSyntaxTreeModule();
	// shutdownSyntacticAnalyzerModule();
	// shutdownBisonActionsModule();
	// shutdownFlexActionsModule();
	// shutdownTypeCheckingModule();
	// shutdownSymbolTableModule();
	logDebugging(logger, "Compilation is done.");
	destroyLogger(logger);
	return compilationStatus;
}
