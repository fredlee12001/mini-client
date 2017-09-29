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
from cryptography import x509
from cryptography.x509 import load_pem_x509_certificate
from cryptography.x509 import  load_pem_x509_csr
from generate_test_utils import read_file
from cryptography.hazmat.primitives import serialization
from cryptography.hazmat.backends import openssl

def load_pem_private_key_from_file(file_name, password=None):
    file_data = read_file(file_name, 'rb')
    private_key = serialization.load_pem_private_key(
        file_data,
        password=password,
        backend=openssl.backend
    )
    return private_key

def load_pem_certificate(file_name):
    file_data = read_file(file_name, 'rb')
    x509_cert = load_pem_x509_certificate(
        file_data,
        backend=openssl.backend
    )
    return x509_cert

def load_ca_cert(ca_cert_file_name):
    try:
        ca_cert = load_pem_certificate(ca_cert_file_name)
    except IOError:
        print ('Certifiate not found')
        raise
    except Exception:  # Multiple exceptions may be raised from crypto lib
        print ('Certifiate not valid')
        raise
    return ca_cert

def load_pem_csr(file_name):
    file_data = read_file(file_name, 'rb')
    x509_csr = load_pem_x509_csr(
        file_data,
        backend=openssl.backend
    )
    return x509_csr

def load_csr(csr_file_name):
    try:
        csr = load_pem_csr(csr_file_name)
    except IOError:
        print ('csr not found')
        raise
    except Exception:  # Multiple exceptions may be raised from crypto lib
        print ('csr not valid')
        raise
    return csr

def load_private_key(file_name):
    try:
        private_key = load_pem_private_key_from_file(file_name)
    except IOError:
        print ('Key not found')
        raise
    except Exception:  # Multiple exceptions may be raised from crypto lib
        print ('Key not valid')
        raise
    return private_key

def check_certificate_match_pkey(cert, private_key):
    cert_public_key = cert.public_key().public_bytes(
        serialization.Encoding.DER, serialization.PublicFormat.SubjectPublicKeyInfo)
    priv_key_public_key = private_key.public_key().public_bytes(
        serialization.Encoding.DER, serialization.PublicFormat.SubjectPublicKeyInfo)
    return cert_public_key == priv_key_public_key



def serialize_private_key(private_key,keys_encoding,key_format):
    encoding = serialization.Encoding[keys_encoding]
    serialized_private = \
            private_key.private_bytes(
            encoding=encoding,
            format=key_format,
            encryption_algorithm=serialization.NoEncryption())
    return serialized_private

def serialize_public_key(public_key,keys_encoding,key_format):
    encoding = serialization.Encoding[keys_encoding]
    serialized_public = \
                    public_key.public_bytes(
                        encoding=encoding,
                        format=key_format)
    return serialized_public

