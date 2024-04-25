#!/bin/bash

# This script will export all projects under the default container workspace to a folder
# on artifactory. This zip can then be imported by setting the ENV_FSP_WORKSPACE_URL environment variable
# in your docker-compose.yml file.
# Default artifactory location:
# http://pbgitap01.rea.renesas.com:81/artifactory/webapp/#/artifacts/browse/tree/General/e2studio_projects

cd ${ENV_E2STUDIO_DEFAULT_WS}

# Generate unique name for this zip
TMP_NAME=$(date +"%Y-%m-%d_%H-%M-%S")_${HOSTNAME}.zip

zip -r ${TMP_NAME} . \
    --exclude ".metadata/*" \
    --exclude "*RemoteSystemsTempFiles/*" \
    --exclude "*/ra/*" \
    --exclude "*.o" \
    --exclude "*.d" \
    --exclude "*.mk" \
    --exclude "*.elf" \
    --exclude "*.srec" \
    -i "*.pincfg" \
    -i "*/configuration.xml" \
    -i "*/.cproject" \
    -i "*/.project" \
    -i "*/.settings/*" \
    -i "*.launch" \
    -i "*/script/*" \
    -i "*/src/*"

# Upload to artifactory so this can be shared
curl -T ${TMP_NAME} ${ENV_ARTIFACT_BASE_PATH}/${ENV_ARTIFACT_EXPORT_REPO}/${TMP_NAME} > tmp.json

echo ""
echo ">>>>>> SUCCESS <<<<<<"
echo "Workspace exported! Use the following share link:"
echo $(jq ".uri" tmp.json | sed "s/\"//g")

read -p "Press enter to close this window."
