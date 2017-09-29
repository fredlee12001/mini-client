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
import time

SERVER = "prov-dev.kfn.arm.com"
SLEEP_SECONDS = 10

while True:
    print "Checking connectivity to server {} ...".format(SERVER)
    errorlevel = subprocess.call("ping -n 1 {} > nul".format(SERVER), shell=True)

    if errorlevel == 0:
        print "Server connectivity exists. Starting Unison."
        subprocess.call("python unison_sync.py")
        print "Unison exited."
    else:
        print "No ping to server {}. Sleeping for {} seconds.".format(SERVER, str(SLEEP_SECONDS))
        time.sleep(SLEEP_SECONDS)
