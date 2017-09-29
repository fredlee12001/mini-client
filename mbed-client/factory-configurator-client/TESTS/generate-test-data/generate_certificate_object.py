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
import datetime
from cryptography.hazmat.primitives import serialization
from cryptography import x509
from cryptography.hazmat.primitives import hashes
from cryptography.hazmat.backends import openssl
from dateutil.relativedelta import relativedelta
from cryptography.x509.oid import NameOID
from generate_test_utils import assets_dict
from generate_crypto_utils import load_private_key
from generate_crypto_utils import load_ca_cert
from generate_crypto_utils import check_certificate_match_pkey
from generate_test_utils import write_data_to_file
from generate_crypto_utils import load_csr


class x509_python_certificate():
    def __init__(self,x509_name,working_dir,csr_file = None):
        self.x509_name = x509_name
        self.working_dir = working_dir
        self.from_date = datetime.datetime.utcnow()
        self.ca_path_length = None
        self.serial_number = x509.random_serial_number()
        self.ca_cert = None
        self.ca_priv_key = None
        self.signing_key = None
        self.csr_file = csr_file
        self.md_alg  = hashes.SHA256()
        self.check_signing_key = True
        self.is_certificate_can_sign = False

    def set_private_key(self, key_file_name):
        self.key_file_name = key_file_name
        self.private_key = load_private_key(self.working_dir+'/'+self.key_file_name)
        self.signing_key =  self.private_key

    def set_valid_from_date(self, start_date):
        self.from_date = start_date

    def set_valid_to_date(self, num_of_days):
        self.to_date = self.from_date + relativedelta(days=num_of_days)

    def set_serial_number(self, serial_number):
        self.serial_number = serial_number

    def set_x509_params(self, country_name,state,locality_name,org_name,org_unit_name,common_name,email_addr):
        self.country_name = country_name
        self.state = state
        self.locality_name = locality_name
        self.org_name = org_name
        self.org_unit_name = org_unit_name
        self.common_name = common_name
        self.email_addr = email_addr

    def set_is_ca_certificate(self,ca_status):
        self.ca_status = ca_status

    def set_md_alg(self,md_alg):
        self.md_alg = md_alg

    def set_ca_data(self,ca_cert_file,ca_priv_key_file,check_signing_key =True):
        self.ca_cert = load_ca_cert(self.working_dir+'/'+ca_cert_file)
        self.ca_priv_key = load_private_key(self.working_dir+'/'+ca_priv_key_file)
        self.signing_key = self.ca_priv_key
        self.check_signing_key = check_signing_key

    def  set_basic_constraints(self,is_certificate_can_sign):
        self.is_certificate_can_sign = is_certificate_can_sign

    def generate_assets(self):
        #Check that CA certificate was set for child certificate
        if self.ca_status == False and ( self.ca_cert == None or self.ca_priv_key == None):
           print ("To create child certificate. please set CA certificate and private key")
           exit(1)

        #Check that CA certificate and its private key match
        if self.ca_status == False and self.check_signing_key == True:
           is_ca_certificate_match_to_ca_private_key = check_certificate_match_pkey(self.ca_cert,self.ca_priv_key)
           if is_ca_certificate_match_to_ca_private_key == False :
               print ("\n CA certificate doesn't match to ca private key")
               exit(1)

        #Defines if certificate is ca or not and certificate chain lenght.
        basic_constraints_ext = x509.BasicConstraints(self.is_certificate_can_sign, self.ca_path_length)

        key_identifier = x509.SubjectKeyIdentifier.from_public_key(self.private_key.public_key())
        #The key usage extension defines the purpose of the key contained in the certificate.
        key_usage_ext = x509.KeyUsage(digital_signature=True,
                                      content_commitment=True,
                                      key_encipherment=False,
                                      data_encipherment=True,
                                      key_agreement=True,
                                      key_cert_sign=True,
                                      crl_sign=True,
                                      encipher_only=False,
                                      decipher_only=False)


        if self.csr_file == None :
            # unicode(self.country_name, "utf-8")
            subject = issuer = x509.Name([
                x509.NameAttribute(NameOID.COUNTRY_NAME, self.country_name.decode('utf-8')),
                x509.NameAttribute(NameOID.STATE_OR_PROVINCE_NAME, self.state.decode('utf-8')),
                x509.NameAttribute(NameOID.LOCALITY_NAME,  self.locality_name.decode('utf-8')),
                x509.NameAttribute(NameOID.ORGANIZATION_NAME,  self.org_name.decode('utf-8')),
                x509.NameAttribute(NameOID.ORGANIZATIONAL_UNIT_NAME,  self.org_unit_name.decode('utf-8')),
                x509.NameAttribute(NameOID.COMMON_NAME,  self.common_name.decode('utf-8')),
                x509.NameAttribute(NameOID.EMAIL_ADDRESS ,  self.email_addr.decode('utf-8')),])

            if self.ca_status == False :
                issuer = self.ca_cert.subject

            #Create certificate data
            cert_builder = x509.CertificateBuilder().subject_name(
                subject
                ).issuer_name(
                issuer
            ).public_key(
                self.private_key.public_key()
            ).serial_number(
                self.serial_number
            ).not_valid_before(
                self.from_date
            ).not_valid_after(
                self.to_date
            ).add_extension(
                basic_constraints_ext,
                critical=False,
            ).add_extension(
                key_usage_ext,
                critical=False,
            ).add_extension(
                key_identifier,
                critical=False
            )

        if self.csr_file != None  and self.ca_status == False:
            print ("\n To create certificate based on csr you should sign it with ca certificate")

        if self.csr_file != None :
            csr = load_csr(self.working_dir+'/'+self.csr_file)
            if not csr.is_signature_valid:
                print ("\n Invalid csr signature!")
                exit(1)

            issuer = self.ca_cert.subject

            cert_builder = x509.CertificateBuilder().subject_name(
                csr.subject
            ).issuer_name(
                issuer
            ).public_key(
                csr.public_key()
            ).serial_number(
                self.serial_number
            ).not_valid_before(
                self.from_date
            ).not_valid_after(
                self.ca_cert.not_valid_after)

        #Sign the created certificate data
        cert = cert_builder.sign(self.signing_key, self.md_alg, openssl.backend)

        #Write created certificate to pem file
        #Add pem certificate  data to assets_dict
        cert_public_bytes = cert.public_bytes(serialization.Encoding.PEM)
        assets_dict[self.x509_name+'.pem'] = cert_public_bytes
        write_data_to_file(self.working_dir+'/'+self.x509_name+'.pem',cert_public_bytes)

        #Write created certificate to der file
        #Add der certificate data to assets_dict
        cert_public_bytes = cert.public_bytes(serialization.Encoding.DER)
        assets_dict[self.x509_name+'.der'] = cert_public_bytes
        write_data_to_file(self.working_dir+'/'+self.x509_name+'.der',cert_public_bytes)
