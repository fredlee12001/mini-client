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

#ifndef M2M_TIMER_PIMPL_H__
#define M2M_TIMER_PIMPL_H__

#include "ns_types.h"
#include "mbed-client/m2mtimerobserver.h"
#include "mbed-client/m2mcorememory.h"

class M2MTimerPimpl : public M2MCoreMemory {
private:

    // Prevents the use of assignment operator
    M2MTimerPimpl& operator=(const M2MTimerPimpl& other);

    // Prevents the use of copy constructor
    M2MTimerPimpl(const M2MTimerPimpl& other);
public:

    /**
     * Constructor.
     */
    M2MTimerPimpl(M2MTimerObserver& _observer);

    /**
     * Destructor.
     */
    virtual ~M2MTimerPimpl();

    /**
     * Starts timer
     * @param interval Timer's interval in milliseconds
     * @param single_shot defines if timer is ticked
     * once or is it restarted everytime timer is expired.
     */
    void start_timer(uint32_t interval, M2MTimerObserver::Type type, bool single_shot = true);

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
     * @brief Start long period timer
     */
    void start_still_left_timer();

    /**
     * @brief Get timer id
     * @return Timer id
     */
    inline int8_t get_timer_id() const;

    /**
     * @brief Get still left time
     * @return Time left in milliseconds
     */
    uint32_t get_still_left_time() const;

private:

    void start();
    void cancel();

private:
    M2MTimerObserver&       _observer;
    uint32_t                _interval;
    uint32_t                _still_left;

    // this is the timer-id of this object, used to map the
    // timer event callback to the correct object.
    int8_t                  _timer_id;

    static int8_t           _tasklet_id;
    static int8_t           _next_timer_id;
    M2MTimerObserver::Type  _type:4;
    bool                    _single_shot:1;


    friend class M2MTimer;
    friend class Test_M2MTimerPimpl_classic;
};

inline int8_t M2MTimerPimpl::get_timer_id() const
{
    return _timer_id;
}

#endif //M2M_TIMER_PIMPL_H__

