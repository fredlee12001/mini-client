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

#ifndef CERTIFICATE_PARSER_H
#define CERTIFICATE_PARSER_H

#include "ns_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
*  \brief A utility function to extract CN field from the mDS certificate and store it to KCM.
*  \param certificate, The certificate to be extracted.
*  \param common_name [OUT], buffer containing CN value. Maximum common name can be 64 bytes.
*  \return True if success, False if failure.
*/
bool extract_cn_from_certificate(const uint8_t* cer, size_t cer_len, char *common_name);

#ifdef __cplusplus
}
#endif
#endif // CERTIFICATE_PARSER_H
