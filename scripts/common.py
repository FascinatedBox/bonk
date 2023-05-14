import re
import subprocess

def _lines_for_input(input_bytes):
    return input_bytes.decode('utf-8').split("\n")

def _window_pattern(window, window_cache):
    result = None

    for i in range(len(window_cache)):
        w = window_cache[i]

        if window == w:
            result = "@%d" % (i)
            break

    if result == None:
        result = "@%d" % (len(window_cache))
        window_cache.append(window)

    return result

def _pattern_for_input(input_bytes):
    input_lines = _lines_for_input(input_bytes)
    result = []
    window_rx = re.compile("window=(0x\\S+)")
    show_rx = re.compile("(0x\\S+)")

    # Types are formatted as "type=<atom id>(atom name)". Replace out the id so
    # that tests don't fail if atoms load in a different order.
    type_rx = re.compile("type=(0x[0-9a-f]+)")

    # Same as above.
    property_rx = re.compile("property=(0x[0-9a-f]+)")
    window_cache = []

    if input_lines[0].find("MIT-MAGIC-COOKIE-1") == -1:
        return input_lines

    for l in input_lines:
        if l.startswith("000:>"):
            continue

        if l.startswith("000"):
            matches = window_rx.findall(l)

            if matches:
                for m in matches:
                    p = _window_pattern(m, window_cache)
                    l = l.replace("window=" + m, "window=" + p)
        elif l.startswith("0x"):
            matches = show_rx.findall(l)

            if matches:
                for m in matches:
                    p = _window_pattern(m, window_cache)
                    l = l.replace(m, p)

        l = re.sub(type_rx, "type=", l)
        l = re.sub(property_rx, "property=", l)
        result.append(l)

    return result

def divider(name):
    return "-----%s-----" % (name)

def exec_path_for_pattern(test_path):
    bonk_args = [test_path]
    p = subprocess.Popen(bonk_args, \
                         stdout=subprocess.PIPE, \
                         stderr=subprocess.PIPE)
    (p_stdout, p_stderr) = p.communicate()
    p_stdout = _pattern_for_input(p_stdout)
    p_stderr = _lines_for_input(p_stderr)

    f = open(test_path)
    source_text = f.read().split("\n")
    f.close()

    result = {
        "source": source_text,
        "stdout": p_stdout,
        "stderr": p_stderr,
        "exitcode": p.returncode,
    }

    return result

def pattern_path_for(script_path):
    return script_path.replace(".sh", ".pattern") \
                      .replace("tests", "patterns")
