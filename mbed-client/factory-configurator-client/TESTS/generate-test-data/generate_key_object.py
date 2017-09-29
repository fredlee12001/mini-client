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
from cryptography.hazmat.primitives import serialization
from cryptography.hazmat.primitives.asymmetric import ec
from cryptography.hazmat.backends import openssl
from generate_test_utils import assets_dict
from generate_test_utils import write_data_to_file
from generate_crypto_utils import serialize_private_key
from generate_crypto_utils import serialize_public_key

class python_ecc_key_pair():
    def __init__(self,key_name,working_dir):
        self.key_name = key_name
        self.working_dir = working_dir
        self.ecc_curve_type = ec.SECP256R1
        self.private_format = serialization.PrivateFormat.PKCS8
        self.public_format = serialization.PublicFormat.SubjectPublicKeyInfo

    def set_ecc_curve_type(self, ecc_curve_type):
       self.ecc_curve_type = ecc_curve_type

    def set_private_format(self, private_format):
        self.private_format = private_format

    def generate_assets(self):
        private_key = ec.generate_private_key(self.ecc_curve_type, backend=openssl.backend)

        serialized_der_private_key = serialize_private_key(private_key,'DER',self.private_format)
        serialized_pem_private_key = serialize_private_key(private_key,'PEM',self.private_format)

        write_data_to_file(self.working_dir+"/private_"+self.key_name+'.der',serialized_der_private_key)
        write_data_to_file(self.working_dir+"/private_"+self.key_name+'.pem',serialized_pem_private_key)

        assets_dict['private_'+self.key_name+'.der'] = serialized_der_private_key
        assets_dict['private_'+self.key_name+'.pem'] = serialized_pem_private_key

        public_key = private_key.public_key()

        serialized_der_public_key = serialize_public_key(public_key,'DER',self.public_format)
        serialized_pem_public_key = serialize_public_key(public_key,'PEM',self.public_format)

        write_data_to_file(self.working_dir+"/public_"+self.key_name+'.der',serialized_der_public_key)
        write_data_to_file(self.working_dir+"/public_"+self.key_name+'.pem',serialized_pem_public_key)

        assets_dict['public_'+self.key_name+'.der'] = serialized_der_public_key
        assets_dict['public_'+self.key_name+'.pem'] = serialized_pem_public_key
