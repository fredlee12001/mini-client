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
from cryptography import x509
from cryptography.hazmat.primitives import hashes
from cryptography.hazmat.backends import openssl
from cryptography.x509.oid import NameOID
from generate_test_utils import assets_dict
from generate_test_utils import write_data_to_file
from generate_crypto_utils import load_private_key

class csr():
    def __init__(self,csr_name,key_file_name,working_dir, country_name,state,locality_name,org_name,org_unit_name,common_name,email_addr):
        self.country_name = country_name
        self.state = state
        self.locality_name = locality_name
        self.org_name = org_name
        self.org_unit_name = org_unit_name
        self.common_name = common_name
        self.email_addr = email_addr
        self.csr_name = csr_name
        self.working_dir = working_dir
        self.private_key = load_private_key(self.working_dir+'/'+key_file_name)


    def generate_assets(self):
        csr_builder =  x509.CertificateSigningRequestBuilder().subject_name(
            x509.Name([x509.NameAttribute(NameOID.COUNTRY_NAME, self.country_name.decode('utf-8')),
            x509.NameAttribute(NameOID.STATE_OR_PROVINCE_NAME, self.state.decode('utf-8')),
            x509.NameAttribute(NameOID.LOCALITY_NAME,  self.locality_name.decode('utf-8')),
            x509.NameAttribute(NameOID.ORGANIZATION_NAME,  self.org_name.decode('utf-8')),
            x509.NameAttribute(NameOID.ORGANIZATIONAL_UNIT_NAME,  self.org_unit_name.decode('utf-8')),
            x509.NameAttribute(NameOID.COMMON_NAME,  self.common_name.decode('utf-8')),
            x509.NameAttribute(NameOID.EMAIL_ADDRESS ,  self.email_addr.decode('utf-8')),]))
        csr = csr_builder.sign(self.private_key, hashes.SHA256(), openssl.backend)

        serialized_csr_pem = csr.public_bytes(serialization.Encoding.PEM)
        serialized_csr_der = csr.public_bytes(serialization.Encoding.DER)

        write_data_to_file(self.working_dir+"/"+self.csr_name+'.pem',serialized_csr_pem)
        write_data_to_file(self.working_dir+"/"+self.csr_name+'.der',serialized_csr_der)

        assets_dict[self.csr_name+'.der'] = serialized_csr_der
        assets_dict[self.csr_name+'.pem'] = serialized_csr_pem
