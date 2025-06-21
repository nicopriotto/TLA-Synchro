#include <stdio.h>
#include <stdlib.h>

#include "shared/CompilerState.h"
#include "shared/Logger.h"
#include "shared/Type.h"
#include "shared/semantic-analysis/SymbolTable.h"
#include "shared/semantic-analysis/ScopeStack.h"
#include "shared/semantic-analysis/TypeChecking.h"
#include "frontend/syntactic-analysis/AbstractSyntaxTree.h"
#include "frontend/syntactic-analysis/SyntacticAnalyzer.h"
#include "frontend/syntactic-analysis/BisonActions.h"
#include "frontend/lexical-analysis/FlexActions.h"
#include "backend/domain-specific/SynchronizationRuntime.h"
#include "backend/code-generation/Generator.h"

/**
* The main entry point of the Compiler.
*/
int main(const int argumentCount, const char** arguments) {
   // Initialize logger
   Logger* logger = createLogger("EntryPoint");
   logDebugging(logger, "=== INITIALIZING MODULES ===");

   // Initialize ALL required modules
   initializeFlexActionsModule();
   initializeBisonActionsModule();
   initializeSyntacticAnalyzerModule();
   initializeSymbolTableModule();
   initializeTypeCheckingModule();
   initializeAbstractSyntaxTreeModule();
	 initializeSynchronizationRuntimeModule();
	 initializeGeneratorModule();

   logDebugging(logger, "=== MODULES INITIALIZED ===");
   
   // Create and initialize compiler state
   CompilerState compilerState;
   memset(&compilerState, 0, sizeof(CompilerState));

   // Initialize success flag to true - will be set to false if errors occur
   compilerState.succeed = true;
   
   // Initialize symbol table with built-in functions
   initSymbolTable(&compilerState.symbolTable);
   
   // Initialize scope stack
   initScopeStack(&compilerState.scopeStack);
   
   logDebugging(logger, "Compiler state initialized successfully");
   
   // Log arguments
   for (int i = 0; i < argumentCount; ++i) {
       logDebugging(logger, "Argument %d: \"%s\"", i, arguments[i]);
   }
   
   logDebugging(logger, "=== STARTING SYNTACTIC ANALYSIS ===");
   
   // Parse the input
   const SyntacticAnalysisStatus syntacticAnalysisStatus = parse(&compilerState);
   
   logDebugging(logger, "=== SYNTACTIC ANALYSIS COMPLETE ===");
   
   // Determine compilation result
   CompilationStatus compilationStatus = FAILED; // Default to failure
   
   if (syntacticAnalysisStatus == ACCEPT) {
       logDebugging(logger, "Syntactic analysis succeeded");
       if (compilerState.succeed) {
           logDebugging(logger, "Semantic analysis succeeded");
           compilationStatus = SUCCEED; // Only succeed if both syntactic and semantic analysis pass
       } else {
           logError(logger, "Semantic analysis failed");
           compilationStatus = FAILED;
       }
   } else {
       logError(logger, "The syntactic-analysis phase rejects the input program.");
       compilationStatus = FAILED;
   }

	if (compilationStatus == SUCCEED) {
		// ----------------------------------------------------------------------------------------
		// Beginning of the Backend... ------------------------------------------------------------
		logDebugging(logger, "Computing/validating program...");
		ComputationResult computationResult = computeProgram(compilerState.abstractSyntaxtTree);
		if (computationResult.succeed) {
			compilerState.value = computationResult.value;
			generate(&compilerState);
		}
		else {
			logError(logger, "The computation phase rejects the input program.");
			compilerState.succeed = false;
		}
		// ...end of the Backend. -----------------------------------------------------------------
		// ----------------------------------------------------------------------------------------
	}
	else {
		logError(logger, "The syntactic analysis rejects the input program.");
		compilerState.succeed = false;
	}
   
   // Clean up resources
   logDebugging(logger, "Releasing AST resources...");
   if (compilerState.abstractSyntaxtTree != NULL) {
       releaseProgram((Program*)compilerState.abstractSyntaxtTree);
   }
   
   logDebugging(logger, "Releasing symbol table...");
   freeSymbolTable(&compilerState.symbolTable);
   
   logDebugging(logger, "Releasing scope stack...");
   freeScopeStack(&compilerState.scopeStack);
   
   logDebugging(logger, "Shutting down modules...");
   shutdownAbstractSyntaxTreeModule();
   shutdownTypeCheckingModule();
   shutdownSymbolTableModule();
   shutdownSyntacticAnalyzerModule();
   shutdownBisonActionsModule();
   shutdownFlexActionsModule();
   
   logDebugging(logger, "Compilation is done.");
   
   // Clean up logger
   destroyLogger(logger);
   
   // Return the correct exit status
   return (int)compilationStatus; // This will be 0 for success, 1 for failure
}
