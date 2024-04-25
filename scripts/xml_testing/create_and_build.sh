#!/bin/bash

# Arguments
# 1st - Is the filter for projects to build. Using "RA6M2" will build all projects from $TMP_DEFAULT_PROJECTS_YML that have "RA6M2" in the name.
# 2nd - Whether to inverse the match of 1st argument. To build for all boards (not MCU part number selection) we can check for != "R7A" instead of searching for "EK","FPB", etc. 0 is do not inverse (default). 1 means to inverse the match.

# Do not run this in pipeline. These variables will already bet set by gitlab-runner.
if [ -z "${CI_PROJECT_DIR:+x}" ]; then
  export CI_PROJECT_DIR=/builds/peaks/peaks
  # Some bash scripts will rely on some of these variables being set. To get all of these variables into environment variables
  # in your local shell run this command.
  source <(yq e '.variables' .gitlab-ci.yml | grep ":" | sed -E 's/:[ ]?/=/g' | sed -E 's/^/export /g') > /dev/null
fi

TMP_E2STUDIO_WORKSPACE=${ENV_WORKSPACE_PATH:-/e2_studio/workspace}
TMP_DEFAULT_PROJECTS_YML=${CI_PROJECT_DIR}/e2_studio/input/default_projects.yml

# Create output dir
mkdir -p ${CI_PROJECT_DIR}/e2_studio/output

# Can pass first argument to filter list of what will be built.
# eg: passing in "Azure" will only build projects with "Azure" in the "name"
if [ ${1:+x} ]; then
  echo "Filtering projects on: ${1}"
  TMP_OUTPUT_YML=${CI_PROJECT_DIR}/e2_studio/output/projects_$(echo ${1} | sed 's/\*/_and_/g').yml
  # Create base file to update
  cp ${TMP_DEFAULT_PROJECTS_YML} ${TMP_OUTPUT_YML}
  # Create new project list based on filter
  export YML_FILTER=${1}
  # This will look through all array entries and keep the ones where the .name member has ${1} as a substring
  # Checking argument 2 to see if we need to inverse (!=) the match
  if [ ${2:+x} ] && [ ${2} -eq 1 ]; then
    echo "Inverting match"
    yq -iP e '(.[] | select(.name != "*" + env(YML_FILTER) + "*")) | . as $item ireduce ([]; . + $item)' ${TMP_OUTPUT_YML}
  else
    yq -iP e '(.[] | select(.name == "*" + env(YML_FILTER) + "*")) | . as $item ireduce ([]; . + $item)' ${TMP_OUTPUT_YML}
  fi
else
  TMP_OUTPUT_YML=${CI_PROJECT_DIR}/e2_studio/output/projects.yml
  cp ${TMP_DEFAULT_PROJECTS_YML} ${TMP_OUTPUT_YML}
fi

# The ease_create_projects.py script is expecting the project list at /e2_studio/input/projects.json. Convert YML project list to JSON
mkdir -p /e2_studio/input
yq e -j ${TMP_OUTPUT_YML} > /e2_studio/input/projects.json

# Create projects based on default list and run through defined sequence in YML file. This can including building multiple times.
mkdir -p ${CI_PROJECT_DIR}/e2_studio/logs
# An exception in the python script in the next command can be masked by the other commands down the pipe. We need to keep the error return
# code from e2studio. We can capture this using the pipefail option. From the bash reference:
#   The exit status of a pipeline is the exit status of the last command in the pipeline, unless the pipefail option is enabled (see The Set Builtin).
#   If pipefail is enabled, the pipeline's return status is the value of the last (rightmost) command to exit with a non-zero status, or zero if all
#   commands exit successfully.
set -o pipefail
# The grep on the end is to ignore the messages output by e2studio. We only want logging messages from our script.
/usr/bin/e2studio --launcher.suppressErrors -nosplash -application org.eclipse.ease.runScript -data ${TMP_E2STUDIO_WORKSPACE} -script ${SCRIPT_DIR:-/e2_studio/scripts/xml_testing}/ease_create_projects.py ${CI_PROJECT_DIR}/e2_studio/logs/default_project_create_stderr.log 2>&1 | tee ${CI_PROJECT_DIR}/e2_studio/logs/default_project_create.log | grep "ease_create_projects.py" --line-buffered
# If an exception occurred that we're not catching in the Python script then e2studio may exit with a non-zero error code
if [ $? -ne 0 ]; then
  echo ""
  cat "${CI_PROJECT_DIR}/e2_studio/logs/default_project_create.log"
  echo ""
  echo "e2 studio exited with non-zero return code. See ${CI_PROJECT_DIR}/e2_studio/logs/default_project_create.log output above."
  exit 1
fi

# Check for any errors. File may be created and empty.
if [ -f ./e2_studio/logs/errors.log ] && [ $(stat --printf="%s" ./e2_studio/logs/errors.log) -gt 0 ]; then
  echo ""
  cat "${CI_PROJECT_DIR}/e2_studio/logs/errors.log"
  echo ""
  echo "Errors were encountered while creating projects. See ${CI_PROJECT_DIR}/e2_studio/logs/errors.log output above."
  exit 1
fi

# Count number of projects
TMP_NUMBER_OF_PROJECTS=$(yq e '. | length' ${TMP_OUTPUT_YML})

# Compare ELFs found. IAR uses *.out. AC6 uses *.axf
find ${TMP_E2STUDIO_WORKSPACE} -name "*.elf" -o -name "*.out" -o -name "*.axf" > found_elfs.log

TMP_NUMBER_OF_ELFS=$(wc -l < found_elfs.log)

echo "Number of projects: ${TMP_NUMBER_OF_PROJECTS}"
echo "Number of elfs    : ${TMP_NUMBER_OF_ELFS}"

# Upload artifact to artifact repo if this is run in pipeline
if [ "${CI:-0}" == "1" ]; then
  curl -ugitlab:${BUILD_ARTIFACT_API_KEY} -T ${TMP_OUTPUT_YML} "${BUILD_ARTIFACT_PATH_FULL}/e2studio_artifacts/$(basename ${TMP_OUTPUT_YML})"
fi
