#!/bin/bash

# Do not run this in pipeline. These variables will already bet set by gitlab-runner.
if [ -z "${CI_PROJECT_DIR:+x}" ]; then
  export CI_PROJECT_DIR=/builds/peaks/peaks
  # Some bash scripts will rely on some of these variables being set. To get all of these variables into environment variables
  # in your local shell run this command.
  source <(yq e '.variables' .gitlab-ci.yml | grep ":" | sed -E 's/:[ ]?/=/g' | sed -E 's/^/export /g') > /dev/null
fi

TMP_E2STUDIO_WORKSPACE=/e2_studio/workspace
TMP_DEFAULT_PROJECTS_YML=${CI_PROJECT_DIR}/e2_studio/input/default_projects.yml
export TMP_DEFAULT_PROJECTS_JSON=$(echo ${TMP_DEFAULT_PROJECTS_YML} | sed 's/\.yml/\.json/g')
TMP_EASE_INFO_YML=/e2_studio/input/ease_info.yml

mkdir -p ${CI_PROJECT_DIR}/e2_studio/logs
mkdir -p $(dirname ${TMP_DEFAULT_PROJECTS_YML})
mkdir -p $(dirname ${TMP_EASE_INFO_YML})

# Create information file for use by EASE script. There is not a way I'm aware of to pass information through command line to EASE
yq -n e '.install_info = env(CI_PROJECT_DIR) + "/e2_studio/input/install_info.json"' > ${TMP_EASE_INFO_YML}
yq e -iP '.default_projects = env(TMP_DEFAULT_PROJECTS_JSON)' ${TMP_EASE_INFO_YML}
yq e -j ${TMP_EASE_INFO_YML} > /e2_studio/input/ease_info.json

# Generate JSON structure with details about FSP that is installed
/usr/bin/e2studio --launcher.suppressErrors -nosplash -application org.eclipse.ease.runScript -data ${TMP_E2STUDIO_WORKSPACE} -script ${CI_PROJECT_DIR}/scripts/xml_testing/ease_gen_default_projects.py > ${CI_PROJECT_DIR}/e2_studio/logs/default_project_gen.log 2>&1

# Take output of first script and make it input to script to create projects
# Convert JSON to YML for future use
yq -P e ${TMP_DEFAULT_PROJECTS_JSON} > ${TMP_DEFAULT_PROJECTS_YML}
