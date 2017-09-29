#!/usr/local/bin/python3
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
import sys
import re
import getopt
import shutil

files_ext_list = [ 'c' , 'cpp', 'h' , 'py' , 'java' , 'mk', 'lua', 'sh', 'groovy', 'json' ]
copyright_max_size=40
ext_copyright = list()
debug_flag=0

def remove_bad_ascii(file_path):     
    f=open(file_path,'rb')     
    data=f.read()     
    f.close()     
    filtered_data = ""
    changed = 0

    for num in data:
        if num<127:
            filtered_data += chr(num)
        else:
            changed = 1
    if changed:
        print("Changing ",file_path) 

    f=open(file_path,'w')     
    f.write(filtered_data)     
    f.close()     
    return changed


def get_copyright_offset(file_data):
    if len(file_data) > 0:
        if file_data[0][0:2] == "#!": # File starts with the shebang line 
            return 1 # Start copyright comment on second line
    return 0

def prepare_copyright(copyright_file, file_type):
    result = ""
    first_line_remove = 0
    i=0
    # First line handling -
    if file_type == "xml":
        result += ("<!--")
        first_line_remove = 4
    try:                     
        file = open(copyright_file, 'r')
    except IOError as e:
        print("I/O error opening %s: %s (%d)" % (copyright_file, e.strerror, e.errno))
        sys.exit(1)

    # Middle lines handling -
    for line in file:
        i = i + 1
        if i == 1:
            line = line[first_line_remove:]

        if file_type == "xml":
            result += line
        elif file_type == "c" or file_type == "cpp" or file_type == "h" or file_type == 'java' or file_type == 'groovy' or file_type == 'json': # Multi-line comment
            result += ("// " + line)
        elif file_type == "py" or file_type == "mk" or file_type == "sh": # Single line comment
            result += ("# " + line)
        elif file_type == "lua": # Single line comment
            result += ("-- " + line)
        else:
            print("Unknown file type: %s" % file_type)
            sys.exit(1)

    # Last line handling -
    if file_type == "xml":
        result = result[:-4] + "-->"

    # Before adding a new-line at the end of the copyright message, make sure there isn't one there already.
    result = result.rstrip("\n")
    result+="\n"

    file.close()

    return result

def get_file_ext(file_name):
    return file_name.split(".")[-1]

def get_file_ext_index(file_ext):
    return files_ext_list.index(file_ext)

def file_valid(file_name):
    file_ext = get_file_ext(file_name)
    if file_ext in files_ext_list:
        return 1
    return 0

def file_hidden(file_name):
    if file_name.startswith('.'):
        return 1
    return 0


def find_copyright_header(file_data):

    # Search for new ARM copyright message.

    i=0
    start_line = 0
    for line in file_data:
        char_num = line.find("----------------------------------------------------------------------------")
        if not char_num == -1:
            start_line = i
            break
        i=i+1

    i=0
    for line in file_data:
        i=i+1
        char_num = line.find("Copyright 2016-2017 ARM Ltd")
        if not char_num == -1:
            for line in file_data[i:]:
                i=i+1
                char_num = line.find("----------------------------------------------------------------------------")
                if not char_num == -1:
                    return start_line, i

    # Search for old ARM copyright message.
    #First old  copyright message               
    i=0
    start_line = 0
    for line in file_data:
        char_num = line.find("Copyright 2017 ARM Ltd.")
        if not char_num == -1:
            start_line = i
            break
        i=i+1

    i=0
    for line in file_data:
        i=i+1
        char_num = line.find("limitations under the License.")
        if not char_num == -1:
           return start_line, i
    
	#Second old  copyright message   
    i=0
    start_line = 0
    for line in file_data:
        char_num = line.find("* Copyright 2014 (c) Discretix Technologies Ltd.")
        if not char_num == -1:
            start_line = i-1 #in this case Copyright is the second line
            break
        i=i+1

    i=0
    for line in file_data:
        i=i+1
        char_num = line.find("***************************************************************/")
        if not char_num == -1:
           return start_line, i

    return -1,-1

def replace_copyright_header(curr_path, file_name):

    # For debugging purposes, this can be uncommented -
    print "The path is " + curr_path


    full_file_name = curr_path + "/" + file_name
    tmp_file_name = "tmp.txt"

    try:
        file = open(full_file_name, 'r')
    except IOError:
        print("Error: file not found ",full_file_name)
        exit(1)

    try:
        file_data = file.readlines()
    except UnicodeDecodeError:
        print("Fixing bad unicode in ",full_file_name)
        file.close()
        remove_bad_ascii(full_file_name)
        file = open(full_file_name, 'r')
        file_data = file.readlines()
    file.close()

    header_first_line, header_last_line = find_copyright_header(file_data[:copyright_max_size])
    if header_last_line == -1:
        print("Header not found: ",full_file_name)
        header_last_line = 0
        header_first_line = 0

    file_ext_index = get_file_ext_index(get_file_ext(file_name))

    out_file = open(tmp_file_name, 'w')
    file_offset = get_copyright_offset(file_data)

    # If the file starts after a shebang line, avoid duplicating it.    
    if file_offset > header_last_line:
        header_last_line = file_offset

    # Write the existing shebang line, if exists.
    out_file.writelines(file_data[0:file_offset])
    # Write the new copyright message.
    out_file.writelines(ext_copyright[file_ext_index])
    # Write whatever lines came before the original copyright message.
    out_file.writelines(file_data[file_offset:header_first_line])
    # Write everything that came after the original copyright message.
    out_file.writelines(file_data[header_last_line:])
    out_file.close()

    try:
        shutil.copyfile(tmp_file_name, full_file_name)
    except IOError:
        print("Error: unable to copy file (file might be non-writeable...) ",full_file_name)
        print("Trying again, setting it to world writable first.")
        try:
            stat_obj = os.stat(full_file_name)
            os.chmod(full_file_name, 0777)
            shutil.copyfile(tmp_file_name, full_file_name)
            os.chmod(full_file_name, stat_obj.st_mode)
        except:
            print("Error: still can't copy file ",full_file_name)
            exit(1)

    return 0

def verify_copyright_header(curr_path, file_name):
    full_file_name = curr_path + "/" + file_name

    try:
        file = open(full_file_name, 'r')
    except IOError:
        print("Error: file not found ",full_file_name)
        exit(1)

    try:
        file_data = file.readlines()
    except UnicodeDecodeError:
        print("Error: reading from ",full_file_name)
        file.close()
        return 1
    file.close()

    header_first_line, header_last_line = find_copyright_header(file_data[:copyright_max_size])
    if header_last_line == -1:
        print("Header not found: ",full_file_name)
        header_last_line = 0
        header_first_line = 0

    file_ext_index = get_file_ext_index(get_file_ext(file_name))
    print(file_ext_index)
 
    lines = "".join(file_data[header_first_line:header_last_line])

    if lines[:-1] != ext_copyright[file_ext_index][:-2]:
        print("Error in file ", full_file_name)
        print(lines[:-1])
        print("-----")
        print(ext_copyright[file_ext_index][:-2])
        print("-----")

    return 0


def run_verify(curr_path):

    for file_name in os.listdir(curr_path):
        full_file_name = curr_path + "/" + file_name

        if os.path.isdir(full_file_name) == True and not file_name.startswith('.'):
            dir_iter(full_file_name)
        else:
            if file_valid(file_name) and not file_hidden(file_name):
                verify_copyright_header(curr_path, file_name)


def dir_iter(curr_path):
    print("Replacing source license at directory  ", curr_path)
    for file_name in os.listdir(curr_path):
        full_file_name = curr_path + "/" + file_name
        if os.path.isdir(full_file_name) == True and not file_name.startswith('.'):
            dir_iter(full_file_name)
        else:
            if file_valid(file_name) and not file_name.startswith('.') and not file_hidden(file_name):
                if debug_flag == 1:
                    print("Replacing header for file ", full_file_name)
                replace_copyright_header(curr_path, file_name)

def dir_list_iter(dir_list):
    #print("dir_list: ", dir_list)
    for curr_path in dir_list:
        dir_iter(curr_path)

def files_iter(files_list):
    print(files_list)
    for file_name in files_list:
        if file_valid(file_name):
            if debug_flag == 1:
                print("Replacing ", file_name)
            replace_copyright_header(".", file_name)
        else:
            print("Error: unsupported file type ",file_name)

def usage():
    print("replace_license_header.py -c <copyright text file> [-v (verify only!)] -d <root directory>|-f <file 1> <file 2>...")

def parse_args():
    copyright_file = ""
    dir_list = []
    files_list = []
    verify = 0
    debug_flag = 0

    for arg in sys.argv[1:]:
        idx = sys.argv.index(arg)
        if arg == "-c":
            copyright_file = sys.argv[idx+1]
        if arg == "-d":
            # empty dir - do nothing
            if idx >= len(sys.argv) - 1:
                    exit(0)
            for dirname in sys.argv[idx+1:]:
                dir_list.append(dirname)
        if arg == "-D":
            print("Debugging..........")
            debug_flag = 1
        if arg == "-f":
            # empty file list - do nothing
            if idx >= len(sys.argv) - 1:
                    exit(0)
            for file in sys.argv[idx+1:]:
                files_list.append(file)
            break
        if arg == "-v":
            verify = 1

    if (copyright_file == ""):
        print("Error: select copyright header file")
        usage()
        exit(1)
    if (len(dir_list) == 0 and len(files_list) == 0):
        print("Error: select files list or directory")
        usage()
        exit(1)
    if (len(dir_list) > 0 and len(files_list) > 0):
        print("Error: select files list OR directory. Not both!")
        usage()
        exit(1)

    if debug_flag == 1:
        print("Arguments:")
        print("dir list: ", dir_list)
        print("files_list: ", files_list)
        print("copyright_file: ", copyright_file)

    return copyright_file, dir_list, files_list, verify

def main ():
    copyright_file, dir_list, files_list, verify = parse_args()

    for ext in files_ext_list:
        ext_copyright.append(prepare_copyright(copyright_file, ext))

    if verify == 1:
        if len(dir_list) == 0:
            print("Error: verify must run on directory")
            exit(1)
        print("Error: verify is not implemented!")
        #run_verify(dir_list)

    if len(dir_list) == 0:
        files_iter(files_list)
    else:
        dir_list_iter(dir_list)

#############################
if __name__ == "__main__":
    main()
