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
from generate_test_utils import assets_dict
from generate_test_utils import cbor_dict

def create_file_headers(h_file, c_file):

#write to h file
    h_file.write("// DO NOT EDIT! This file is automatically generated!\n")
    h_file.write("\n")

#write to c file
    c_file.write("// DO NOT EDIT! This file is automatically generated!\n")
    c_file.write("// If, against common sense, this file is edited manually - be warned that\n")
    c_file.write("// the accompanying header file contains the buffer sizes - so must be edited\n")
    c_file.write("// as well!\n")
    c_file.write("\n")
    c_file.write("#include <stdint.h>\n")
    c_file.write("\n")

def bytes_length(data):
    return len(data)

def integer_length(data):
    return len(str(data))
	
def str_length(data):
    return len(data)

data_type_options = {
    int : integer_length,
	long : integer_length,
	bytes : bytes_length,
	str: str_length,
}

# assign int, long to a specific name
def data_to_definition(name, data):
    data_length = data_type_options[type(data)](data)

    h_def  = ""
    h_def += "extern uint64_t " + name + ";\n"

    c_def = "uint64_t " + name + " = " + str(data) + ";\n"
    return h_def, c_def

def name_and_data_to_definition(name, data):
    data_length = data_type_options[type(data)](data)

    h_def  = ""
    h_def += "extern const char " +  name + "_name" +"[" + str(len(name) + 1) + "];\n"
    h_def += "extern const uint8_t " + name + "[" + str(data_length) + "];\n"

    c_def  = "const char " + name + "_name" + "[] = \"" + name + "\";\n"
    c_def += "const uint8_t " + name + "[" + str(data_length) + "] = "
    c_def += binary_to_c_byte_array(data)
    return h_def, c_def


def binary_to_c_byte_array(data):
    result = ""
    result += "{ "
    for byte in data:
        # If each byte is a character, convert it to its decimal value.
        if type(byte) is str:
            byte = ord(byte)
        result += "0x" + ("%02x" % byte) + ", "
    result += " };\n"

    return result


def write_assets_to_c_files(h_file, c_file, prefix):
    message_constants_comment = "//assets auto generated from open ssl library -\n\n"
    h_file.write(message_constants_comment)
    c_file.write(message_constants_comment)

    #write assets
    for asset_name, asset_binary_data in assets_dict.iteritems():
            c_file.write("// Generated from " + asset_name + "\n")

            if (type(asset_binary_data) is bytes):
                asset_name = asset_name[:-4] + asset_name[-3:] #remove "." for names in C files
                (h_def, c_def) = name_and_data_to_definition(prefix + asset_name, asset_binary_data)
            elif (type(asset_binary_data) is int) or (type(asset_binary_data) is long):
			    (h_def, c_def) = data_to_definition(prefix + asset_name, asset_binary_data)
            else:
			    raise "Unsupported data type, exiting..."

            h_file.write(h_def)
            c_file.write(c_def)

    h_file.write("\n\n")
    c_file.write("\n\n")

    # write cbors
    for cbor_name, cbor_binary_data in cbor_dict.iteritems():
            c_file.write("// Generated from " + cbor_name +".bin" + "\n")
            (h_def, c_def) = name_and_data_to_definition(prefix + cbor_name, cbor_binary_data)
            h_file.write(h_def)
            c_file.write(c_def)
