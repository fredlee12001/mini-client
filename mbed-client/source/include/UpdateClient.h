//----------------------------------------------------------------------------
// The confidential and proprietary information contained in this file may
// only be used by a person authorised under and to the extent permitted
// by a subsisting licensing agreement from ARM Limited or its affiliates.
//
// (C) COPYRIGHT 2016 ARM Limited or its affiliates.
// ALL RIGHTS RESERVED
//
// This entire notice must be reproduced on all copies of this file
// and copies of this file may only be made by a person if such person is
// permitted to do so under the terms of a subsisting license agreement
// from ARM Limited or its affiliates.
//----------------------------------------------------------------------------

#ifndef MBED_CLOUD_CLIENT_UPDATE_CLIENT_H
#define MBED_CLOUD_CLIENT_UPDATE_CLIENT_H

#ifdef MBED_CLOUD_CLIENT_USER_CONFIG_FILE
#include MBED_CLOUD_CLIENT_USER_CONFIG_FILE
#endif

#ifdef MBED_CLOUD_CLIENT_SUPPORT_UPDATE
#include "mbed-client/m2minterface.h"
#include "update-client-hub/update_client_public.h"

namespace UpdateClient
{
    /**
     * Error codes used by the Update Client.
     *
     * Warning: a recoverable error occured, no user action required.
     * Error  : a recoverable error occured, action required. E.g. the
     *          application has to free some space and let the Update
     *          Service try again.
     * Fatal  : a non-recoverable error occured, application should safe
     *          ongoing work and reboot the device.
     */
    enum {
        WarningBase                     = 0x0400, // Range reserved for Update Error from 0x0400 - 0x04FF
        WarningCertificateNotFound      = WarningBase + ARM_UC_WARNING_CERTIFICATE_NOT_FOUND,
        WarningIdentityNotFound         = WarningBase + ARM_UC_WARNING_IDENTITY_NOT_FOUND,
        WarningVendorMismatch           = WarningBase + ARM_UC_WARNING_VENDOR_MISMATCH,
        WarningClassMismatch            = WarningBase + ARM_UC_WARNING_CLASS_MISMATCH,
        WarningDeviceMismatch           = WarningBase + ARM_UC_WARNING_DEVICE_MISMATCH,
        WarningCertificateInvalid       = WarningBase + ARM_UC_WARNING_CERTIFICATE_INVALID,
        WarningSignatureInvalid         = WarningBase + ARM_UC_WARNING_SIGNATURE_INVALID,
        WarningURINotFound              = WarningBase + ARM_UC_WARNING_URI_NOT_FOUND,
        WarningRollbackProtection       = WarningBase + ARM_UC_WARNING_ROLLBACK_PROTECTION,
        WarningUnknown                  = WarningBase + ARM_UC_WARNING_UNKNOWN,
        WarningCertificateInsertion,
        ErrorBase,
        ErrorWriteToStorage             = ErrorBase + ARM_UC_ERROR_WRITE_TO_STORAGE,
        FatalBase
    };

    enum {
        RequestInvalid                  = ARM_UCCC_REQUEST_INVALID,
        RequestDownload                 = ARM_UCCC_REQUEST_DOWNLOAD,
        RequestInstall                  = ARM_UCCC_REQUEST_INSTALL
    };

    /**
     * \brief Initialization function for the Update Client.
     * \param Callback to error handler.
     */
    void UpdateClient(FP1<void, int32_t> callback);

    /**
     * \brief Populate M2MObjectList with Update Client objects.
     * \details The function takes an existing object list and adds LWM2M
     *          objects needed by the Update Client.
     *
     * \param list M2MObjectList reference.
     */
    void populate_object_list(M2MObjectList& list);

    /**
     * \brief Registers a callback function for authorizing firmware downloads and reboots.
     * \param handler Callback function.
     */
    void set_update_authorize_handler(void (*handler)(int32_t request));

    /**
     * \brief Authorize request passed to authorization handler.
     * \param request Request being authorized.
     */
    void update_authorize(int32_t request);

    /**
     * \brief Registers a callback function for monitoring download progress.
     * \param handler Callback function.
     */
    void set_update_progress_handler(void (*handler)(uint32_t progress, uint32_t total));
}
#endif //#ifdef MBED_CLOUD_CLIENT_SUPPORT_UPDATE

#endif // MBED_CLOUD_CLIENT_UPDATE_CLIENT_H
