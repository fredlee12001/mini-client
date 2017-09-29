#!/bin/bash
# ----------------------------------------------------------------------------
# Copyright 2016-2017 ARM Ltd.
#  
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#  
#     http://www.apache.org/licenses/LICENSE-2.0
#  
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# ----------------------------------------------------------------------------
       
if [[ "$1" == "--help" ]] ; then
cat <<END
Usage: /bin/bash $0 [rebuild]
* [rebuild] is an optianl parameter to rebuild all docker images that used for document generation.
END
  exit
fi

set -e
set -x #debug option

export PROJECT_TOP=`pwd`

#Source list for doxygen builder.All new sources should be added here
SOURCE_LIST="$PROJECT_TOP/factory-configurator-client/factory-configurator-client/*.h $PROJECT_TOP/key-config-manager/key-config-manager/*.h $PROJECT_TOP/fcc-bundle-handler/fcc-bundle-handler/*.h $PROJECT_TOP/DOXYGEN_FRONTPAGE.md"


DOC_OUT_DIR="../docs" #should be one lever above run directory
BUILD_DOCS_SCRIPT_VERSION="FCC1.2" #documentation version
SLUG_NAME="factory-configurator-client" #project name
SOURCE_DIR_NAME="factory_configurator_client" #source dir name
DOC_BUILDER_REV=f4de61230132bcd2e7df10797fab7488ca629711 #doc-builder repository revision 
DOC_BUILDER_DOCKER_REV=c3b78454310c8ba5570f26a081d3174ca7211d9e #doc-builder-docker repository revision
DOCKER_TAG_NAME=factory-client #our docker tag name. 
DOCKER_DEV_IMAGE_NAME="" # used only for rebuild option. Will contain local image name for development perpose 

REBUILD="" #rebuild all dockers images

#Clean doxygen directory  
rm -rf out/doxygen

#Clone repository for doxygen image instalation
git clone -c core.filemode=false --no-checkout -q  git@github.com:ARMmbed/doc-builder-docker.git out/doxygen/docker_builder
cd out/doxygen/docker_builder && git checkout -q "$DOC_BUILDER_DOCKER_REV"

#Create directory with all needed for doxygen builder files
DOXYGEN_SRC_DIR="$DOC_OUT_DIR"/"$BUILD_DOCS_SCRIPT_VERSION"/"$SLUG_NAME"/build_artifacts/"$SOURCE_DIR_NAME"
mkdir -p "$DOXYGEN_SRC_DIR"
cp $SOURCE_LIST "$DOXYGEN_SRC_DIR"

#Set our revision of docker builder
sed -i -e "s/git@github.com\/ARMmbed\/doc-builder.git@v0.1.0#egg=doc-builder/git@github.com\/ARMmbed\/doc-builder.git#$DOC_BUILDER_REV#egg=doc-builder/g" doc-builder-image/install.sh

#Change all "*.sh" files to be executable 
find . -name '*.sh' -type f | xargs chmod +x

#Get rebuild mode parameter
if [ -n "$1" ] ; then
   REBUILD=$1

fi
#If rebuild mode, delete previuse local docker images if exists and create new docker image for local development.
#To push the local image to regestry after the developing finished use mannualy command /shared/scripts/push_pull_image/push_pull_image.sh push mbed-doc-builder:factory-client
if [[ $REBUILD == "rebuild" ]]; then
    if [[ "$(docker images -q mbed-doc-builder:$DOCKER_TAG_NAME 2> /dev/null)" != "" ]]; then
        docker rmi -f mbed-doc-builder:"$DOCKER_TAG_NAME"
    fi
    
	if [[ "$(docker images -q mbed-doc-builder:latest 2> /dev/null)" != "" ]]; then
        docker rmi -f mbed-doc-builder:latest
    fi
    
	if [[ "$(docker images -q mbed-doc-builder-base:latest 2> /dev/null)" != "" ]]; then
        docker rmi -f mbed-doc-builder-base:latest
    fi

	#create new images
    ./create_images.sh

    #Tag it with our tag-name
    docker tag mbed-doc-builder:latest  mbed-doc-builder:"$DOCKER_TAG_NAME"

	#use local development image
	sed -i -e  "s/IMAGE_NAME=mbed-doc-builder/IMAGE_NAME=mbed-doc-builder:$DOCKER_TAG_NAME/g" build_docs.sh

	#after development was finished and before you push the changes to Git repository push your new script to regestry using mannualy command
	#/shared/scripts/push_pull_image/push_pull_image.sh push mbed-doc-builder:factory-client

else #We are not in developer mode, use our image in registry
     sed -i -e  "s/IMAGE_NAME=mbed-doc-builder/IMAGE_NAME=prov-docker\.kfn\.arm\.com:5000\/mbed-doc-builder:$DOCKER_TAG_NAME/g" build_docs.sh 
fi

#copy our doxygen scripts to run directory
cp $PROJECT_TOP/devenv/doxygen/doc_build_script_doxygen.py ./

INPUT_PARAMS="$SOURCE_DIR_NAME/DOXYGEN_FRONTPAGE.md $SOURCE_DIR_NAME"

#update common variables in our doxygen script
sed -i -e  "s/VERSION = \"\"/VERSION = \"$BUILD_DOCS_SCRIPT_VERSION\"/g" doc_build_script_doxygen.py #set VERSION
sed -i -e  "s/\"slug\": \"\"/\"slug\": \"$SLUG_NAME\"/g" doc_build_script_doxygen.py #set slag value 
sed -i -e  "s/\"source\": \"\"/\"source\": \"$SOURCE_DIR_NAME\"/g" doc_build_script_doxygen.py #set source path value
sed -i -e  "s/\"INPUT\": \"\"/\"INPUT\": \"$SOURCE_DIR_NAME\"/g" doc_build_script_doxygen.py #set source directory

#Run created image with input script for doxygen files
./build_docs.sh doc_build_script_doxygen.py "$DOC_OUT_DIR"

 