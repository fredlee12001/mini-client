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


from generate_test_objects import ECCKey 
from generate_test_objects import RSAKey 
from generate_test_objects import SymmetricKey
from generate_test_objects import Csr
from generate_test_objects import CfgParam
from generate_test_objects import KeyConfig
from generate_test_objects import CertificateConfig
from generate_test_objects import CsrConfig
from generate_test_objects import x509
from generate_test_objects import CborConfiguration
from generate_test_objects import CommPacket
from generate_test_objects import generate_cbor_data
from generate_test_objects import StoreGeneralData
from generate_certificate_object import x509_python_certificate
from generate_key_object import python_ecc_key_pair
from generate_csr_object import csr
from dateutil.relativedelta import relativedelta
import datetime
import os
import time



# The number of elapsed seconds since Epoch (a fixed moment in time)
# and it is the same in any timezone.
secs_since_epoch = int(time.time())

# Generates keys
def generate_asym_test_keys(outDir):

    #key 1 parametrs
    key1 = ECCKey()
    key1.set_key_name("key1")
    key1.set_key_size("secp256r1")
    key1.set_out_dir(outDir)
    key1.generate_assets()

    #key 2 parametrs
    key2 = ECCKey()
    key2.set_key_name("key2")
    key2.set_key_size("secp256r1")
    key2.set_out_dir(outDir)
    key2.generate_assets()

    #key 3 parameters
    key3 = RSAKey()
    key3.set_key_name("key3")
    key3.set_key_size("1024")
    key3.set_out_dir(outDir)
    key3.generate_assets()

def generate_sym_test_keys(outDir):
    key4 = SymmetricKey()
    key4.set_key_name("key4")
    key4.set_key_size("32")
    key4.set_out_dir(outDir)
    key4.generate_assets()


# Generates CSRs
def generate_test_csrs(outDir):

    #csr1 parameters
    csr1 = Csr("csr1")
    csr1.set_private_key("priv_ecc_key1.pem")  #created based on key1 private key
    csr1.set_csr_params("UK","Kfar-neter","Netania","ARM","IOT","Device","device01@arm.com","1111","ARM")
    csr1.set_out_dir(outDir)
    csr1.generate_assets()

    #csr2 parameters
    csr2 = Csr("csr2")
    csr2.set_private_key("priv_rsa_key3.pem")  #created based on key3 private key
    csr2.set_csr_params("UK","Kfar-neter","Natania","ARM","IOT","Device","device01@arm.com","1111","ARM")
    csr2.set_out_dir(outDir)
    csr2.generate_assets()

# Generates certificates
def generate_test_certificates(outDir):

    #certificate 1 params
    certificate1 = x509("certificate1")
    certificate1.set_private_key("priv_ecc_key1.pem")  #created based on key1 private key
    certificate1.set_x509_params("UK","Kfar-neter","Netania","ARM","IOT","Device","device01@arm.com","1111","ARM")
    certificate1.set_out_dir(outDir)
    certificate1.set_days_to_expiration(5000)
    certificate1.generate_assets()

    #certificate 2 params
    certificate2 = x509("certificate2")
    certificate2.set_private_key("priv_rsa_key3.pem")  #created based on key3 private key
    certificate2.set_x509_params("UK","Kfar-neter","Netania","ARM","IOT","Device","device01@arm.com","1111","ARM")
    certificate2.set_out_dir(outDir)
    certificate2.set_days_to_expiration(5000)
    #cert2 generate cbor params
    certificate2.generate_assets()



def generate_python_ecc_keys(outDir):
    python_ecc_key = python_ecc_key_pair("ecc_python_key_1",outDir)
    python_ecc_key.generate_assets()

    python_ecc_key = python_ecc_key_pair("ecc_python_key_2",outDir)
    python_ecc_key.generate_assets()

    python_ecc_key = python_ecc_key_pair("ecc_python_key_3",outDir)
    python_ecc_key.generate_assets()

    python_ecc_key = python_ecc_key_pair("ecc_python_key_4",outDir)
    python_ecc_key.generate_assets()

    python_ecc_key = python_ecc_key_pair("ecc_python_key_5",outDir)
    python_ecc_key.generate_assets()



def generate_ptyhon_csr(outDir):
    x509_csr = csr("csr_pyth_1","priv_ecc_key1.pem",outDir,u"UK",u"Kfar-neter",u"Netania","ARM","IOT","Device","device01@arm.com")
    x509_csr.generate_assets()

def generate_python_x509_certificates(outDir):

    #############################################################################################
    #Chain of 5 certificates (ca->1 chald->2 child->3 child->4 child)
    x509_ca_chain_cert = x509_python_certificate("x509_pth_ca_chain_1",outDir)
    x509_ca_chain_cert.set_private_key("private_ecc_python_key_1.pem")
    x509_ca_chain_cert.set_valid_from_date(datetime.datetime.utcnow())
    x509_ca_chain_cert.set_valid_to_date(3567)
    x509_ca_chain_cert.set_serial_number(25462452)
    x509_ca_chain_cert.set_is_ca_certificate(True)
    x509_ca_chain_cert.set_basic_constraints(True)
    x509_ca_chain_cert.set_x509_params(u"UK",u"Kfar-neter",u"Netania","ARM","IOT","Device","device01@arm.com")
    x509_ca_chain_cert.generate_assets()

    x509_chain_ch_1 = x509_python_certificate("x509_pth_chain_child_1",outDir)
    x509_chain_ch_1.set_private_key("private_ecc_python_key_2.pem")
    x509_chain_ch_1.set_valid_from_date(datetime.datetime.utcnow())
    x509_chain_ch_1.set_valid_to_date(5412)
    x509_chain_ch_1.set_serial_number(35634432)
    x509_chain_ch_1.set_is_ca_certificate(False)
    x509_chain_ch_1.set_basic_constraints(True)
    x509_chain_ch_1.set_ca_data('x509_pth_ca_chain_1.pem','private_ecc_python_key_1.pem')
    x509_chain_ch_1.set_x509_params("UK","Kfar-neter","Netania","ARM","IOT","EP_fdsf_1","device01@arm.com")
    x509_chain_ch_1.generate_assets()

    x509_chain_ch_2 = x509_python_certificate("x509_pth_chain_child_2",outDir)
    x509_chain_ch_2.set_private_key("private_ecc_python_key_3.pem")
    x509_chain_ch_2.set_valid_from_date(datetime.datetime.utcnow())
    x509_chain_ch_2.set_valid_to_date(5856)
    x509_chain_ch_2.set_serial_number(3452345)
    x509_chain_ch_2.set_is_ca_certificate(False)
    x509_chain_ch_2.set_basic_constraints(True)
    x509_chain_ch_2.set_ca_data('x509_pth_chain_child_1.pem','private_ecc_python_key_2.pem')
    x509_chain_ch_2.set_x509_params("UK","Kfar-neter","Netania","ARM","IOT","EP_fdsf_2","device01@arm.com")
    x509_chain_ch_2.generate_assets()

    x509_chain_ch_3 = x509_python_certificate("x509_pth_chain_child_3",outDir)
    x509_chain_ch_3.set_private_key("private_ecc_python_key_4.pem")
    x509_chain_ch_3.set_valid_from_date(datetime.datetime.utcnow())
    x509_chain_ch_3.set_valid_to_date(5000)
    x509_chain_ch_3.set_serial_number(234555)
    x509_chain_ch_3.set_is_ca_certificate(False)
    x509_chain_ch_3.set_basic_constraints(True)
    x509_chain_ch_3.set_ca_data('x509_pth_chain_child_2.pem','private_ecc_python_key_3.pem')
    x509_chain_ch_3.set_x509_params("UK","Kfar-neter","Netania","ARM","IOT","EP_fdsf_3","device01@arm.com")
    x509_chain_ch_3.generate_assets()


    x509_chain_ch_4 = x509_python_certificate("x509_pth_chain_child_4",outDir)
    x509_chain_ch_4.set_private_key("private_ecc_python_key_5.pem")
    x509_chain_ch_4.set_valid_from_date(datetime.datetime.utcnow())
    x509_chain_ch_4.set_valid_to_date(5123)
    x509_chain_ch_4.set_serial_number(8424567)
    x509_chain_ch_4.set_is_ca_certificate(False)
    x509_chain_ch_4.set_ca_data('x509_pth_chain_child_3.pem','private_ecc_python_key_4.pem')
    x509_chain_ch_4.set_x509_params("UK","Kfar-neter","Netania","ARM","IOT","EP_fdsf_4","device01@arm.com")
    x509_chain_ch_4.generate_assets()

    #############################################################################################
    #Pair of valid CA and child certificates
    x509_ca_cert = x509_python_certificate("x509_pth_ca",outDir)
    x509_ca_cert.set_private_key("priv_ecc_key1.pem")
    x509_ca_cert.set_valid_from_date(datetime.datetime.utcnow())
    x509_ca_cert.set_valid_to_date(6000)
    x509_ca_cert.set_serial_number(12334)
    x509_ca_cert.set_is_ca_certificate(True)
    x509_ca_cert.set_basic_constraints(True)
    x509_ca_cert.set_x509_params(u"UK",u"Kfar-neter",u"Netania","ARM","IOT","Device","device01@arm.com")
    x509_ca_cert.generate_assets()

    x509_ch_cert = x509_python_certificate("x509_pth_ca_child",outDir)
    x509_ch_cert.set_private_key("priv_ecc_key2.pem")
    x509_ch_cert.set_valid_from_date(datetime.datetime.utcnow())
    x509_ch_cert.set_valid_to_date(8000)
    x509_ch_cert.set_serial_number(56432)
    x509_ch_cert.set_is_ca_certificate(False)
    x509_ch_cert.set_ca_data('x509_pth_ca.pem','priv_ecc_key1.pem')
    x509_ch_cert.set_x509_params("UK","Kfar-neter","Netania","ARM","IOT","EP_fdsf","device01@arm.com")
    x509_ch_cert.generate_assets()
    #############################################################################################

    #############################################################################################
    #Child Certificate created based on csr:
    #FIXME: certificate based on csr failed with invalid curve error.Check if the problem in pal or in csr.
    #x509_cert_from_csr = x509_python_certificate("x509_pth_cert_from_csr",outDir,"csr_pyth_1.pem")
    #x509_cert_from_csr.set_private_key("priv_ecc_key1.pem")
    #x509_cert_from_csr.set_valid_from_date(datetime.datetime.utcnow())
    #x509_cert_from_csr.set_valid_to_date(6000)
    #x509_cert_from_csr.set_serial_number(12334)
    #x509_cert_from_csr.set_is_ca_certificate(False)
    #x509_cert_from_csr.set_ca_data('x509_pth_ca.pem','priv_ecc_key1.pem')
    #x509_cert_from_csr.generate_assets()

    #############################################################################################
    #Pair of valid CA and child certificate
    x509_ca_cert_1 = x509_python_certificate("x509_pth_ca_1",outDir)
    x509_ca_cert_1.set_private_key("priv_ecc_key2.pem")
    x509_ca_cert_1.set_valid_from_date(datetime.datetime.utcnow())
    x509_ca_cert_1.set_valid_to_date(6000)
    x509_ca_cert_1.set_serial_number(12334)
    x509_ca_cert_1.set_is_ca_certificate(True)
    x509_ca_cert_1.set_basic_constraints(True)
    x509_ca_cert_1.set_x509_params(u"UK",u"Kfar-neter",u"Netania","ARM","IOT","Device","device01@arm.com")
    x509_ca_cert_1.generate_assets()

    x509_ch_cert_1 = x509_python_certificate("x509_pth_child_1",outDir)
    x509_ch_cert_1.set_private_key("priv_ecc_key1.pem")
    x509_ch_cert_1.set_valid_from_date(datetime.datetime.utcnow())
    x509_ch_cert_1.set_valid_to_date(8000)
    x509_ch_cert_1.set_serial_number(56432)
    x509_ch_cert_1.set_is_ca_certificate(False)
    x509_ch_cert_1.set_ca_data('x509_pth_ca_1.pem','priv_ecc_key2.pem')
    x509_ch_cert_1.set_x509_params("UK","Kfar-neter","Netania","ARM","IOT","EP_fdsf","device01@arm.com")
    x509_ch_cert_1.generate_assets()
    #############################################################################################

    #############################################################################################
    #Pair of valid  CA (valid from date is less than utcnow) and expired child certificate
    x509_ca_cert_2 = x509_python_certificate("x509_pth_ca_2",outDir)
    x509_ca_cert_2.set_private_key("priv_ecc_key1.pem")
    x509_ca_cert_2.set_valid_from_date(datetime.datetime.utcnow() - relativedelta(days=50))
    x509_ca_cert_2.set_valid_to_date(4568)
    x509_ca_cert_2.set_serial_number(12334)
    x509_ca_cert_2.set_is_ca_certificate(True)
    x509_ca_cert_2.set_basic_constraints(True)
    x509_ca_cert_2.set_x509_params(u"UK",u"Kfar-neter",u"Netania","ARM","IOT","Device","device01@arm.com")
    x509_ca_cert_2.generate_assets()

    x509_ch_cert_2 = x509_python_certificate("x509_pth_child_expired",outDir)
    x509_ch_cert_2.set_private_key("priv_ecc_key1.pem")
    x509_ch_cert_2.set_valid_from_date(datetime.datetime.utcnow()- relativedelta(days=50))
    x509_ch_cert_2.set_valid_to_date(20)
    x509_ch_cert_2.set_serial_number(56432)
    x509_ch_cert_2.set_is_ca_certificate(False)
    x509_ch_cert_2.set_ca_data('x509_pth_ca_2.pem','priv_ecc_key1.pem')
    x509_ch_cert_2.set_x509_params("UK","Kfar-neter","Netania","ARM","IOT","EP_fdsf","device01@arm.com")
    x509_ch_cert_2.generate_assets()
    #############################################################################################



    #############################################################################################
    #Pair of CA and child certificates valid in the future.
    x509_ca_cert_3 = x509_python_certificate("x509_pth_ca_3_future",outDir)
    x509_ca_cert_3.set_private_key("priv_ecc_key2.pem")
    x509_ca_cert_3.set_valid_from_date(datetime.datetime.utcnow() + relativedelta(days=994))
    x509_ca_cert_3.set_valid_to_date(2000)
    x509_ca_cert_3.set_serial_number(3452345)
    x509_ca_cert_3.set_is_ca_certificate(True)
    x509_ca_cert_3.set_basic_constraints(True)
    x509_ca_cert_3.set_x509_params(u"UK",u"Kfar-neter",u"Netania","ARM","IOT","Device","device01@arm.com")
    x509_ca_cert_3.generate_assets()

    x509_ch_cert_3 = x509_python_certificate("x509_pth_child_future",outDir)
    x509_ch_cert_3.set_private_key("priv_ecc_key1.pem")
    x509_ch_cert_3.set_valid_from_date(datetime.datetime.utcnow() + relativedelta(days=994))
    x509_ch_cert_3.set_valid_to_date(20)
    x509_ch_cert_3.set_serial_number(11111)
    x509_ch_cert_3.set_is_ca_certificate(False)
    x509_ch_cert_3.set_ca_data('x509_pth_ca_3_future.pem','priv_ecc_key2.pem')
    x509_ch_cert_3.set_x509_params("UK","Kfar-neter","Netania","ARM","IOT","EP_fdsf","device01@arm.com")
    x509_ch_cert_3.generate_assets()
    #############################################################################################

    #############################################################################################
    #Pair of CA and child . when child signed with wrong key
    x509_ca_cert_4= x509_python_certificate("x509_pth_ca_4",outDir)
    x509_ca_cert_4.set_private_key("priv_ecc_key1.pem")
    x509_ca_cert_4.set_valid_from_date(datetime.datetime.utcnow())
    x509_ca_cert_4.set_valid_to_date(2000)
    x509_ca_cert_4.set_serial_number(3452345)
    x509_ca_cert_4.set_is_ca_certificate(True)
    x509_ca_cert_4.set_basic_constraints(True)
    x509_ca_cert_4.set_x509_params(u"UK",u"Kfar-neter",u"Netania","ARM","IOT","Device","device01@arm.com")
    x509_ca_cert_4.generate_assets()

    x509_ch_cert_4 = x509_python_certificate("x509_pyth_child_4",outDir)
    x509_ch_cert_4.set_private_key("priv_ecc_key2.pem")
    x509_ch_cert_4.set_valid_from_date(datetime.datetime.utcnow())
    x509_ch_cert_4.set_valid_to_date(5500)
    x509_ch_cert_4.set_serial_number(11111)
    x509_ch_cert_4.set_is_ca_certificate(False)
    x509_ch_cert_4.set_ca_data('x509_pth_ca_3_future.pem','private_ecc_python_key_1.pem',False)
    x509_ch_cert_4.set_x509_params("UK","Kfar-neter","Netania","ARM","IOT","EP_fdsf","device01@arm.com")
    x509_ch_cert_4.generate_assets()

    #############################################################################################
    #Pair of expired valid  CA and child certificate
    x509_ca_cert_5 = x509_python_certificate("x509_pth_expr_ca_5",outDir)
    x509_ca_cert_5.set_private_key("priv_ecc_key1.pem")
    x509_ca_cert_5.set_valid_from_date(datetime.datetime.utcnow() - relativedelta(days=1000))
    x509_ca_cert_5.set_valid_to_date(10)
    x509_ca_cert_5.set_serial_number(12334)
    x509_ca_cert_5.set_is_ca_certificate(True)
    x509_ca_cert_5.set_basic_constraints(True)
    x509_ca_cert_5.set_x509_params(u"UK",u"Kfar-neter",u"Netania","ARM","IOT","Device","device01@arm.com")
    x509_ca_cert_5.generate_assets()

    x509_ch_cert_5 = x509_python_certificate("x509_pth_expr_child_5",outDir)
    x509_ch_cert_5.set_private_key("priv_ecc_key2.pem")
    x509_ch_cert_5.set_valid_from_date(datetime.datetime.utcnow()- relativedelta(days=999))
    x509_ch_cert_5.set_valid_to_date(5)
    x509_ch_cert_5.set_serial_number(56432)
    x509_ch_cert_5.set_is_ca_certificate(False)
    x509_ch_cert_5.set_ca_data('x509_pth_expr_ca_5.pem','priv_ecc_key1.pem')
    x509_ch_cert_5.set_x509_params("UK","Kfar-neter","Netania","ARM","IOT","EP_fdsf","device01@arm.com")
    x509_ch_cert_5.generate_assets()
    #############################################################################################
    # Generates x509 cert
def generate_test_x509(outDir):

    key5 = ECCKey()
    key5.set_key_name("key5")
    key5.set_key_size("secp384r1")
    key5.set_out_dir(outDir)
    key5.generate_assets()

    key6 = ECCKey()
    key6.set_key_name("key6")
    key6.set_key_size("secp521r1")
    key6.set_out_dir(outDir)
    key6.generate_assets()

    x509_2_intrm = x509("x509_2_ca")
    x509_2_intrm.set_private_key("priv_ecc_key2.pem")  #created based on key1 private key
    x509_2_intrm.set_x509_params("UK","Kfar-neter","Netania","ARM","IOT","Device","device01@arm.com","1111","ARM")
    x509_2_intrm.set_out_dir(outDir)
    x509_2_intrm.set_days_to_expiration(9000)
    x509_2_intrm.set_child_days_to_expiration(6000)
    x509_2_intrm.set_interm_cert_key("priv_ecc_key1.pem") #created based on key2 private key
    x509_2_intrm.generate_assets()

    x509_1 = x509("x509_1")
    x509_1.set_private_key("priv_ecc_key1.pem")  #created based on key1 private key
    x509_1.set_x509_params("UK","Kfar-neter","Netania","ARM","IOT","Device","device01@arm.com","1111","ARM")
    x509_1.set_out_dir(outDir)
    x509_1.set_days_to_expiration(9000)
    x509_1.set_child_days_to_expiration(6000)
    x509_1.generate_assets()


    x509_1_intrm = x509("x509_1_ca")
    x509_1_intrm.set_private_key("priv_ecc_key1.pem")  #created based on key1 private key
    x509_1_intrm.set_x509_params("UK","Kfar-neter","Netania","ARM",".arm.com","Device","device01@arm.com","1111","ARM")
    x509_1_intrm.set_out_dir(outDir)
    x509_1_intrm.set_days_to_expiration(8000)
    x509_1_intrm.set_child_days_to_expiration(8000)
    x509_1_intrm.set_interm_cert_key("priv_ecc_key2.pem") #created based on key2 private key
    x509_1_intrm.set_child_name("EP_fdsf")
    x509_1_intrm.generate_assets()

    #does not work for now in open SSL , should be fixed or implemetned on python
    #x509_1_exp = x509("x509_1_exp")
    #x509_1_exp.set_private_key("priv_ecc_key1.pem")  #created based on key1 private key
    #x509_1_exp.set_x509_params("UK","Kfar-neter","Netania","ARM","IOT","Device","device01@arm.com","1111","ARM")
    #x509_1_exp.set_out_dir(outDir)
    #x509_1_exp.set_expierd_cert()
    #x509_1_exp.generate_assets()

    #x509_1_future = x509("x509_1_future")
    #x509_1_future.set_private_key("priv_ecc_key1.pem")  #created based on key1 private key
    #x509_1_future.set_x509_params("UK","Kfar-neter","Netania","ARM","IOT","Device","device01@arm.com","1111","ARM")
    #x509_1_future.set_out_dir(outDir)
    #x509_1_future.set_future_cert()
    #x509_1_future.generate_assets()

    x509_1_md = x509("x509_1_md")
    x509_1_md.set_private_key("priv_ecc_key1.pem")  #created based on key1 private key
    x509_1_md.set_x509_params("UK","Kfar-neter","Netania","ARM","IOT","Device","device01@arm.com","1111","ARM")
    x509_1_md.set_out_dir(outDir)
    x509_1_md.set_md_alg("sha224")
    x509_1_md.generate_assets()

    x509_1_inv_key = x509("x509_1_inv_key")
    x509_1_inv_key.set_private_key("priv_ecc_key5.pem")  #created based on key1 private key
    x509_1_inv_key.set_x509_params("UK","Kfar-neter","Netania","ARM","IOT","Device","device01@arm.com","1111","ARM")
    x509_1_inv_key.set_out_dir(outDir)
    x509_1_inv_key.generate_assets()

    x509_2_inv_key = x509("x509_2_inv_key")
    x509_2_inv_key.set_private_key("priv_ecc_key6.pem")  #created based on key1 private key
    x509_2_inv_key.set_x509_params("UK","Kfar-neter","Netania","ARM","IOT","Device","device01@arm.com","1111","ARM")
    x509_2_inv_key.set_out_dir(outDir)
    x509_2_inv_key.generate_assets()


    x509_1_ch_inv_key = x509("x509_1_ch_inv_key")
    x509_1_ch_inv_key.set_private_key("priv_ecc_key1.pem")  #created based on key1 private key
    x509_1_ch_inv_key.set_x509_params("UK","Kfar-neter","Netania","ARM","IOT","Device","device01@arm.com","1111","ARM")
    x509_1_ch_inv_key.set_out_dir(outDir)
    x509_1_ch_inv_key.set_interm_cert_key("priv_ecc_key5.pem") #created based on key5 private key
    x509_1_ch_inv_key.generate_assets()

    x509_1_ch_inv_md = x509("x509_1_ch_inv_md")
    x509_1_ch_inv_md.set_private_key("priv_ecc_key1.pem")  #created based on key1 private key
    x509_1_ch_inv_md.set_x509_params("UK","Kfar-neter","Netania","ARM","IOT","Device","device01@arm.com","1111","ARM")
    x509_1_ch_inv_md.set_out_dir(outDir)
    x509_1_ch_inv_md.set_md_alg("sha512")
    x509_1_ch_inv_md.set_interm_cert_key("priv_ecc_key2.pem") #created based on key2 private key
    x509_1_ch_inv_md.generate_assets()

    x509_2_ch_inv_md = x509("x509_2_ch_inv_md")
    x509_2_ch_inv_md.set_private_key("priv_ecc_key1.pem")  #created based on key1 private key
    x509_2_ch_inv_md.set_x509_params("UK","Kfar-neter","Netania","ARM","IOT","Device","device01@arm.com","1111","ARM")
    x509_2_ch_inv_md.set_out_dir(outDir)
    x509_2_ch_inv_md.set_md_alg("sha1")
    x509_2_ch_inv_md.set_interm_cert_key("priv_ecc_key2.pem") #created based on key2 private key
    x509_2_ch_inv_md.generate_assets()


    x509_3_intrm = x509("x509_3_ca")
    x509_3_intrm.set_private_key("priv_ecc_key1.pem")  #created based on key1 private key
    x509_3_intrm.set_x509_params("UK","Kfar-neter","Netania","ARM","IOT","Device","device01@arm.com","1111","ARM")
    x509_3_intrm.set_out_dir(outDir)
    x509_3_intrm.set_days_to_expiration(6000)
    x509_3_intrm.set_child_days_to_expiration(2000)
    x509_3_intrm.set_interm_cert_key("priv_ecc_key2.pem") #created based on key2 private key
    x509_3_intrm.generate_assets()

# Generates common params (for now only generated random data of given size via openssl rand)
def generate_test_config_params(outDir):

    common_param1= CfgParam("url")
    common_param1.set_name("bootstrap_url")
    common_param1.set_out_dir(outDir)
    common_param1.set_size("512")
    common_param1.generate_assets()

def generate_test_cbors(out_dir):
    #cbors generation
    c1  = CborConfiguration()
    c1.set_cbor_groups()
    c1.add_ecc_private_key("BtsDTLSDevice","priv_ecc_key1.der")
    c1.add_ecc_public_key("UpdateVerification", "pub_ecc_key1.der")
    c1.add_certificate("RootCA", "x509_1_ca.der")
    generate_cbor_data("cbor1", out_dir, c1)

    c2  = CborConfiguration()
    c2.set_cbor_groups()
    c2.add_ecc_private_key("E2EDevice","priv_ecc_key1.der")
    #c2.add_csr("DTLSDevice", "priv_ecc_key2.der")
    c2.add_certificate("UpdateVerification", "x509_1_ca.der")
    c2.add_certificate("E2EUpdate", "x509_1_ca.der")
    generate_cbor_data("cbor2", out_dir, c2)

    c3  = CborConfiguration()
    c3.set_cbor_groups()
    c3.add_ecc_private_key("E2EDevice","priv_ecc_key2.der")
    c3.set_serial_number("AACB")
    c3.set_endpoint_name("EP_fdsf")
    generate_cbor_data("cbor3", out_dir, c3)

    cbor_wrong_key_group_name  = CborConfiguration()
    cbor_wrong_key_group_name.set_cbor_wrong_keys_group()
    cbor_wrong_key_group_name.add_ecc_private_key_wrong_key_group_name("E2EDevice","priv_ecc_key2.der")
    cbor_wrong_key_group_name.set_serial_number("AACB")
    cbor_wrong_key_group_name.set_endpoint_name("EP_fdsf")
    generate_cbor_data("cbor_wrong_key_group_name", out_dir, cbor_wrong_key_group_name)

    cbor_wrong_scheme_version  = CborConfiguration()
    cbor_wrong_scheme_version.set_cbor_groups()
    cbor_wrong_scheme_version.set_cbor_scheme_version("0.0.2")
    cbor_wrong_scheme_version.add_ecc_private_key("E2EDevice","priv_ecc_key2.der")
    cbor_wrong_scheme_version.set_serial_number("AACB")
    cbor_wrong_scheme_version.set_endpoint_name("EP_fdsf")
    generate_cbor_data("cbor_wrong_scheme_version", out_dir, cbor_wrong_scheme_version)

    cbor_wrong_certificates_group_name  = CborConfiguration()
    cbor_wrong_certificates_group_name.set_cbor_wrong_certificate_group()
    cbor_wrong_certificates_group_name.add_ecc_private_key("E2EDevice","priv_ecc_key1.der")
    cbor_wrong_certificates_group_name.add_certificate_wrong_group_name("UpdateVerification", "x509_1_ca.der")
    cbor_wrong_certificates_group_name.add_certificate_wrong_group_name("E2EUpdate", "x509_1_ca.der")
    generate_cbor_data("cbor_wrong_certificates_group_name", out_dir, cbor_wrong_certificates_group_name)

    cbor_wrong_key_name_field  = CborConfiguration()
    cbor_wrong_key_name_field.set_cbor_groups()
    cbor_wrong_key_name_field.add_ecc_private_key("E2EDevice","priv_ecc_key1.der")
    cbor_wrong_key_name_field.add_ecc_public_key_with_wrong_name_field("UpdateVerification", "pub_ecc_key1.der")
    cbor_wrong_key_name_field.add_certificate("UpdateVerification", "x509_1_ca.der")
    cbor_wrong_key_name_field.add_certificate("E2EUpdate", "x509_1_ca.der")
    generate_cbor_data("cbor_wrong_key_name_field", out_dir, cbor_wrong_key_name_field)


    cbor_without_scheme_version_group  = CborConfiguration()
    cbor_without_scheme_version_group.set_cbor_groups_without_scheme_version()
    cbor_without_scheme_version_group.add_ecc_private_key("E2EDevice","priv_ecc_key1.der")
    cbor_without_scheme_version_group.add_certificate("UpdateVerification", "x509_1_ca.der")
    cbor_without_scheme_version_group.add_certificate("E2EUpdate", "x509_1_ca.der")
    generate_cbor_data("cbor_without_scheme_version_group", out_dir, cbor_without_scheme_version_group)

    cbor_wrong_ecc_private_key  = CborConfiguration()
    cbor_wrong_ecc_private_key.set_cbor_groups()
    cbor_wrong_ecc_private_key.add_ecc_private_key("E2EDevice","priv_rsa_key3.der")
    cbor_wrong_ecc_private_key.add_certificate("UpdateVerification", "x509_1_ca.der")
    cbor_wrong_ecc_private_key.add_certificate("E2EUpdate", "x509_1_ca.der")
    generate_cbor_data("cbor_wrong_ecc_private_key", out_dir, cbor_wrong_ecc_private_key)

    cbor_wrong_certificate  = CborConfiguration()
    cbor_wrong_certificate.set_cbor_groups()
    cbor_wrong_certificate.add_ecc_private_key("E2EDevice","priv_ecc_key1.der")
    cbor_wrong_certificate.add_certificate("UpdateVerification", "priv_rsa_key3.der")
    generate_cbor_data("cbor_wrong_certificate", out_dir, cbor_wrong_certificate)


    cbor_store_request_with_csr  = CborConfiguration()
    cbor_store_request_with_csr.set_cbor_groups()
    cbor_store_request_with_csr.set_cbor_csr_group()
    cbor_store_request_with_csr.add_ecc_private_key("E2EDevice","priv_ecc_key1.der")
    cbor_store_request_with_csr.add_csr("DTLSDevice", "priv_ecc_key2.der")
    cbor_store_request_with_csr.add_certificate("UpdateVerification", "x509_1_ca.der")
    cbor_store_request_with_csr.add_certificate("E2EUpdate", "x509_1_ca.der")
    generate_cbor_data("cbor_store_request_with_csr", out_dir, cbor_store_request_with_csr)

    cbor_empty_configuration_parameter  = CborConfiguration()
    cbor_empty_configuration_parameter.set_cbor_groups()
    cbor_empty_configuration_parameter.add_ecc_private_key("E2EDevice","priv_ecc_key1.der")
    cbor_empty_configuration_parameter.add_certificate("UpdateVerification", "x509_1_ca.der")
    cbor_empty_configuration_parameter.add_certificate("E2EUpdate", "x509_1_ca.der")
    cbor_empty_configuration_parameter.set_serial_number("")
    generate_cbor_data("cbor_empty_configuration_parameter", out_dir, cbor_empty_configuration_parameter)

    cbor_with_all_mandatory_parameters  = CborConfiguration()
    cbor_with_all_mandatory_parameters.set_cbor_groups()
    #Set all configuration parameters
    cbor_with_all_mandatory_parameters.set_serial_number("A12FC-45")
    cbor_with_all_mandatory_parameters.set_endpoint_name("EP_fdsf")
    cbor_with_all_mandatory_parameters.set_bootstrap_uri("coap://?aid=bootstrap.arm.com")
    cbor_with_all_mandatory_parameters.set_manf_name("Toshiba")
    cbor_with_all_mandatory_parameters.set_current_time(secs_since_epoch)
    cbor_with_all_mandatory_parameters.set_device_type("TEMP-SENSOR")
    cbor_with_all_mandatory_parameters.set_model_number("KR-54-FS")
    cbor_with_all_mandatory_parameters.set_hardware_version("0.1.5")
    cbor_with_all_mandatory_parameters.set_memory_size_version(102400)
    cbor_with_all_mandatory_parameters.set_utc_offset("+02:00")
    cbor_with_all_mandatory_parameters.set_device_time_zone("America/New York")
    cbor_with_all_mandatory_parameters.set_bootstrap_mode(1)
    cbor_with_all_mandatory_parameters.set_current_time(secs_since_epoch)
    #Set bootstrap objects
    cbor_with_all_mandatory_parameters.add_certificate("mbed.BootstrapServerCACert", "x509_1_ca.der")
    cbor_with_all_mandatory_parameters.add_certificate("mbed.BootstrapDeviceCert", "x509_1_ca_child.der")
    cbor_with_all_mandatory_parameters.add_ecc_private_key("mbed.BootstrapDevicePrivateKey","priv_ecc_key1.der")
    cbor_with_all_mandatory_parameters.add_certificate("mbed.FirmwareIntegrityCACert", "x509_1_ca.der")
    cbor_with_all_mandatory_parameters.add_certificate("mbed.FirmwareIntegrityCert", "x509_1_ca_child.der")
    generate_cbor_data("cbor_with_all_mandatory_parameters", out_dir, cbor_with_all_mandatory_parameters)

    #Set all mandatory field + current time (optional field)
    cbor_with_current_time = cbor_with_all_mandatory_parameters
    cbor_with_current_time.set_current_time(secs_since_epoch)
    generate_cbor_data("cbor_with_current_time", out_dir, cbor_with_current_time)
	
	#Store field to C file to be use by our unit-tests
    current_time = StoreGeneralData("current_time", secs_since_epoch)
    current_time.generate_data()

    cbor_without_endpoint_name_parameter  = CborConfiguration()
    cbor_without_endpoint_name_parameter.set_cbor_groups()
    #Set all configuration parameters
    cbor_without_endpoint_name_parameter.set_serial_number("A12FC-45")
    cbor_without_endpoint_name_parameter.set_account_id(0xA65369B6589987655264)
    #cbor_without_endpoint_name_parameter.set_endpoint_name("EP_fdsf")
    cbor_without_endpoint_name_parameter.set_bootstrap_uri("coap://bootstrap?aid.arm.com")
    cbor_without_endpoint_name_parameter.set_manf_name("Toshiba")
    cbor_without_endpoint_name_parameter.set_device_type("TEMP-SENSOR")
    cbor_without_endpoint_name_parameter.set_model_number("KR-54-FS")
    cbor_without_endpoint_name_parameter.set_hardware_version("0.1.5")
    cbor_without_endpoint_name_parameter.set_memory_size_version(102400)
    cbor_without_endpoint_name_parameter.set_current_time(secs_since_epoch)
    cbor_without_endpoint_name_parameter.set_utc_offset("+11:00")
    cbor_without_endpoint_name_parameter.set_device_time_zone("Europe/Paris")
    cbor_without_endpoint_name_parameter.set_bootstrap_mode(1)
    #Set bootstrap objects
    cbor_without_endpoint_name_parameter.add_certificate("mbed.BootstrapServerCACert", "x509_1_ca.der")
    cbor_without_endpoint_name_parameter.add_certificate("mbed.LwM2MServerCACert", "x509_1_ca.der")
    cbor_without_endpoint_name_parameter.add_certificate("mbed.BootstrapDeviceCert", "x509_1_ca_child.der")
    cbor_without_endpoint_name_parameter.add_certificate("mbed.FirmwareIntegrityCACert", "x509_1_ca.der")
    cbor_without_endpoint_name_parameter.add_certificate("mbed.FirmwareIntegrityCert", "x509_1_ca.der")
    cbor_without_endpoint_name_parameter.add_ecc_private_key("mbed.BootstrapDevicePrivateKey","priv_ecc_key1.der")
    generate_cbor_data("cbor_without_endpoint_name_parameter", out_dir, cbor_without_endpoint_name_parameter)

    cbor_without_bootstrap_mode_parameter  = CborConfiguration()
    cbor_without_bootstrap_mode_parameter.set_cbor_groups()
    #Set all configuration parameters
    cbor_without_bootstrap_mode_parameter.set_serial_number("A12FC-45")
    cbor_without_bootstrap_mode_parameter.set_account_id(0xA65369B6589987655264)
    cbor_without_bootstrap_mode_parameter.set_endpoint_name("EP_fdsf")
    cbor_without_bootstrap_mode_parameter.set_bootstrap_uri("coap://bootstrap.arm.com?aid=")
    cbor_without_bootstrap_mode_parameter.set_manf_name("Toshiba")
    cbor_without_bootstrap_mode_parameter.set_device_type("TEMP-SENSOR")
    cbor_without_bootstrap_mode_parameter.set_model_number("KR-54-FS")
    cbor_without_bootstrap_mode_parameter.set_hardware_version("0.1.5")
    cbor_without_bootstrap_mode_parameter.set_memory_size_version(102400)
    cbor_without_bootstrap_mode_parameter.set_current_time(secs_since_epoch)
    cbor_without_bootstrap_mode_parameter.set_utc_offset("-03:00")
    cbor_without_bootstrap_mode_parameter.set_device_time_zone("America/New York")
    #cbor_without_endpoint_name_parameters.set_bootstrap_mode(1)
     #Set bootstrap objects
    cbor_without_bootstrap_mode_parameter.add_certificate("mbed.BootstrapServerCACert", "x509_1_ca.der")
    cbor_without_bootstrap_mode_parameter.add_certificate("mbed.LwM2MServerCACert", "x509_1_ca.der")
    cbor_without_bootstrap_mode_parameter.add_certificate("mbed.BootstrapDeviceCert", "x509_1_ca_child.der")
    cbor_without_bootstrap_mode_parameter.add_certificate("mbed.FirmwareIntegrityCACert", "x509_1_ca.der")
    cbor_without_bootstrap_mode_parameter.add_certificate("mbed.FirmwareIntegrityCert", "x509_1_ca.der")
    
    cbor_without_bootstrap_mode_parameter.add_ecc_private_key("mbed.BootstrapDevicePrivateKey","priv_ecc_key1.der")
    generate_cbor_data("cbor_without_bootstrap_mode_parameter", out_dir, cbor_without_bootstrap_mode_parameter)

    cbor_without_dtls_private_key_parameter  = CborConfiguration()
    cbor_without_dtls_private_key_parameter.set_cbor_groups()
    #Set all configuration parameters
    cbor_without_dtls_private_key_parameter.set_serial_number("A12FC-45")
    cbor_without_dtls_private_key_parameter.set_account_id(0xA65369B6589987655264)
    cbor_without_dtls_private_key_parameter.set_endpoint_name("EP_fdsf")
    cbor_without_dtls_private_key_parameter.set_bootstrap_uri("coap://?aid=bootstrap.arm.com")
    cbor_without_dtls_private_key_parameter.set_manf_name("Toshiba")
    cbor_without_dtls_private_key_parameter.set_device_type("TEMP-SENSOR")
    cbor_without_dtls_private_key_parameter.set_model_number("KR-54-FS")
    cbor_without_dtls_private_key_parameter.set_hardware_version("0.1.5")
    cbor_without_dtls_private_key_parameter.set_memory_size_version(102400)
    cbor_without_dtls_private_key_parameter.set_current_time(secs_since_epoch)
    cbor_without_dtls_private_key_parameter.set_utc_offset("+02:00")
    cbor_without_dtls_private_key_parameter.set_device_time_zone("America/New York")
    cbor_without_dtls_private_key_parameter.set_bootstrap_mode(1)
     #Set bootstrap objects
    cbor_without_dtls_private_key_parameter.add_certificate("mbed.BootstrapServerCACert", "x509_1_ca.der")
    cbor_without_dtls_private_key_parameter.add_certificate("mbed.LwM2MServerCACert", "x509_1_ca.der")
    cbor_without_dtls_private_key_parameter.add_certificate("mbed.BootstrapDeviceCert", "x509_1_ca_child.der")
    cbor_without_dtls_private_key_parameter.add_certificate("mbed.FirmwareIntegrityCACert", "x509_1_ca.der")
    #cbor_without_dtls_private_key_parameter.add_ecc_private_key("BtsDTLSDevice","priv_ecc_key1.der")
    generate_cbor_data("cbor_without_dtls_private_key_parameter", out_dir, cbor_without_dtls_private_key_parameter)

    cbor_bootstrap_mode_without_lwm2m_ca_parameter  = CborConfiguration()
    cbor_bootstrap_mode_without_lwm2m_ca_parameter.set_cbor_groups()
    #Set all configuration parameters
    cbor_bootstrap_mode_without_lwm2m_ca_parameter.set_serial_number("A12FC-45")
    cbor_bootstrap_mode_without_lwm2m_ca_parameter.set_account_id(0xA65369B6589987655264)
    cbor_bootstrap_mode_without_lwm2m_ca_parameter.set_endpoint_name("EP_fdsf")
    cbor_bootstrap_mode_without_lwm2m_ca_parameter.set_bootstrap_uri("coap://bootstrap.arm.com&aid=")
    cbor_bootstrap_mode_without_lwm2m_ca_parameter.set_manf_name("Toshiba")
    cbor_bootstrap_mode_without_lwm2m_ca_parameter.set_device_type("TEMP-SENSOR")
    cbor_bootstrap_mode_without_lwm2m_ca_parameter.set_model_number("KR-54-FS")
    cbor_bootstrap_mode_without_lwm2m_ca_parameter.set_hardware_version("0.1.5")
    cbor_bootstrap_mode_without_lwm2m_ca_parameter.set_memory_size_version(102400)
    cbor_bootstrap_mode_without_lwm2m_ca_parameter.set_current_time(secs_since_epoch)
    cbor_bootstrap_mode_without_lwm2m_ca_parameter.set_utc_offset("+02:00")
    cbor_bootstrap_mode_without_lwm2m_ca_parameter.set_device_time_zone("UTC-05")
    cbor_bootstrap_mode_without_lwm2m_ca_parameter.set_bootstrap_mode(1)
    #Set bootstrap objects
    cbor_bootstrap_mode_without_lwm2m_ca_parameter.add_certificate("mbed.BootstrapServerCACert", "x509_1_ca.der")
    cbor_bootstrap_mode_without_lwm2m_ca_parameter.add_certificate("mbed.BootstrapDeviceCert", "x509_1_ca_child.der")
    cbor_bootstrap_mode_without_lwm2m_ca_parameter.add_certificate("mbed.FirmwareIntegrityCACert", "x509_1_ca.der")
    cbor_bootstrap_mode_without_lwm2m_ca_parameter.add_certificate("mbed.FirmwareIntegrityCert", "x509_1_ca.der")
    cbor_bootstrap_mode_without_lwm2m_ca_parameter.add_ecc_private_key("mbed.BootstrapDevicePrivateKey","priv_ecc_key1.der")
    generate_cbor_data("cbor_bootstrap_mode_without_lwm2m_ca_parameter", out_dir, cbor_bootstrap_mode_without_lwm2m_ca_parameter)

    cbor_bootstrap_false_without_lwm2m_ca_parameter  = CborConfiguration()
    cbor_bootstrap_false_without_lwm2m_ca_parameter.set_cbor_groups()
    #Set all configuration parameters
    cbor_bootstrap_false_without_lwm2m_ca_parameter.set_serial_number("A12FC-45")
    cbor_bootstrap_false_without_lwm2m_ca_parameter.set_account_id(0xA65369B6589987655264)
    cbor_bootstrap_false_without_lwm2m_ca_parameter.set_endpoint_name("EP_fdsf")
    cbor_bootstrap_false_without_lwm2m_ca_parameter.set_bootstrap_uri("coap://bootstrap&aid=.arm.com")
    cbor_bootstrap_false_without_lwm2m_ca_parameter.set_manf_name("Toshiba")
    cbor_bootstrap_false_without_lwm2m_ca_parameter.set_device_type("TEMP-SENSOR")
    cbor_bootstrap_false_without_lwm2m_ca_parameter.set_model_number("KR-54-FS")
    cbor_bootstrap_false_without_lwm2m_ca_parameter.set_hardware_version("0.1.5")
    cbor_bootstrap_false_without_lwm2m_ca_parameter.set_memory_size_version(102400)
    cbor_bootstrap_false_without_lwm2m_ca_parameter.set_current_time(secs_since_epoch)
    cbor_bootstrap_false_without_lwm2m_ca_parameter.set_utc_offset("+02:00")
    cbor_bootstrap_false_without_lwm2m_ca_parameter.set_device_time_zone("UTC-05")
    cbor_bootstrap_false_without_lwm2m_ca_parameter.set_bootstrap_mode(0)
    cbor_bootstrap_false_without_lwm2m_ca_parameter.add_ecc_private_key("mbed.BootstrapDevicePrivateKey","priv_ecc_key1.der")
    generate_cbor_data("cbor_bootstrap_false_without_lwm2m_ca_parameter", out_dir, cbor_bootstrap_false_without_lwm2m_ca_parameter)

    cbor_wrong_bootstrap_mode_parameter  = CborConfiguration()
    cbor_wrong_bootstrap_mode_parameter.set_cbor_groups()
    #Set all configuration parameters
    cbor_wrong_bootstrap_mode_parameter.set_serial_number("A12FC-45")
    cbor_wrong_bootstrap_mode_parameter.set_account_id(0xA65369B6589987655264)
    cbor_wrong_bootstrap_mode_parameter.set_endpoint_name("EP_fdsf")
    cbor_wrong_bootstrap_mode_parameter.set_bootstrap_uri("coap://lwm2m&aid=.com")
    cbor_wrong_bootstrap_mode_parameter.set_manf_name("Toshiba")
    cbor_wrong_bootstrap_mode_parameter.set_device_type("TEMP-SENSOR")
    cbor_wrong_bootstrap_mode_parameter.set_model_number("KR-54-FS")
    cbor_wrong_bootstrap_mode_parameter.set_hardware_version("0.1.5")
    cbor_wrong_bootstrap_mode_parameter.set_memory_size_version(102400)
    cbor_wrong_bootstrap_mode_parameter.set_current_time(secs_since_epoch)
    cbor_wrong_bootstrap_mode_parameter.set_utc_offset("+02:00")
    cbor_wrong_bootstrap_mode_parameter.set_device_time_zone("UTC-05")
    cbor_wrong_bootstrap_mode_parameter.set_bootstrap_mode(0)
    #Set bootstrap objects
    cbor_wrong_bootstrap_mode_parameter.add_certificate("mbed.BootstrapServerCACert", "x509_1_ca.der")
    cbor_wrong_bootstrap_mode_parameter.add_certificate("mbed.BootstrapDeviceCert", "x509_1_ca_child.der")
    cbor_wrong_bootstrap_mode_parameter.add_certificate("mbed.FirmwareIntegrityCACert", "x509_1_ca.der")
    cbor_wrong_bootstrap_mode_parameter.add_certificate("mbed.FirmwareIntegrityCert", "x509_1_ca.der")
    cbor_wrong_bootstrap_mode_parameter.add_ecc_private_key("mbed.BootstrapDevicePrivateKey","priv_ecc_key1.der")
    generate_cbor_data("cbor_wrong_bootstrap_mode_parameter", out_dir, cbor_wrong_bootstrap_mode_parameter)

    cbor_without_root_ca_parameter  = CborConfiguration()
    cbor_without_root_ca_parameter.set_cbor_groups()
    #Set all configuration parameters
    cbor_without_root_ca_parameter.set_serial_number("A12FC-45")
    cbor_without_root_ca_parameter.set_account_id(0xA65369B6589987655264)
    cbor_without_root_ca_parameter.set_endpoint_name("EP_fdsf")
    cbor_without_root_ca_parameter.set_bootstrap_uri("coap://lwm2m&aid=.com")
    cbor_without_root_ca_parameter.set_manf_name("Toshiba")
    cbor_without_root_ca_parameter.set_device_type("TEMP-SENSOR")
    cbor_without_root_ca_parameter.set_model_number("KR-54-FS")
    cbor_without_root_ca_parameter.set_hardware_version("0.1.5")
    cbor_without_root_ca_parameter.set_memory_size_version(102400)
    cbor_without_root_ca_parameter.set_current_time(secs_since_epoch)
    cbor_without_root_ca_parameter.set_utc_offset("+02:00")
    cbor_without_root_ca_parameter.set_device_time_zone("UTC-05")
    cbor_without_root_ca_parameter.set_bootstrap_mode(1)
    #Set bootstrap objects
    #cbor_without_root_ca_parameter.add_certificate("mbed.BootstrapServerCACert", "x509_1_ca.der")
    cbor_without_root_ca_parameter.add_certificate("mbed.BootstrapDeviceCert", "x509_1_ca_child.der")
    cbor_without_root_ca_parameter.add_certificate("mbed.FirmwareIntegrityCACert", "x509_1_ca.der")
    cbor_without_root_ca_parameter.add_certificate("mbed.FirmwareIntegrityCert", "x509_1_ca.der")
    cbor_without_root_ca_parameter.add_ecc_private_key("mbed.BootstrapDevicePrivateKey","priv_ecc_key1.der")
    generate_cbor_data("cbor_without_root_ca_parameter", out_dir, cbor_without_root_ca_parameter)

    cbor_wrong_bootstrap_uri_parameter  = CborConfiguration()
    cbor_wrong_bootstrap_uri_parameter.set_cbor_groups()
    #Set all configuration parameters
    cbor_wrong_bootstrap_uri_parameter.set_serial_number("A12FC-45")
    cbor_wrong_bootstrap_uri_parameter.set_account_id(0xA65369B6589987655264)
    cbor_wrong_bootstrap_uri_parameter.set_endpoint_name("EP_fdsf")
    cbor_wrong_bootstrap_uri_parameter.set_bootstrap_uri("soap://lwm2m.com")
    cbor_wrong_bootstrap_uri_parameter.set_manf_name("Toshiba")
    cbor_wrong_bootstrap_uri_parameter.set_device_type("TEMP-SENSOR")
    cbor_wrong_bootstrap_uri_parameter.set_model_number("KR-54-FS")
    cbor_wrong_bootstrap_uri_parameter.set_hardware_version("0.1.5")
    cbor_wrong_bootstrap_uri_parameter.set_memory_size_version(102400)
    cbor_wrong_bootstrap_uri_parameter.set_current_time(secs_since_epoch)
    cbor_wrong_bootstrap_uri_parameter.set_utc_offset("+02:00")
    cbor_wrong_bootstrap_uri_parameter.set_device_time_zone("UTC-05")
    cbor_wrong_bootstrap_uri_parameter.set_bootstrap_mode(1)
    #Set bootstrap objects
    cbor_wrong_bootstrap_uri_parameter.add_certificate("mbed.LwM2MServerCACert", "x509_1_ca.der")
    cbor_wrong_bootstrap_uri_parameter.add_certificate("mbed.BootstrapServerCACert", "x509_1_ca.der")
    cbor_wrong_bootstrap_uri_parameter.add_certificate("mbed.BootstrapDeviceCert", "x509_1_ca_child.der")
    cbor_wrong_bootstrap_uri_parameter.add_certificate("mbed.FirmwareIntegrityCACert", "x509_1_ca.der")
    cbor_wrong_bootstrap_uri_parameter.add_certificate("mbed.FirmwareIntegrityCert", "x509_1.der")
    cbor_wrong_bootstrap_uri_parameter.add_ecc_private_key("mbed.BootstrapDevicePrivateKey","priv_ecc_key1.der")
    generate_cbor_data("cbor_wrong_bootstrap_uri_parameter", out_dir, cbor_wrong_bootstrap_uri_parameter)


    cbor_without_serial_number_parameter  = CborConfiguration()
    cbor_without_serial_number_parameter.set_cbor_groups()
    #Set all configuration parameters
    #cbor_without_serial_number_parameter.set_serial_number("A12FC-45")
    cbor_without_serial_number_parameter.set_account_id(0xA65369B6589987655264)
    cbor_without_serial_number_parameter.set_endpoint_name("EP_fdsf")
    cbor_without_serial_number_parameter.set_bootstrap_uri("coap://lwm2m?aid=.com")
    cbor_without_serial_number_parameter.set_manf_name("Toshiba")
    cbor_without_serial_number_parameter.set_device_type("TEMP-SENSOR")
    cbor_without_serial_number_parameter.set_model_number("KR-54-FS")
    cbor_without_serial_number_parameter.set_hardware_version("0.1.5")
    cbor_without_serial_number_parameter.set_memory_size_version(102400)
    cbor_without_serial_number_parameter.set_current_time(secs_since_epoch)
    cbor_without_serial_number_parameter.set_utc_offset("+02:00")
    cbor_without_serial_number_parameter.set_device_time_zone("UTC-05")
    cbor_without_serial_number_parameter.set_bootstrap_mode(1)
    #Set bootstrap objects
    cbor_without_serial_number_parameter.add_certificate("mbed.BootstrapServerCACert", "x509_1_ca.der")
    cbor_without_serial_number_parameter.add_certificate("mbed.LwM2MServerCACert", "x509_1_ca.der")
    cbor_without_serial_number_parameter.add_certificate("mbed.BootstrapDeviceCert", "x509_1_ca_child.der")
    cbor_without_serial_number_parameter.add_ecc_private_key("mbed.BootstrapDevicePrivateKey","priv_ecc_key1.der")
    generate_cbor_data("cbor_without_serial_number_parameter", out_dir, cbor_without_serial_number_parameter)


    cbor_lwm2m_bootstrap_parameters  = CborConfiguration()
    cbor_lwm2m_bootstrap_parameters.set_cbor_groups()
    #Set all configuration parameters
    cbor_lwm2m_bootstrap_parameters.set_serial_number("A12FC-45")
    cbor_lwm2m_bootstrap_parameters.set_account_id(0xA65369B6589987655264)
    cbor_lwm2m_bootstrap_parameters.set_endpoint_name("EP_fdsf")
    #cbor_lwm2m_bootstrap_parameters.set_bootstrap_uri("coap://bootstrap.arm.com")
    cbor_lwm2m_bootstrap_parameters.set_lwm2m_uri("coaps://lwm2m.arm.com&aid=.arm.com")
    cbor_lwm2m_bootstrap_parameters.set_manf_name("Toshiba")
    cbor_lwm2m_bootstrap_parameters.set_device_type("TEMP-SENSOR")
    cbor_lwm2m_bootstrap_parameters.set_model_number("KR-54-FS")
    cbor_lwm2m_bootstrap_parameters.set_hardware_version("0.1.5")
    cbor_lwm2m_bootstrap_parameters.set_memory_size_version(102400)
    cbor_lwm2m_bootstrap_parameters.set_current_time(secs_since_epoch)
    cbor_lwm2m_bootstrap_parameters.set_utc_offset("+02:00")
    cbor_lwm2m_bootstrap_parameters.set_device_time_zone("UTC-05")
    cbor_lwm2m_bootstrap_parameters.set_bootstrap_mode(0)
    #Set bootstrap objects
    cbor_lwm2m_bootstrap_parameters.add_certificate("mbed.LwM2MServerCACert", "x509_1_ca.der")
    cbor_lwm2m_bootstrap_parameters.add_certificate("mbed.LwM2MDeviceCert", "x509_1_ca_child.der")
    cbor_lwm2m_bootstrap_parameters.add_certificate("mbed.FirmwareIntegrityCACert", "x509_1_ca.der")
    cbor_lwm2m_bootstrap_parameters.add_certificate("mbed.FirmwareIntegrityCert", "x509_1_ca.der")
    cbor_lwm2m_bootstrap_parameters.add_ecc_private_key("mbed.LwM2MDevicePrivateKey","priv_ecc_key1.der")
    cbor_lwm2m_bootstrap_parameters.add_ecc_private_key("mbed.BootstrapDevicePrivateKey","priv_ecc_key1.der")
    generate_cbor_data("cbor_lwm2m_bootstrap_parameters", out_dir, cbor_lwm2m_bootstrap_parameters)

	#blob with self signed CA certificate
    cbor_self_signed_ca  = CborConfiguration()
    cbor_self_signed_ca.set_cbor_groups()
    #Set all configuration parameters
    cbor_self_signed_ca.set_serial_number("A12FC-45")
    cbor_self_signed_ca.set_account_id(0xA65369B6589987655264)
    cbor_self_signed_ca.set_endpoint_name("Device")
    cbor_self_signed_ca.set_bootstrap_uri("coaps://?aid=bootstrap.arm.com")
    cbor_self_signed_ca.set_manf_name("Toshiba")
    cbor_self_signed_ca.set_device_type("TEMP-SENSOR")
    cbor_self_signed_ca.set_model_number("KR-54-FS")
    cbor_self_signed_ca.set_hardware_version("0.1.5")
    cbor_self_signed_ca.set_memory_size_version(102400)
    cbor_self_signed_ca.set_current_time(secs_since_epoch)
    cbor_self_signed_ca.set_utc_offset("+02:00")
    cbor_self_signed_ca.set_device_time_zone("UTC-05")
    cbor_self_signed_ca.set_bootstrap_mode(1)
    #Set bootstrap objects
    cbor_self_signed_ca.add_certificate("mbed.BootstrapServerCACert", "x509_1_ca.der")
    cbor_self_signed_ca.add_certificate("mbed.LwM2MServerCACert", "x509_1_ca.der")
    cbor_self_signed_ca.add_certificate("mbed.BootstrapDeviceCert", "x509_1_ca.der")
    cbor_self_signed_ca.add_ecc_private_key("mbed.BootstrapDevicePrivateKey","priv_ecc_key1.der")
    generate_cbor_data("cbor_self_signed_ca", out_dir, cbor_self_signed_ca)

    cbor_lwm2m_self_signed_ca  = CborConfiguration()
    cbor_lwm2m_self_signed_ca.set_cbor_groups()
    #Set all configuration parameters
    cbor_lwm2m_self_signed_ca.set_serial_number("A12FC-45")
    cbor_lwm2m_self_signed_ca.set_account_id(0xA65369B6589987655264)
    cbor_lwm2m_self_signed_ca.set_endpoint_name("Device")
    #cbor_lwm2m_self_signed_ca.set_bootstrap_uri("coap://bootstrap.arm.com")
    cbor_lwm2m_self_signed_ca.set_lwm2m_uri("coap://lwm2m.arm.com?aid=.arm.com")
    cbor_lwm2m_self_signed_ca.set_manf_name("Toshiba")
    cbor_lwm2m_self_signed_ca.set_device_type("TEMP-SENSOR")
    cbor_lwm2m_self_signed_ca.set_model_number("KR-54-FS")
    cbor_lwm2m_self_signed_ca.set_hardware_version("0.1.5")
    cbor_lwm2m_self_signed_ca.set_memory_size_version(102400)
    cbor_lwm2m_self_signed_ca.set_current_time(secs_since_epoch)
    cbor_lwm2m_self_signed_ca.set_utc_offset("+02:00")
    cbor_lwm2m_self_signed_ca.set_device_time_zone("UTC-05")
    cbor_lwm2m_self_signed_ca.set_bootstrap_mode(0)
    #Set bootstrap objects
    cbor_lwm2m_self_signed_ca.add_certificate("mbed.LwM2MServerCACert", "x509_1_ca.der")
    cbor_lwm2m_self_signed_ca.add_certificate("mbed.LwM2MDeviceCert", "x509_1_ca.der")
    cbor_lwm2m_self_signed_ca.add_certificate("mbed.FirmwareIntegrityCACert", "x509_1_ca.der")
    cbor_lwm2m_self_signed_ca.add_certificate("mbed.FirmwareIntegrityCert", "x509_1_ca.der")
    cbor_lwm2m_self_signed_ca.add_ecc_private_key("mbed.LwM2MDevicePrivateKey","priv_ecc_key1.der")
    generate_cbor_data("cbor_lwm2m_self_signed_ca", out_dir, cbor_lwm2m_self_signed_ca)


    cbor_lwm2m_without_time  = CborConfiguration()
    cbor_lwm2m_without_time.set_cbor_groups()
    #Set all configuration parameters
    cbor_lwm2m_without_time.set_serial_number("A12FC-45")
    cbor_lwm2m_without_time.set_endpoint_name("EP_fdsf")
    cbor_lwm2m_without_time.set_manf_name("Toshiba")
    cbor_lwm2m_without_time.set_device_type("TEMP-SENSOR")
    cbor_lwm2m_without_time.set_model_number("KR-54-FS")
    cbor_lwm2m_without_time.set_lwm2m_uri("coap://lwm2m?aid=.arm.com")
    cbor_lwm2m_without_time.set_hardware_version("0.1.5")
    cbor_lwm2m_without_time.set_memory_size_version(102400)
    cbor_lwm2m_without_time.set_utc_offset("+02:00")
    cbor_lwm2m_without_time.set_device_time_zone("UTC-05")
    cbor_lwm2m_without_time.set_bootstrap_mode(0)
    #Set bootstrap objects
    cbor_lwm2m_without_time.add_certificate("mbed.LwM2MServerCACert", "x509_1_ca.der")
    cbor_lwm2m_without_time.add_certificate("mbed.LwM2MDeviceCert", "x509_1_ca_child.der")
    cbor_lwm2m_without_time.add_certificate("mbed.FirmwareIntegrityCACert", "x509_1_ca.der")
    cbor_lwm2m_without_time.add_certificate("mbed.FirmwareIntegrityCert", "x509_1_ca.der")
    cbor_lwm2m_without_time.add_ecc_private_key("mbed.LwM2MDevicePrivateKey","priv_ecc_key1.der")
    generate_cbor_data("cbor_lwm2m_without_time", out_dir, cbor_lwm2m_without_time)

    cbor_with_empty_endpoint  = CborConfiguration()
    cbor_with_empty_endpoint.set_cbor_groups()
    #Set all configuration parameters
    cbor_with_empty_endpoint.set_serial_number("A12FC-45")
    cbor_with_empty_endpoint.set_endpoint_name("")
    cbor_with_empty_endpoint.set_bootstrap_uri("coaps://?aid=bootstrap.arm.com")
    cbor_with_empty_endpoint.set_manf_name("Toshiba")
    cbor_with_empty_endpoint.set_device_type("TEMP-SENSOR")
    cbor_with_empty_endpoint.set_model_number("KR-54-FS")
    cbor_with_empty_endpoint.set_hardware_version("0.1.5")
    cbor_with_empty_endpoint.set_memory_size_version(102400)
    cbor_with_empty_endpoint.set_utc_offset("+02:00")
    cbor_with_empty_endpoint.set_device_time_zone("UTC-05")
    cbor_with_empty_endpoint.set_bootstrap_mode(1)
    #Set bootstrap objects
    cbor_with_empty_endpoint.add_certificate("mbed.BootstrapServerCACert", "x509_1_ca.der")
    cbor_with_empty_endpoint.add_certificate("mbed.BootstrapDeviceCert", "x509_1_ca_child.der")
    cbor_with_empty_endpoint.add_ecc_private_key("mbed.BootstrapDevicePrivateKey","priv_ecc_key1.der")
    generate_cbor_data("cbor_with_empty_endpoint", out_dir, cbor_with_empty_endpoint)


    cbor_uri_without_aid  = CborConfiguration()
    cbor_uri_without_aid.set_cbor_groups()
    #Set all configuration parameters
    cbor_uri_without_aid.set_serial_number("A12FC-45")
    cbor_uri_without_aid.set_endpoint_name("EP_fdsf")
    cbor_uri_without_aid.set_bootstrap_uri("coap://bootstrap.arm.com")
    cbor_uri_without_aid.set_manf_name("Toshiba")
    cbor_uri_without_aid.set_device_type("TEMP-SENSOR")
    cbor_uri_without_aid.set_model_number("KR-54-FS")
    cbor_uri_without_aid.set_current_time(secs_since_epoch)
    cbor_uri_without_aid.set_hardware_version("0.1.5")
    cbor_uri_without_aid.set_memory_size_version(102400)
    cbor_uri_without_aid.set_utc_offset("+02:00")
    cbor_uri_without_aid.set_device_time_zone("UTC-05")
    cbor_uri_without_aid.set_bootstrap_mode(1)
    #Set bootstrap objects
    cbor_uri_without_aid.add_certificate("mbed.BootstrapServerCACert", "x509_1_ca.der")
    cbor_uri_without_aid.add_certificate("mbed.BootstrapDeviceCert", "x509_1_ca_child.der")
    cbor_uri_without_aid.add_ecc_private_key("mbed.BootstrapDevicePrivateKey","priv_ecc_key1.der")
    generate_cbor_data("cbor_uri_without_aid", out_dir, cbor_uri_without_aid)

    cbor_uri_wrong_prefix_location  = CborConfiguration()
    cbor_uri_wrong_prefix_location.set_cbor_groups()
    #Set all configuration parameters
    cbor_uri_wrong_prefix_location.set_serial_number("A12FC-45")
    cbor_uri_wrong_prefix_location.set_endpoint_name("EP_fdsf")
    cbor_uri_wrong_prefix_location.set_bootstrap_uri("bootstrap?aid.arm.coap://com")
    cbor_uri_wrong_prefix_location.set_lwm2m_uri("coaps://lwm2m.arm.com&aid=1234")
    cbor_uri_wrong_prefix_location.set_manf_name("Toshiba")
    cbor_uri_wrong_prefix_location.set_device_type("TEMP-SENSOR")
    cbor_uri_wrong_prefix_location.set_model_number("KR-54-FS")
    cbor_uri_wrong_prefix_location.set_hardware_version("0.1.5")
    cbor_uri_wrong_prefix_location.set_memory_size_version(102400)
    cbor_uri_wrong_prefix_location.set_device_time_zone("UTC-05")
    cbor_uri_wrong_prefix_location.set_utc_offset("+02:00")
    cbor_uri_wrong_prefix_location.set_current_time(secs_since_epoch)
    cbor_uri_wrong_prefix_location.set_bootstrap_mode(1)
    #Set bootstrap objects
    cbor_uri_wrong_prefix_location.add_certificate("mbed.BootstrapServerCACert", "x509_1_ca.der")
    cbor_uri_wrong_prefix_location.add_certificate("mbed.BootstrapDeviceCert", "x509_1_ca_child.der")
    cbor_uri_wrong_prefix_location.add_ecc_private_key("mbed.BootstrapDevicePrivateKey","priv_ecc_key1.der")
    generate_cbor_data("cbor_uri_wrong_prefix_location", out_dir, cbor_uri_wrong_prefix_location)


    cbor_empty_uri  = CborConfiguration()
    cbor_empty_uri.set_cbor_groups()
    #Set all configuration parameters
    cbor_empty_uri.set_serial_number("A12FC-45")
    cbor_empty_uri.set_endpoint_name("EP_fdsf")
    cbor_empty_uri.set_bootstrap_uri("")
    cbor_empty_uri.set_lwm2m_uri("coaps://lwm2m.arm.com&aid=1234")
    cbor_empty_uri.set_manf_name("Toshiba")
    cbor_empty_uri.set_device_type("TEMP-SENSOR")
    cbor_empty_uri.set_model_number("KR-54-FS")
    cbor_empty_uri.set_hardware_version("0.1.5")
    cbor_empty_uri.set_memory_size_version(102400)
    cbor_empty_uri.set_device_time_zone("UTC-05")
    cbor_empty_uri.set_current_time(secs_since_epoch)
    cbor_empty_uri.set_utc_offset("+02:00")
    cbor_empty_uri.set_bootstrap_mode(1)
    #Set bootstrap objects
    cbor_empty_uri.add_certificate("mbed.BootstrapServerCACert", "x509_1_ca.der")
    cbor_empty_uri.add_certificate("mbed.BootstrapDeviceCert", "x509_1_ca_child.der")
    cbor_empty_uri.add_ecc_private_key("mbed.BootstrapDevicePrivateKey","priv_ecc_key1.der")
    generate_cbor_data("cbor_empty_uri", out_dir, cbor_empty_uri)


    cbor_without_time_zone  = CborConfiguration()
    cbor_without_time_zone.set_cbor_groups()
    #Set all configuration parameters
    cbor_without_time_zone.set_serial_number("A12FC-45")
    cbor_without_time_zone.set_endpoint_name("EP_fdsf")
    cbor_without_time_zone.set_bootstrap_uri("coap://?aid=bootstrap.arm.com")
    cbor_without_time_zone.set_manf_name("Toshiba")
    cbor_without_time_zone.set_device_type("TEMP-SENSOR")
    cbor_without_time_zone.set_model_number("KR-54-FS")
    cbor_without_time_zone.set_hardware_version("0.1.5")
    cbor_without_time_zone.set_memory_size_version(102400)
    cbor_without_time_zone.set_bootstrap_mode(1)
    cbor_without_time_zone.set_utc_offset("+02:00")
    cbor_without_time_zone.set_current_time(secs_since_epoch)
    #Set bootstrap objects
    cbor_without_time_zone.add_certificate("mbed.BootstrapServerCACert", "x509_1_ca.der")
    cbor_without_time_zone.add_certificate("mbed.BootstrapDeviceCert", "x509_1_ca_child.der")
    cbor_without_time_zone.add_ecc_private_key("mbed.BootstrapDevicePrivateKey","priv_ecc_key1.der")
    generate_cbor_data("cbor_without_time_zone", out_dir, cbor_without_time_zone)




    cbor_without_utc_offset  = CborConfiguration()
    cbor_without_utc_offset.set_cbor_groups()
    #Set all configuration parameters
    cbor_without_utc_offset.set_serial_number("A12FC-45")
    cbor_without_utc_offset.set_endpoint_name("EP_fdsf")
    cbor_without_utc_offset.set_bootstrap_uri("coap://?aid=bootstrap.arm.com")
    cbor_without_utc_offset.set_manf_name("Toshiba")
    cbor_without_utc_offset.set_device_type("TEMP-SENSOR")
    cbor_without_utc_offset.set_model_number("KR-54-FS")
    cbor_without_utc_offset.set_hardware_version("0.1.5")
    cbor_without_utc_offset.set_memory_size_version(102400)
    cbor_without_utc_offset.set_device_time_zone("UTC-05")
    cbor_without_utc_offset.set_current_time(secs_since_epoch)

    cbor_without_utc_offset.set_bootstrap_mode(1)
    #Set bootstrap objects
    cbor_without_utc_offset.add_certificate("mbed.BootstrapServerCACert", "x509_1_ca.der")
    cbor_without_utc_offset.add_certificate("mbed.BootstrapDeviceCert", "x509_1_ca_child.der")
    cbor_without_utc_offset.add_ecc_private_key("mbed.BootstrapDevicePrivateKey","priv_ecc_key1.der")
    generate_cbor_data("cbor_without_utc_offset", out_dir, cbor_without_utc_offset)



    cbor_with_wrong_utc_offset  = CborConfiguration()
    cbor_with_wrong_utc_offset.set_cbor_groups()
    #Set all configuration parameters
    cbor_with_wrong_utc_offset.set_serial_number("A12FC-45")
    cbor_with_wrong_utc_offset.set_endpoint_name("EP_fdsf")
    cbor_with_wrong_utc_offset.set_bootstrap_uri("coap://?aid=bootstrap.arm.com")
    cbor_with_wrong_utc_offset.set_manf_name("Toshiba")
    cbor_with_wrong_utc_offset.set_device_type("TEMP-SENSOR")
    cbor_with_wrong_utc_offset.set_model_number("KR-54-FS")
    cbor_with_wrong_utc_offset.set_hardware_version("0.1.5")
    cbor_with_wrong_utc_offset.set_memory_size_version(102400)
    cbor_with_wrong_utc_offset.set_device_time_zone("UTC-05")
    cbor_with_wrong_utc_offset.set_current_time(secs_since_epoch)
    cbor_with_wrong_utc_offset.set_utc_offset("+9k:00")
    
    cbor_with_wrong_utc_offset.set_bootstrap_mode(1)
    #Set bootstrap objects
    cbor_with_wrong_utc_offset.add_certificate("mbed.BootstrapServerCACert", "x509_1_ca.der")
    cbor_with_wrong_utc_offset.add_certificate("mbed.BootstrapDeviceCert", "x509_1_ca_child.der")
    cbor_with_wrong_utc_offset.add_ecc_private_key("mbed.BootstrapDevicePrivateKey","priv_ecc_key1.der")
    generate_cbor_data("cbor_with_wrong_utc_offset", out_dir, cbor_with_wrong_utc_offset)
    

    cbor_with_wrong_utc_offset_sign  = CborConfiguration()
    cbor_with_wrong_utc_offset_sign.set_cbor_groups()
    #Set all configuration parameters
    cbor_with_wrong_utc_offset_sign.set_serial_number("A12FC-45")
    cbor_with_wrong_utc_offset_sign.set_endpoint_name("EP_fdsf")
    cbor_with_wrong_utc_offset_sign.set_bootstrap_uri("coap://?aid=bootstrap.arm.com")
    cbor_with_wrong_utc_offset_sign.set_manf_name("Toshiba")
    cbor_with_wrong_utc_offset_sign.set_device_type("TEMP-SENSOR")
    cbor_with_wrong_utc_offset_sign.set_model_number("KR-54-FS")
    cbor_with_wrong_utc_offset_sign.set_hardware_version("0.1.5")
    cbor_with_wrong_utc_offset_sign.set_memory_size_version(102400)
    cbor_with_wrong_utc_offset_sign.set_device_time_zone("UTC-05")
    cbor_with_wrong_utc_offset_sign.set_current_time(secs_since_epoch)
    cbor_with_wrong_utc_offset_sign.set_utc_offset("#92:00")

    cbor_with_wrong_utc_offset_sign.set_bootstrap_mode(1)
    #Set bootstrap objects
    cbor_with_wrong_utc_offset_sign.add_certificate("mbed.BootstrapServerCACert", "x509_1_ca.der")
    cbor_with_wrong_utc_offset_sign.add_certificate("mbed.BootstrapDeviceCert", "x509_1_ca_child.der")
    cbor_with_wrong_utc_offset_sign.add_ecc_private_key("mbed.BootstrapDevicePrivateKey","priv_ecc_key1.der")
    generate_cbor_data("cbor_with_wrong_utc_offset_sign", out_dir, cbor_with_wrong_utc_offset_sign)






    cbor_with_wrong_value_of_bootstrap_mode  = CborConfiguration()
    cbor_with_wrong_value_of_bootstrap_mode.set_cbor_groups()
    #Set all configuration parameters
    cbor_with_wrong_value_of_bootstrap_mode.set_serial_number("A12FC-45")
    cbor_with_wrong_value_of_bootstrap_mode.set_endpoint_name("EP_fdsf")
    cbor_with_wrong_value_of_bootstrap_mode.set_bootstrap_uri("coap://?aid=bootstrap.arm.com")
    cbor_with_wrong_value_of_bootstrap_mode.set_manf_name("Toshiba")
    cbor_with_wrong_value_of_bootstrap_mode.set_device_type("TEMP-SENSOR")
    cbor_with_wrong_value_of_bootstrap_mode.set_model_number("KR-54-FS")
    cbor_with_wrong_value_of_bootstrap_mode.set_hardware_version("0.1.5")
    cbor_with_wrong_value_of_bootstrap_mode.set_memory_size_version(102400)
    cbor_with_wrong_value_of_bootstrap_mode.set_utc_offset("+02:00")
    cbor_with_wrong_value_of_bootstrap_mode.set_device_time_zone("America/New York")
    cbor_with_wrong_value_of_bootstrap_mode.set_bootstrap_mode(6)
    #Set bootstrap objects
    cbor_with_wrong_value_of_bootstrap_mode.add_certificate("mbed.BootstrapServerCACert", "x509_1_ca.der")
    cbor_with_wrong_value_of_bootstrap_mode.add_certificate("mbed.BootstrapDeviceCert", "x509_1_ca_child.der")
    cbor_with_wrong_value_of_bootstrap_mode.add_ecc_private_key("mbed.BootstrapDevicePrivateKey","priv_ecc_key1.der")
    generate_cbor_data("cbor_with_wrong_value_of_bootstrap_mode", out_dir, cbor_with_wrong_value_of_bootstrap_mode)


    cbor_with_empty_manf_name  = CborConfiguration()
    cbor_with_empty_manf_name.set_cbor_groups()
    #Set all configuration parameters
    cbor_with_empty_manf_name.set_serial_number("A12FC-45")
    cbor_with_empty_manf_name.set_endpoint_name("EP_fdsf")
    cbor_with_empty_manf_name.set_bootstrap_uri("coap://?aid=bootstrap.arm.com")
    cbor_with_empty_manf_name.set_manf_name("")
    cbor_with_empty_manf_name.set_device_type("TEMP-SENSOR")
    cbor_with_empty_manf_name.set_model_number("KR-54-FS")
    cbor_with_empty_manf_name.set_hardware_version("0.1.5")
    cbor_with_empty_manf_name.set_memory_size_version(102400)
    cbor_with_empty_manf_name.set_utc_offset("+02:00")
    cbor_with_empty_manf_name.set_device_time_zone("America/New York")
    cbor_with_empty_manf_name.set_bootstrap_mode(1)
    #Set bootstrap objects
    cbor_with_empty_manf_name.add_certificate("mbed.BootstrapServerCACert", "x509_1_ca.der")
    cbor_with_empty_manf_name.add_certificate("mbed.BootstrapDeviceCert", "x509_1_ca_child.der")
    cbor_with_empty_manf_name.add_ecc_private_key("mbed.BootstrapDevicePrivateKey","priv_ecc_key1.der")
    generate_cbor_data("cbor_with_empty_manf_name", out_dir, cbor_with_empty_manf_name)

    cbor_with_wrong_integrity_chain_parameters  = CborConfiguration()
    cbor_with_wrong_integrity_chain_parameters.set_cbor_groups()
    #Set all configuration parameters
    cbor_with_wrong_integrity_chain_parameters.set_serial_number("A12FC-45")
    cbor_with_wrong_integrity_chain_parameters.set_endpoint_name("EP_fdsf")
    cbor_with_wrong_integrity_chain_parameters.set_bootstrap_uri("coap://?aid=bootstrap.arm.com")
    cbor_with_wrong_integrity_chain_parameters.set_manf_name("Toshiba")
    cbor_with_wrong_integrity_chain_parameters.set_device_type("TEMP-SENSOR")
    cbor_with_wrong_integrity_chain_parameters.set_model_number("KR-54-FS")
    cbor_with_wrong_integrity_chain_parameters.set_hardware_version("0.1.5")
    cbor_with_wrong_integrity_chain_parameters.set_memory_size_version(102400)
    cbor_with_wrong_integrity_chain_parameters.set_utc_offset("+02:00")
    cbor_with_wrong_integrity_chain_parameters.set_device_time_zone("America/New York")
    cbor_with_wrong_integrity_chain_parameters.set_bootstrap_mode(1)
    cbor_with_wrong_integrity_chain_parameters.set_current_time(secs_since_epoch)
    #Set bootstrap objects
    cbor_with_wrong_integrity_chain_parameters.add_certificate("mbed.BootstrapServerCACert", "x509_1_ca.der")
    cbor_with_wrong_integrity_chain_parameters.add_certificate("mbed.BootstrapDeviceCert", "x509_1_ca_child.der")
    cbor_with_wrong_integrity_chain_parameters.add_ecc_private_key("mbed.BootstrapDevicePrivateKey","priv_ecc_key1.der")
    cbor_with_wrong_integrity_chain_parameters.add_certificate("mbed.FirmwareIntegrityCACert", "x509_1_ca.der")
    cbor_with_wrong_integrity_chain_parameters.add_certificate("mbed.FirmwareIntegrityCert", "x509_2_ca_child.der")
    generate_cbor_data("cbor_with_wrong_integrity_chain_parameters", out_dir, cbor_with_wrong_integrity_chain_parameters)

    cbor_with_integ_cert_expiration_less_than_10 = CborConfiguration()
    cbor_with_integ_cert_expiration_less_than_10.set_cbor_groups()
    #Set all configuration parameters
    cbor_with_integ_cert_expiration_less_than_10.set_serial_number("A12FC-45")
    cbor_with_integ_cert_expiration_less_than_10.set_endpoint_name("EP_fdsf")
    cbor_with_integ_cert_expiration_less_than_10.set_bootstrap_uri("coap://?aid=bootstrap.arm.com")
    cbor_with_integ_cert_expiration_less_than_10.set_manf_name("Toshiba")
    cbor_with_integ_cert_expiration_less_than_10.set_device_type("TEMP-SENSOR")
    cbor_with_integ_cert_expiration_less_than_10.set_model_number("KR-54-FS")
    cbor_with_integ_cert_expiration_less_than_10.set_hardware_version("0.1.5")
    cbor_with_integ_cert_expiration_less_than_10.set_memory_size_version(102400)
    cbor_with_integ_cert_expiration_less_than_10.set_utc_offset("+02:00")
    cbor_with_integ_cert_expiration_less_than_10.set_device_time_zone("America/New York")
    cbor_with_integ_cert_expiration_less_than_10.set_bootstrap_mode(1)
    cbor_with_integ_cert_expiration_less_than_10.set_current_time(secs_since_epoch)
    #Set bootstrap objects
    cbor_with_integ_cert_expiration_less_than_10.add_certificate("mbed.BootstrapServerCACert", "x509_1_ca.der")
    cbor_with_integ_cert_expiration_less_than_10.add_certificate("mbed.BootstrapDeviceCert", "x509_1_ca_child.der")
    cbor_with_integ_cert_expiration_less_than_10.add_ecc_private_key("mbed.BootstrapDevicePrivateKey","priv_ecc_key1.der")
    cbor_with_integ_cert_expiration_less_than_10.add_certificate("mbed.FirmwareIntegrityCACert", "x509_3_ca.der")
    cbor_with_integ_cert_expiration_less_than_10.add_certificate("mbed.FirmwareIntegrityCert", "x509_3_ca_child.der")
    generate_cbor_data("cbor_with_integ_cert_expiration_less_than_10", out_dir, cbor_with_integ_cert_expiration_less_than_10)

    cbor_lwm2m_with_wrong_cn  = CborConfiguration()
    cbor_lwm2m_with_wrong_cn.set_cbor_groups()
    #Set all configuration parameters
    cbor_lwm2m_with_wrong_cn.set_serial_number("A12FC-45")
    cbor_lwm2m_with_wrong_cn.set_account_id(0xA65369B6589987655264)
    cbor_lwm2m_with_wrong_cn.set_endpoint_name("EP_fdsf")
    #cbor_lwm2m_with_wrong_cn.set_bootstrap_uri("coap://bootstrap.arm.com")
    cbor_lwm2m_with_wrong_cn.set_lwm2m_uri("coap://lwm2m.arm.com?aid=")
    cbor_lwm2m_with_wrong_cn.set_manf_name("Toshiba")
    cbor_lwm2m_with_wrong_cn.set_device_type("TEMP-SENSOR")
    cbor_lwm2m_with_wrong_cn.set_model_number("KR-54-FS")
    cbor_lwm2m_with_wrong_cn.set_hardware_version("0.1.5")
    cbor_lwm2m_with_wrong_cn.set_memory_size_version(102400)
    cbor_lwm2m_with_wrong_cn.set_current_time(secs_since_epoch)
    cbor_lwm2m_with_wrong_cn.set_utc_offset("+02:00")
    cbor_lwm2m_with_wrong_cn.set_device_time_zone("UTC-05")
    cbor_lwm2m_with_wrong_cn.set_bootstrap_mode(0)
    #Set bootstrap objects
    cbor_lwm2m_with_wrong_cn.add_certificate("mbed.LwM2MServerCACert", "x509_1_ca.der")
    cbor_lwm2m_with_wrong_cn.add_certificate("mbed.LwM2MDeviceCert", "x509_1_ca.der")
    cbor_lwm2m_with_wrong_cn.add_certificate("mbed.FirmwareIntegrityCACert", "x509_1_ca.der")
    cbor_lwm2m_with_wrong_cn.add_certificate("mbed.FirmwareIntegrityCert", "x509_1_ca.der")
    cbor_lwm2m_with_wrong_cn.add_ecc_private_key("mbed.LwM2MDevicePrivateKey","priv_ecc_key1.der")
    generate_cbor_data("cbor_lwm2m_with_wrong_cn", out_dir, cbor_lwm2m_with_wrong_cn)

    cbor_wrong_key_format = CborConfiguration()
    cbor_wrong_key_format.set_cbor_groups()
    cbor_wrong_key_format.add_ecc_private_key("mbed.WrongKeyFormat", "priv_ecc_key1.der", "kuku")
    generate_cbor_data("cbor_wrong_key_format", out_dir, cbor_wrong_key_format)

    cbor_certificate_null_format = CborConfiguration()
    cbor_certificate_null_format.set_cbor_groups()
    cbor_certificate_null_format.add_certificate("mbed.CertificateNullFormat",  "x509_1_ca.der", '\0')
    generate_cbor_data("cbor_certificate_null_format", out_dir, cbor_certificate_null_format)

    cbor_certificate_pem_format = CborConfiguration()
    cbor_certificate_pem_format.set_cbor_groups()
    cbor_certificate_null_format.add_certificate("mbed.CertificatePemFormat", "priv_ecc_key2.der", 'pem')
    generate_cbor_data("cbor_certificate_pem_format", out_dir, cbor_certificate_pem_format)

    #----------------------------------------------------------------------------
    #-----------     Certificate creatred with python                ------------
    #----------------------------------------------------------------------------

     #CBOR with partial parameters
    cbor_partial_parameters_1  = CborConfiguration()
    cbor_partial_parameters_1.set_cbor_groups()
    #Set all configuration parameters
    cbor_partial_parameters_1.set_serial_number("A12FC-45")
    cbor_partial_parameters_1.set_endpoint_name("EP_fdsf")
    cbor_partial_parameters_1.set_bootstrap_uri("coap://?aid=bootstrap.arm.com")
    cbor_partial_parameters_1.set_manf_name("Toshiba")
    cbor_partial_parameters_1.set_device_type("TEMP-SENSOR")
    cbor_partial_parameters_1.set_model_number("KR-54-FS")
    cbor_partial_parameters_1.set_hardware_version("0.1.5")
    cbor_partial_parameters_1.set_memory_size_version(102400)
    cbor_partial_parameters_1.set_utc_offset("+02:00")
    cbor_partial_parameters_1.set_device_time_zone("America/New York")
    cbor_partial_parameters_1.set_current_time(secs_since_epoch)
    cbor_partial_parameters_1.set_bootstrap_mode(1)
    #Set bootstrap objects
    cbor_partial_parameters_1.add_certificate("mbed.BootstrapServerCACert","x509_1_ch_inv_md.der")
    #cbor_partial_parameters_1.add_certificate("mbed.BootstrapDeviceCert", "x509_pth_ca_child.der")
    cbor_partial_parameters_1.add_ecc_private_key("mbed.BootstrapDevicePrivateKey","priv_ecc_key1.der")
    #cbor_partial_parameters_1.add_certificate("mbed.FirmwareIntegrityCACert", "x509_pth_ca_1.der")
    #cbor_partial_parameters_1.add_certificate("mbed.FirmwareIntegrityCert", "x509_pth_child_1.der")
    generate_cbor_data("cbor_partial_parameters_1", out_dir, cbor_partial_parameters_1)

    cbor_partial_parameters_2  = CborConfiguration()
    cbor_partial_parameters_2.set_cbor_groups()
    cbor_partial_parameters_2.set_serial_number("A12FC-45")
    cbor_partial_parameters_2.set_endpoint_name("EP_fdsf")
    cbor_partial_parameters_2.set_bootstrap_uri("coap://?aid=bootstrap.arm.com")
    cbor_partial_parameters_2.set_manf_name("Toshiba")
    cbor_partial_parameters_2.set_device_type("TEMP-SENSOR")
    cbor_partial_parameters_2.set_model_number("KR-54-FS")
    cbor_partial_parameters_2.set_hardware_version("0.1.5")
    cbor_partial_parameters_2.set_memory_size_version(102400)
    cbor_partial_parameters_2.set_utc_offset("+02:00")
    cbor_partial_parameters_2.set_device_time_zone("America/New York")
    cbor_partial_parameters_2.set_current_time(secs_since_epoch)
    cbor_partial_parameters_2.set_bootstrap_mode(1)
    cbor_partial_parameters_2.add_certificate("mbed.BootstrapServerCACert","x509_pth_ca.der")
    generate_cbor_data("cbor_partial_parameters_2", out_dir, cbor_partial_parameters_2)

    cbor_partial_parameters_3  = CborConfiguration()
    cbor_partial_parameters_3.set_cbor_groups()
    cbor_partial_parameters_3.add_certificate("mbed.BootstrapDeviceCert", "x509_pth_ca_child.der")
    generate_cbor_data("cbor_partial_parameters_3", out_dir, cbor_partial_parameters_3)

    cbor_partial_parameters_4  = CborConfiguration()
    cbor_partial_parameters_4.set_cbor_groups()
    cbor_partial_parameters_4.add_certificate("mbed.FirmwareIntegrityCACert", "x509_pth_ca_1.der")
    cbor_partial_parameters_4.add_certificate("mbed.FirmwareIntegrityCert", "x509_pth_child_1.der")
    generate_cbor_data("cbor_partial_parameters_4", out_dir, cbor_partial_parameters_4)

    #CBOR with all mandatory parameters
    cbor_python_certificates  = CborConfiguration()
    cbor_python_certificates.set_cbor_groups()
    #Set all configuration parameters
    cbor_python_certificates.set_serial_number("A12FC-45")
    cbor_python_certificates.set_endpoint_name("EP_fdsf")
    cbor_python_certificates.set_bootstrap_uri("coap://?aid=bootstrap.arm.com")
    cbor_python_certificates.set_manf_name("Toshiba")
    cbor_python_certificates.set_device_type("TEMP-SENSOR")
    cbor_python_certificates.set_model_number("KR-54-FS")
    cbor_python_certificates.set_hardware_version("0.1.5")
    cbor_python_certificates.set_memory_size_version(102400)
    cbor_python_certificates.set_utc_offset("+02:00")
    cbor_python_certificates.set_device_time_zone("America/New York")
    cbor_python_certificates.set_current_time(secs_since_epoch)
    cbor_python_certificates.set_bootstrap_mode(1)
    #Set bootstrap objects
    cbor_python_certificates.add_certificate("mbed.BootstrapServerCACert", "x509_pth_ca.der")
    cbor_python_certificates.add_certificate("mbed.BootstrapDeviceCert", "x509_pth_ca_child.der")
    cbor_python_certificates.add_ecc_private_key("mbed.BootstrapDevicePrivateKey","priv_ecc_key1.der")
    cbor_python_certificates.add_certificate("mbed.FirmwareIntegrityCACert", "x509_pth_ca_1.der")
    cbor_python_certificates.add_certificate("mbed.FirmwareIntegrityCert", "x509_pth_child_1.der")
    generate_cbor_data("cbor_python_certificates", out_dir, cbor_python_certificates)

    #CBOR with mbed.FirmwareIntegrityCert expired certificate
    cbor_python_future_firmw_intg_certificates  = CborConfiguration()
    cbor_python_future_firmw_intg_certificates.set_cbor_groups()
    #Set all configuration parameters
    cbor_python_future_firmw_intg_certificates.set_serial_number("A12FC-45")
    cbor_python_future_firmw_intg_certificates.set_endpoint_name("EP_fdsf")
    cbor_python_future_firmw_intg_certificates.set_bootstrap_uri("coap://?aid=bootstrap.arm.com")
    cbor_python_future_firmw_intg_certificates.set_manf_name("Toshiba")
    cbor_python_future_firmw_intg_certificates.set_device_type("TEMP-SENSOR")
    cbor_python_future_firmw_intg_certificates.set_model_number("KR-54-FS")
    cbor_python_future_firmw_intg_certificates.set_hardware_version("0.1.5")
    cbor_python_future_firmw_intg_certificates.set_memory_size_version(102400)
    cbor_python_future_firmw_intg_certificates.set_utc_offset("+02:00")
    cbor_python_future_firmw_intg_certificates.set_device_time_zone("America/New York")
    cbor_python_future_firmw_intg_certificates.set_current_time(secs_since_epoch)
    cbor_python_future_firmw_intg_certificates.set_bootstrap_mode(1)
    #Set bootstrap objects
    cbor_python_future_firmw_intg_certificates.add_certificate("mbed.BootstrapServerCACert", "x509_pth_ca.der")
    cbor_python_future_firmw_intg_certificates.add_certificate("mbed.BootstrapDeviceCert", "x509_pth_ca_child.der")
    cbor_python_future_firmw_intg_certificates.add_ecc_private_key("mbed.BootstrapDevicePrivateKey","priv_ecc_key1.der")
    cbor_python_future_firmw_intg_certificates.add_certificate("mbed.FirmwareIntegrityCACert", "x509_pth_ca_3_future.der")
    cbor_python_future_firmw_intg_certificates.add_certificate("mbed.FirmwareIntegrityCert", "x509_pth_child_future.der")
    generate_cbor_data("cbor_python_future_firmw_intg_certificates", out_dir, cbor_python_future_firmw_intg_certificates)


    #CBOR with mbed.FirmwareIntegrityCert expired certificate
    cbor_python_child_expired_certificates  = CborConfiguration()
    cbor_python_child_expired_certificates.set_cbor_groups()
    #Set all configuration parameters
    cbor_python_child_expired_certificates.set_serial_number("A12FC-45")
    cbor_python_child_expired_certificates.set_endpoint_name("EP_fdsf")
    cbor_python_child_expired_certificates.set_bootstrap_uri("coap://?aid=bootstrap.arm.com")
    cbor_python_child_expired_certificates.set_manf_name("Toshiba")
    cbor_python_child_expired_certificates.set_device_type("TEMP-SENSOR")
    cbor_python_child_expired_certificates.set_model_number("KR-54-FS")
    cbor_python_child_expired_certificates.set_hardware_version("0.1.5")
    cbor_python_child_expired_certificates.set_memory_size_version(102400)
    cbor_python_child_expired_certificates.set_utc_offset("+02:00")
    cbor_python_child_expired_certificates.set_device_time_zone("America/New York")
    cbor_python_child_expired_certificates.set_current_time(secs_since_epoch)
    cbor_python_child_expired_certificates.set_bootstrap_mode(1)
    #Set bootstrap objects
    cbor_python_child_expired_certificates.add_certificate("mbed.BootstrapServerCACert", "x509_pth_ca.der")
    cbor_python_child_expired_certificates.add_certificate("mbed.BootstrapDeviceCert", "x509_pth_ca_child.der")
    cbor_python_child_expired_certificates.add_ecc_private_key("mbed.BootstrapDevicePrivateKey","priv_ecc_key1.der")
    cbor_python_child_expired_certificates.add_certificate("mbed.FirmwareIntegrityCACert", "x509_pth_ca_2.der")
    cbor_python_child_expired_certificates.add_certificate("mbed.FirmwareIntegrityCert", "x509_pth_child_expired.der")
    generate_cbor_data("cbor_python_child_expired_certificates", out_dir, cbor_python_child_expired_certificates)


    #CBOR with mbed.BootstrapDeviceCert expired certificate
    cbor_python_device_expired_certificate  = CborConfiguration()
    cbor_python_device_expired_certificate.set_cbor_groups()
    #Set all configuration parameters
    cbor_python_device_expired_certificate.set_serial_number("A12FC-45")
    cbor_python_device_expired_certificate.set_endpoint_name("EP_fdsf")
    cbor_python_device_expired_certificate.set_bootstrap_uri("coap://?aid=bootstrap.arm.com")
    cbor_python_device_expired_certificate.set_manf_name("Toshiba")
    cbor_python_device_expired_certificate.set_device_type("TEMP-SENSOR")
    cbor_python_device_expired_certificate.set_model_number("KR-54-FS")
    cbor_python_device_expired_certificate.set_hardware_version("0.1.5")
    cbor_python_device_expired_certificate.set_memory_size_version(102400)
    cbor_python_device_expired_certificate.set_utc_offset("+02:00")
    cbor_python_device_expired_certificate.set_device_time_zone("America/New York")
    cbor_python_device_expired_certificate.set_current_time(secs_since_epoch)
    cbor_python_device_expired_certificate.set_bootstrap_mode(1)
    #Set bootstrap objects
    cbor_python_device_expired_certificate.add_certificate("mbed.BootstrapServerCACert", "x509_pth_ca.der")
    cbor_python_device_expired_certificate.add_certificate("mbed.BootstrapDeviceCert", "x509_pth_child_expired.der")
    cbor_python_device_expired_certificate.add_ecc_private_key("mbed.BootstrapDevicePrivateKey","priv_ecc_key1.der")
    cbor_python_device_expired_certificate.add_certificate("mbed.FirmwareIntegrityCACert", "x509_pth_ca_2.der")
    cbor_python_device_expired_certificate.add_certificate("mbed.FirmwareIntegrityCert", "x509_pth_child_1.der")
    generate_cbor_data("cbor_python_device_expired_certificate", out_dir, cbor_python_device_expired_certificate)


    #CBOR with FW integrated certificate with untrusted signature
    cbor_python_fw_integrity_intrusted_certificate  = CborConfiguration()
    cbor_python_fw_integrity_intrusted_certificate.set_cbor_groups()
    #Set all configuration parameters
    cbor_python_fw_integrity_intrusted_certificate.set_serial_number("A12FC-45")
    cbor_python_fw_integrity_intrusted_certificate.set_endpoint_name("EP_fdsf")
    cbor_python_fw_integrity_intrusted_certificate.set_bootstrap_uri("coap://?aid=bootstrap.arm.com")
    cbor_python_fw_integrity_intrusted_certificate.set_manf_name("Toshiba")
    cbor_python_fw_integrity_intrusted_certificate.set_device_type("TEMP-SENSOR")
    cbor_python_fw_integrity_intrusted_certificate.set_model_number("KR-54-FS")
    cbor_python_fw_integrity_intrusted_certificate.set_hardware_version("0.1.5")
    cbor_python_fw_integrity_intrusted_certificate.set_memory_size_version(102400)
    cbor_python_fw_integrity_intrusted_certificate.set_utc_offset("+02:00")
    cbor_python_fw_integrity_intrusted_certificate.set_device_time_zone("America/New York")
    cbor_python_fw_integrity_intrusted_certificate.set_current_time(secs_since_epoch)
    cbor_python_fw_integrity_intrusted_certificate.set_bootstrap_mode(1)
    #Set bootstrap objects
    cbor_python_fw_integrity_intrusted_certificate.add_certificate("mbed.BootstrapServerCACert", "x509_pth_ca.der")
    cbor_python_fw_integrity_intrusted_certificate.add_certificate("mbed.BootstrapDeviceCert", "x509_pth_ca_child.der")
    cbor_python_fw_integrity_intrusted_certificate.add_ecc_private_key("mbed.BootstrapDevicePrivateKey","priv_ecc_key1.der")
    cbor_python_fw_integrity_intrusted_certificate.add_certificate("mbed.FirmwareIntegrityCACert", "x509_pth_ca_4.der")
    cbor_python_fw_integrity_intrusted_certificate.add_certificate("mbed.FirmwareIntegrityCert", "x509_pyth_child_4.der")
    generate_cbor_data("cbor_python_fw_integrity_intrusted_certificate", out_dir, cbor_python_fw_integrity_intrusted_certificate)


    #CBOR with BootstrapServerCACert created based on python csr
    #cbor_python_csr_certificate  = CborConfiguration()
    #cbor_python_csr_certificate.set_cbor_groups()
    #Set all configuration parameters
    #cbor_python_csr_certificate.set_serial_number("A12FC-45")
    #cbor_python_csr_certificate.set_endpoint_name("EP_fdsf")
    #cbor_python_csr_certificate.set_bootstrap_uri("coap://?aid=bootstrap.arm.com")
    #cbor_python_csr_certificate.set_manf_name("Toshiba")
    #cbor_python_csr_certificate.set_device_type("TEMP-SENSOR")
    #cbor_python_csr_certificate.set_model_number("KR-54-FS")
    #cbor_python_csr_certificate.set_hardware_version("0.1.5")
    #cbor_python_csr_certificate.set_memory_size_version(102400)
    #cbor_python_csr_certificate.set_utc_offset("+02:00")
    #cbor_python_csr_certificate.set_device_time_zone("America/New York")
    #cbor_python_csr_certificate.set_current_time(secs_since_epoch)
    #cbor_python_csr_certificate.set_bootstrap_mode(1)
    #Set bootstrap objects
    #cbor_python_csr_certificate.add_certificate("mbed.BootstrapServerCACert", "x509_pth_cert_from_csr.der")
    #cbor_python_csr_certificate.add_certificate("mbed.BootstrapDeviceCert", "x509_pth_ca_child.der")
    #cbor_python_csr_certificate.add_ecc_private_key("mbed.BootstrapDevicePrivateKey","priv_ecc_key1.der")
    #cbor_python_csr_certificate.add_certificate("mbed.FirmwareIntegrityCACert", "x509_pth_ca_1.der")
    #cbor_python_csr_certificate.add_certificate("mbed.FirmwareIntegrityCert", "x509_pth_child_1.der")
    #generate_cbor_data("cbor_python_csr_certificate", out_dir, cbor_python_csr_certificate)

	#----------------------------------------------------------------------------



 
	#----------------------------------------------------------------------------

def generate_test_comm_packets(out_dir):
    #comm packet generation
    p1 = CommPacket("comm_valid_packet")
    p1.set_out_dir(out_dir)
    p1.build_valid_comm_packet("cbor1")
    p1.generate_assets()

    p2 = CommPacket("comm_short_length_packet")
    p2.set_out_dir(out_dir)
    p2.build_valid_comm_packet("cbor1")
    p2.set_length(10)
    p2.generate_assets()

    p3 = CommPacket("comm_long_data_packet")
    p3.set_out_dir(out_dir)
    p3.build_valid_comm_packet("cbor3")
    p3.set_data("0x452345")
    p3.set_length(2)
    p3.generate_assets()

    p4 = CommPacket("comm_invalid_signature_packet")
    p4.set_out_dir(out_dir)
    p4.build_valid_comm_packet("cbor1")
    p4.set_signature("34234454523423423423423400908293819283913745203842093840253434093059404590")
    p4.generate_assets()
