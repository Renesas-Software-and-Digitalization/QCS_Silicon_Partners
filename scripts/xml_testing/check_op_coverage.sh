#!/bin/bash

SCRIPT_DIR=$(dirname ${0})

# Check that we have use each operation at least once

# Get a list of all possible ops that can be used
cat ${SCRIPT_DIR}/ease_create_projects.py | grep -E "^\s+if seq\['op'\]" | cut -d '=' -f 3 | cut -d "'" -f 2 | sort | uniq > possible_ops.log

# Get a list of all the ops used in the tests
find ${SCRIPT_DIR}/.xml_tests -type f > test_files.log

rm -f test_ops.log

while read in; do
  cat ${in} | grep -E "^\s+\- op" | cut -d ':' -f 2 | cut -d ' ' -f 2 >> test_ops.log
done < test_files.log

cat test_ops.log | sort | uniq >> tested_ops.log

# Compare possible versus what has been tested
diff --ignore-trailing-space possible_ops.log tested_ops.log

if [ $? -ne 0 ]; then
  echo "Not all operations covered by tests"
  exit 1
fi
