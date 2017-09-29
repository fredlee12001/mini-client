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

#ifndef M2M_DTLS_TIMER_PIMPL_H__
#define M2M_DTLS_TIMER_PIMPL_H__

#include "ns_types.h"
#include "mbed-client/m2mtimerobserver.h"
#include "mbed-client/m2mcorememory.h"

class M2MDtlsTimerPimpl : public M2MCoreMemory {
private:

    // Prevents the use of assignment operator
    M2MDtlsTimerPimpl& operator=(const M2MDtlsTimerPimpl& other);

    // Prevents the use of copy constructor
    M2MDtlsTimerPimpl(const M2MDtlsTimerPimpl& other);
public:

    /**
     * Constructor.
     */
    M2MDtlsTimerPimpl(M2MTimerObserver& _observer);

    /**
     * Destructor.
     */
    virtual ~M2MDtlsTimerPimpl();

    /**
     * @brief Starts timer in DTLS manner
     * @param intermediate_interval Intermediate interval to use, must be smaller than tiotal (usually 1/4 of total)
     * @param total_interval Total interval to use; This is the timeout value of a DTLS packet
     */
    void start_timer(uint32_t intermediate_interval, uint32_t total_interval);

    /**
     * Stops timer.
     * This cancels the ongoing timer.
     */
    void stop_timer();

    /**
     * Callback function for timer completion.
     */
    void timer_expired();

    /**
     * @brief Checks if the intermediate interval has passed
     * @return true if interval has passed, false otherwise
     */
    bool is_intermediate_interval_passed();

    /**
     * @brief Checks if the total interval has passed
     * @return true if interval has passed, false otherwise
     */
    bool is_total_interval_passed();

    /**
     * @brief Get timer id
     * @return Timer id
     */
    inline int8_t get_timer_id() const;

private:
    M2MTimerObserver&   _observer;
    uint32_t            _left_interval;

    // this is the timer-id of this object, used to map the
    // timer event callback to the correct object.
    int8_t              _timer_id;

    static int8_t       _tasklet_id;
    static int8_t       _next_timer_id;
    uint8_t             _status:2;

    friend class M2MTimer;
    friend class Test_M2MDtlsimerPimpl_classic;
};

inline int8_t M2MDtlsTimerPimpl::get_timer_id() const
{
    return _timer_id;
}

#endif //M2M_DTLS_TIMER_PIMPL_H__
