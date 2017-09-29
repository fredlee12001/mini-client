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
# A script to update the license header of SPV source files
# This script is using replace_license_header.py and provides the specific
# subdirectories to handle (to avoid directories with external sources).

cd `dirname $0`
MY_DIR=`pwd`
FCC_TOP=$MY_DIR/../..

FCC_LICENSE_FILE=$MY_DIR/copyright_prop.txt

# Directories to include for replace_license_header.py - relative to FCC_TOP
FCC_DIRS2INCLUDE=\
"crypto-service "\
"factory-configurator-client "\
"fcc-bundle-handler "\
"fcc-output-info-handler "\
"ftcd-comm-base "\
"ftcd-comm-serial "\
"devenv "\
"ftcd-comm-socket "\
"key-config-manager "\
"logger "\
"mbed-trace-helper "\
"storage "\
"TESTS "\
"utils "

# Directories to exclude from within FCC_DIRS2INCLUDE
FCC_DIRS2EXCLUDE=

FCC_FILES2EXCLUDE=

echo "Applying license update..."
cd $FCC_TOP
python $MY_DIR/replace_license_header.py -c $FCC_LICENSE_FILE -d $FCC_DIRS2INCLUDE
if [[ $? -ne 0 ]]; then echo Error on license update.; exit 1; fi

echo "Reverting license at subdirs: $FCC_DIRS2EXCLUDE"
cd $FCC_TOP
for d in $FCC_DIRS2EXCLUDE; do
	git checkout -- $d
done
for f in $FCC_FILES2EXCLUDE; do
	git checkout -- $f
done

echo
echo "Done."
exit 0

