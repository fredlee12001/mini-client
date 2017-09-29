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

import subprocess
import yaml
import sys
import os

# Read global configuration file located in %APPDATA% directory (on Windows).
def readGlobalConfigFile():
    CONFIG_FILE_NAME = "unison_sync_conf.yaml"

    appDataDir = os.getenv("APPDATA")
    if appDataDir is None:
        sys.stderr.write("APPDATA environment variable isn't defined. Are you running in Windows?\n")
        sys.exit(1)

    globalConfigPath = appDataDir + "/" + CONFIG_FILE_NAME
    try:
        with open(globalConfigPath, "rt") as f:
            conf = yaml.load(f)
    except:
        sys.stderr.write("Error reading global configuration file!")
        sys.stderr.write("(searched for it in '" + globalConfigPath + "')!")
        sys.stderr.write("Did you forget to copy and edit the reference file from the script directory?")
        sys.exit(1)
    return conf

# Read "ignores" parameters file.
def readIgnoresListFile():
    with open("unison_sync_ignores.txt", "rt") as f:
        ignores = f.read().split("\n")
    return ignores

def main():
    conf = readGlobalConfigFile()
    ignoresList = readIgnoresListFile()
    ignoresParams = " ".join(ignoresList)

    command = \
        'unison-windows-bin/unison ' + conf["root1"] + ' ' + conf["root2"] + ' ' + \
        '-repeat watch ' + \
        '-batch ' + \
        '-times ' + \
        '-prefer newer ' + \
        '-sshcmd ' + conf["ssh"] + ' ' + \
        ignoresParams

    print "Running '" + command + "'."
    subprocess.call(command)


main()
