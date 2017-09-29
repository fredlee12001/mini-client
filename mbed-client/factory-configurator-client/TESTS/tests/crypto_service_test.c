// ----------------------------------------------------------------------------
// Copyright 2016-2017 ARM Ltd.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// ----------------------------------------------------------------------------


/* Unit test for pv_crypto library and TZInfra crypto functionality */
#include "unity_fixture.h"
#include "kcm_status.h"
#include "pv_log.h"
#include "pv_macros.h"
#include "cs_hash.h"
#include "cs_der_keys.h"
#include "cs_der_certs.h"
#include "testdata.h"
#include "key_config_manager.h"
#include "factory_configurator_client.h"
#include "storage.h"
#include "time.h"
#include "fcc_test_common_helpers.h"

extern test_certificate_chain_data_s test_certificate_vector[CERTIFICATE_TEST_ITERATIONS][NUMBER_OF_CERTIFICATES_IN_CHAIN];


const uint8_t pem_x509_cert[] = "-----BEGIN CERTIFICATE-----\r\n"
                                "MIICTTCCAfOgAwIBAgIJALwWE3L4vqw5MAoGCCqGSM49BAMCMIGCMQswCQYDVQQG\r\n"
                                "EwJVSzETMBEGA1UECAwKS2Zhci1uZXRlcjEQMA4GA1UEBwwHTmV0YW5pYTEMMAoG\r\n"
                                "A1UECgwDQVJNMQwwCgYDVQQLDANJT1QxDzANBgNVBAMMBkRldmljZTEfMB0GCSqG\r\n"
                                "SIb3DQEJARYQZGV2aWNlMDFAYXJtLmNvbTAeFw0xNzAxMjQxNzIyMDhaFw0xNzAy\r\n"
                                "MjMxNzIyMDhaMIGCMQswCQYDVQQGEwJVSzETMBEGA1UECAwKS2Zhci1uZXRlcjEQ\r\n"
                                "MA4GA1UEBwwHTmV0YW5pYTEMMAoGA1UECgwDQVJNMQwwCgYDVQQLDANJT1QxDzAN\r\n"
                                "BgNVBAMMBkRldmljZTEfMB0GCSqGSIb3DQEJARYQZGV2aWNlMDFAYXJtLmNvbTBZ\r\n"
                                "MBMGByqGSM49AgEGCCqGSM49AwEHA0IABIkzitPXh/E3lE2sZsfgnwBwT7MZeh5o\r\n"
                                "Lh7fwSezH+509h9OLPYbjQ1+QcKJ+HwrkK0Jsn2n6qMCScIS665C8DejUDBOMB0G\r\n"
                                "A1UdDgQWBBRzXeZ6lLtA0b4eJRR4wVq3XVp46jAfBgNVHSMEGDAWgBRzXeZ6lLtA\r\n"
                                "0b4eJRR4wVq3XVp46jAMBgNVHRMEBTADAQH/MAoGCCqGSM49BAMCA0gAMEUCIQDG\r\n"
                                "QfV3QKRH8g5eDizvZ4NjnFVvjrw3n55g7vUBRB2IYAIgYO35v6fSV8CRFNbA1wnQ\r\n"
                                "Ui6TU5YPCzAVEQt5OO8eVkA=\r\n"
                                "-----END CERTIFICATE-----\r\n";


#define CS_TEST_MAX_DIGEST_SIZE CS_SHA256_SIZE
typedef struct {
    cs_hash_mode_e mode;
    uint8_t data_in[256];
    uint32_t data_in_size;
    uint8_t expected_digest[CS_TEST_MAX_DIGEST_SIZE];
} cs_test_sha256_template_s;



cs_test_sha256_template_s cs_hash_test_vectors[] = {
    {
        .mode = CS_SHA256,
        .data_in = {0x6b,0xc1,0xbe,0xe2,0x2e,0x40,0x9f,0x96,0xe9,0x3d,0x7e,0x11,0x73,0x93,0x17,0x2a},
        .data_in_size = 16,
        .expected_digest ={
            0xa0,0x63,0xdf,0x83,0xa8,0xc2,0x8a,0x49,0xda,0xf4,0xae,0xba,0x0e,0x29,0xee,0x7b,
            0x21,0x77,0xe8,0x51,0x10,0x72,0x94,0x4c,0x3d,0x29,0x9c,0xf7,0x7d,0xc8,0x3e,0x7a
        },
    },
    {
        .mode = CS_SHA256,
        .data_in = {
            0x6b,0xc1,0xbe,0xe2,0x2e,0x40,0x9f,0x96,0xe9,0x3d,0x7e,0x11,0x73,0x93,0x17,0x2a,
            0x65,0xa2,0x32,0xd6,0xbc,0xd0,0xf9,0x39,0xed,0x1f,0xe1,0x28,0xc1,0x3b,0x0e,0x1b
        },
        .data_in_size = 32,
        .expected_digest ={
            0x75,0xcf,0xb3,0x9b,0x62,0xc4,0x74,0x92,0x1e,0x2a,0xad,0x97,0x9c,0x21,0x0f,0x8b,
            0x69,0x18,0x0a,0x9d,0x58,0xe9,0xf2,0x96,0xa4,0xb9,0x90,0x4a,0xe6,0xe7,0xaa,0x40
        },
    },
    {
        .mode = CS_SHA256,
        .data_in = {
            0x99,0xfd,0x18,0xa3,0x5d,0x50,0x81,0x84,0xa6,0xf3,0x61,0xc6,0x7c,0xd9,0xb1,0x0b,
            0x4c,0xd1,0xd8,0xb2,0x46,0x57,0x2a,0x4d,0x03,0xb0,0xae,0x55,0x6b,0x36,0x24,0x1d,
            0xd6,0xf0,0x46,0x05,0x71,0x65,0x4f,0xf0,0xe4,0xb2,0xba,0xf8,0x31,0xdb,0x4c,0x60,
            0xdf,0x5f,0x54,0xc9,0x59,0x0f,0x32,0xa9,0x91,0x1f,0x16,0xfa,0xe8,0x7e,0x0a,0x2f,
            0x52
        },
        .data_in_size = 65,
        .expected_digest ={
            0x34,0x70,0xCD,0x54,0x7B,0x0A,0x11,0x5F,0xE0,0x5C,0xEB,0xBC,0x07,0xBA,0x91,0x88,
            0x27,0x20,0x25,0x6B,0xB2,0x7A,0x66,0x89,0x1A,0x4B,0xB7,0x17,0x11,0x04,0x86,0x6F
        },
    }
};

extern bool gTestIsChainExists;

static bool set_approximate_now_time(void)
{
    palStatus_t pal_status;
    uint64_t time = 0;

    // Firstly - check if time is set, if yes, no need to set the time again.
    // In FreeRTOS and mbedOS the RTC is not connected hence we must set the current time
    // to some reasonable value for our own certificate time validation test cases
    time = pal_osGetTime();
    if (time != 0) {
        return true;
    }

    // Time wasn't set - set the time to some reasonable value
    pal_status = pal_osSetTime(testdata_current_time);
    if (PAL_SUCCESS != pal_status) {
        return false;
    }

    return true;
}


TEST_GROUP(crypto_service);

TEST_SETUP(crypto_service)
{
    bool success = set_approximate_now_time();
    TEST_ASSERT_TRUE(success);

    gTestIsChainExists = false;
}

TEST_TEAR_DOWN(crypto_service)
{
    gTestIsChainExists = false;
}

TEST(crypto_service, cs_hash)
{
    uint8_t digest[CS_TEST_MAX_DIGEST_SIZE];
    kcm_status_e status;
    uint32_t i;


    for (i = 0; i < PV_ARRAY_LENGTH(cs_hash_test_vectors); i++) {
        status = cs_hash(cs_hash_test_vectors[i].mode,
                         cs_hash_test_vectors[i].data_in,
                         cs_hash_test_vectors[i].data_in_size,
                         digest,
                         sizeof(digest));
        TEST_ASSERT_EQUAL(status, KCM_STATUS_SUCCESS);
        TEST_ASSERT_EQUAL_MEMORY(cs_hash_test_vectors[i].expected_digest, digest, sizeof(cs_hash_test_vectors[i].expected_digest));
    }

}

TEST(crypto_service, cs_der_priv_key_verify)
{
    kcm_status_e status;
    uint8_t priv_key_err[sizeof(testdata_priv_ecc_key1der)];

    // FIXME: Currently mbedTLS returns a different status on mbedOS and Linux.
    //        To avoid this, PAL should mask any crypto error into a unified set of error codes.
    //        This test should be revisit once issue has been resolve.
    //        Refer to JIRA issue: IOTPAL-337

    //verify private key in der format
    status = cs_der_priv_key_verify(testdata_priv_ecc_key1der, sizeof(testdata_priv_ecc_key1der));
    TEST_ASSERT_EQUAL(KCM_STATUS_SUCCESS, status);

    //verify private key in der format with not supported curve(secp521r1)
    status = cs_der_priv_key_verify(testdata_priv_ecc_key5der, sizeof(testdata_priv_ecc_key5der));
    TEST_ASSERT_NOT_EQUAL(KCM_STATUS_SUCCESS, status);
    //TEST_ASSERT_EQUAL(CS_ERR_UNSUPPORTED_CURVE, status);

    //verify private key with damaged DER format
    memcpy(priv_key_err, testdata_priv_ecc_key1der, sizeof(priv_key_err));
    priv_key_err[sizeof(priv_key_err) / 2] = priv_key_err[sizeof(priv_key_err) / 2] + 1;
    status = cs_der_priv_key_verify(priv_key_err, sizeof(priv_key_err));
    TEST_ASSERT_NOT_EQUAL(KCM_STATUS_SUCCESS, status);
    //TEST_ASSERT_EQUAL(CS_ERR_PARSING_DER_PRIVATE_KEY, status);

    //try to verify private key in der format using public key function
    status = cs_der_public_key_verify(testdata_priv_ecc_key1der, sizeof(testdata_priv_ecc_key1der));
    TEST_ASSERT_NOT_EQUAL(KCM_STATUS_SUCCESS, status);
    //TEST_ASSERT_EQUAL(CS_ERR_PARSING_DER_PUBLIC_KEY, status);
}

TEST(crypto_service, cs_der_priv_key_verify_illegal_params)
{
    kcm_status_e status;

    status = cs_der_priv_key_verify(NULL, sizeof(testdata_priv_ecc_key1der));
    TEST_ASSERT_EQUAL(status, KCM_STATUS_INVALID_PARAMETER);

    status = cs_der_priv_key_verify(testdata_priv_ecc_key1der, 0);
    TEST_ASSERT_EQUAL(status, KCM_STATUS_INVALID_PARAMETER);

    // FIXME: Currently mbedTLS returns a different status on mbedOS and Linux.
    //        To avoid this, PAL should mask any crypto error into a unified set of error codes.
    //        This test should be revisit once issue has been resolve.
    //        Refer to JIRA issue: IOTPAL-337
    status = cs_der_priv_key_verify(testdata_priv_ecc_key1der, sizeof(testdata_priv_ecc_key1der) - 1);
    TEST_ASSERT_NOT_EQUAL(KCM_STATUS_SUCCESS, status);
    //TEST_ASSERT_EQUAL(status, CS_ERR_PARSING_DER_PRIVATE_KEY);
}


TEST(crypto_service, cs_der_pub_key_verify)
{
    kcm_status_e status;
    uint8_t pub_key_err[sizeof(testdata_pub_ecc_key1der)];

    // FIXME: Currently mbedTLS returns a different status on mbedOS and Linux.
    //        To avoid this, PAL should mask any crypto error into a unified set of error codes.
    //        This test should be revisit once issue has been resolve.
    //        Refer to JIRA issue: IOTPAL-337

    //verify public key in der format
    status = cs_der_public_key_verify(testdata_pub_ecc_key1der, sizeof(testdata_pub_ecc_key1der));
    TEST_ASSERT_EQUAL(KCM_STATUS_SUCCESS, status);

    //verify public key in der format with not supported curve(secp521r1)
    status = cs_der_public_key_verify(testdata_pub_ecc_key5der, sizeof(testdata_pub_ecc_key5der));
    TEST_ASSERT_NOT_EQUAL(KCM_STATUS_SUCCESS, status);
    //TEST_ASSERT_EQUAL(CS_ERR_UNSUPPORTED_CURVE, status);

    //verify public key with damaged DER format
    memcpy(pub_key_err, testdata_pub_ecc_key1der, sizeof(pub_key_err));
    pub_key_err[sizeof(pub_key_err) / 2] = pub_key_err[sizeof(pub_key_err) / 2] + 1;
    status = cs_der_public_key_verify(pub_key_err, sizeof(pub_key_err));
    TEST_ASSERT_NOT_EQUAL(KCM_STATUS_SUCCESS, status);
    //TEST_ASSERT_EQUAL(CS_ERR_PARSING_DER_PUBLIC_KEY, status);

    //try to verify public key in der format using private key function
    status = cs_der_priv_key_verify(testdata_pub_ecc_key1der, sizeof(testdata_pub_ecc_key1der));
    TEST_ASSERT_NOT_EQUAL(KCM_STATUS_SUCCESS, status);
    //TEST_ASSERT_EQUAL(CS_ERR_PARSING_DER_PRIVATE_KEY, status);
}

TEST(crypto_service, cs_der_pub_key_verify_illegal_params)
{
    kcm_status_e status;

    status = cs_der_public_key_verify(NULL, sizeof(testdata_pub_ecc_key1der));
    TEST_ASSERT_EQUAL(status, KCM_STATUS_INVALID_PARAMETER);

    status = cs_der_public_key_verify(testdata_pub_ecc_key1der, 0);
    TEST_ASSERT_EQUAL(status, KCM_STATUS_INVALID_PARAMETER);

    // FIXME: Currently mbedTLS returns a different status on mbedOS and Linux.
    //        To avoid this, PAL should mask any crypto error into a unified set of error codes.
    //        This test should be revisit once issue has been resolve.

    status = cs_der_public_key_verify(testdata_pub_ecc_key1der, sizeof(testdata_pub_ecc_key1der) - 1);
    TEST_ASSERT_NOT_EQUAL(KCM_STATUS_SUCCESS, status);
    //TEST_ASSERT_EQUAL(status, CS_ERR_PARSING_DER_PUBLIC_KEY);
}


TEST(crypto_service, cs_der_self_signed_cert_verify)
{
    kcm_status_e status;

    //verify self-signed certificate in der format
    status = cs_verify_der_x509_cert(testdata_x509_1der, sizeof(testdata_x509_1der), true, NULL,0);
    TEST_ASSERT_EQUAL(status, KCM_STATUS_SUCCESS);
}

//Shelly FIXME: wait for PAL fixes
IGNORE_TEST(crypto_service, cs_der_verify_cert_time)
{
    kcm_status_e status;

    //1. self singed cert
    //verify expired certificate. Clock should be configured and  after it MBEDTLS_HAVE_TIME_DATE flag should be added to mbedtls flags
    status = cs_verify_der_x509_cert(testdata_x509_pth_expr_ca_5der, sizeof(testdata_x509_pth_expr_ca_5der), true, NULL, 0);
    TEST_ASSERT_EQUAL(KCM_CRYPTO_STATUS_CERT_EXPIRED, status);

    //3. expired child certificate (not self-signed)
    //verify expired certificate. Clock should be configured and  after it MBEDTLS_HAVE_TIME_DATE flag should be added to mbedtls flags
    status = cs_verify_der_x509_cert(testdata_x509_pth_expr_child_5der, sizeof(testdata_x509_pth_expr_child_5der), true, NULL, 0);
    TEST_ASSERT_EQUAL(KCM_CRYPTO_STATUS_CERT_EXPIRED, status);
}

TEST(crypto_service, cs_der_verify_no_full_verify)
{
    kcm_status_e status;

    //Verify expired certificate. Should return SUCCESS although the certificate expired because it does not perform full validation, only parsing
    status = cs_verify_der_x509_cert(testdata_x509_pth_expr_ca_5der, sizeof(testdata_x509_pth_expr_ca_5der), false, NULL, 0);
    TEST_ASSERT_EQUAL(KCM_STATUS_SUCCESS, status);
}

TEST(crypto_service, cs_wrong_signature_of_child)
{
    kcm_status_e status;

    //Verify certificate with wrong signature. Should return SUCCESS because verify it does not perform full validation, only parsing
    status = cs_verify_der_x509_cert(testdata_x509_pyth_child_4der, sizeof(testdata_x509_pyth_child_4der), false, NULL, 0);
    TEST_ASSERT_EQUAL(KCM_STATUS_SUCCESS, status);

    //Verify certificate with wrong signature. Should return SUCCESS because verify it does not perform full validation, only parsing
    status = cs_verify_der_x509_cert(testdata_x509_pyth_child_4der, sizeof(testdata_x509_pyth_child_4der), true, NULL, 0);
    TEST_ASSERT_EQUAL(KCM_STATUS_SUCCESS, status);
}
//open after fixing of PAL bug  308
IGNORE_TEST(crypto_service, cs_der_verify_keys_and_sign_params)
{
    kcm_status_e status;

    //1.self-signed certificate
    //verify self-signed certificate with not supported md alg// FAILED
    status = cs_verify_der_x509_cert(testdata_x509_1_mdder, sizeof(testdata_x509_1_mdder), true, NULL, 0);
    TEST_ASSERT_EQUAL(KCM_CRYPTO_STATUS_CERT_MD_ALG, status);

    //Tania FIXME: wait for PAL-301 fixes
    //verify self-signed certificate with not supported ecdsa public key curve
    status = cs_verify_der_x509_cert(testdata_x509_1_inv_keyder, sizeof(testdata_x509_1_inv_keyder), true, NULL, 0);
    TEST_ASSERT_EQUAL(KCM_CRYPTO_STATUS_CERT_PUB_KEY, status);

    status = cs_verify_der_x509_cert(testdata_x509_2_inv_keyder, sizeof(testdata_x509_2_inv_keyder), true, NULL, 0);
    TEST_ASSERT_EQUAL(KCM_CRYPTO_STATUS_PARSING_DER_CERT, status);

    //3. child certificate (not self-signed)
    //verify self-signed certificate with not supported md alg// FAILED
    status = cs_verify_der_x509_cert(testdata_x509_1_ch_inv_mdder, sizeof(testdata_x509_1_ch_inv_mdder), true, NULL, 0);
    TEST_ASSERT_EQUAL(KCM_CRYPTO_STATUS_PARSING_DER_CERT, status);

    status = cs_verify_der_x509_cert(testdata_x509_2_ch_inv_mdder, sizeof(testdata_x509_2_ch_inv_mdder), true, NULL, 0);
    TEST_ASSERT_EQUAL(KCM_CRYPTO_STATUS_PARSING_DER_CERT, status);

    //Tania FIXME: wait for PAL-301 fixes
    //verify self-signed certificate with not supported ecdsa public key curve
    status = cs_verify_der_x509_cert(testdata_x509_1_ch_inv_key_childder, sizeof(testdata_x509_1_ch_inv_key_childder), true, NULL, 0);
    TEST_ASSERT_EQUAL(KCM_CRYPTO_STATUS_CERT_PUB_KEY, status);
}

TEST(crypto_service, cs_der_cert_chain_verify)
{
    kcm_status_e status;
    kcm_status_e kcm_err;

    const char cert_name[] = "test_cert";

    kcm_err = kcm_init();
    TEST_ASSERT_EQUAL(KCM_STATUS_SUCCESS, kcm_err);

    /* Shelly: We will check it after we will check untrusted certificates not self-signed certificates
    (void)kcm_certificate_delete((uint8_t*)cert_name, sizeof(cert_name));
     TEST_ASSERT_EQUAL(KCM_STATUS_SUCCESS, kcm_err);

     //verify not self-signed certificate
     status = cs_verify_der_x509_cert(testdata_x509_1_ca_childder, sizeof(testdata_x509_1_ca_childder),, true);
     TEST_ASSERT_EQUAL(CS_ERR_CERT_NOT_TRUSTED, status);*/

    //save trusted self-signed certificate for not self-signed certificate from previous step
    kcm_err = kcm_item_store((uint8_t*)cert_name, sizeof(cert_name), KCM_CERTIFICATE_ITEM, true, testdata_x509_1_cader, sizeof(testdata_x509_1_cader), NULL);
    TEST_ASSERT_EQUAL(KCM_STATUS_SUCCESS, kcm_err);

    gTestIsChainExists = true;
    //verify not self-signed certificate with its chain
    status = cs_verify_der_x509_cert(testdata_x509_1_ca_childder, sizeof(testdata_x509_1_ca_childder), true, NULL, 0);
    TEST_ASSERT_EQUAL(KCM_STATUS_SUCCESS, status);

    //clean database
    kcm_err = kcm_item_delete((uint8_t*)cert_name, sizeof(cert_name), KCM_CERTIFICATE_ITEM);
    TEST_ASSERT_EQUAL(KCM_STATUS_SUCCESS, kcm_err);

    gTestIsChainExists = false;
    //finalize
    kcm_err = kcm_finalize();
    TEST_ASSERT(kcm_err == KCM_STATUS_SUCCESS);

}

TEST(crypto_service, cs_der_not_trusted_no_chain_verify)
{
    kcm_status_e status;

    //verify not self-signed certificate without chain
    status = cs_verify_der_x509_cert(testdata_x509_1_ca_childder, sizeof(testdata_x509_1_ca_childder), true, NULL, 0);
    TEST_ASSERT_EQUAL(KCM_STATUS_SUCCESS, status);
}

TEST(crypto_service, cs_der_get_cert_attribute)
{
    kcm_status_e status;
    uint8_t *cn_buffer = NULL;
    uint8_t dummy_buf[1];
    size_t actual_size_of_attribute = 0;
    uint64_t valid_from = 0;
    uint64_t valid_to = 0;
    uint64_t start_of_2017 = 1483228800;
    uint64_t seconds_in_8000_days = 691200000;

    //get size of CN certificate
    status = cs_attr_get_data_x509_cert(testdata_x509_1_cader, sizeof(testdata_x509_1_cader), CS_CN_ATTRIBUTE_TYPE,dummy_buf, 0, &actual_size_of_attribute);
    TEST_ASSERT_EQUAL(KCM_STATUS_INSUFFICIENT_BUFFER, status);

    cn_buffer = malloc(actual_size_of_attribute);

    //get data of CN certificate
    status = cs_attr_get_data_x509_cert(testdata_x509_1_cader, sizeof(testdata_x509_1_cader), CS_CN_ATTRIBUTE_TYPE, cn_buffer, actual_size_of_attribute, &actual_size_of_attribute);
    TEST_ASSERT_EQUAL(KCM_STATUS_SUCCESS, status);
    TEST_ASSERT_EQUAL_STRING(cn_buffer, "Device");

    actual_size_of_attribute = 0;

    //get size of valid from attribute
    status = cs_attr_get_data_x509_cert(testdata_x509_1_cader, sizeof(testdata_x509_1_cader), CS_VALID_FROM_ATTRIBUTE_TYPE, dummy_buf, 0, &actual_size_of_attribute);
    TEST_ASSERT_EQUAL(KCM_STATUS_INSUFFICIENT_BUFFER, status);
    TEST_ASSERT_EQUAL(actual_size_of_attribute, sizeof(uint64_t));

    //get valid from data
    status = cs_attr_get_data_x509_cert(testdata_x509_1_cader, sizeof(testdata_x509_1_cader), CS_VALID_FROM_ATTRIBUTE_TYPE, (uint8_t*)&valid_from, actual_size_of_attribute, &actual_size_of_attribute);
    TEST_ASSERT_EQUAL(KCM_STATUS_SUCCESS, status);
    TEST_ASSERT(valid_from > start_of_2017);

    actual_size_of_attribute = 0;
    //get size of valid to attribute
    status = cs_attr_get_data_x509_cert(testdata_x509_1_cader, sizeof(testdata_x509_1_cader), CS_VALID_TO_ATTRIBUTE_TYPE, dummy_buf, 0, &actual_size_of_attribute);
    TEST_ASSERT_EQUAL(KCM_STATUS_INSUFFICIENT_BUFFER, status);
    TEST_ASSERT_EQUAL(actual_size_of_attribute, sizeof(uint64_t));

    //get valid to certificate
    status = cs_attr_get_data_x509_cert(testdata_x509_1_cader, sizeof(testdata_x509_1_cader), CS_VALID_TO_ATTRIBUTE_TYPE, (uint8_t*)&valid_to, actual_size_of_attribute, &actual_size_of_attribute);
    TEST_ASSERT_EQUAL(KCM_STATUS_SUCCESS, status);
    TEST_ASSERT(valid_to - valid_from == seconds_in_8000_days);
    actual_size_of_attribute = 0;


    free(cn_buffer);
}
TEST(crypto_service, cs_der_get_cert_attribute_illegal_params)
{
    kcm_status_e status;
    uint8_t dummy_buf[1];
    size_t actual_size_of_attribute = 0;

    //get size of wrong attribute
    status = cs_attr_get_data_x509_cert(testdata_x509_1_cader, sizeof(testdata_x509_1_cader), CS_MAX_ATTRIBUTE_TYPE, dummy_buf, 0, &actual_size_of_attribute);
    TEST_ASSERT_EQUAL(KCM_CRYPTO_STATUS_INVALID_X509_ATTR, status);

    //actual size pointer is NULL
    status = cs_attr_get_data_x509_cert(testdata_x509_1_cader, sizeof(testdata_x509_1_cader), CS_CN_ATTRIBUTE_TYPE,dummy_buf, 0, NULL);
    TEST_ASSERT_EQUAL(KCM_STATUS_INVALID_PARAMETER, status);

    //size of certificate is 0
    status = cs_attr_get_data_x509_cert(testdata_x509_1_cader, 0, CS_CN_ATTRIBUTE_TYPE,dummy_buf, 0, &actual_size_of_attribute);
    TEST_ASSERT_EQUAL(KCM_STATUS_INVALID_PARAMETER, status);

    //certificate is NULL
    status = cs_attr_get_data_x509_cert(NULL, sizeof(testdata_x509_1_cader), CS_CN_ATTRIBUTE_TYPE,dummy_buf, 0, &actual_size_of_attribute);
    TEST_ASSERT_EQUAL(KCM_STATUS_INVALID_PARAMETER, status);

    //out buffer is NULL
    status = cs_attr_get_data_x509_cert(testdata_x509_1_cader, sizeof(testdata_x509_1_cader), CS_CN_ATTRIBUTE_TYPE,NULL, 0, &actual_size_of_attribute);
    TEST_ASSERT_EQUAL(KCM_STATUS_INVALID_PARAMETER, status);
}
TEST(crypto_service, cs_der_cert_multiple_chain)
{
    palStatus_t pal_status = PAL_SUCCESS;
    palX509Handle_t x509_cert_1 = NULLPTR;
    palX509Handle_t x509_cert_2 = NULLPTR;
    palX509Handle_t x509_cert_chain = NULLPTR;
    int test_iteration = 0;
    int certificate_index = 0;

    for (test_iteration = 0; test_iteration < CERTIFICATE_TEST_ITERATIONS; test_iteration++) {

        //Allocate and Init certificate handler
        pal_status = pal_x509Initiate(&x509_cert_1);
        TEST_ASSERT_EQUAL(pal_status, PAL_SUCCESS);

        pal_status = pal_x509Initiate(&x509_cert_2);
        TEST_ASSERT_EQUAL(pal_status, PAL_SUCCESS);

        pal_status = pal_x509Initiate(&x509_cert_chain);
        TEST_ASSERT_EQUAL(pal_status, PAL_SUCCESS);

        for (certificate_index = 0; certificate_index < NUMBER_OF_CERTIFICATES_IN_CHAIN; certificate_index++) {
            //Create chain according to certificates order
            pal_status = pal_x509CertParse(x509_cert_chain, test_certificate_vector[test_iteration][certificate_index].certificate, test_certificate_vector[test_iteration][certificate_index].certificate_size);
            TEST_ASSERT_EQUAL(pal_status, PAL_SUCCESS);
        }
        //Parse certificate 1
        pal_status = pal_x509CertParse(x509_cert_1, test_certificate_vector[test_iteration][4].certificate, test_certificate_vector[test_iteration][4].certificate_size);
        TEST_ASSERT_EQUAL(pal_status, PAL_SUCCESS);

        //Parse certificate 2
        pal_status = pal_x509CertParse(x509_cert_2, test_certificate_vector[test_iteration][2].certificate, test_certificate_vector[test_iteration][2].certificate_size);
        TEST_ASSERT_EQUAL(pal_status, PAL_SUCCESS);

        //Verify created certificate chain
        pal_status = pal_x509CertVerify(x509_cert_chain, x509_cert_chain);
        TEST_ASSERT_EQUAL(pal_status, PAL_SUCCESS);

        //Verify certificate 1 using created chain
        pal_status = pal_x509CertVerify(x509_cert_1, x509_cert_chain);
        TEST_ASSERT_EQUAL(pal_status, PAL_SUCCESS);

        //Verify certificate 2 using created chain
        pal_status = pal_x509CertVerify(x509_cert_2, x509_cert_chain);
        TEST_ASSERT_EQUAL(pal_status, PAL_SUCCESS);

        pal_status = pal_x509Free(&x509_cert_1);
        TEST_ASSERT_EQUAL(pal_status, PAL_SUCCESS);

        pal_status = pal_x509Free(&x509_cert_2);
        TEST_ASSERT_EQUAL(pal_status, PAL_SUCCESS);

        pal_status = pal_x509Free(&x509_cert_chain);
        TEST_ASSERT_EQUAL(pal_status, PAL_SUCCESS);
    }
}


TEST_GROUP_RUNNER(crypto_service)
{
    RUN_TEST_CASE(crypto_service, cs_hash);
    RUN_TEST_CASE(crypto_service, cs_der_priv_key_verify);
    RUN_TEST_CASE(crypto_service, cs_der_priv_key_verify_illegal_params);
    RUN_TEST_CASE(crypto_service, cs_der_pub_key_verify);
    RUN_TEST_CASE(crypto_service, cs_der_pub_key_verify_illegal_params);
    RUN_TEST_CASE(crypto_service, cs_der_self_signed_cert_verify);
    RUN_TEST_CASE(crypto_service, cs_der_cert_chain_verify);
    RUN_TEST_CASE(crypto_service, cs_der_verify_keys_and_sign_params);
    RUN_TEST_CASE(crypto_service, cs_der_verify_cert_time);
    RUN_TEST_CASE(crypto_service, cs_der_not_trusted_no_chain_verify);
    RUN_TEST_CASE(crypto_service, cs_der_verify_no_full_verify);
    RUN_TEST_CASE(crypto_service, cs_der_get_cert_attribute);
    RUN_TEST_CASE(crypto_service, cs_der_get_cert_attribute_illegal_params);
    RUN_TEST_CASE(crypto_service, cs_wrong_signature_of_child);
    RUN_TEST_CASE(crypto_service, cs_der_cert_multiple_chain);

}
