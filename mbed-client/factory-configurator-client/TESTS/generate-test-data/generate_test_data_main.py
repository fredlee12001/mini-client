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
import argparse
from generate_test_utils import parse_command_line
from generate_test_data import generate_asym_test_keys
from generate_test_data import generate_sym_test_keys
from generate_test_data import generate_test_csrs
from generate_test_data import generate_test_certificates
from generate_test_data import generate_test_config_params
from generate_test_data import generate_test_cbors
from generate_test_data import generate_test_comm_packets
from generate_test_data import generate_python_x509_certificates
from generate_test_data import generate_test_x509
from generate_test_data import generate_python_ecc_keys
from generate_test_data import generate_ptyhon_csr
from convert_test_data_to_c import create_file_headers
from convert_test_data_to_c import write_assets_to_c_files



def main():

    print("### Starting - generate_test_data.py\n")
    #*********************************
    #Parse command arguments
    #*********************************
    args = parse_command_line()

    #generate assets
    generate_asym_test_keys(args.files_dir)
    generate_sym_test_keys(args.files_dir)
    generate_test_csrs(args.files_dir)
    generate_test_certificates(args.files_dir)
    generate_test_x509(args.files_dir)
    generate_test_config_params(args.files_dir)
    generate_python_ecc_keys(args.files_dir)
    generate_ptyhon_csr(args.files_dir)
    generate_python_x509_certificates(args.files_dir)


    #create CBOR test files
    generate_test_cbors(args.files_dir)

    # Convert generated files to c and h files
    with open(args.h_file, "wt") as h_file:
        with open(args.c_file, "wt") as c_file:
            create_file_headers(h_file, c_file)
            write_assets_to_c_files(h_file, c_file, args.prefix)
            h_file.close()
            c_file.close()

    #generate comm packet
    generate_test_comm_packets(args.files_dir)



if __name__ == "__main__":
    main()

