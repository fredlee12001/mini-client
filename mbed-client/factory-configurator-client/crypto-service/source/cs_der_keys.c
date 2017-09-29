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
#include "cs_der_keys.h"
#include "pal.h"
#include "cs_utils.h"

//For now only EC keys supported!!!
static kcm_status_e der_key_verify(const uint8_t *der_key, size_t der_key_length, palKeyToCheck_t key_type)
{
    kcm_status_e kcm_status = KCM_STATUS_SUCCESS;
    palStatus_t pal_status = PAL_SUCCESS;
    palECKeyHandle_t keyHandle = NULLPTR;
    palCurveHandle_t grp = NULLPTR;
    palGroupIndex_t pal_grp_idx;
    bool verified = false;


    SA_PV_ERR_RECOVERABLE_RETURN_IF((der_key == NULL), KCM_STATUS_INVALID_PARAMETER, "Invalid der_key pointer");
    SA_PV_ERR_RECOVERABLE_RETURN_IF((der_key_length <= 0), KCM_STATUS_INVALID_PARAMETER, "Invalid der_key_length");

    //Create new key handler
    pal_status = pal_ECKeyNew(&keyHandle);
    SA_PV_ERR_RECOVERABLE_RETURN_IF((PAL_SUCCESS != pal_status), cs_error_handler(pal_status), "pal_ECKeyNew failed ");

    //Parse the key from DER format
    if (key_type == PAL_CHECK_PRIVATE_KEY) {
        pal_status = pal_parseECPrivateKeyFromDER(der_key, der_key_length, keyHandle);
        SA_PV_ERR_RECOVERABLE_GOTO_IF((PAL_SUCCESS != pal_status), kcm_status = cs_error_handler(pal_status), exit, "pal_parseECPrivateKeyFromDER failed ");
    } else {
        pal_status = pal_parseECPublicKeyFromDER(der_key, der_key_length, keyHandle);
        SA_PV_ERR_RECOVERABLE_GOTO_IF((PAL_SUCCESS != pal_status), kcm_status = cs_error_handler(pal_status), exit, "pal_parseECPublicKeyFromDER failed ");
    }

    //retrieve key curve from key handle
    pal_status = pal_ECKeyGetCurve(keyHandle, &pal_grp_idx);
    SA_PV_ERR_RECOVERABLE_GOTO_IF((PAL_SUCCESS != pal_status), kcm_status = cs_error_handler(pal_status), exit, "pal_ECKeyGetCurve failed ");

    //Allocate curve handler
    pal_status = pal_ECGroupInitAndLoad(&grp, pal_grp_idx);
    SA_PV_ERR_RECOVERABLE_GOTO_IF((PAL_SUCCESS != pal_status), kcm_status = cs_error_handler(pal_status), exit, "pal_parseECPrivateKeyFromDER failed ");

    //Perform key verification
    pal_status = pal_ECCheckKey(grp, keyHandle, key_type, &verified);
    SA_PV_ERR_RECOVERABLE_GOTO_IF(((PAL_SUCCESS != pal_status) || (verified != true)), kcm_status = cs_error_handler(pal_status), exit, "pal_ECCheckKey failed ");


exit:
    //Free curve handle
    (void)pal_ECGroupFree(&grp);
    //Free key handler
    (void)pal_ECKeyFree(&keyHandle);

    if (kcm_status == KCM_STATUS_SUCCESS) {//Shelly should be uncomment after bug fixing
        SA_PV_ERR_RECOVERABLE_RETURN_IF((grp != NULLPTR /*|| keyHandle != NULLPTR*/), KCM_STATUS_ERROR, "Free handle failed ");
    }

    return kcm_status;
}

kcm_status_e cs_der_priv_key_verify(const uint8_t *key, size_t key_length)
{
    kcm_status_e kcm_status = KCM_STATUS_SUCCESS;

    kcm_status =  der_key_verify(key, key_length, PAL_CHECK_PRIVATE_KEY);
    SA_PV_ERR_RECOVERABLE_RETURN_IF((kcm_status != KCM_STATUS_SUCCESS), kcm_status, "Private key verification failed");

    return kcm_status;
}

kcm_status_e cs_der_public_key_verify(const uint8_t *der_key, size_t der_key_length)
{
    kcm_status_e kcm_status = KCM_STATUS_SUCCESS;

    kcm_status = der_key_verify(der_key, der_key_length, PAL_CHECK_PUBLIC_KEY);
    SA_PV_ERR_RECOVERABLE_RETURN_IF((kcm_status != KCM_STATUS_SUCCESS), kcm_status, "Public key verification failed");

    return kcm_status;
}

