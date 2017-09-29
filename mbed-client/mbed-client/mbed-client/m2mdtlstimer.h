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
#ifndef M2M_DTLS_TIMER_H
#define M2M_DTLS_TIMER_H

#include <stdint.h>
#include "mbed-client/m2mtimerobserver.h"
#include "mbed-client/m2mcorememory.h"
#include "mbed-client-classic/m2mdtlstimerpimpl.h"


/*! \file m2mdtlstimer.h
* \brief M2MDtlsTimer.
* DTLS Timer class for mbed client.
*/
class M2MDtlsTimer : public M2MCoreMemory {
private:
    // Prevents the use of assignment operator
    M2MDtlsTimer& operator=(const M2MDtlsTimer& other);

    // Prevents the use of copy constructor
    M2MDtlsTimer(const M2MDtlsTimer& other);

public:

    /**
    * Constructor.
    */
    M2MDtlsTimer(M2MTimerObserver& observer);

    /**
    * Destructor.
    */
    ~M2MDtlsTimer();

    /**
     * \brief Starts the timer in DTLS manner.
     * \param intermediate_interval The intermediate interval to use, must be smaller than total (usually 1/4 of total).
     * \param total_interval The total interval to use; This is the timeout value of a DTLS packet.
     * \param type The type of the timer.
     */
    void start_timer(uint32_t intermediate_interval, uint32_t total_interval);

    /**
    * \brief Stops the timer.
    * This cancels the ongoing timer.
    */
    void stop_timer();

    /**
     * \brief Checks if the intermediate interval has passed.
     * \return True if the interval has passed, else false.
     */
    bool is_intermediate_interval_passed();

    /**
     * \brief Checks if the total interval has passed.
     * \return True if the interval has passed, else false.
     */
    bool is_total_interval_passed();

private:

    M2MTimerObserver&   _observer;
    M2MDtlsTimerPimpl   _private_impl;
    friend class Test_M2MDtlsTimerImpl_linux;
};

#endif // M2M_DTLS_TIMER_H
