#!/usr/bin/env bash

cd "$(dirname "$(realpath "$0")")"

MAKE_PATTERNS="0"
SANDBOX="0"
TARGET=""

function usage()
{
    cat << EOF
Usage: run-tests.sh <arguments>

Options:
  -h, --help             display this help and exit
  -p, --patterns         rebuild pattern files for targets
  -s, --sandbox          instead of running tests, run 'bash' in Xephyr
  -t, --target <test>    execute only <test>
                         (default: execute tests/*.sh)
EOF
}

while [[ $# -gt 0 ]]; do
    case $1 in
        -h|--help)
            usage
            exit 0
            ;;
        -p|--patterns)
            MAKE_PATTERNS="1"
            shift
            ;;
        -s|--sandbox)
            SANDBOX="1"
            shift
            ;;
        -t|--target)
            TARGET="$2"
            shift
            shift
            ;;
        *)
            echo "run-tests.sh: Error: Invalid argument '$1'."
            usage
            exit 1
            ;;
    esac
done

cmake -DWITH_COVERAGE=on .
cmake --build .

if [ "$SANDBOX" == "1" ]; then
    echo -e "\ntest-bash.sh: Execute commands in the Xephyr sandbox. Enter/Ctrl-c to exit."
    ./scripts/run-with-env.sh read
    exit $?
fi

if [ "$MAKE_PATTERNS" == "1" ]; then
    if [ "$TARGET" != "" ]; then
        ./scripts/run-with-env.sh scripts/patternize.py "$TARGET"
    else
        ./scripts/run-with-env.sh scripts/patternize.py tests/t_*.sh
    fi
fi

if [ "$TARGET" != "" ]; then
    ./scripts/run-with-env.sh scripts/verify.py "$TARGET"
    exit $?
fi

lcov -z --directory . > /dev/null
./scripts/run-with-env.sh scripts/verify.py tests/t_*.sh
lcov \
    --directory . \
    --capture \
    --output-file .report.info \
    > /dev/null 2> /dev/null
lcov \
    --directory . \
    --remove .report.info "/usr/*" \
    --output-file .report.info \
    > /dev/null 2> /dev/null
lcov --list .report.info
