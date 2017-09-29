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
#include "mbed-client/m2mdtlstimer.h"
#include "mbed-client/m2mtimerobserver.h"


M2MDtlsTimer::M2MDtlsTimer(M2MTimerObserver& observer)
: _observer(observer),
 _private_impl(observer)
{
}

M2MDtlsTimer::~M2MDtlsTimer()
{
}

void M2MDtlsTimer::start_timer(uint32_t intermediate_interval, uint32_t total_interval){
    _private_impl.start_timer(intermediate_interval, total_interval);
}

void M2MDtlsTimer::stop_timer()
{
    _private_impl.stop_timer();
}

bool M2MDtlsTimer::is_intermediate_interval_passed(){
    return _private_impl.is_intermediate_interval_passed();
}

bool M2MDtlsTimer::is_total_interval_passed(){
    return _private_impl.is_total_interval_passed();
}
