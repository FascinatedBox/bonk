#!/usr/bin/env python
# Usage: patternize.py <path to test file>
# patternize.py: Run a test file and record the output it makes into a pattern
#                to check against later. The generated pattern file should be
#                manually inspected to make sure it is sensible.

import sys
from common import (divider, exec_path_for_pattern, pattern_path_for)

def write_divider(f, pattern, name):
    print(divider(name), file=f)
    print(str((pattern[name])), file=f)

def write_std_lines(f, input_lines, name):
    print(divider(name), file=f)

    for l in input_lines:
        print(l, file=f)

def build_pattern(script_path, pattern_path):
    pattern = exec_path_for_pattern(script_path)
    f = open(pattern_path, "w")

    write_std_lines(f, pattern["source"], "source")
    write_std_lines(f, pattern["stdout"], "stdout")
    write_std_lines(f, pattern["stderr"], "stderr")
    write_divider(f, pattern, "exitcode")
    f.close()

if __name__ == '__main__':
    for script_path in sys.argv[1:]:
        if script_path.endswith(".sh") == False:
            print("patternize.py: Filename does not end with '.sh'. Skipping.")
            continue

        pattern_path = pattern_path_for(script_path)
        build_pattern(script_path, pattern_path)
        print("patternize.py: Generated pattern at '%s'." % (pattern_path))
