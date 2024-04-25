#!/bin/bash

if [ ! ${REPO_URL:+x} ]; then
  REPO_URL="http://pbgitap01.rea.renesas.com/peaks/peaks.git"
fi

# Default is to start clean
RESTART_CHOICE=3

# See if this script has already been run. If so, then give option to clean and go instead of starting over
if [ -d /builds/peaks/peaks ]; then
  echo "It looks like you've already run this script before. You can update your branch"
  echo "and go again or start over."
  echo "  1. Leave everything as-is and choose new YML file"
  echo "  2. Clean local repo, delete e2studio workspace, pull latest changes,"
  echo "     and choose new YML file"
  echo "  3. Start over. Delete local repo and e2studio workspace."
  echo ""
  echo "Choose an option and hit enter."
  echo ""
  read RESTART_CHOICE

  if [ ${RESTART_CHOICE} -lt 1 ] || [ ${RESTART_CHOICE} -gt 3 ]; then
      echo "ERROR:Invalid option selected."
      read -p "Press enter to exit."
      exit 1
  fi
fi

# Save off IAR configuration for e2 studio
cp /e2_studio/workspace/.metadata/.plugins/org.eclipse.core.runtime/.settings/com.iar.ide.common.ui.prefs /tmp

if [ ${RESTART_CHOICE} -eq 3 ]; then

  echo "This script is intended to help you quickly perform XML testing for a module."
  echo "This script will:"
  echo "  1. Clone down the peaks repo"
  echo "  2. Ask you which branch to checkout"
  echo "  3. Scan for XML testing YML files and ask which one you wish to test"
  echo "  4. Perform the XML testing."
  echo ""
  read -p "Please close e2 studio before continuing. Press enter to continue."
  echo ""

  rm -rf /e2_studio/workspace
  mkdir -p /e2_studio/workspace
  rm -rf /builds/peaks

  # See core.pager here: https://git-scm.com/book/en/v2/Customizing-Git-Git-Configuration
  git config --global core.pager 'more'

  mkdir -p /builds/peaks

  cd /builds/peaks

  echo "Choose which branch to clone. Start typing branch name to filter list."
  echo ""
  # Read branches without cloning. We do this so we can do a shallow clone of only the branch you want.
  git_branch=$(git ls-remote --heads ${REPO_URL} | sed -e 's/.*\trefs\/heads\///' | fzf)

  echo "Chosen branch: ${git_branch}"

  git clone --depth=1 --branch=${git_branch} ${REPO_URL}

  cd peaks

else

  cd /builds/peaks/peaks

  # Repo is already local, need to update to latest commit on current branch
  if [ ${RESTART_CHOICE} -eq 2 ]; then
    rm -rf e2_studio
    git clean -fd
    git reset HEAD --hard
    git pull
    rm -rf /e2_studio/workspace
    mkdir -p /e2_studio/workspace
  fi
fi

# Restore IAR configuration for e2 studio
mkdir -p /e2_studio/workspace/.metadata/.plugins/org.eclipse.core.runtime/.settings/
cp /tmp/com.iar.ide.common.ui.prefs /e2_studio/workspace/.metadata/.plugins/org.eclipse.core.runtime/.settings/

echo "Choose a YML file to test."
echo ""

yml_file=$(find -name "*.yml" -type f -path "*/.xml_tests/*" | fzf)

echo "Chosen YML file: ${yml_file}"

mkdir -p e2_studio/input

# Check if this is a list of projects or templates to process
if [ "$(yq e '. | has("templates")' ${yml_file})" == "true" ]; then
  python3 scripts/xml_testing/create_module_test_list.py -i ${yml_file} -o tmp_out.yml
  cat tmp_out.yml >> e2_studio/input/default_projects.yml
else
  cat ${yml_file} >> e2_studio/input/default_projects.yml
fi;

# Create and build projects if projects are found to build. Results will be put in /e2_studio/output/ directory.
if [ -f e2_studio/input/default_projects.yml ]; then
  echo "Creating projects and building"
  ./scripts/xml_testing/create_and_build.sh
else
  echo "No e2 studio projects found to build"
fi

echo ""
read -p "Done. Press enter to close this terminal."
