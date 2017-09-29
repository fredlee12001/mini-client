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
#include <stdio.h>
#include "pv_log.h"
#include "cs_hash.h"
#include "pal_Crypto.h"
#include "pal_errors.h"


kcm_status_e cs_error_handler(palStatus_t pal_status)
{
    switch (pal_status) {
        case PAL_SUCCESS:
            return KCM_STATUS_SUCCESS;
        case PAL_ERR_NOT_SUPPORTED_CURVE:
            return KCM_CRYPTO_STATUS_UNSUPPORTED_CURVE;
        case PAL_ERR_INVALID_ARGUMENT:
            return KCM_STATUS_INVALID_PARAMETER;
        case PAL_ERR_CREATION_FAILED:
            return KCM_STATUS_OUT_OF_MEMORY;
        case PAL_ERR_CERT_PARSING_FAILED:
            return KCM_CRYPTO_STATUS_PARSING_DER_CERT;
        case PAL_ERR_X509_BADCERT_EXPIRED:
            return KCM_CRYPTO_STATUS_CERT_EXPIRED;
        case PAL_ERR_X509_BADCERT_FUTURE:
            return KCM_CRYPTO_STATUS_CERT_FUTURE;
        case PAL_ERR_X509_BADCERT_BAD_MD:
            return KCM_CRYPTO_STATUS_CERT_MD_ALG;
        case PAL_ERR_X509_BADCERT_BAD_PK:
            return KCM_CRYPTO_STATUS_CERT_PUB_KEY_TYPE;
        case PAL_ERR_X509_BADCERT_NOT_TRUSTED:
            return KCM_CRYPTO_STATUS_CERT_NOT_TRUSTED;
        case PAL_ERR_X509_BADCERT_BAD_KEY:
            return KCM_CRYPTO_STATUS_CERT_PUB_KEY;
        case PAL_ERR_PARSING_PUBLIC_KEY:
            return KCM_CRYPTO_STATUS_PARSING_DER_PUBLIC_KEY;
        case PAL_ERR_PARSING_PRIVATE_KEY:
            return KCM_CRYPTO_STATUS_PARSING_DER_PRIVATE_KEY;
        case PAL_ERR_PRIVATE_KEY_VARIFICATION_FAILED:
            return KCM_CRYPTO_STATUS_PRIVATE_KEY_VERIFICATION_FAILED;
        case PAL_ERR_PUBLIC_KEY_VARIFICATION_FAILED:
             return KCM_CRYPTO_STATUS_PUBLIC_KEY_VERIFICATION_FAILED;
        case PAL_ERR_PK_UNKNOWN_PK_ALG:
            return KCM_CRYPTO_STATUS_PK_UNKNOWN_PK_ALG;
        case PAL_ERR_PK_KEY_INVALID_FORMAT:
            return KCM_CRYPTO_STATUS_PK_KEY_INVALID_FORMAT;
        case PAL_ERR_PK_INVALID_PUBKEY_AND_ASN1_LEN_MISMATCH:
            return KCM_CRYPTO_STATUS_INVALID_PK_PUBKEY;
        case PAL_ERR_ECP_INVALID_KEY:
            return KCM_CRYPTO_STATUS_ECP_INVALID_KEY;
        case  PAL_ERR_PK_KEY_INVALID_VERSION:
            return KCM_CRYPTO_STATUS_PK_KEY_INVALID_VERSION;
        case PAL_ERR_PK_PASSWORD_REQUIRED:
            return KCM_CRYPTO_STATUS_PK_PASSWORD_REQUIRED;
        case PAL_ERR_NO_MEMORY:
            return KCM_STATUS_OUT_OF_MEMORY;
        case PAL_ERR_BUFFER_TOO_SMALL:
            return KCM_STATUS_INSUFFICIENT_BUFFER;
        case PAL_ERR_INVALID_X509_ATTR:
            return KCM_CRYPTO_STATUS_INVALID_X509_ATTR;         
        default:
           return  KCM_STATUS_ERROR;
    }
}
