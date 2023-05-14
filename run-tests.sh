lcov -z --directory . > /dev/null
./scripts/run-with-env.sh scripts/verify.py tests/t_*.sh
lcov \
    --directory . \
    --capture --output-file .report.info \
    > /dev/null 2> /dev/null
lcov \
    --remove .report.info "/usr/*" \
    --output-file .report.info \
    > /dev/null
genhtml -o reportdata .report.info > /dev/null
