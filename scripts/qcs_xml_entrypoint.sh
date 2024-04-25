#!/bin/bash

set -e

usage()
{
  echo "NAME:"
  echo "  qcs_xml_entrypoint.sh"
  echo
  echo "SYNOPSIS:"
  echo "  qcs_xml_entrypoint.sh [OPTIONS]"
  echo
  echo "DESCRIPTION:"
  echo "  This script is the entrypoint of QCS development for proceeding --"
  echo "    1. *Build Project*: Clean/incremental build existing e2s/qcs project"
  echo "    2. *XML Testing*:   To generate qcs/e2s project from YAML templates (standalone or module templates) then build the project"
  echo "  With the support of --"
  echo "    * Cloning from specified remote Git repo"
  echo "    * Installing/reinstalling specified FSP packs from official releases"
  echo
  echo "OPTIONS:"
  echo
  echo " Local Environment Relative -"
  echo "  -w    <PATH>              Path to QCS/e2s workspace.               [default: \$ENV_WORKSPACE_PATH or current folder]"
  echo "  -b    <PATH>              Build path where to look up for projects [default: \$ENV_BUILD_PATH or current folder]"
  echo "                            Also the local path for cloning the remote git repo, if specified"
  echo "  -l    <PATH>              Intermediate path between build path and project."
  echo "                            A use cae of this option is for Git repos contain intermediate folders."
  echo "                            e.g. for the case of repo structure *f1/f2/project*, use *-b /tmp -l repo/f1/f2 -b project*"
  echo "  -p    <NAME>              Project name.                            [default: \$ENV_PROJECT or 'project']"
  echo "  -k                        By default, workspace will be recreated on every execution."
  echo "                            If any other use cases, specifying -k will retain the existing workspace."
  echo
  echo " FSP Packs Download Configurations -"
  echo "  -fg   <VERSION>           Set the FSP version to force a re-download and re-installation from the official GitHub release page"
  echo "                            [i.e. given *4.6.0* to download FSP_Packs_v4.6.0.zip or *latest* to download the latest release version]"
  echo
  echo " Git Repo Relative -"
  echo "  -o    <URI>               Remote Git repo clone URL                [default: \$REPO_URL]"
  echo "  -t    <BRANCH>            Remote branch to checkout                [default: \$REPO_BRANCH]"
  echo "  -r    f/u/n               What to do with the local repo"
  echo "    f:                      Force clone from remote repo. The default behavior if given project folder is empty"
  echo "    u:                      Retrieve latest updates from remote repo and continue"
  echo "    n:                      Do nothing and leave as-is"
  echo
  echo " Build Existing Project Relative -"
  echo "  -d    a/*                 Build project directly without template combination processes"
  echo "                            Note: when set, -y -m -s -c values will be ignored"
  echo "    a:                      Build all existing configurations at once"
  echo "    *:                      Build the given configuration by name. i.e. given '-d Debug' will trigger a clean build with 'Debug' configuration"
  echo "  -i                        Incrementally build the project. This option only affects if -d is given"
  echo "                            Also, given this option implies -k to keep current workspace"
  echo
  echo " XML Testing Relative, to generate QCS project from YAML template files -"
  echo "  -yp   <PATH>              Project template file                    [default: \$ENV_YML_PROJECT if set]"
  echo "                            Note: when set, -yc -m -s -c values will be ignored"
  echo "  -yc   <PATH>/<YML>        Component based project definition       [default: \$ENV_YML_COMPONENTS if set]"
  echo "                            Note: when set, -m -s -c values will be ignored"
  echo "  -m    <VALUE>             MCU/OS name of the template              [default: \$ENV_YML_MCU if set]"
  echo "  -s    <VALUE>             Comma separated SENSORS template names   [default: \$ENV_YML_SENSORS if set]"
  echo "  -c    <VALUE>             Comma separated CONNECTIVITY names       [default: \$ENV_YML_CONNECTIVITY if set]"
  echo
  echo " Miscellaneous -"
  echo "  -z    <PATH>              Zip project content from workspace path to the specified file path."
  echo "  -j    <PATH>[,<PATH>...]  Run e2s code formatting on the given comma separated paths (relative to project folder)."
  echo "  -v                        Verbose/Debug mode."
  echo "  -?                        Show this help."
  echo
}

show()
{
  echo "Current settings --"
  echo ""
  echo " - REPO URL:              ${REPO_URL}"
  echo " - REPO BRANCH:           ${REPO_BRANCH}"
  echo " - FSP VERSION:           ${ENV_FSP_RELEASE}"
  echo " - WORKSPACE DIR:         ${ENV_WORKSPACE_PATH}"
  echo " - PROJECT DIR:           ${CI_PROJECT_DIR}"
  echo " - SCRIPTS DIR:           ${SCRIPT_DIR}"
  echo " - PROJECT TEMPLATE:      ${ENV_YML_PROJECT}"
  echo " - PROJECT COMPONENTS:    ${ENV_YML_COMPONENTS}"
  echo " - MCU TEMPLATE:          ${ENV_YML_MCU}"
  echo " - SENSORS TEMPLATE:      ${ENV_YML_SENSORS}"
  echo " - CONNECTIVITY TEMPLATE: ${ENV_YML_CONNECTIVITY}"
  echo " - VERBOSE:               ${ENV_DEBUG}"
  echo ""
}

exec_build()
{
  TMP_DB_CONF=all
  if [ "${ENV_DIRECT_BUILD_TARGET}" != "a" ]; then
    TMP_DB_CONF="${ENV_PROJECT}/${ENV_DIRECT_BUILD_TARGET}"
  fi

  TMP_PROJ_PATH="${CI_PROJECT_DIR}"
  if [ "${ENV_INTERMEDIATE_PATH:+x}" ]; then
    TMP_PROJ_PATH="${ENV_BUILD_PATH}/${ENV_INTERMEDIATE_PATH}/${ENV_PROJECT}"
  fi

  TMP_BUILD_OPTS=""
  if [ ${ENV_CLEAN_WORKSPACE:-1} -eq 1 ]; then
    TMP_BUILD_OPTS="-import ${TMP_PROJ_PATH}"
  fi

  exec_format "$TMP_PROJ_PATH"

  echo
  echo "[=== DIRECT BUILD ===]"
  echo
  echo "... direct ${ENV_DIRECT_BUILD_ACTION:-cleanBuild} the [$TMP_DB_CONF] configuration"
  if [ "${ENV_DEBUG:+x}" ]; then
      echo "...... options to use: ${TMP_BUILD_OPTS}"
  fi
  echo
  /bin/e2studio --launcher.suppressErrors -nosplash -application org.eclipse.cdt.managedbuilder.core.headlessbuild -data ${ENV_WORKSPACE_PATH} ${TMP_BUILD_OPTS} -${ENV_DIRECT_BUILD_ACTION:-cleanBuild} "${TMP_DB_CONF}"
}

exec_format()
{
  if [ ${ENV_CF_FOLDERS:+x} ]; then
    TMP_FMT_CODE_PATHS="$(find "$1" -type d -regex ".*/$(basename $1)/\(${ENV_CF_FOLDERS//,/\\|}\)" | paste -sd ' ')"
    echo
    echo "[=== Formatting ===]"
    echo
    echo "Formatting source code in ${ENV_CF_FOLDERS}"
    TMP_FMT_OPTS=""
    if [ "${ENV_DEBUG:+x}" ]; then
        TMP_FMT_OPTS="-verbose"
    fi
    echo
    /usr/bin/e2studio --launcher.suppressErrors -nosplash -application org.eclipse.cdt.core.CodeFormatter $TMP_FMT_OPTS $TMP_FMT_CODE_PATHS -data "${ENV_WORKSPACE_PATH}" 2>&1 | grep -E "^(Start|Format|Done).*" | grep -v -e '^$'
  fi
}

exec_archive()
{
  if [ ${ENV_ARCHIVE:+x} ]; then

    PRJ_PATH="$(find "${ENV_WORKSPACE_PATH}" -type d -regex '.*/workspace/[a-zA-Z0-9]+_[a-zA-Z0-9_]+' | head)"
    if [ ${PRJ_PATH:+x} ]; then
      TMP_ZIP_OPT=""
      if [ -z $ENV_DEBUG ]; then
        TMP_ZIP_OPT="-q"
      fi

      echo
      echo "[=== Archiving ===]"
      echo "Archiving current project to ${ENV_ARCHIVE}.zip"
      echo
      cd "${PRJ_PATH}"
      zip -r ${TMP_ZIP_OPT} "${ENV_ARCHIVE}" . -x ./Debug/\*
      cd ~-
    fi
  fi
}

while getopts "o:t:f:b:p:w:r:y:m:s:c:d:l:z:j:kiv?" argv
do
  case $argv in
    o)
      REPO_URL=$OPTARG
      ;;
    t)
      REPO_BRANCH=$OPTARG
      ;;
    f)
      case "$OPTARG" in
        g)
          FSP_INSTALL=1
          ENV_FSP_RELEASE="${!OPTIND}"; OPTIND=$(( $OPTIND + 1 ))
          ;;
      esac;;
    b)
      ENV_BUILD_PATH=$OPTARG
      ;;
    p)
      ENV_PROJECT=$OPTARG
      ;;
    w)
      ENV_WORKSPACE_PATH=$OPTARG
      ;;
    l)
      ENV_INTERMEDIATE_PATH=$OPTARG
      ENV_REPO_NAME=$(echo $ENV_INTERMEDIATE_PATH | cut -f 1 -d '/')
      ;;
    r)
      if [ "${OPTARG}" == "f" ]; then
        RESTART_CHOICE=3
      elif [ "${OPTARG}" == "u" ]; then
        RESTART_CHOICE=2
      elif [ "${OPTARG}" == "n" ]; then
        RESTART_CHOICE=1
      fi
      ;;
    y)
      case "$OPTARG" in
        p)
          ENV_YML_PROJECT="${!OPTIND}"; OPTIND=$(( $OPTIND + 1 ))
          ;;
        c)
          ENV_YML_COMPONENTS="${!OPTIND}"; OPTIND=$(( $OPTIND + 1 ))
          ;;
      esac;;
    m)
      ENV_YML_MCU=$OPTARG
      ;;
    s)
      ENV_YML_SENSORS=$OPTARG
      ;;
    c)
      ENV_YML_CONNECTIVITY=$OPTARG
      ;;
    d)
      ENV_DIRECT_BUILD_TARGET=$OPTARG
      ;;
    i)
      ENV_CLEAN_WORKSPACE=0
      ENV_DIRECT_BUILD_ACTION="build"
      ;;
    j)
      ENV_CF_FOLDERS=$OPTARG
      ;;
    z)
      ENV_ARCHIVE=$OPTARG
      ;;
    k)
      ENV_CLEAN_WORKSPACE=0
      ;;
    v)
      ENV_DEBUG=1
      ;;
    \?)
      usage
      show
      exit
      ;;
  esac
done

if [ ! ${ENV_BUILD_PATH:+x} ]; then
  if [ ! ${CI_PROJECT_DIR:+x} ]; then
    ENV_BUILD_PATH="$(pwd)"
  else
    ENV_BUILD_PATH="$(dirname "$CI_PROJECT_DIR")"
  fi
fi

if [ ! ${ENV_PROJECT:+x} ]; then
  if [ ! ${CI_PROJECT_DIR:+x} ]; then
    ENV_PROJECT="project"
  else
    ENV_PROJECT="$(basename "$CI_PROJECT_DIR")"
  fi
fi

if [ ! ${ENV_WORKSPACE_PATH:+x} ]; then
  ENV_WORKSPACE_PATH="$(pwd)"
fi

if [ ${ENV_REPO_NAME:+x} ]; then
  export CI_PROJECT_DIR="${ENV_BUILD_PATH}/${ENV_REPO_NAME}"
else
  export CI_PROJECT_DIR="${ENV_BUILD_PATH}/${ENV_PROJECT}"
fi
export SCRIPT_DIR="$(dirname "$(readlink -f $0)")/xml_testing"

if [ ${ENV_DEBUG:-0} -eq 1 ]; then
  export DEBUG=1
else
  unset DEBUG
fi

show

echo "[=== Verifying Local Repo ===]"
echo

# See if this script has already been run. If so, then give option to clean and go instead of starting over
if [ -d "${CI_PROJECT_DIR}" ] && [ -z "${RESTART_CHOICE}" ]; then
  echo "It looks like you've already run this script before. You can update your branch"
  echo "and go again or start over."
  echo "  1. Leave local repo as-is, delete e2studio workspace and choose new YML file"
  echo "  2. Clean local repo, delete e2studio workspace, pull latest changes,"
  echo "     and choose new YML file"
  echo "  3. Start over. Delete local repo and e2studio workspace."
  echo ""
  echo "Choose an option and hit enter."
  echo ""
  read -r RESTART_CHOICE

  if [ ${RESTART_CHOICE} -lt 1 ] || [ ${RESTART_CHOICE} -gt 3 ]; then
      echo "ERROR: Invalid option selected."
      read -r -p "Press enter to exit."
      exit 1
  fi
elif [ ! -d "${CI_PROJECT_DIR}" ] && [ -z "${RESTART_CHOICE}" ]; then
    RESTART_CHOICE=3
fi

# backup iar settings
if [ -f "${ENV_WORKSPACE_PATH}/.metadata/.plugins/org.eclipse.core.runtime/.settings/com.iar.ide.common.ui.prefs" ]; then
  mkdir -p "/e2_studio/prefs_backup" && \
  cp "${ENV_WORKSPACE_PATH}/.metadata/.plugins/org.eclipse.core.runtime/.settings/com.iar.ide.common.ui.prefs" "/e2_studio/prefs_backup/"
  echo "... backup IAR settings"
  echo
fi

if [ ${ENV_CLEAN_WORKSPACE:-1} -eq 1 ]; then
  echo "... clean up e2studio workspace"
  echo
  rm -rf "${ENV_WORKSPACE_PATH}"
fi
mkdir -p "${ENV_WORKSPACE_PATH}"

if [ "${RESTART_CHOICE}" == "3" ]; then

  rm -rf "${CI_PROJECT_DIR}"

  # See core.pager here: https://git-scm.com/book/en/v2/Customizing-Git-Git-Configuration
  git config --global core.pager 'more'

  mkdir -p "${CI_PROJECT_DIR}"

  cd "${ENV_BUILD_PATH}"

  if [ ! ${REPO_BRANCH:+x} ]; then
    echo "Choose which branch to clone. Start typing branch name to filter list."
    echo ""
    # Read branches without cloning. We do this so we can do a shallow clone of only the branch you want.
    REPO_BRANCH=$(git ls-remote --heads ${REPO_URL} | sed -e 's/.*\trefs\/heads\///' | fzf)

    echo "Chosen branch: ${REPO_BRANCH}"
  fi

  git clone --depth=1 --branch=${REPO_BRANCH} ${REPO_URL} ${ENV_REPO_NAME:-$ENV_PROJECT}

else

  # Repo is already local, need to update to latest commit on current branch
  if [ "${RESTART_CHOICE}" == "2" ]; then
    rm -rf e2_studio
    git clean -fd
    git reset HEAD --hard
    git pull
  fi
fi

if [ ! -d "${CI_PROJECT_DIR}" ]; then
  echo
  echo "... something wrong caused failure of retrieving/updating/maintaining the project content."
  echo
  exit 9
fi

# Restore IAR configuration for e2 studio
if [ -f "/e2_studio/prefs_backup/com.iar.ide.common.ui.prefs" ]; then
  mkdir -p "${ENV_WORKSPACE_PATH}/.metadata/.plugins/org.eclipse.core.runtime/.settings/" && \
    cp "/e2_studio/prefs_backup/com.iar.ide.common.ui.prefs" "${ENV_WORKSPACE_PATH}/.metadata/.plugins/org.eclipse.core.runtime/.settings/"
  echo "... restored IAR settings"
  echo
fi

if [ -z "$FSP_INSTALL" ]; then
  unset ENV_FSP_CLEAN_INSTALL
  unset ENV_FSP_SKIP_INTERNAL
else
  export ENV_FSP_CLEAN_INSTALL=1
  export ENV_FSP_SKIP_INTERNAL=1
  if [ ! ${ENV_FSP_RELEASE:+x} ] || [ "${ENV_FSP_RELEASE}" = "latest" ]; then
    export ENV_FSP_PACKS_URL="$(curl -s https://api.github.com/repos/renesas/fsp/releases/latest | grep "browser_download_url.*FSP_Packs_v.*zip" | cut -d : -f 2,3 | tr -d \")"
  else
    export ENV_FSP_PACKS_URL="https://github.com/renesas/fsp/releases/download/v${ENV_FSP_RELEASE}/FSP_Packs_v${ENV_FSP_RELEASE}.zip"
  fi

  echo "[=== Reinstalling FSP Packs ===]"
  echo
  echo "... from ${ENV_FSP_PACKS_URL}"
  echo
  CI=1 "${SCRIPT_DIR}/../install_packs.sh"
  echo
fi

PROJECT_CHOICE=1

if [ ${ENV_YML_PROJECT:+x} ] && [ -f "${ENV_YML_PROJECT}" ]; then
  PROJECT_CHOICE=2
elif [ ${ENV_YML_COMPONENTS:+x} ] || [ ${ENV_YML_MCU:+x} ] || [ ${ENV_YML_MCU:+x} ] || [ ${ENV_YML_MCU:+x} ]; then
  PROJECT_CHOICE=1
elif [ ${ENV_DIRECT_BUILD_TARGET:+x} ]; then
  PROJECT_CHOICE=3
else
  echo
  echo "Please specify how would you like to perform the XML testing."
  echo "  1. Create new project template by combining existing modules."
  echo "  2. Select one existing project template"
  echo "  3. Directly build from the clone project content"
  echo

  read -r PROJECT_CHOICE
fi

if [ ${PROJECT_CHOICE} -eq 3 ]; then

  exec_build

else
  echo
  echo "[=== XML Testing ===]"
  echo
  cd "${CI_PROJECT_DIR}"
  mkdir -p "e2_studio/input"

  if [ ${PROJECT_CHOICE} -eq 1 ]; then
    if [[ -z $(pip list | grep iterfzf) ]]; then
      pip install iterfzf
    fi
    if [[ -z $(pip list | grep ruamel) ]]; then
      pip install ruamel.yaml
    fi
    python3 "${SCRIPT_DIR}/brewery.py" -o e2_studio/input/default_projects.yml -p "${ENV_YML_COMPONENTS}" -m "${ENV_YML_MCU}" -s "${ENV_YML_SENSORS}" -c "${ENV_YML_CONNECTIVITY}"
  else
    rm -rf e2_studio/input/default_projects.yml
    yml_file=${ENV_YML_PROJECT:-$(find . -name "*.yml" -type f -path "*/.xml_tests/*" | fzf)}
    echo "Chosen YML file: ${yml_file}"
    # Check if this is a list of projects or templates to process
    if [ "$(yq e '. | has("templates")' ${yml_file})" == "true" ]; then
      python3 "${SCRIPT_DIR}/create_module_test_list.py" -i "${yml_file}" -o tmp_out.yml
      cat tmp_out.yml >> e2_studio/input/default_projects.yml
    else
      cat ${yml_file} >> e2_studio/input/default_projects.yml
    fi;
  fi

  # Create and build projects if projects are found to build. Results will be put in /e2_studio/output/ directory.
  if [ -f e2_studio/input/default_projects.yml ]; then
    echo "Creating and building the projects..."
    echo
    CI=0 "${SCRIPT_DIR}/create_and_build.sh"

    exec_format "$(find "${ENV_WORKSPACE_PATH}" -type d -regex '.*/workspace/[a-zA-Z0-9]+_[a-zA-Z0-9_]+' | head)"
  else
    echo "No e2 studio projects found to build"
  fi

fi

exec_archive

echo
echo "All QCS processes completed."
echo
