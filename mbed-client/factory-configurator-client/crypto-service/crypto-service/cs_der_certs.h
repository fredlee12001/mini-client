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

#ifndef __CS_DER_CERTS_H__
#define __CS_DER_CERTS_H__

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include "kcm_status.h"

/*
* Types certificate's attributes
*/
typedef enum cs_certificate_attribute_type_ {
    CS_CN_ATTRIBUTE_TYPE,
    CS_VALID_FROM_ATTRIBUTE_TYPE,
    CS_VALID_TO_ATTRIBUTE_TYPE,
    CS_OU_ATTRIBUTE_TYPE,
    CS_MAX_ATTRIBUTE_TYPE
} cs_certificate_attribute_type_e;


//extern const char g_fcc_certificate_cn_attribute[];
//extern const char g_fcc_certificate_valid_to_attribute[];
//extern const char g_fcc_certificate_valid_from_attribute[];

/**Verify x509 certificate in DER format.
* Currently support PEM format too, this feature should be removed in the future.
* In case ca_certificate is present, the function will verify current certificate against given ca certificate.
* Otherwise the function will retrieve certificate chain and verify the certificate against the retrieved data.
*
*@cert[in] – DER format certificate.
*@cert_length[in] - certificate length
*@full_verify[in] - if the value is true full certificate verification will be performed, 
*               otherwise the function will only parse the certificate to check that all mandatory fields exists.
*@ca_cert[in] – DER format ca certificate.Can be NULL
*@ca_cert_length[in] - ca certificate length.Must be 0 in case ca_cert is NULL.
* @return
*     KCM_STATUS_SUCCESS on success, otherwise appropriate error from  kcm_status_e.
*/
kcm_status_e cs_verify_der_x509_cert(const uint8_t *cert, size_t cert_length, bool full_verify, const uint8_t *ca_cert, size_t ca_cert_length);

/**Verify that x509 certificate in DER format is self-signed.
*
*@cert[in] – DER format certificate.
*@cert_length[in] – certificate length
*@is_self_signed[out] - if the value is true the certificate is self-signed, otherwise not self-sined
* @return
*     KCM_STATUS_SUCCESS on success, otherwise appropriate error from  kcm_status_e.
*/
kcm_status_e cs_is_self_signed_der_x509_cert(const uint8_t *cert, size_t cert_length, bool* is_self_signed);


/**Gets current attribute from certificate
*
*@cert[in] – DER format certificate.
*@cert_length[in] – certificate length.
*@cs_attribute_type[in] - certificate attribute type
*@attribute_output_buffer[out] -pointer to output attribute buffer.
*@max_size_of_attribute_output_buffer[in] -size of output attribute buffer.
*@actual_size_of_attribute_output_buffer[out] -actual size of attribute.

*
* note in case of "KCM_STATUS_INSUFFICIENT_BUFFER" error the required size will be assigned into the "actual_size_of_output" parameter.
* @return
*     KCM_STATUS_SUCCESS on success, otherwise appropriate error from  kcm_status_e.
*/

kcm_status_e  cs_attr_get_data_x509_cert(const uint8_t *cert,
                                         size_t cert_length,
                                         cs_certificate_attribute_type_e cs_attribute_type,
                                         uint8_t *attribute_output_buffer,
                                         size_t max_size_of_attribute_output_buffer,
                                         size_t *actual_size_of_attribute_output_buffer);

#ifdef __cplusplus
}
#endif

#endif  // __CS_DER_CERTS_H__

