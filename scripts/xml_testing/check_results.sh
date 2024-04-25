#!/bin/bash

# Do not run this in pipeline. These variables will already bet set by gitlab-runner.
if [ -z "${CI_PROJECT_DIR:+x}" ]; then
  export CI_PROJECT_DIR=/builds/peaks/peaks
  # Some bash scripts will rely on some of these variables being set. To get all of these variables into environment variables
  # in your local shell run this command.
  source <(yq e '.variables' .gitlab-ci.yml | grep ":" | sed -E 's/:[ ]?/=/g' | sed -E 's/^/export /g') > /dev/null
fi

# Check that number of projects built matches number of original projects specified. We're splitting up builds, so this is just checking we didn't miss any.
total_projects=$(grep -c "board_or_device" ${CI_PROJECT_DIR}/e2_studio/input/default_projects.yml)
echo "Total projects = ${total_projects}"

# Iterate over output YMLs and record total projects built
find ${CI_PROJECT_DIR}/e2_studio/output/ -name "*.yml" > yml_output_files.log
built_projects=0
mismatch_found=0

mkdir -p e2_studio/logs

# Read each YML file and do the following:
# 1. Record how many projects were built. We then sum these and compare against how many should have been built in total
while read yml_file; do
  built_projects=$((built_projects + $(grep -c "board_or_device" ${yml_file})))
  echo "  ${yml_file} - new total = ${built_projects}"
done < yml_output_files.log

echo "Built projects = ${built_projects}"

# Verify numbers match or fail
if [ ${total_projects} -ne ${built_projects} ]; then
  echo "ERROR - Not all projects were built"
  mismatch_found=$((mismatch_found + 1))
fi

echo "Total mismatches: ${mismatch_found}"
