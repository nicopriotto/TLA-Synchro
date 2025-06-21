#!/bin/bash

set -u

# Resolve absolute path to project root
BASE_PATH="$(cd "$(dirname "$0")/../.." && pwd)"
cd "$BASE_PATH"

# Color codes (portable for macOS)
GREEN='\033[0;32m'
RED='\033[0;31m'
OFF='\033[0m'

STATUS=0

echo "Compiler should accept..."
echo ""

for test in src/test/c/accept/*; do
	if [ -f "$test" ]; then
		if cat "$test" | build/Compiler >/dev/null 2>&1; then
			echo -e "    $(basename "$test"), ${GREEN}and it does${OFF} (status 0)"
		else
			STATUS=1
			echo -e "    $(basename "$test"), ${RED}but it rejects${OFF} (non-zero status)"
		fi
	fi
done
echo ""

echo "Compiler should reject..."
echo ""

for test in src/test/c/reject/*; do
	if [ -f "$test" ]; then
		if ! cat "$test" | build/Compiler >/dev/null 2>&1; then
			echo -e "    $(basename "$test"), ${GREEN}and it does${OFF} (non-zero status)"
		else
			STATUS=1
			echo -e "    $(basename "$test"), ${RED}but it accepts${OFF} (status 0)"
		fi
	fi
done
echo ""

echo "All done."
exit $STATUS
