/*
 * Copyright (c) 2015 ARM Limited. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef M2M_BASE_STUB_H
#define M2M_BASE_STUB_H

#include "m2mbase.h"
#include "nsdlaccesshelper_stub.h"

//some internal test related stuff
namespace m2mbase_stub
{
    extern uint16_t uint16_value;
    extern uint32_t uint32_value;
    extern uint16_t int_value;
    extern int32_t name_id_value;
    extern const char *string_value;
    extern const char *object_instance_name;
    extern const char *resource_name;
    extern const char *resource_name_inst;
    extern char *path;
    extern bool free_path_member_on_delete;
    extern bool bool_value;
    extern M2MBase::BaseType base_type;
    extern M2MBase::Mode mode_value;
    extern M2MBase::Operation operation;
    extern void *void_value;
    extern M2MObservationHandler *observe;
    extern M2MReportHandler *report;
    extern M2MBase::Observation observation_level_value;
    extern bool is_value_updated_function_set;
    void clear();
    extern uint16_t object_msg_id;
    extern uint16_t resource_msg_id;
    extern uint16_t object_instance_msg_id;
    extern bool use_real_uri_path;
    extern int32_t ret_counter;
}

#endif // M2M_BASE_STUB_H

