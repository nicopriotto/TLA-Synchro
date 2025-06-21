#!/bin/bash

set -e

# Resolve absolute path to project root
BASE_PATH="$(cd "$(dirname "$0")/../.." && pwd)"
cd "$BASE_PATH"

echo "Building compiler..."

# Create build directory
mkdir -p build

# Generate lexer and parser
echo "Generating lexer..."
flex --outfile=src/main/c/frontend/lexical-analysis/FlexScanner.c src/main/c/frontend/lexical-analysis/FlexPatterns.l

echo "Generating parser..."
bison --defines=src/main/c/frontend/syntactic-analysis/BisonParser.h --output=src/main/c/frontend/syntactic-analysis/BisonParser.c src/main/c/frontend/syntactic-analysis/BisonGrammar.y

# Compile the project
echo "Compiling..."
cmake -S . -B build
cmake --build build

echo "Build completed successfully!"
