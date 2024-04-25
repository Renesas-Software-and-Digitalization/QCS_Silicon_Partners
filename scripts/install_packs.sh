#!/bin/bash

# This will print error. If interactive mode is enabled then it will give user chance to see it.
# 1st argument is error code
# 2nd argument is error message
error_and_exit() {
  echo $2
  if [ ${TMP_INTERACTIVE} -ne 0 ]; then
    read -p "Please press enter to exit."
  fi
  exit $1
}

TMP_INTERACTIVE=0

# This script can be used after first install if it is called with "-it" as the 1st argument
if [ ${1:+x} ]; then
  if [ "${1}" = "-it" ]; then
    TMP_INTERACTIVE=1
  fi
fi

if [ ${ENV_FSP_PULL_NIGHTLY:-0} -eq 1 ] && [ ${ENV_FSP_PACKS_URL:+x} ]; then
  echo ">>>>>> ERROR <<<<<<"
  echo "In your docker-compose.yml file you have set ENV_FSP_PULL_NIGHTLY to 1 and"
  echo "also set a value to ENV_FSP_PACKS_URL. These options are mutually"
  echo "exclusive."
  echo ""
  error_and_exit 1 "Please pick one and create this container again."
fi

TMP_INSTALL_CLEAN=${ENV_FSP_CLEAN_INSTALL:-1}

if [ ${TMP_INTERACTIVE} -eq 1 ]; then
  echo "Interactive mode chosen. You must be on the company network for this to work."
  echo ""
  read -p "Please close e2 studio before continuing. Press enter to continue."
  echo ""
  echo "Chose type of installation:"
  echo "  1. Clean install. This will delete any existing packs and install new ones."
  echo "  2. Add new packs. This will keep existing packs and add additional ones."

  read TMP_INSTALL_TYPE

  if [ "${TMP_INSTALL_TYPE}" = "1" ]; then
    TMP_INSTALL_CLEAN=1
  elif [ "${TMP_INSTALL_TYPE}" = "2" ]; then
    TMP_INSTALL_CLEAN=0
  else
    error_and_exit 2 "ERROR:Invalid option selected."
  fi

  echo ""
  echo "Choose type of packs to install:"
  echo "  1. Latest nightly. This will download the latest nightly from Artifactory"
  echo "     for this image tag (latest or future) and install."
  echo "  2. Custom packs. You will be asked to provide a URL to a zip archive with "
  echo "     the packs. This could be a link from a development branch on Artifactory."
  echo "  3. Fuzzy find from Artifactory. You will be presented list of from fsp/manual"
  echo ""

  read TMP_PACK_TYPE

  if [ "${TMP_PACK_TYPE}" = "1" ]; then
    ENV_FSP_PULL_NIGHTLY=1
  elif [ "${TMP_PACK_TYPE}" = "2" ]; then
    ENV_FSP_PULL_NIGHTLY=0
    read -p "Enter URL:" ENV_FSP_PACKS_URL
    echo ""
  elif [ "${TMP_PACK_TYPE}" = "3" ]; then
    ENV_FSP_PULL_NIGHTLY=0
    # Get base directory
    manual_dir=$(curl -s --request GET "${ENV_ARTIFACT_BASE_PATH}/api/storage/${ENV_ARTIFACT_FSP_REPO}/manual" | grep '"uri" : "/' | cut -d '/' -f 2 | cut -d '"' -f 1 | fzf)
    # Pick a build. "--tac" is to reverse order so latest pack is first option selected
    manual_build=$(curl -s --request GET "${ENV_ARTIFACT_BASE_PATH}/api/storage/${ENV_ARTIFACT_FSP_REPO}/manual/${manual_dir}" | grep '"uri" : "/' | cut -d '/' -f 2 | cut -d '"' -f 1 | fzf --tac)
    # Get pack name
    manual_packs=$(curl -s --request GET "${ENV_ARTIFACT_BASE_PATH}/api/storage/${ENV_ARTIFACT_FSP_REPO}/manual/${manual_dir}/${manual_build}/packs" | grep '/FSP_Packs_v' | cut -d '/' -f2 | cut -d '"' -f 1)
    ENV_FSP_PACKS_URL="${ENV_ARTIFACT_BASE_PATH}/${ENV_ARTIFACT_FSP_REPO}/manual/${manual_dir}/${manual_build}/packs/${manual_packs}"
    echo ""
  else
    error_and_exit 3 "ERROR:Invalid option selected."
  fi
fi

if [ ${ENV_FSP_PULL_NIGHTLY:-0} -eq 1 ]; then
  # Get array of nightly build folders
  TMP_NIGHTLY_FOLDER=$(curl -s --request GET "${ENV_ARTIFACT_BASE_PATH}/api/storage/${ENV_ARTIFACT_FSP_REPO}/nightly/${ENV_FSP_BRANCH_NAME}" | jq '.children[-1].uri' | sed 's/\///g' | sed 's/"//g')
  echo "Download nightly - ${TMP_NIGHTLY_FOLDER} - from branch - ${ENV_FSP_BRANCH_NAME}"
  # Get name of packs. The pack name has git hash in name. eg: FSP_Packs_v2.4.0-alpha0%2B20210222.40b1e777.zip
  TMP_NIGHTLY_PACK_NAME=$(curl -s --request GET "${ENV_ARTIFACT_BASE_PATH}/api/storage/${ENV_ARTIFACT_FSP_REPO}/nightly/${ENV_FSP_BRANCH_NAME}/${TMP_NIGHTLY_FOLDER}/packs" | jq '.children[-1].uri' | sed 's/\///g' | sed 's/"//g')
  # Construct URL for the rest of the script to use.
  ENV_FSP_PACKS_URL="${ENV_ARTIFACT_BASE_PATH}/${ENV_ARTIFACT_FSP_REPO}/nightly/${ENV_FSP_BRANCH_NAME}/${TMP_NIGHTLY_FOLDER}/packs/${TMP_NIGHTLY_PACK_NAME}"
fi

# Replace "%2B" with "+" in URL if present.
ENV_FSP_PACKS_URL=$(echo ${ENV_FSP_PACKS_URL} | sed -e 's/%2B/+/g')

if [ ${ENV_FSP_PACKS_URL:+x} ]; then
  TMP_FSP_PACK_FILENAME=${ENV_FSP_PACKS_URL##*/}
  # Take off FSP_Packs_v
  TMP_FSP_PACK_VERSION=${TMP_FSP_PACK_FILENAME:11}
  # Take off extension
  TMP_FSP_PACK_VERSION=${TMP_FSP_PACK_VERSION%.*}

  echo "Installing FSP v${TMP_FSP_PACK_VERSION}. Please wait for confirmation"
  echo "that this has completed before using e2 studio."

  # Find e2 studio support area in user home directory
  TMP_E2STUDIO_DATA_DIR="$(find /root/.eclipse -mindepth 1 -maxdepth 1 -regextype sed -regex ".*/com\.renesas\.platform_[0-9]*" -type d)"

  # Delete existing FSP installation (not e2 studio)
  if [ ${TMP_INSTALL_CLEAN} -eq 1 ]; then
    rm -rf ${TMP_E2STUDIO_DATA_DIR}
  fi

  # If first run, then create clean workspace
  if [ ${TMP_INTERACTIVE} -eq 0 ]; then

    # Save off IAR configuration for e2 studio
    if [ -f ${ENV_E2STUDIO_DEFAULT_WS}/.metadata/.plugins/org.eclipse.core.runtime/.settings/com.iar.ide.common.ui.prefs ]; then
      cp ${ENV_E2STUDIO_DEFAULT_WS}/.metadata/.plugins/org.eclipse.core.runtime/.settings/com.iar.ide.common.ui.prefs /tmp
    fi

    rm -rf ${ENV_E2STUDIO_DEFAULT_WS}

    # Make default e2studio workspace dir and restore IAR configuration for e2 studio
    if [ -f /tmp/com.iar.ide.common.ui.prefs ]; then
      mkdir -p ${ENV_E2STUDIO_DEFAULT_WS}/.metadata/.plugins/org.eclipse.core.runtime/.settings/
      cp /tmp/com.iar.ide.common.ui.prefs ${ENV_E2STUDIO_DEFAULT_WS}/.metadata/.plugins/org.eclipse.core.runtime/.settings/
    fi

    # This will recreate the workspace and e2 studio data directoy
    /usr/bin/e2studio --launcher.suppressErrors -nosplash -application org.eclipse.cdt.managedbuilder.core.headlessbuild \
      -data ${ENV_E2STUDIO_DEFAULT_WS} \
      -cleanBuild all > /tmp/e2studio.log 2>&1
  fi

  # Download FSP pack

  # If the pack is available in the cache directory, attempt to copy from there.
  if [ ${ENV_FSP_PACK_CACHE_DIR:+x} ] && [ -f ${ENV_FSP_PACK_CACHE_DIR}${BUILD_ARTIFACT_FOLDER}/${TMP_FSP_PACK_FILENAME} ]; then
    echo "  - Downloading FSP packs from cache: ${ENV_FSP_PACK_CACHE_DIR}${BUILD_ARTIFACT_FOLDER}/${TMP_FSP_PACK_FILENAME}"
    timeout 60s cp ${ENV_FSP_PACK_CACHE_DIR}${BUILD_ARTIFACT_FOLDER}/${TMP_FSP_PACK_FILENAME} .
    if [ $? -eq 0 ]; then
      TMP_CACHE_PULL_SUCCESS=1
    fi
  fi

  # If we could not pull packs from cache, download from Artifactory
  if [ -z ${TMP_CACHE_PULL_SUCCESS} ]; then
    echo "  - Downloading FSP packs: ${ENV_FSP_PACKS_URL}"
    wget --quiet --progress=bar:force:noscroll ${ENV_FSP_PACKS_URL}
  fi

  # Check if download was successful
  if [ $? -ne 0 ]; then
    error_and_exit 4 "ERROR: Packs could not be downloaded from: ${ENV_FSP_PACKS_URL}"
  fi

  # Install FSP packs into e2studio data directory
  unzip -o -q -d ${TMP_E2STUDIO_DATA_DIR} ${TMP_FSP_PACK_FILENAME}

  # Check to make sure valid zip was given
  if [ $? -ne 0 ]; then
    error_and_exit 5 "ERROR: Packs could not be extracted from: ${TMP_FSP_PACK_FILENAME}"
  fi

  # To skip installation of INTERNAL packs, set environment variable ENV_FSP_SKIP_INTERNAL to 1 in docker-compose.yml
  if [ ${ENV_FSP_SKIP_INTERNAL:-1} -eq 0 ]; then
    # Download INTERNAL packs too
    TMP_FSP_PACK_INTERNAL_URL=$(echo ${ENV_FSP_PACKS_URL} | sed 's/FSP_Packs/FSP_Packs_INTERNAL/g')
    TMP_FSP_PACK_INTERNAL_FILENAME=${TMP_FSP_PACK_INTERNAL_URL##*/}

     # If the pack is available in the cache directory, attempt to copy from there.
    if [ ${ENV_FSP_PACK_CACHE_DIR:+x} ] && [ -f ${ENV_FSP_PACK_CACHE_DIR}${BUILD_ARTIFACT_FOLDER}/${TMP_FSP_PACK_INTERNAL_FILENAME} ]; then
      echo "  - Downloading FSP Internal packs from cache: ${ENV_FSP_PACK_CACHE_DIR}${BUILD_ARTIFACT_FOLDER}/${TMP_FSP_PACK_INTERNAL_FILENAME}"
      timeout 60s cp ${ENV_FSP_PACK_CACHE_DIR}${BUILD_ARTIFACT_FOLDER}/${TMP_FSP_PACK_INTERNAL_FILENAME} .
      if [ $? -eq 0 ]; then
        TMP_CACHE_PULL_INTERNAL_SUCCESS=1
      fi
    fi

    # If we could not pull packs from cache, download from Artifactory
    if [ -z ${TMP_CACHE_PULL_INTERNAL_SUCCESS} ]; then
      echo "  - Downloading FSP INTERNAL packs: ${TMP_FSP_PACK_INTERNAL_URL}"
      wget --quiet --progress=bar:force:noscroll ${TMP_FSP_PACK_INTERNAL_URL}
    fi

    # Check if download was successful
    if [ $? -ne 0 ]; then
      error_and_exit 4 "ERROR: Packs could not be downloaded from: ${TMP_FSP_PACK_INTERNAL_URL}"
    fi

    # Install Internal FSP packs into e2studio data directory
    unzip -o -q -d ${TMP_E2STUDIO_DATA_DIR} ${TMP_FSP_PACK_INTERNAL_FILENAME}

    # Check to make sure valid zip was given
    if [ $? -ne 0 ]; then
      error_and_exit 5 "ERROR: Packs could not be extracted from: ${TMP_FSP_PACK_INTERNAL_FILENAME}"
    fi
  fi

  # Have e2studio install the packs
  echo "  - Installing new FSP packs"
  /usr/bin/e2studio --launcher.suppressErrors -nosplash -application org.eclipse.ease.runScript \
    -data ${ENV_E2STUDIO_DEFAULT_WS} \
    -script /e2_studio/scripts/minimum_ease.py > /tmp/e2studio.log 2>&1 || true

  # Record what happened
  echo "Using custom FSP packs - ${ENV_FSP_PACKS_URL}" > /e2_studio/startup.log
  echo "${TMP_FSP_PACK_VERSION}" > /e2_studio/fsp.version.log
else
  echo "Using default - FSP v${ENV_FSP_RELEASE}."
  echo "Using default image - ${ENV_FSP_RELEASE}" > /e2_studio/startup.log
  echo "${ENV_FSP_RELEASE}" > /e2_studio/fsp.version.log
fi

# If a workspace is given for input, then import it. This only occurs on the first run
if [ ${ENV_FSP_WORKSPACE_URL:+x} ] && [ ${TMP_INTERACTIVE} -eq 0 ]; then
  TMP_WORKSPACE_FILENAME=${ENV_FSP_WORKSPACE_URL##*/}

  echo "Installing workspace projects ${TMP_WORKSPACE_FILENAME}."
  echo "Please wait for confirmation that this has completed before using e2 studio."

  # Download workspace zip
  echo "  - Downloading workspace: ${ENV_FSP_WORKSPACE_URL}"
  wget --quiet --progress=bar:force:noscroll ${ENV_FSP_WORKSPACE_URL}

  # Check if download was successful
  if [ $? -ne 0 ]; then
    error_and_exit 6 "ERROR: Workspace could not be downloaded from: ${ENV_FSP_WORKSPACE_URL}"
  fi

  # Unzip workspace projects into default e2studio workspace directory
  unzip -o -q -d ${ENV_E2STUDIO_DEFAULT_WS} ${TMP_WORKSPACE_FILENAME}

  # Check to make sure valid zip was given
  if [ $? -ne 0 ]; then
    error_and_exit 7 "ERROR: Workspace could not be extracted from: ${TMP_WORKSPACE_FILENAME}"
  fi

  # Import projects
  /usr/bin/e2studio --launcher.suppressErrors -nosplash -application org.eclipse.cdt.managedbuilder.core.headlessbuild \
    -data ${ENV_E2STUDIO_DEFAULT_WS} \
    -importAll "${ENV_E2STUDIO_DEFAULT_WS}"

  echo "  - Workspace projects imported successfully"
fi

# Connect to IAR license server if specified
if [ -n "${ENV_IAR_LICENSE_HOST:+x}" ]; then
  /opt/iarsystems/${ENV_IAR_DEB%.*}/common/bin/lightlicensemanager setup --host ${ENV_IAR_LICENSE_HOST}
  if [ $? -ne 0 ]; then
    echo ""
    echo ">>>>>> WARNING <<<<<<"
    echo "Provided IAR license server cannot be reached: ${ENV_IAR_LICENSE_HOST}"
    echo "No IAR projects can be built currently."
  fi
fi

if [ -z ${ENV_IAR_LICENSE_HOST:+x} ]; then
  echo ""
  echo ">>>>>> INFO <<<<<<"
  echo "No IAR license server host provided. No IAR projects can be built currently."
fi

# If this is not CI environment, then give user indication that install has completed
if [ -z ${CI:+x} ]; then
  # Start e2 studio
  nohup /usr/bin/e2studio -data ${ENV_E2STUDIO_DEFAULT_WS} > /dev/null 2>&1 &
  echo ""
  echo "Installation complete. e2 studio can now be used."
  read -p "Press enter to close this terminal."
fi
