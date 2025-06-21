#!/bin/bash

set -euxo pipefail

BASE_PATH="$(cd "$(dirname "$0")/../.." && pwd)"
cd "$BASE_PATH"

# Check if Homebrew is installed
if ! command -v brew &> /dev/null; then
  echo "Homebrew not found. Please install Homebrew first: https://brew.sh/"
  exit 1
fi

# Install required packages using Homebrew
brew install bison
brew install cmake
brew install flex
brew install gcc
brew install make

echo "All done."
