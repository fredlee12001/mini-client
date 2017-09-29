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

#include <stdint.h>
#include "pal.h"
#include "mbed-trace/mbed_trace.h"

#define TRACE_GROUP "cert_parser"

bool extract_cn_from_certificate(const uint8_t* cer, size_t cer_len, char* common_name)
{
    tr_debug("extract_cn_from_certificate");
    palX509Handle_t cert;
    size_t len;
    palStatus_t ret;

    if(PAL_SUCCESS != (ret = pal_x509Initiate(&cert))) {
        tr_error("extract_cn_from_certificate - cert init failed: %d", (int)ret);
        pal_x509Free(&cert);
        return false;
    }
    if(PAL_SUCCESS != (ret = pal_x509CertParse(cert, cer, cer_len))) {
        tr_error("extract_cn_from_certificate - cert parse failed: %d", (int)ret);
        pal_x509Free(&cert);
        return false;
    }
    if(PAL_SUCCESS != (ret = pal_x509CertGetAttribute(cert, PAL_X509_CN_ATTR, common_name, 64, &len))) {
        tr_error("extract_cn_from_certificate - cert attr get failed: %d", (int)ret);
        pal_x509Free(&cert);
        return false;
    }

    pal_x509Free(&cert);
    return true;
}

