#!/bin/bash

set -euo pipefail

# Get absolute path to project root
BASE_PATH="$(cd "$(dirname "$0")/../.." && pwd)"
cd "$BASE_PATH"

# Ensure input is provided
if [[ $# -lt 1 ]]; then
  echo "Usage: $0 <input-file> [compiler-args...]"
  exit 1
fi

INPUT="$1"
shift 1

# Ensure input file exists
if [[ ! -f "$INPUT" ]]; then
  echo "Input file '$INPUT' not found."
  exit 1
fi

# Ensure compiler is executable
if [[ ! -x build/Compiler ]]; then
  echo "'build/Compiler' not found or not executable."
  exit 1
fi

# Run the compiler
cat "$INPUT" | build/Compiler "$@"
