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

from cbor_module import Exportable
from generate_test_utils import shell_process
from generate_test_utils import save_to_file
from generate_test_utils import assets_dict
from generate_test_utils import cbor_dict
from cbor_module import to_cbor
from cbor_module import to_cbor_byte_array
import hashlib
import array
import struct
import os


#from convert_test_data_to_c import *

class AssetItem():

    def set_out_dir(self,out_dir):
        self.out_dir = out_dir


    def generate_assets(self):# Abstract method, defined by convention only
        raise NotImplementedError("Subclass must implement generate_assets method")


class KeyItem(AssetItem):

    def set_key_size(self, key_size):
        self.key_size = key_size



class ECCKey(KeyItem):


    def set_key_name(self, key_name):
        self.priv_key_name = "priv_ecc_" + key_name
        self.pub_key_name = "pub_ecc_" + key_name

    def generate_assets(self):

        #Generate and store private key in pem format
        shell_process('openssl  ecparam -name {0} -genkey -noout'.format(self.key_size,self), self.out_dir, self.priv_key_name + ".pem", out='yes')
        #Generate public key in pem format
        shell_process('cd {1} && openssl ec -in {0}.pem -pubout'.format(self.priv_key_name, self.out_dir), self.out_dir, self.pub_key_name + ".pem", out='yes')
        #Convert public key in der format
        shell_process('cd {1} && openssl ec -pubin -inform PEM -in {0}.pem -outform DER'.format(self.pub_key_name, self.out_dir), self.out_dir, self.pub_key_name + ".der", out='yes')
        #Convert private key to der format
        shell_process('cd {1} && openssl ec -in {0}.pem -outform DER'.format(self.priv_key_name ,self.out_dir), self.out_dir, self.priv_key_name + ".der", out='yes')


class RSAKey(KeyItem):

    def set_key_name(self, key_name):
    
        self.priv_key_name = "priv_rsa_" + key_name
        self.pub_key_name = "pub_rsa_" + key_name

    def generate_assets(self):
        if self.key_size !="1024":
            print "\nWrong RSA key size"
            exit(1)
        #Generate and store RSA private key in pem format
        shell_process('cd {1} && openssl genrsa {0}'.format(self.key_size, self.out_dir), self.out_dir, self.priv_key_name + ".pem", out='yes')
        #Generate and store RSA public key in pem format
        shell_process('cd {1} && openssl rsa -in {0}.pem  -outform PEM -pubout'.format(self.priv_key_name, self.out_dir), self.out_dir, self.pub_key_name + ".pem", out='yes')
        #Convert and store private key to der format
        shell_process('cd {1} && openssl rsa -in {0}.pem -outform DER'.format(self.priv_key_name, self.out_dir), self.out_dir, self.priv_key_name + ".der", out='yes')
        #Convert and store public key to der format
        shell_process('cd {1} && openssl rsa -pubin -inform PEM -in {0}.pem -outform DER'.format(self.pub_key_name, self.out_dir), self.out_dir, self.pub_key_name + ".der", out='yes')


class SymmetricKey(KeyItem):

    def set_key_name(self, key_name):
        self.key_name = "aes" + key_name

    def generate_assets(self):
       shell_process('cd {1} && openssl rand -base64 {0}'.format(self.key_size,self.out_dir), self.out_dir, self.key_name + ".key", out='yes')


class Csr(AssetItem):
    def __init__(self, csr_name):
        self.csr_name = csr_name

    def set_private_key(self, key_file_name):
        self.key_file_name = key_file_name

    def set_csr_params(self, country_name,state,locality_name,org_name,org_unit_name,common_name,email_addr,password,opt_company_name):
        self.country_name = country_name +'\n'
        self.state = state +'\n'
        self.locality_name = locality_name +'\n'
        self.org_name = org_name +'\n'
        self.org_unit_name = org_unit_name +'\n'
        self.common_name = common_name +'\n'
        self.email_addr = email_addr +'\n'
        self.password = password +'\n'
        self.opt_company_name = opt_company_name +'\n'

    def generate_assets(self):
        csr_params= self.country_name + self.state + self.locality_name + self.org_name + self.org_unit_name + self.common_name + self.email_addr + self.password + self.opt_company_name
        #self.csr_name = "csr"+ self.csr_name
        shell_process('cd  {1} && openssl req  -new -key {0}'.format(self.key_file_name,self.out_dir), self.out_dir, self.csr_name + ".csr", out='yes', input=csr_params)
        shell_process('cd  {1} && openssl req -in {0}.csr -outform der'.format(self.csr_name,self.out_dir), self.out_dir, self.csr_name + ".der", out='yes')



class x509(AssetItem):
    def __init__(self,x509_name):
        self.x509_name = x509_name
        self.intermediate = False
        self.cert_dates_command = ""
        self.md_alg_command = ""
        self.number_of_days = 2000
        self.child_number_of_days = 2000
        self.child_name = "Child CERT" +'\n'

    def set_private_key(self, key_file_name):
        self.key_file_name = key_file_name
    

    def set_days_to_expiration(self, number_of_days):
        self.number_of_days = number_of_days

    def set_child_days_to_expiration(self,child_number_of_days):
        self.child_number_of_days = child_number_of_days

    def set_interm_cert_key(self, key_file_name):
        self.key_file_name_interm = key_file_name
        self.interm_cert_name = self.x509_name + "_child"
        self.intermediate = True

    def set_x509_params(self, country_name,state,locality_name,org_name,org_unit_name,common_name,email_addr,password,opt_company_name):
        self.country_name = country_name +'\n'
        self.state = state +'\n'
        self.locality_name = locality_name +'\n'
        self.org_name = org_name +'\n'
        self.org_unit_name = org_unit_name +'\n'
        self.common_name = common_name +'\n'
        self.email_addr = email_addr +'\n'
        self.password = password +'\n'
        self.opt_company_name = opt_company_name +'\n'

    def set_md_alg(self,md_alg):
        self.md_alg_command = "-" +  md_alg

    def set_child_name(self,child_name):
        self.child_name = child_name
    def set_expierd_cert(self):
        self.cert_dates_command = ""#"-startdate 140124083742Z"
        self.number_of_days = 22

    def set_future_cert(self):
         self.cert_dates_command = ""#-startdate 270719083742Z"

    def generate_assets(self):
        x509_params= self.country_name + self.state + self.locality_name + self.org_name + self.org_unit_name + self.common_name + self.email_addr + self.password + self.opt_company_name

        shell_process('cd  {2} &&  openssl  req  -new  -key {0} -out {1}.csr '.format(self.key_file_name,self.x509_name,self.out_dir, self.number_of_days,self.cert_dates_command),input=x509_params)
        shell_process('cd  {2} && openssl x509 -req -signkey {0} -in {1}.csr -days {3} {5}'.format(self.key_file_name,self.x509_name,self.out_dir, self.number_of_days,self.cert_dates_command,self.md_alg_command),self.out_dir, self.x509_name + ".pem", out = 'yes', input=x509_params )
        shell_process('cd  {1} && openssl  x509 -in {0}.pem -outform der '.format(self.x509_name,self.out_dir), self.out_dir, self.x509_name + ".der",  out = 'yes')
        if(self.intermediate == True):
           #Names in x509_params_ca should be different from x509_params names
           self.common_name = self.child_name +'\n'
           x509_params_ca= self.country_name + self.state + self.locality_name + self.org_name + self.org_unit_name + self.common_name + self.email_addr + self.password + self.opt_company_name
           shell_process('cd  {2} && openssl  req  -new -key {0} -extensions v3_ca  -out {1}.csr'.format(self.key_file_name_interm,self.interm_cert_name,self.out_dir, self.x509_name),input=x509_params_ca)
           shell_process('cd  {1} && openssl x509 -req -in {0}.csr -CA {3}.pem  -CAkey {2} -out {0}.pem -days {5} -set_serial 3'.format(self.interm_cert_name,self.out_dir,self.key_file_name, self.x509_name,self.key_file_name_interm,self.child_number_of_days), input="Shelly" + '\n')
           shell_process('cd  {1} && openssl  x509 -in {0}.pem -outform der '.format(self.interm_cert_name,self.out_dir), self.out_dir, self.interm_cert_name + ".der",  out = 'yes')





class CfgParam(AssetItem):
    def __init__(self, common_param_type):
        self.common_param_type = common_param_type

    def set_name(self,param_name):
        self.param_name = param_name

    def set_size(self,param_lenght):
        self.param_lenght = param_lenght

    def generate_assets(self):
        #Generate param using openssl rand
        shell_process('cd {1}  && openssl rand -base64 {0}'.format(self.param_lenght,self.out_dir), self.out_dir, self.param_name + ".bin", out = 'yes')

##########################CBOR classes######################

class KeyConfig(Exportable):
    """Key Configuration class
    Contains Key information and configuration
    """
    def set_key_fields(self, key_name, key_type, fmt='der', data=None, acl=None):
        self.Name = key_name
        self.Type = key_type
        self.Format = fmt
        self.Data = data
        #self.ACL = acl

    def set_key_wrong_name_field(self, key_name, key_type, fmt='der', data=None, acl=None):
        self.Field = key_name
        self.Type = key_type
        self.Format = fmt
        self.Data = data
        #self.ACL = acl

    def __eq__(self, other):
        return self.__dict__ == other.__dict__



class GeneralData(Exportable):
    """General Data class
    Contains general information
    """

    def __init__(self, data_name, data_value, acl=None):
        self.Name = data_name
        self.Data = data_value
        #self.ACL = acl

    def __eq__(self, other):
        return self.__dict__ == other.__dict__


class CsrConfig(Exportable):
    """General Data class
    Contains general information
    """

    def __init__(self, data_name, data_value, acl=None):
        self.Name = data_name
        self.Data = data_value
        #self.ACL = acl

    def __eq__(self, other):
        return self.__dict__ == other.__dict__


class CertificateConfig(Exportable):
    """Certificate configuration class
    Contains the certification value in X509 format and expiration time
    """
    def __init__(self, cert_name, cert_bytes, cert_format, acl=None):
        self.Name = cert_name
        self.Format = cert_format
        self.Data = cert_bytes
        #self.ACL = acl

    def __eq__(self, other):
        return self.__dict__ == other.__dict__


class CborConfiguration(Exportable):
    """Main Device Configuration Class
    """

    def set_cbor_groups(self):
        # Current configuration schema is according to given version...
        self.SchemeVersion = "0.0.1"
        self.Keys = list()
        self.Certificates = list()
        self.ConfigParams = list()

    def set_cbor_wrong_keys_group(self):
        # Current configuration schema is according to given version...
        self.SchemeVersion = "0.0.1"
        self.WrongKeyGroupName = list()
        self.Certificates = list()
        self.ConfigParams = list()

    def set_cbor_groups_without_scheme_version(self):
        # Current configuration schema is according to given version...
        self.Keys = list()
        self.Certificates = list()
        self.ConfigParams = list()

    def set_cbor_wrong_certificate_group(self):
        # Current configuration schema is according to given version...
        self.SchemeVersion = "0.0.1"
        self.Keys = list()
        self.certificates = list()
        self.ConfigParams = list()

    def set_cbor_scheme_version(self, value):
        self.SchemeVersion = value

    def set_cbor_csr_group(self):
        # Current configuration schema is according to given version...
        self.Csrs = list()

    def add_ecc_public_key(self, key_name, key_file,
                       key_type="ECCPublic",
                       fmt="der",
                       acl=None):
        key = KeyConfig()
        key.set_key_fields(key_name, key_type, fmt, assets_dict[key_file], acl)
        self.Keys.append(key)


    def add_ecc_public_key_with_wrong_name_field(self, key_name, key_file,
                       key_type="ECCPublic",
                       fmt="der",
                       acl=None):
        key = KeyConfig()
        key.set_key_wrong_name_field(key_name, key_type, fmt, assets_dict[key_file], acl)
        self.Keys.append(key)

    def add_ecc_private_key_wrong_key_group_name(self, key_name, key_file,
                       key_type="ECCPublic",
                       fmt="der",
                       acl=None):
        key = KeyConfig()
        key.set_key_fields(key_name, key_type, fmt, assets_dict[key_file], acl)
        self.WrongKeyGroupName.append(key)

    def add_ecc_private_key(self, key_name, key_file,
                        key_type="ECCPrivate",
                        fmt="der",
                        acl=None):
        key = KeyConfig()
        key.set_key_fields(key_name, key_type, fmt, assets_dict[key_file], acl)
        self.Keys.append(key)

    def add_rsa_public_key(self, key_name, key_file,
                       key_type="RSAPublic",
                       fmt="der",
                       acl=None):
        key = KeyConfig()
        key.set_key_fields(key_name, key_type, fmt, assets_dict[key_file], acl)
        self.Keys.append(key)

    def add_rsa_private_key(self, key_name, key_file,
                        key_type="RSAPrivate",
                        fmt="der",
                        acl=None):
        key = KeyConfig()
        key.set_key_fields(key_name, key_type, fmt, assets_dict[key_file], acl)
        self.Keys.append(key)

    def add_csr(self, csr_name, csr_file,
                        csr_format="der",
                        acl=None):
        self.Csrs.append(CsrConfig(csr_name, assets_dict[csr_file],csr_format))

    def add_certificate(self, cert_name, cert_file,
                        cert_format="der",
                        acl=None):
        self.Certificates.append(
            CertificateConfig(cert_name, assets_dict[cert_file], cert_format, acl))

    def add_certificate_wrong_group_name(self, cert_name, cert_file,
                        cert_format="der",
                        acl=None):
        self.certificates.append(
            CertificateConfig(cert_name, assets_dict[cert_file], cert_format, acl))


    def add_general_data(self, data_name, data_value, acl=None):
        self.ConfigParams.append(GeneralData(data_name, data_value, acl))

    #Device general information
    def set_bootstrap_mode(self, bootstrap_mode):
        self.add_general_data("mbed.UseBootstrap", bootstrap_mode)
    
    # Account ID is not mandatory
    def set_account_id(self, account_id):
         self.add_general_data("mbed.AccountID", account_id)

    def set_endpoint_name(self, serial_number):
         self.add_general_data("mbed.EndpointName", serial_number)

    #LWM2M device object information
    def set_manf_name(self, manufacturer_name):
        self.add_general_data("mbed.Manufacturer", manufacturer_name)

    def set_model_number(self, model_number):
        self.add_general_data("mbed.ModelNumber", model_number)

    def set_device_type(self, device_type):
        self.add_general_data("mbed.DeviceType", device_type)

    def set_hardware_version(self, hardware_version):
        self.add_general_data("mbed.HardwareVersion", hardware_version)

    def set_memory_size_version(self, memory_size):
        self.add_general_data("mbed.MemoryTotalKB", memory_size)

    def set_serial_number(self, serial_number):
        self.add_general_data("mbed.SerialNumber", serial_number)

    def set_current_time(self, current_time):
        self.add_general_data("mbed.CurrentTime", current_time)

    def set_utc_offset(self, utc_offset):
        self.add_general_data("mbed.UTCOffset", utc_offset)

    def set_device_time_zone(self, dev_time_zone):
        self.add_general_data("mbed.Timezone", dev_time_zone)

    #Bootstrap configuration
    def set_bootstrap_uri(self, bootstrap_uri):
        self.add_general_data("mbed.BootstrapServerURI", bootstrap_uri)

    def set_lwm2m_uri(self, lwm2m_uri):
        self.add_general_data("mbed.LwM2MServerURI", lwm2m_uri)





def generate_cbor_data(name, out_dir, cbor_data):
    cbor_binary = to_cbor(cbor_data)
    cbor_byte_array_repr = to_cbor_byte_array(cbor_data)
    save_to_file(out_dir, name, cbor_binary, "bin")
    save_to_file(out_dir, name, cbor_byte_array_repr, "text")
    # save in dictionary
    cbor_dict[name] = cbor_binary


class CommPacket(AssetItem):

    FTD_TOKEN = 0x766f72706465626d #mbedprov

    def __init__(self, name):
        self.name = name

    def build_valid_comm_packet(self, cbor_file):
        self.token = self.FTD_TOKEN
        self.length = len(cbor_dict[cbor_file])
        self.data = cbor_dict[cbor_file]
        self.signature = hashlib.sha256(array.array('B', self.data).tostring()).digest()

    #currently should not be used, since serial on the device side hangs if wrong token sent
    def set_token(self, token):
        self.token = token

    def set_length(self, comm_packet_length):
        self.length = comm_packet_length

    def set_data(self, cbor_data):
        self.data = cbor_data

    def set_signature(self, comm_packet_signature):
        self.signature = comm_packet_signature

    def generate_assets(self):
        fcc_comm_packet = struct.pack('<QI', self.token, self.length) + array.array('B', self.data).tostring() + self.signature
        save_to_file(self.out_dir, self.name, fcc_comm_packet, "bin")

########################## Generate specific data ######################

class StoreGeneralData():
    def __init__(self, name, data):
        self.name = name
        self.data = data
	
    def generate_data(self):
        assets_dict[self.name] = self.data
	
