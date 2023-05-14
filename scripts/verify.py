#!/usr/bin/env python
# Usage: verify.py <path to test file>
# verify.py: Run a test file to determine if the output matches what's expected.
#            Some replacements are performed: Window ids are simplified,
#            inconsistent fields are removed, and some output not considered.

import sys
from common import (divider, exec_path_for_pattern, pattern_path_for)

def read_pattern_file(path):
    f = open(path, "r")
    lines = f.read().split("\n")
    f.close()

    p_command = None
    p_stdout = []
    p_stderr = []
    p_exit = None

    if lines[0] != divider("source"):
        print("Error: %s does not look like a pattern file (wrong header)." % (path))
        return None

    section = None
    data = {}
    start_index = 0

    for i in range(len(lines)):
        l = lines[i]

        if l.startswith("-----"):
            if section != None:
                area = lines[start_index:i]
                data[section] = area

            section = l[5:-5]
            start_index = i + 1

    area = lines[start_index:]
    data[section] = area

    result = {
        "source": data["source"],
        "stdout": data["stdout"],
        "stderr": data["stderr"],
        "exitcode": int(data["exitcode"][0]),
    }

    return result

def process_pattern(script_path):
    pattern_path = pattern_path_for(script_path)
    input_pattern = read_pattern_file(pattern_path)
    exec_pattern = exec_path_for_pattern(script_path)

    if input_pattern == exec_pattern:
        print("OK: Test pattern '%s'." % (script_path))
    else:
        print("Error: Mismatch in pattern '%s'." % (script_path))

if __name__ == '__main__':
    for target in sys.argv[1:]:
        process_pattern(target)
