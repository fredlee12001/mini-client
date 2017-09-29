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

import os
import pickle
import re
import sys
import shutil
import subprocess
import tempfile
import argparse
import re
from subprocess import Popen, PIPE, STDOUT

assets_dict = dict()
cbor_dict = dict()


def save_to_file(path, filename, content, filetype=None):

    complete_name = os.path.join(path, filename)
    if filetype == "bin": #binary file output
        handler = open(complete_name + ".bin", 'wb')
    elif filetype == "text" : #text output
        handler = open(complete_name + ".hex", 'w')
    else:
        handler = open(complete_name, 'w')
    handler.write(content)
    handler.close()

def shell_process(cmd, out_dir = None, out_name = None, out=None,input=None):
    """ 
    Execute shell command and get its output and input as optional parameter
    @params:
        cmd   - Required  : command to execute
    Retruns stdout of the process
    """
    out_data= ""
    print cmd

    if (out and input):
        p = Popen(cmd, shell=True, stdout=subprocess.PIPE, stdin=subprocess.PIPE, stderr=subprocess.PIPE)
        p.stdin.write(input)
        out_data, error = p.communicate()
    elif out:
        p = Popen(cmd, shell=True, stdout=subprocess.PIPE,stderr=subprocess.PIPE)
        out_data, error = p.communicate()
    elif input:
        p = Popen(cmd, shell=True, stdin=subprocess.PIPE,stderr=subprocess.PIPE)
        p.stdin.write(input)
        error = p.communicate()
    else :
        p = Popen(cmd, shell=True, stderr=subprocess.PIPE)
        error = p.communicate()

    if p.returncode != 0:
        sys.stdout.write("\n The command {0} failed, please check the command, error is :\n {1}".format(cmd, error))
        exit(1)

    if (out):
        save_to_file(out_dir, out_name, out_data)
        assets_dict[out_name] = out_data

def parse_command_line():
    parser = argparse.ArgumentParser(description = 'Generate test data')
    parser.add_argument('-cf', '--c_file',help='path to c data file')
    parser.add_argument('-hf', '--h_file',help='path to h data file')
    parser.add_argument('-d','--data_dir',help = 'directory of data c and h files')
    parser.add_argument('-f','--files_dir',help = 'directory of out data files')
    parser.add_argument('-p','--prefix',help = 'prefix of data buffers')


    if (len(sys.argv)) != 6 :
        parser.print_help()
        exit (1) 

    return parser.parse_args()
def read_file(file_path, mode='r'):
    print "The path is:"+file_path
    with open(file_path, mode=mode) as file:
        data = file.read()
    return data

def write_data_to_file(file_path,data):
    open_mode = 'wb'
    with open(file_path, open_mode) as f:
        f.write(data)
    print('wrote data into file: ' + file_path)
