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

#include "factory_configurator_client.h"
#include "key_config_manager.h"
#include "pv_error_handling.h"
#include "fcc_verification.h"
#include "storage.h"
#include "fcc_defs.h"
/**
* Device general info
*/
const char g_fcc_use_bootstrap_parameter_name[] = "mbed.UseBootstrap";
const char g_fcc_endpoint_parameter_name[] = "mbed.EndpointName";
const char g_fcc_account_id_parameter_name[] = "mbed.AccountID";
/**
* Device meta data
*/
const char g_fcc_manufacturer_parameter_name[] = "mbed.Manufacturer";
const char g_fcc_model_number_parameter_name[] = "mbed.ModelNumber";
const char g_fcc_device_type_parameter_name[] = "mbed.DeviceType";
const char g_fcc_hardware_version_parameter_name[] = "mbed.HardwareVersion";
const char g_fcc_memory_size_parameter_name[] = "mbed.MemoryTotalKB";
const char g_fcc_device_serial_number_parameter_name[] = "mbed.SerialNumber";
/**
* Time Synchronization
*/
const char g_fcc_current_time_parameter_name[] = "mbed.CurrentTime";
const char g_fcc_device_time_zone_parameter_name[] = "mbed.Timezone";
const char g_fcc_offset_from_utc_parameter_name[] = "mbed.UTCOffset";
/**
* Bootstrap configuration
*/
const char g_fcc_bootstrap_server_ca_certificate_name[] = "mbed.BootstrapServerCACert";
const char g_fcc_bootstrap_server_crl_name[] = "mbed.BootstrapServerCRL";
const char g_fcc_bootstrap_server_uri_name[] = "mbed.BootstrapServerURI";
const char g_fcc_bootstrap_device_certificate_name[] = "mbed.BootstrapDeviceCert";
const char g_fcc_bootstrap_device_private_key_name[] = "mbed.BootstrapDevicePrivateKey";
/**
* LWm2m configuration
*/
const char g_fcc_lwm2m_server_ca_certificate_name[] = "mbed.LwM2MServerCACert";
const char g_fcc_lwm2m_server_crl_name[] = "mbed.LwM2MServerCRL";
const char g_fcc_lwm2m_server_uri_name[] = "mbed.LwM2MServerURI";
const char g_fcc_lwm2m_device_certificate_name[] = "mbed.LwM2MDeviceCert";
const char g_fcc_lwm2m_device_private_key_name[] = "mbed.LwM2MDevicePrivateKey";
/**
* Firmware integrity
*/
const char g_fcc_firmware_integrity_ca_certificate_name[] = "mbed.FirmwareIntegrityCACert";
const char g_fcc_firmware_integrity_certificate_name[] = "mbed.FirmwareIntegrityCert";

static bool g_is_fcc_initialized = false;

fcc_status_e fcc_init(void)
{
    kcm_status_e status;

    SA_PV_LOG_INFO_FUNC_ENTER_NO_ARGS();

    if (g_is_fcc_initialized) {
        // No need for second initialization
        return FCC_STATUS_SUCCESS;
    }

    //FIXME: add relevant error handling - general task for all APIs.
    status = kcm_init();
    SA_PV_ERR_RECOVERABLE_RETURN_IF((status == KCM_STATUS_STORAGE_ERROR), FCC_STATUS_KCM_STORAGE_ERROR, "Failed init KCM, got ESFS error");
    SA_PV_ERR_RECOVERABLE_RETURN_IF((status != KCM_STATUS_SUCCESS), FCC_STATUS_ERROR, "Failed init KCM");

    //Initialize output info handler
    fcc_init_output_info_handler();

    g_is_fcc_initialized = true;

    SA_PV_LOG_INFO_FUNC_EXIT_NO_ARGS();

    return FCC_STATUS_SUCCESS;
}

fcc_status_e fcc_finalize(void)
{
    kcm_status_e status;

    SA_PV_LOG_INFO_FUNC_ENTER_NO_ARGS();

    SA_PV_ERR_RECOVERABLE_RETURN_IF((!g_is_fcc_initialized), FCC_STATUS_NOT_INITIALIZED, "FCC not initialized");

    //FIXME: add relevant error handling - general task for all APIs.
    status = kcm_finalize();
    SA_PV_ERR_RECOVERABLE_RETURN_IF((status == KCM_STATUS_NOT_INITIALIZED), FCC_STATUS_NOT_INITIALIZED, "Failed finalize KCM\n");
    SA_PV_ERR_RECOVERABLE_RETURN_IF((status != KCM_STATUS_SUCCESS), FCC_STATUS_ERROR, "Failed finalize KCM");

    //Finalize output info handler
    fcc_clean_output_info_handler();

    g_is_fcc_initialized = false;

    SA_PV_LOG_INFO_FUNC_EXIT_NO_ARGS();

    return FCC_STATUS_SUCCESS;
}

fcc_status_e fcc_storage_delete()
{
    kcm_status_e status = KCM_STATUS_SUCCESS;

    SA_PV_LOG_INFO_FUNC_ENTER_NO_ARGS();

    SA_PV_ERR_RECOVERABLE_RETURN_IF((!g_is_fcc_initialized), FCC_STATUS_NOT_INITIALIZED, "FCC not initialized");

    status = storage_reset();
    SA_PV_ERR_RECOVERABLE_RETURN_IF((status == KCM_STATUS_ESFS_ERROR), FCC_STATUS_KCM_STORAGE_ERROR, "Failed init KCM. got ESFS error");
    SA_PV_ERR_RECOVERABLE_RETURN_IF((status != KCM_STATUS_SUCCESS), FCC_STATUS_ERROR, "Failed storage reset");

    SA_PV_LOG_INFO_FUNC_EXIT_NO_ARGS();
    return FCC_STATUS_SUCCESS;
}


fcc_output_info_s* fcc_get_error_and_warning_data(void)
{
    SA_PV_LOG_INFO_FUNC_ENTER_NO_ARGS();

    SA_PV_ERR_RECOVERABLE_RETURN_IF((!g_is_fcc_initialized), NULL, "FCC not initialized");

    return get_output_info();
}

fcc_status_e fcc_verify_device_configured_4mbed_cloud(void)
{
    fcc_status_e  fcc_status =  FCC_STATUS_SUCCESS;
    bool use_bootstrap = false;
    bool success = false;

    SA_PV_LOG_INFO_FUNC_ENTER_NO_ARGS();

    SA_PV_ERR_RECOVERABLE_RETURN_IF((!g_is_fcc_initialized), FCC_STATUS_NOT_INITIALIZED, "FCC not initialized");

    /*Initialize fcc_output_info_s structure. 
    In case output indo struct is not empty in the beginning of the verify process we will clean it.*/
    fcc_clean_output_info_handler();

    //Check entropy initialization
    success = fcc_is_entropy_initialized();
    SA_PV_ERR_RECOVERABLE_RETURN_IF((success != true), fcc_status = FCC_STATUS_ENTROPY_ERROR, "Entropy is not initialized");

    //Check time synchronization
    fcc_status = fcc_check_time_synchronization();
    SA_PV_ERR_RECOVERABLE_RETURN_IF((fcc_status != FCC_STATUS_SUCCESS), fcc_status, "Failed to check time synhronization");

    //Get bootstrap mode
    fcc_status = fcc_get_bootstrap_mode(&use_bootstrap);
    SA_PV_ERR_RECOVERABLE_RETURN_IF((fcc_status != FCC_STATUS_SUCCESS), fcc_status , "Failed to get bootstrap mode");

    // Check general info
    fcc_status = fcc_check_device_general_info();
    SA_PV_ERR_RECOVERABLE_RETURN_IF((fcc_status != FCC_STATUS_SUCCESS), fcc_status , "Failed to check general info");

    //Check device meta-data
    fcc_status = fcc_check_device_meta_data();
    SA_PV_ERR_RECOVERABLE_RETURN_IF((fcc_status != FCC_STATUS_SUCCESS), fcc_status, "Failed to check configuration parameters");

    //Check device security objects
    fcc_status = fcc_check_device_security_objects(use_bootstrap);
    SA_PV_ERR_RECOVERABLE_RETURN_IF((fcc_status != FCC_STATUS_SUCCESS), fcc_status, "Failed to check device security objects");

    //Check firmware integrity
    fcc_status = fcc_check_firmware_update_integrity();
    SA_PV_ERR_RECOVERABLE_RETURN_IF((fcc_status != FCC_STATUS_SUCCESS), fcc_status, "Failed to check device security objects");
    
    SA_PV_LOG_INFO_FUNC_EXIT_NO_ARGS();
    
    return fcc_status;
}

