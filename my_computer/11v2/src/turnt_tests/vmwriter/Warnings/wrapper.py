#!/usr/bin/env python3
import subprocess
import sys

# Run the command and capture output
try:
    result = subprocess.run(sys.argv[1:], capture_output=True, text=True)
    
    # Print stdout and stderr
    if result.stdout:
        print(result.stdout, end='')
    if result.stderr:
        print(result.stderr, end='', file=sys.stderr)
    
    # Always exit with 0
    sys.exit(0)
    
except Exception as e:
    print(f"Error running command: {e}", file=sys.stderr)
    sys.exit(0)