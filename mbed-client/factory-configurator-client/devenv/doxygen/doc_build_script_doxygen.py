#!/usr/bin/env python
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

from doc_builder.backends.doxygen import DoxygenBuilder
from doc_builder.backends.markdown import MarkdownBuilder
from doc_builder.exceptions import BuildError

# This is the configuration for the factory configurator client documentation..
# It is a Python dict with meta info about the project, along with a list of the source files to create the documentation from.
#"slug", "source", "INPUT" - defiened in doxygen.sh and shoud be updated from doxygen.sh in rum time.
CONFIG = {
        "title": "factory configurator client API references",
        "description": "The API references for the factory configurator client.",
        "slug": "",
        "type": "doxygen",
        "source": "",
        "options": {
		"INPUT": ""
		}
}
 
# The builder needs to be passed a version (type string), e.g. "1.1". defiened in doxygen.sh and shoud be updated from doxygen.sh in rum time.
VERSION = ""

# The path of the directory on your computer where you want the docs to be generated (type string).
# Currently set to build docs in same location as this script.
BUILD_DIR = "."

# If you have private GitHub files, you'll need a personal access token which can access those files.
# Change this value to your access token (type string), or leave it as None.
GITHUB_TOKEN = None #"20a1018a78127835df72978747782d8bf6329654"

# If you have files on AWS, you'll need an access key and secret access key.
# Change these values to your keys (type string), or leave them as None.
AWS_ACCESS_KEY = None
AWS_SECRET_ACCESS_KEY = None


if __name__ == "__main__":
    # Initialize the builder
    if CONFIG["type"] == "markdown":
        builder = MarkdownBuilder(
            VERSION, CONFIG,
            BUILD_DIR,
            github_token=GITHUB_TOKEN,
            aws_access_key=AWS_ACCESS_KEY,
            aws_secret_access_key=AWS_SECRET_ACCESS_KEY
        )
    elif CONFIG["type"] == "doxygen":
        builder = DoxygenBuilder(
            VERSION, CONFIG,
            BUILD_DIR,
            github_token=GITHUB_TOKEN,
            aws_access_key=AWS_ACCESS_KEY,
            aws_secret_access_key=AWS_SECRET_ACCESS_KEY
        )

    # Run the build command
    print("Building docs...")
    try: 
        result=builder.build()
        if result == None :
            print ("Docs built! Exit with status SUCCESS ")
        else :
	        print ("Docs built FAILED, status = %s" %result)
	        exit(1);
    except:
            exit(1)