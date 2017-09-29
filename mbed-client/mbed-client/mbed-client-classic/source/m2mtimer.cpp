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
#include "mbed-client/m2mtimer.h"
#include "mbed-client/m2mtimerobserver.h"


M2MTimer::M2MTimer(M2MTimerObserver& observer)
:_private_impl(observer)
{
}

M2MTimer::~M2MTimer()
{
}

void M2MTimer::start_timer( uint32_t interval,
                            M2MTimerObserver::Type type,
                            bool single_shot)
{
    _private_impl.start_timer(interval,
                               type,
                               single_shot);
}

void M2MTimer::stop_timer()
{
    _private_impl.stop_timer();
}
