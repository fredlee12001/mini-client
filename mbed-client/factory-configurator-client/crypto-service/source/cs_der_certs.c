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

#include "pv_error_handling.h"
#include "cs_der_certs.h"
#include "cs_der_keys.h"
#include "pal.h"
#include "cs_utils.h"
#include "stdbool.h"
#include "key_config_manager.h" // for patch, should be deleted

bool gTestIsChainExists = false;

static kcm_status_e get_x509_attr_size (palX509Handle_t x509_cert, palX509Attr_t attr, size_t *attr_size_out)
{
    palStatus_t pal_status = PAL_SUCCESS;
    uint8_t dummy_buf[1];

    pal_status = pal_x509CertGetAttribute(x509_cert, attr, dummy_buf, 0, attr_size_out);
    SA_PV_ERR_RECOVERABLE_RETURN_IF((pal_status != PAL_ERR_BUFFER_TOO_SMALL), cs_error_handler(pal_status), "pal_x509CertGetAttribute to get attribute size failed");

    return KCM_STATUS_SUCCESS;
}

static kcm_status_e cs_get_attribute_data_x509_cert(const uint8_t *cert,
                                               size_t cert_length,
                                               palX509Attr_t attribute,
                                               uint8_t* output,
                                               size_t size_of_output,
                                               size_t* actual_size_of_output)
{
    kcm_status_e kcm_status = KCM_STATUS_SUCCESS;
    palStatus_t pal_status = PAL_SUCCESS;
    palX509Handle_t x509_cert = NULLPTR;

    //Allocate and Init certificate handler
    pal_status = pal_x509Initiate(&x509_cert);
    SA_PV_ERR_RECOVERABLE_RETURN_IF((pal_status != PAL_SUCCESS), cs_error_handler(pal_status), "pal_x509Initiate failed");

    //Parse Certificate.
    pal_status = pal_x509CertParse(x509_cert, cert, cert_length);
    SA_PV_ERR_RECOVERABLE_GOTO_IF((pal_status != PAL_SUCCESS), kcm_status = cs_error_handler(pal_status), exit, "pal_x509CertParse failed");

    //Get the attribute
    pal_status = pal_x509CertGetAttribute(x509_cert, attribute, output, size_of_output, actual_size_of_output);

    SA_PV_ERR_RECOVERABLE_GOTO_IF((pal_status != PAL_SUCCESS), kcm_status = cs_error_handler(pal_status), exit, "pal_x509CertGetAttribute failed");

exit:
    pal_x509Free(&x509_cert);

    return kcm_status;
}


static kcm_status_e is_self_signed_cert(palX509Handle_t x509_cert, bool *is_self_signed)
{
    kcm_status_e kcm_status = KCM_STATUS_SUCCESS;
    palStatus_t pal_status = PAL_SUCCESS;

    uint8_t *cert_subject = NULL;
    uint8_t *cert_issuer = NULL;
    size_t subject_size = 0, issuer_size = 0;

    //Self-signed certificate is certificate with subject attribute = issuer attribute

    //get and check issuer and subject sizes
    kcm_status = get_x509_attr_size(x509_cert, PAL_X509_SUBJECT_ATTR, &subject_size);
    SA_PV_ERR_RECOVERABLE_RETURN_IF((kcm_status != KCM_STATUS_SUCCESS), kcm_status, "get_x509_attr_size PAL_X509_SUBJECT_ATTR failed");

    kcm_status = get_x509_attr_size(x509_cert, PAL_X509_ISSUER_ATTR, &issuer_size);
    SA_PV_ERR_RECOVERABLE_RETURN_IF((kcm_status != KCM_STATUS_SUCCESS), kcm_status, "get_x509_attr_size PAL_X509_ISSUER_ATTR failed");

    //If issuer and subject attributes have different length it is not self-signed certificate
    if (subject_size != issuer_size) {
        *is_self_signed = false;
        return KCM_STATUS_SUCCESS;
    }

    //Get and check attributes values
    cert_subject = malloc(subject_size);
    SA_PV_ERR_RECOVERABLE_GOTO_IF((cert_subject == NULL), kcm_status = KCM_STATUS_OUT_OF_MEMORY, exit, "Allocate subject attribute failed");

    pal_status = pal_x509CertGetAttribute(x509_cert, PAL_X509_SUBJECT_ATTR, cert_subject, subject_size, &subject_size);
    SA_PV_ERR_RECOVERABLE_GOTO_IF((pal_status != PAL_SUCCESS), kcm_status = cs_error_handler(pal_status), exit,"pal_x509CertGetAttribute PAL_X509_SUBJECT_ATTR failed %d ", (int)cs_error_handler(pal_status));

    cert_issuer = malloc(issuer_size);
    SA_PV_ERR_RECOVERABLE_GOTO_IF((cert_subject == NULL), kcm_status = KCM_STATUS_OUT_OF_MEMORY, exit, "Allocate issuer attribute failed");

    pal_status = pal_x509CertGetAttribute(x509_cert, PAL_X509_ISSUER_ATTR, cert_issuer, issuer_size, &issuer_size);
    SA_PV_ERR_RECOVERABLE_GOTO_IF((pal_status != PAL_SUCCESS), kcm_status = cs_error_handler(pal_status), exit, "pal_x509CertGetAttribute PAL_X509_ISSUER_ATTR failed %d", (int)kcm_status);

    if (memcmp(cert_issuer, cert_subject, issuer_size) == 0) {
        *is_self_signed = true;
    } else {
        *is_self_signed = false;
    }

exit:
    free(cert_subject);
    free(cert_issuer);

    return kcm_status;
}

//Shelly: temporary function, should be removed after
static kcm_status_e get_cert_chain(palX509Handle_t x509_cert, palX509Handle_t *x509_chain)
{

    //Shelly TBD
    //Retrieve Distinguish Name of certificate(for 1.2 certificate name) // for now only self signed certificate or we need a hard coded table to build certificate chain
    //Read Root Certificates using Distinguish Name as file name
    //Create certificate chain using pal_x509CertParse()

    /*Shelly: TEST PATCH */
    {
        kcm_status_e kcm_err;
        palStatus_t pal_status = PAL_SUCCESS;
        uint8_t cert_buf[1000];
        size_t cert_size;
        const char *trusted_cert_name = "test_cert";

        if (gTestIsChainExists) {//  we want to check certificate chain  for test only
            kcm_err = kcm_item_get_data((uint8_t*)trusted_cert_name, sizeof("test_cert"), KCM_CERTIFICATE_ITEM, cert_buf, sizeof(cert_buf), &cert_size); //read previously saved root certificate
            SA_PV_ERR_RECOVERABLE_RETURN_IF((kcm_err != KCM_STATUS_SUCCESS), KCM_STATUS_ERROR, "kcm_certificate_get error %d", kcm_err);

            //Allocate and Init certificate handler
            pal_status = pal_x509Initiate(x509_chain);
            SA_PV_ERR_RECOVERABLE_RETURN_IF((pal_status != PAL_SUCCESS), KCM_STATUS_ERROR, "pal_x509Initiate failed");

            //Add root certificate to trusted chain
            if (pal_x509CertParse(*x509_chain, cert_buf, cert_size) != PAL_SUCCESS) {
                pal_x509Free(x509_chain);
                SA_PV_ERR_RECOVERABLE_RETURN_IF((pal_status != PAL_SUCCESS), KCM_STATUS_ERROR, "pal_x509CertParse failed");
            }
            //return trusted chain
            return KCM_STATUS_SUCCESS;
        }
    }// *Shelly: TEST PATCH end

    return KCM_STATUS_SUCCESS;
}

kcm_status_e cs_verify_der_x509_cert(const uint8_t *cert, size_t cert_length, bool full_verify, const uint8_t *ca_cert, size_t ca_cert_length)
{
    kcm_status_e kcm_status = KCM_STATUS_SUCCESS;
    palStatus_t pal_status = PAL_SUCCESS;
    palX509Handle_t x509_cert = NULLPTR;
    palX509Handle_t x509_cert_chain = NULLPTR;
    bool is_chain_allocated = false;
    bool is_self_signed = false;

    SA_PV_ERR_RECOVERABLE_RETURN_IF((cert == NULL), KCM_STATUS_INVALID_PARAMETER, "Invalid cert pointer");
    SA_PV_ERR_RECOVERABLE_RETURN_IF((cert_length <= 0), KCM_STATUS_INVALID_PARAMETER, "Invalid cert_length");
    SA_PV_ERR_RECOVERABLE_RETURN_IF((cert_length <= 0), KCM_STATUS_INVALID_PARAMETER, "Invalid cert_length");
    SA_PV_ERR_RECOVERABLE_RETURN_IF(((ca_cert_length != 0 && ca_cert == NULL) || (ca_cert != NULL && ca_cert_length == 0)), 
                                    KCM_STATUS_INVALID_PARAMETER, "Invalid ca cert parameters");


    //Allocate and Init certificate handler
    pal_status = pal_x509Initiate(&x509_cert);
    SA_PV_ERR_RECOVERABLE_RETURN_IF((pal_status != PAL_SUCCESS), cs_error_handler(pal_status), "pal_x509Initiate failed");

    //Parse Certificate.
    pal_status = pal_x509CertParse(x509_cert, cert, cert_length);
    SA_PV_ERR_RECOVERABLE_GOTO_IF((pal_status != PAL_SUCCESS), kcm_status = cs_error_handler(pal_status), exit, "pal_x509CertParse failed");

    //Perform certificate validation only if full verification needed
    if (full_verify) {

        kcm_status = is_self_signed_cert(x509_cert, &is_self_signed);
        SA_PV_ERR_RECOVERABLE_GOTO_IF((kcm_status != KCM_STATUS_SUCCESS), kcm_status = kcm_status, exit, "Self signed verification failed");

        if (is_self_signed) { // Send the certificate itself as trusted chain
            x509_cert_chain = x509_cert;
        } else { //Try to create certificate chain, if chain not created for specific certificate, NULL should a parameter of x509_cert_chain
            if (ca_cert == NULL) { //Get certificate chain in case ca certificate is not present
                kcm_status = get_cert_chain(x509_cert, &x509_cert_chain);
                SA_PV_ERR_RECOVERABLE_GOTO_IF((kcm_status != KCM_STATUS_SUCCESS), kcm_status = kcm_status, exit, "get_cert_chain failed %d", kcm_status);
                if (x509_cert_chain != NULLPTR) {
                    is_chain_allocated = true;
                }
            } else { //Use ca certificate for certificate validation
                //Allocate and Init ca certificate handler
                pal_status = pal_x509Initiate(&x509_cert_chain);
                SA_PV_ERR_RECOVERABLE_GOTO_IF((pal_status != PAL_SUCCESS), cs_error_handler(pal_status), exit, "pal_x509Initiate failed");
                
                is_chain_allocated = true;
                
                //Parse ca Certificate.
                pal_status = pal_x509CertParse(x509_cert_chain, ca_cert, ca_cert_length);
                SA_PV_ERR_RECOVERABLE_GOTO_IF((pal_status != PAL_SUCCESS), kcm_status = cs_error_handler(pal_status), exit, "pal_x509CertParse failed");
                
            }
        }

        //Verify certificate using created certificate chain
        pal_status = pal_x509CertVerify(x509_cert, x509_cert_chain);
        if (is_self_signed == false && is_chain_allocated == false) { // Shelly: for now we don't return UNTRUSTED error for not self-signed certificate without trusted chain
            if (pal_status == PAL_ERR_X509_BADCERT_NOT_TRUSTED) {
                SA_PV_LOG_INFO("Certificate not trusted ");
                pal_status = PAL_SUCCESS;
            }
        }
        SA_PV_ERR_RECOVERABLE_GOTO_IF((pal_status != PAL_SUCCESS), kcm_status = cs_error_handler(pal_status), exit, "pal_x509CertVerify failed %" PRIu32 "", pal_status);
    } //End of full verify
exit:
    pal_x509Free(&x509_cert);

    if (is_chain_allocated) {
        pal_x509Free(&x509_cert_chain);
    }
    return kcm_status;
}

kcm_status_e cs_is_self_signed_der_x509_cert(const uint8_t *cert, size_t cert_length, bool* is_self_signed)
{
    kcm_status_e kcm_status = KCM_STATUS_SUCCESS;
    palStatus_t pal_status = PAL_SUCCESS;
    palX509Handle_t x509_cert = NULLPTR;

    SA_PV_ERR_RECOVERABLE_RETURN_IF((cert == NULL), KCM_STATUS_INVALID_PARAMETER, "Invalid cert pointer");
    SA_PV_ERR_RECOVERABLE_RETURN_IF((cert_length <= 0), KCM_STATUS_INVALID_PARAMETER, "Invalid cert_length");
    SA_PV_ERR_RECOVERABLE_RETURN_IF((is_self_signed == NULL), KCM_STATUS_INVALID_PARAMETER, "Invalid is_self_signed pointer");

    //Allocate and Init certificate handler
    pal_status = pal_x509Initiate(&x509_cert);
    SA_PV_ERR_RECOVERABLE_RETURN_IF((pal_status != PAL_SUCCESS), cs_error_handler(pal_status), "pal_x509Initiate failed");

    //Parse Certificate.
    pal_status = pal_x509CertParse(x509_cert, cert, cert_length);
    SA_PV_ERR_RECOVERABLE_GOTO_IF((pal_status != PAL_SUCCESS), kcm_status = cs_error_handler(pal_status), exit, "pal_x509CertParse failed");

    kcm_status = is_self_signed_cert(x509_cert, is_self_signed);
    SA_PV_ERR_RECOVERABLE_GOTO_IF((kcm_status != KCM_STATUS_SUCCESS), kcm_status = kcm_status, exit, "Self signed verification failed");

exit:
    pal_x509Free(&x509_cert);

    return kcm_status;
}

static kcm_status_e cs_get_x509_cert_attribute_type(cs_certificate_attribute_type_e cs_attribute_type, palX509Attr_t *attribute_type)
{
    kcm_status_e kcm_status = KCM_STATUS_SUCCESS;


    switch (cs_attribute_type) {
        case CS_CN_ATTRIBUTE_TYPE:
            *attribute_type = PAL_X509_CN_ATTR;
            break;
        case CS_VALID_TO_ATTRIBUTE_TYPE:
            *attribute_type = PAL_X509_VALID_TO;
            break;
        case CS_VALID_FROM_ATTRIBUTE_TYPE:
            *attribute_type = PAL_X509_VALID_FROM;
            break;
        case CS_OU_ATTRIBUTE_TYPE:
            *attribute_type = PAL_X509_OU_ATTR;
            break;
        default:
            SA_PV_ERR_RECOVERABLE_RETURN_IF((true), KCM_CRYPTO_STATUS_INVALID_X509_ATTR, "Invalid cert attribute");
    }

    return kcm_status;
}
kcm_status_e  cs_attr_get_data_x509_cert(const uint8_t *cert,
                                         size_t cert_length,
                                         cs_certificate_attribute_type_e cs_attribute_type,
                                         uint8_t *attribute_output_buffer,
                                         size_t max_size_of_attribute_output_buffer,
                                         size_t *actual_size_of_attribute_output_buffer)
{
    kcm_status_e kcm_status = KCM_STATUS_SUCCESS;
    palX509Attr_t attribute_type;

    SA_PV_ERR_RECOVERABLE_RETURN_IF((cert == NULL), KCM_STATUS_INVALID_PARAMETER, "Invalid cert pointer");
    SA_PV_ERR_RECOVERABLE_RETURN_IF((cert_length <= 0), KCM_STATUS_INVALID_PARAMETER, "Invalid cert_length");
    SA_PV_ERR_RECOVERABLE_RETURN_IF((attribute_output_buffer == NULL), KCM_STATUS_INVALID_PARAMETER, "Invalid output pointer");
    SA_PV_ERR_RECOVERABLE_RETURN_IF((actual_size_of_attribute_output_buffer == NULL), KCM_STATUS_INVALID_PARAMETER, "Invalid actual_size_of_output pointer");

    kcm_status = cs_get_x509_cert_attribute_type(cs_attribute_type, &attribute_type);
    SA_PV_ERR_RECOVERABLE_RETURN_IF((kcm_status != KCM_STATUS_SUCCESS), kcm_status, "cs_get_x509_cert_attribute_type failed");

    kcm_status = cs_get_attribute_data_x509_cert(cert, cert_length, attribute_type, attribute_output_buffer, max_size_of_attribute_output_buffer, actual_size_of_attribute_output_buffer);
    SA_PV_ERR_RECOVERABLE_RETURN_IF((kcm_status != KCM_STATUS_SUCCESS), kcm_status, "cs_get_attribute_data_x509_cert failed");

    if (attribute_type == PAL_X509_CN_ATTR) {
        if (*actual_size_of_attribute_output_buffer > 0) {
            *actual_size_of_attribute_output_buffer = *actual_size_of_attribute_output_buffer - 1;
        }
    }

    return kcm_status;
};


