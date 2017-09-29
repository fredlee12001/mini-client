/*
 * Copyright (c) 2016 ARM Limited. All rights reserved.
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
#ifndef M2M_TIMER_PIMPL_STUB_H
#define M2M_TIMER_PIMPL_STUB_H

#include "mbed-client-classic/m2mtimerpimpl.h"

//some internal test related stuff
namespace m2mtimerpimpl_stub
{
    extern bool bool_value;
    extern bool visited;
    void clear();
}

#endif // M2M_TIMER_IMPL_STUB_H
