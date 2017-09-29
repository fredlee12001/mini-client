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

#ifndef __FACTORY_CONFIGURATOR_CLIENT_H__
#define __FACTORY_CONFIGURATOR_CLIENT_H__

#include <stdlib.h>
#include <inttypes.h>
#include "fcc_status.h"
#include "fcc_output_info_handler.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
* @file factory_configurator_client.h
*  \brief factory configurator client APIs.
*/

/* === Initialization and Finalization === */

/** Initiates the FCC module.
*   Allocates and initializes file storage resources.
*
*   @returns
*       FCC_STATUS_SUCCESS in case of success or one of the `::fcc_status_e` errors otherwise.
*/
fcc_status_e fcc_init(void);


/** Finalizes the FCC module.
*   Finalizes and frees file storage resources.
*
*    @returns
*       FCC_STATUS_SUCCESS in case of success or one of the `::fcc_status_e` errors otherwise.
*/
fcc_status_e fcc_finalize(void);

/* === Factory clean operation === */

/** Cleans from the device all data that was saved during the factory process.
*  Should be called if the process failed and needs to be executed again.
*
*   @returns
*       FCC_STATUS_SUCCESS in case of success or one of the `::fcc_status_e` errors otherwise.
*/
fcc_status_e fcc_storage_delete(void);
/* === Warning and errors data operations === */

/** The function retrieves pointer to warning and errors structure.
*  Should be called after fcc_verify_device_configured_4mbed_cloud, when possible warning and errors was
*  stored in the structure.
*  The structure contains data of last fcc_verify_device_configured_4mbed_cloud run.*
*   @returns pointer to fcc_output_info_s structure.
*/
fcc_output_info_s* fcc_get_error_and_warning_data(void);

/* === Verification === */

/** Verifies that all mandatory fields needed to connect to mbed Cloud are in place on the device.
 *  Should be called in the end of the factory process
 *
 *    @returns
 *       FCC_STATUS_SUCCESS in case of success or one of the `::fcc_status_e` errors otherwise.
 */
fcc_status_e fcc_verify_device_configured_4mbed_cloud(void);

/* === Developer flow === */

/** This API is for developers only.
*   You can download the `mbed_cloud_dev_credentials.c` file from the portal and thus, skip running FCU on PC side.
*   The API reads all credentials from the `mbed_cloud_dev_credentials.c` file and stores them in the KCM.
*
*   @returns
*       FCC_STATUS_SUCCESS in case of success or one of the `::fcc_status_e` errors otherwise.
*/
fcc_status_e fcc_developer_flow(void);

#ifndef __DOXYGEN__
/* === CSR generation === */

/** Generates bootstrap CSR from a given private and public keys in DER encoding scheme.
*   Further design is needed
*
*     @param key_name The key name to fetch from storage(public/private).
*     @param key_name_len The key name len.
*     @param bootstrap_csr_out Pointer to generated bootstrap CSR.
*     @param bootstrap_csr_size_out Size of the CSR.
*
*     @returns
*        Operation status.
*/
fcc_status_e fcc_bootstrap_csr_generate(const uint8_t *key_name, size_t key_name_len,
        uint8_t **bootstrap_csr_out, size_t *bootstrap_csr_size_out);


/** Generates E2E CSR from a given private and public keys
*   Further design is needed
*
*     @param key_name The key name to fetch from storage(public/private).
*     @param key_name_len The key name len.
*     @param e2e_csr_out Pointer to generated E2E CSR.
*     @param e2e_csr_size_out Size of the E2E CSR.
*
*     @returns
*        Operation status.
*/
fcc_status_e fcc_e2e_csr_generate(const uint8_t *key_name, size_t key_name_len,
                                    uint8_t **e2e_csr_out, size_t *e2e_csr_size_out);
#endif
#ifdef __cplusplus
}
#endif

#endif //__FACTORY_CONFIGURATOR_CLIENT_H__
