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

#ifndef __CS_DER_KEYS_H__
#define __CS_DER_KEYS_H__

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#include "kcm_status.h"

typedef enum {
    CS_SECP256R1
} cs_curve_name_e;



/**Verify private Key In DER format. For now only EC keys supported
*
*@key_data – DER format private key data.
*@key_data_length – key data size
* @return
*     KCM_STATUS_SUCCESS on success, otherwise appropriate error from  kcm_status_e.
*/

kcm_status_e cs_der_priv_key_verify(const uint8_t* key, size_t key_length);

/**Verify public Key In DER format. For now only EC keys supported
*
*@key_data – DER format puclic key data.
*@key_data_length – key data size
* @return
*     KCM_STATUS_SUCCESS on success, otherwise appropriate error from  kcm_status_e.
*/

kcm_status_e cs_der_public_key_verify(const uint8_t* key, size_t key_length);

#ifdef __cplusplus
}
#endif

#endif  //__CS_DER_KEYS_H__

