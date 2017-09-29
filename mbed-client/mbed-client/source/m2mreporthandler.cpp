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

// fixup the compilation on ARMCC for PRId32
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include "mbed-client/m2mreportobserver.h"
#include "mbed-client/m2mconstants.h"
#include "mbed-client/m2mtimer.h"
#include "include/m2mreporthandler.h"
#include "mbed-trace/mbed_trace.h"
#include <string.h>
#include <stdlib.h>

#define TRACE_GROUP "mClt"

M2MReportHandler::M2MReportHandler(M2MReportObserver &observer)
: _observer(observer),
  _is_under_observation(false),
  _observation_level(M2MBase::None),
  _attribute_state(0),
  _lwm2m_item(NULL),
  _token_length(0),
  _notify(false),
  _pmin_exceeded(false),
  _pmax_exceeded(false),
  _observation_number(1)
{
    tr_debug("M2MReportHandler::M2MReportHandler()");
    memset(_token, 0, 8);
}

M2MReportHandler::~M2MReportHandler()
{
    tr_debug("M2MReportHandler::~M2MReportHandler()");    
    lwm2m_free_attributes_list();
}

void M2MReportHandler::set_under_observation(bool observed)
{
    tr_debug("M2MReportHandler::set_under_observation(observed %d)", (int)observed);

    _is_under_observation = observed;

    stop_timers();
    if(observed) {
        handle_timers();
    }
    else {
        set_default_values();
    }
}

void M2MReportHandler::set_value(float value)
{
    tr_debug("M2MReportHandler::set_value() - current %f", value);
    tr_debug("M2MReportHandler::set_value() - UNDER OBSERVATION");
    if (check_threshold_values(value)) {
        schedule_report();
    }
    else {
        tr_debug("M2MReportHandler::set_value - value not in range");
        _notify = false;
        if ((_attribute_state & M2MReportHandler::Lt) == M2MReportHandler::Lt ||
                (_attribute_state & M2MReportHandler::Gt) == M2MReportHandler::Gt ||
                (_attribute_state & M2MReportHandler::St) == M2MReportHandler::St) {
            tr_debug("M2MReportHandler::set_value - stop pmin timer");
            lwm2m_item_s::lwm2m_value * timer_value = lwm2m_get_attribute(LWM2M_PMIN_TIMER);
            if(timer_value) {
                M2MTimer *timer = (M2MTimer*)timer_value->void_value;
                timer->stop_timer();
            }
            _pmin_exceeded = true;
        }
    }

    lwm2m_item_s::lwm2m_value * high_step = lwm2m_get_attribute(LWM2M_HIGH_STEP);
    lwm2m_item_s::lwm2m_value * st = lwm2m_get_attribute(LWM2M_ST);
    lwm2m_item_s::lwm2m_value * low_step = lwm2m_get_attribute(LWM2M_LOW_STEP);

    if(st) {
        if(high_step) {
            high_step->float_value = value + st->float_value;
        }
        if(low_step) {
            low_step->float_value = value - st->float_value;
        }
        tr_debug("M2MReportHandler::set_notification_attribute st to %f", st->float_value);
    }
}

void M2MReportHandler::set_notification_trigger(uint16_t obj_instance_id)
{
    tr_debug("M2MReportHandler::set_notification_trigger(): %d", obj_instance_id);
    // Add to array if not there yet
    m2m::Vector<uint16_t>::const_iterator it;
    it = _changed_instance_ids.begin();
    bool found = false;
    for ( ; it != _changed_instance_ids.end(); it++) {
        if ((*it) == obj_instance_id) {
            found = true;
            break;
        }
    }
    if (!found) {
        _changed_instance_ids.push_back(obj_instance_id);
    }

    schedule_report();
}

bool M2MReportHandler::parse_notification_attribute(const char *query,
                                                    M2MBase::BaseType type,
                                                    M2MResourceInstance::ResourceType resource_type)
{
    tr_debug("M2MReportHandler::parse_notification_attribute(Query %s, Base type %d)", query, (int)type);
    bool success = false;
    const char* sep_pos = strchr(query, '&');
    const char* rest = query;
    if( sep_pos != NULL ){
        char query_options[5][20];
        lwm2m_item_s::lwm2m_value * pmin_value = lwm2m_get_attribute(LWM2M_PMIN);
        int32_t pmin;
        if(pmin_value) {
            pmin = pmin_value->int_value;
        }
        lwm2m_item_s::lwm2m_value * pmax_value = lwm2m_get_attribute(LWM2M_PMAX);
        int32_t pmax;
        if(pmax_value) {
            pmax = pmax_value->int_value;
        }
        lwm2m_item_s::lwm2m_value * lt_value = lwm2m_get_attribute(LWM2M_LT);
        float lt;
        if(lt_value) {
            lt = lt_value->float_value;
        }
        lwm2m_item_s::lwm2m_value * gt_value = lwm2m_get_attribute(LWM2M_GT);
        float gt;
        if(gt_value) {
            gt = gt_value->float_value;
        }
        lwm2m_item_s::lwm2m_value * st_value = lwm2m_get_attribute(LWM2M_ST);
        float st;
        if(st_value) {
            st = st_value->float_value;
        }
        lwm2m_item_s::lwm2m_value * high_value = lwm2m_get_attribute(LWM2M_HIGH_STEP);
        float high;
        if(high_value) {
            high = high_value->float_value;
        }
        lwm2m_item_s::lwm2m_value * low_value = lwm2m_get_attribute(LWM2M_LOW_STEP);
        float low;
        if(low_value) {
            low = low_value->float_value;
        }

        uint8_t attr = _attribute_state;

        memset(query_options, 0, sizeof(query_options[0][0]) * 5 * 20);
        uint8_t num_options = 0;
        while( sep_pos != NULL && num_options < 5){
            size_t len = (size_t)(sep_pos-rest);
            if( len > 19 ){
                len = 19;
            }
            memcpy(query_options[num_options], rest, len);
            sep_pos++;
            rest = sep_pos;
            sep_pos = strchr(rest, '&');
            num_options++;
        }
        if( num_options < 5 && strlen(rest) > 0){
            size_t len = (size_t)strlen(rest);
            if( len > 19 ){
                len = 19;
            }
            memcpy(query_options[num_options++], rest, len);
        }

        for (int option = 0; option < num_options; option++) {
            success = set_notification_attribute(query_options[option],type, resource_type);
            if (!success) {
                tr_debug("M2MReportHandler::parse_notification_attribute - break");
                break;
            }
        }

        if(success) {
             success = check_attribute_validity();
        }
        else {
            tr_debug("M2MReportHandler::parse_notification_attribute - not valid query");
            lwm2m_item_s::lwm2m_value * pmin_value = lwm2m_get_attribute(LWM2M_PMIN);
            if(pmin_value) {
                pmin_value->int_value = pmin;
            }
            lwm2m_item_s::lwm2m_value * pmax_value = lwm2m_get_attribute(LWM2M_PMAX);
            if(pmax_value) {
                pmax_value->int_value = pmax;
            }
            lwm2m_item_s::lwm2m_value * st_value = lwm2m_get_attribute(LWM2M_ST);
            if(st_value) {
                st_value->float_value = st;
            }
            lwm2m_item_s::lwm2m_value * lt_value = lwm2m_get_attribute(LWM2M_LT);
            if(lt_value) {
                lt_value->float_value = lt;
            }
            lwm2m_item_s::lwm2m_value * gt_value = lwm2m_get_attribute(LWM2M_GT);
            if(gt_value) {
                gt_value->float_value = gt;
            }
            lwm2m_item_s::lwm2m_value * high_value = lwm2m_get_attribute(LWM2M_HIGH_STEP);
            if(high_value) {
                high_value->float_value = high;
            }
            lwm2m_item_s::lwm2m_value * low_value = lwm2m_get_attribute(LWM2M_LOW_STEP);
            if(low_value) {
                low_value->float_value = low;
            }
            _attribute_state = attr;
        }
    }
    else {
        if(set_notification_attribute(query, type, resource_type)) {
            success = check_attribute_validity();
        }
    }

    return success;
}

void M2MReportHandler::timer_expired(M2MTimerObserver::Type type)
{
    switch(type) {
        case M2MTimerObserver::PMinTimer: {
            tr_debug("M2MReportHandler::timer_expired - PMIN");
            _pmin_exceeded = true;
            lwm2m_item_s::lwm2m_value * pmin_value = lwm2m_get_attribute(LWM2M_PMIN);
            int32_t pmin = pmin_value->int_value;
            if (_notify ||
                    (pmin > 0 &&
                     (_attribute_state & M2MReportHandler::Pmax) != M2MReportHandler::Pmax)){
                report();
            }
        }
        break;
        case M2MTimerObserver::PMaxTimer: {
            tr_debug("M2MReportHandler::timer_expired - PMAX");
            _pmax_exceeded = true;
            if (_pmin_exceeded ||
                    (_attribute_state & M2MReportHandler::Pmin) != M2MReportHandler::Pmin ) {
                report();
            }
        }
        break;
        default:
            break;
    }
}

bool M2MReportHandler::set_notification_attribute(const char* option,
                                                  M2MBase::BaseType type,
                                                  M2MResourceInstance::ResourceType resource_type)
{
    tr_debug("M2MReportHandler::set_notification_attribute()");
    bool success = false;
    char attribute[20];
    char value[20];
    memset(&attribute, 0, 20);
    memset(&value, 0, 20);

    const char* pos = strstr(option, EQUAL);
    if( pos != NULL ){
        memcpy(attribute, option, (size_t)(pos-option));
        pos++;
        memcpy(value, pos, strlen(pos));
    }else{
        memcpy(attribute, option, (size_t)strlen(option) + 1);
    }

    if (strlen(value)) {
        if (strcmp(attribute, PMIN) == 0) {
            lwm2m_item_s * attribute = (lwm2m_item_s*)M2MCoreMemory::memory_alloc(sizeof(lwm2m_item_s));
            attribute->attribute_name = LWM2M_PMIN;
            attribute->value.int_value = atoi(value);
            lwm2m_set_attribute(attribute);

            M2MTimer *pmin_timer = new M2MTimer(*this);
            lwm2m_item_s * attribute_min_timer = (lwm2m_item_s*)M2MCoreMemory::memory_alloc(sizeof(lwm2m_item_s));
            attribute_min_timer->attribute_name = LWM2M_PMIN_TIMER;
            attribute_min_timer->value.void_value = pmin_timer;
            lwm2m_set_attribute(attribute_min_timer);

            success = true;
            _attribute_state |= M2MReportHandler::Pmin;
            tr_debug("M2MReportHandler::set_notification_attribute %s to %" PRId32, attribute, attribute->value.int_value);
        }
        else if(strcmp(attribute, PMAX) == 0) {
            lwm2m_item_s * attribute = (lwm2m_item_s*)M2MCoreMemory::memory_alloc(sizeof(lwm2m_item_s));
            attribute->attribute_name = LWM2M_PMAX;
            attribute->value.int_value = atoi(value);
            lwm2m_set_attribute(attribute);

            M2MTimer *pmax_timer = new M2MTimer(*this);
            lwm2m_item_s * attribute_max_timer = (lwm2m_item_s*)M2MCoreMemory::memory_alloc(sizeof(lwm2m_item_s));
            attribute_max_timer->attribute_name = LWM2M_PMAX_TIMER;
            attribute_max_timer->value.void_value = pmax_timer;
            lwm2m_set_attribute(attribute_max_timer);

            success = true;
            _attribute_state |= M2MReportHandler::Pmax;
            tr_debug("M2MReportHandler::set_notification_attribute %s to %" PRId32, attribute, attribute->value.int_value);
        }
        else if(strcmp(attribute, GT) == 0 &&
                (M2MBase::Resource == type)){
            lwm2m_item_s * attribute = (lwm2m_item_s*)M2MCoreMemory::memory_alloc(sizeof(lwm2m_item_s));
            attribute->attribute_name = LWM2M_GT;
            attribute->value.float_value = atof(value);
            lwm2m_set_attribute(attribute);
            success = true;
            _attribute_state |= M2MReportHandler::Gt;
            tr_debug("M2MReportHandler::set_notification_attribute %s to %f", attribute, attribute->value.float_value);
        }
        else if(strcmp(attribute, LT) == 0 &&
                (M2MBase::Resource == type)){
            lwm2m_item_s * attribute = (lwm2m_item_s*)M2MCoreMemory::memory_alloc(sizeof(lwm2m_item_s));
            attribute->attribute_name = LWM2M_LT;
            attribute->value.float_value = atof(value);
            lwm2m_set_attribute(attribute);
            success = true;
            _attribute_state |= M2MReportHandler::Lt;
            tr_debug("M2MReportHandler::set_notification_attribute %s to %f", attribute, attribute->value.float_value);
        }
        else if((strcmp(attribute, ST_SIZE) == 0 || (strcmp(attribute, STP) == 0))
                && (M2MBase::Resource == type)){
            lwm2m_item_s * attribute = (lwm2m_item_s*)M2MCoreMemory::memory_alloc(sizeof(lwm2m_item_s));
            attribute->attribute_name = LWM2M_ST;
            attribute->value.float_value = atof(value);
            lwm2m_set_attribute(attribute);
            success = true;

            lwm2m_item_s::lwm2m_value * high_step = lwm2m_get_attribute(LWM2M_HIGH_STEP);
            lwm2m_item_s::lwm2m_value * st = lwm2m_get_attribute(LWM2M_ST);
            lwm2m_item_s::lwm2m_value * low_step = lwm2m_get_attribute(LWM2M_LOW_STEP);

            _attribute_state |= M2MReportHandler::St;
        }
        // Return false if try to set gt,lt or st when the resource type is something else than numerical
        if ((resource_type != M2MResourceInstance::INTEGER &&
                resource_type != M2MResourceInstance::FLOAT) &&
                ((_attribute_state & M2MReportHandler::Gt) == M2MReportHandler::Gt ||
                (_attribute_state & M2MReportHandler::Lt) == M2MReportHandler::Lt ||
                (_attribute_state & M2MReportHandler::St) == M2MReportHandler::St)) {
            tr_debug("M2MReportHandler::set_notification_attribute - not numerical resource");
            success = false;
        }
    }
    return success;
}

void M2MReportHandler::schedule_report()
{
    tr_debug("M2MReportHandler::schedule_report()");
    _notify = true;
    if ((_attribute_state & M2MReportHandler::Pmin) != M2MReportHandler::Pmin ||
         _pmin_exceeded) {
        report();
    }
}

void M2MReportHandler::report()
{
    tr_debug("M2MReportHandler::report()");
    if(_notify) {
        if (_pmin_exceeded) {
            tr_debug("M2MReportHandler::report()- send with PMIN expiration");
        } else {
            tr_debug("M2MReportHandler::report()- send with VALUE change");
        }
        _pmin_exceeded = false;
        _pmax_exceeded = false;
        _notify = false;
        _observation_number++;
        if(_observation_number == 1) {
            // Increment the observation number by 1 if it is already 1 because CoAP specification has reserved 1 for DEREGISTER notification
            _observation_number++;
        }
        _observer.observation_to_be_sent(_changed_instance_ids, observation_number());
        _changed_instance_ids.clear();
        lwm2m_item_s::lwm2m_value * timer_value = lwm2m_get_attribute(LWM2M_PMAX_TIMER);
        if(timer_value) {
            M2MTimer *timer = (M2MTimer*)timer_value->void_value;
            timer->stop_timer();
        }
    }
    else {
        if (_pmax_exceeded) {
            tr_debug("M2MReportHandler::report()- send with PMAX expiration");
            _observation_number++;
            if(_observation_number == 1) {
                // Increment the observation number by 1 if it is already 1 because CoAP specification has reserved 1 for DEREGISTER notification
                _observation_number++;
            }
            _observer.observation_to_be_sent(_changed_instance_ids, observation_number(),true);
            _changed_instance_ids.clear();
        }
        else {
            tr_debug("M2MReportHandler::report()- no need to send");
        }
    }
    handle_timers();    
}

void M2MReportHandler::handle_timers()
{
    tr_debug("M2MReportHandler::handle_timers()");
    uint64_t time_interval = 0;
    int32_t pmax = -1;
    int32_t pmin = 1;
    if ((_attribute_state & M2MReportHandler::Pmin) == M2MReportHandler::Pmin) {
        lwm2m_item_s::lwm2m_value * pmin_value = lwm2m_get_attribute(LWM2M_PMIN);
        if(pmin_value) {
            pmin_value->int_value = pmin;
        }
        lwm2m_item_s::lwm2m_value * pmax_value = lwm2m_get_attribute(LWM2M_PMAX);
        if(pmax_value) {
            pmax_value->int_value = pmax;
        }
        if (pmin == pmax) {
            _pmin_exceeded = true;
        } else {
            _pmin_exceeded = false;
            time_interval = (uint64_t) ((uint64_t)pmin * 1000);
            tr_debug("M2MReportHandler::handle_timers() - Start PMIN interval: %d", (int)time_interval);
            lwm2m_item_s::lwm2m_value * timer_value = lwm2m_get_attribute(LWM2M_PMIN_TIMER);
            if(timer_value) {
                M2MTimer *timer = (M2MTimer*)timer_value->void_value;
                timer->start_timer(time_interval,
                                         M2MTimerObserver::PMinTimer,
                                         true);
            }

        }
    }
    if ((_attribute_state & M2MReportHandler::Pmax) == M2MReportHandler::Pmax) {
        if (pmax > 0) {
            time_interval = (uint64_t) ((uint64_t)pmax * 1000);
            tr_debug("M2MReportHandler::handle_timers() - Start PMAX interval: %d", (int)time_interval);
            lwm2m_item_s::lwm2m_value * timer_value = lwm2m_get_attribute(LWM2M_PMAX_TIMER);
            if(timer_value) {
                M2MTimer *timer = (M2MTimer*)timer_value->void_value;
                timer->start_timer(time_interval,
                                   M2MTimerObserver::PMaxTimer,
                                   true);
            }
        }
    }
}

bool M2MReportHandler::check_attribute_validity()
{
    bool success = true;
    float lt = 0.0f;
    lwm2m_item_s::lwm2m_value * lt_step = lwm2m_get_attribute(LWM2M_LT);
    if(lt_step) {
        lt = lt_step->float_value;
        tr_debug("Lt: %f", lt);
    }
    float gt = 0.0f;
    lwm2m_item_s::lwm2m_value * gt_step = lwm2m_get_attribute(LWM2M_GT);
    if(gt_step) {
        gt = gt_step->float_value;
        tr_debug("Gt: %f", gt_step);
    }
    float st = 0.0f;
    lwm2m_item_s::lwm2m_value * st_step = lwm2m_get_attribute(LWM2M_ST);
    if(st_step) {
        st = st_step->float_value;
        tr_debug("Step: %f", st);
    }
    lwm2m_item_s::lwm2m_value * pmin = lwm2m_get_attribute(LWM2M_PMIN);
    lwm2m_item_s::lwm2m_value * pmax = lwm2m_get_attribute(LWM2M_PMAX);
    if ((_attribute_state & M2MReportHandler::Pmax) == M2MReportHandler::Pmax &&
            ((pmax->int_value >= -1.0) && (pmin->int_value > pmax->int_value))) {
        success = false;
    }
    float low = lt + 2 * st;
    if ((_attribute_state & M2MReportHandler::Gt) == M2MReportHandler::Gt &&
            (low >= gt)) {
        success = false;
    }
    return success;
}

void M2MReportHandler::stop_timers()
{
    tr_debug("M2MReportHandler::stop_timers()");

    _pmin_exceeded = false;
    lwm2m_item_s::lwm2m_value * timer_value = lwm2m_get_attribute(LWM2M_PMIN_TIMER);
    if(timer_value) {
        M2MTimer *timer = (M2MTimer*)timer_value->void_value;
        timer->stop_timer();
    }

    _pmax_exceeded = false;
    timer_value = lwm2m_get_attribute(LWM2M_PMAX_TIMER);
    if(timer_value) {
        M2MTimer *timer = (M2MTimer*)timer_value->void_value;
        timer->stop_timer();
    }

    tr_debug("M2MReportHandler::stop_timers() - out");
}

void M2MReportHandler::set_default_values()
{
    tr_debug("M2MReportHandler::set_default_values");
    _pmin_exceeded = false;
    _pmax_exceeded = false;
    _attribute_state = 0;
    _changed_instance_ids.clear();
}

bool M2MReportHandler::check_threshold_values(float value)
{
    tr_debug("M2MReportHandler::check_threshold_values");

    float low_step = 0.0f;
    float high_step = 0.0f;
    lwm2m_item_s::lwm2m_value * lw_step = lwm2m_get_attribute(LWM2M_LOW_STEP);
    if(lw_step) {
        low_step = lw_step->float_value;
        tr_debug("Low step: %f", low_step);
    }
    lwm2m_item_s::lwm2m_value * hg_step = lwm2m_get_attribute(LWM2M_HIGH_STEP);
    if(hg_step) {
        high_step = hg_step->float_value;
        tr_debug("High step: %f", high_step);
    }
    tr_debug("Current value: %f", value);

    bool can_send = false;
    // Check step condition
    if ((_attribute_state & M2MReportHandler::St) == M2MReportHandler::St) {
        if ((value >= high_step||
            value <= low_step)) {
            can_send = true;
        }
        else {
            if ((_attribute_state & M2MReportHandler::Lt) == M2MReportHandler::Lt ||
                    (_attribute_state & M2MReportHandler::Gt) == M2MReportHandler::Gt ) {
                can_send = check_gt_lt_params(value);
            }
            else {
                can_send = false;
            }
        }
    }
    else {
        can_send = check_gt_lt_params(value);
    }
    tr_debug("M2MReportHandler::check_threshold_values - value in range = %d", (int)can_send);
    return can_send;
}

bool M2MReportHandler::check_gt_lt_params(float value)
{
    tr_debug("M2MReportHandler::check_gt_lt_params");
    float lt = 0.0f;
    float gt = 0.0f;
    lwm2m_item_s::lwm2m_value * lt_step = lwm2m_get_attribute(LWM2M_LT);
    if(lt_step) {
        lt = lt_step->float_value;
        tr_debug("Lt: %f", lt);
    }
    lwm2m_item_s::lwm2m_value * gt_step = lwm2m_get_attribute(LWM2M_GT);
    if(gt_step) {
        gt = gt_step->float_value;
        tr_debug("Gt: %f", gt);
    }
    bool can_send = false;
    // GT & LT set.
    if ((_attribute_state & (M2MReportHandler::Lt | M2MReportHandler::Gt))
             == (M2MReportHandler::Lt | M2MReportHandler::Gt)) {
        if (value > gt || value < lt) {
            can_send = true;
        }
        else {
            can_send = false;
        }
    }
    // Only LT
    else if ((_attribute_state & M2MReportHandler::Lt) == M2MReportHandler::Lt &&
           (_attribute_state & M2MReportHandler::Gt) == 0 ) {
        if (value < lt) {
            can_send = true;
        }
        else {
            can_send = false;
        }
    }
    // Only GT
    else if ((_attribute_state & M2MReportHandler::Gt) == M2MReportHandler::Gt &&
           (_attribute_state & M2MReportHandler::Lt) == 0 ) {
        if (value > gt) {
            can_send = true;
        }
        else {
            can_send = false;
        }
    }
    // GT & LT not set.
    else {
        can_send = true;
    }
    tr_debug("M2MReportHandler::check_gt_lt_params - value in range = %d", (int)can_send);
    return can_send;
}

uint8_t M2MReportHandler::attribute_flags() const
{
    return _attribute_state;
}

void M2MReportHandler::set_observation_token(const uint8_t *token, const uint8_t length)
{
    memcpy(_token, token, length);
    _token_length = length;
}

const uint8_t *M2MReportHandler::get_observation_token() const
{
    return _token;
}

uint8_t M2MReportHandler::get_observation_token_length() const
{
    return _token_length;
}

uint16_t M2MReportHandler::observation_number() const
{
    return _observation_number;
}

void M2MReportHandler::add_observation_level(M2MBase::Observation obs_level)
{
    _observation_level = (M2MBase::Observation)(_observation_level | obs_level);
}

void M2MReportHandler::remove_observation_level(M2MBase::Observation obs_level)
{
    _observation_level = (M2MBase::Observation)(_observation_level & ~obs_level);
}

M2MBase::Observation M2MReportHandler::observation_level() const
{
    return _observation_level;
}

bool M2MReportHandler::is_under_observation() const
{
    return _is_under_observation;
}

void M2MReportHandler::lwm2m_free_attributes_list()
{
    lwm2m_item_s *item = _lwm2m_item;
    if (item) {
        while (item->attribute_name != LWM2M_END) {
            lwm2m_free_attribute(item);
            item++;
        }
        M2MCoreMemory::memory_free(_lwm2m_item);
        _lwm2m_item = NULL;
    }
}

bool M2MReportHandler::lwm2m_set_attribute(const lwm2m_item_s *attribute)
{
    unsigned int item_count = 0;
    lwm2m_item_s *item = _lwm2m_item;
    // Count the number of attributes for reallocation, update in place though
    // if the attribute already existed
    while (item != NULL) {
        item_count++;
        if (item->attribute_name == LWM2M_END) {
            break;
        }
        // Check if attribute already exists or if there is NOP we can overwrite
        if (item->attribute_name == attribute->attribute_name || item->attribute_name == LWM2M_NOP) {
            // Found attribute or NOP, overwrite it
            lwm2m_free_attribute(item);
            item->attribute_name = attribute->attribute_name;
            if(item->attribute_name == LWM2M_GT || item->attribute_name == LWM2M_LT || item->attribute_name == LWM2M_ST ||
               item->attribute_name == LWM2M_HIGH_STEP || item->attribute_name == LWM2M_LOW_STEP) {
                item->value.float_value = attribute->value.float_value;
            } else if(item->attribute_name == LWM2M_PMIN || item->attribute_name == LWM2M_PMAX) {
                item->value.int_value = attribute->value.int_value;
            } else if(item->attribute_name == LWM2M_PMIN_TIMER|| item->attribute_name == LWM2M_PMAX_TIMER) {
                item->value.void_value = attribute->value.void_value;
            }
            return true;
        }
        item++;
    }
    // Attribute did not yet exist (ptr was null or ATTR_END was first one)
    if (item_count > 0) {
        // List already had some attributes, so reallocate
        size_t new_size = (item_count + 1) * sizeof(lwm2m_item_s);
        item = _lwm2m_item;
        _lwm2m_item = (lwm2m_item_s*)realloc(item, new_size);
        if (_lwm2m_item == NULL) {
            // realloc failed, put back original pointer and return false
            _lwm2m_item = item;
            return false;
        }
        // And move item ptr to ATTR_END to update that and last attribute
        item = &(_lwm2m_item[item_count - 1]);
    }
    else {
        // No attributes, so allocate first time (1 struct for attribute and 1 struct for ATTR_END)
        _lwm2m_item = (lwm2m_item_s*)malloc(2 * sizeof(lwm2m_item_s));
        if (_lwm2m_item == NULL) {
            return false;
        }
        item = _lwm2m_item;
    }
    item->attribute_name = attribute->attribute_name;
    if(item->attribute_name == LWM2M_GT || item->attribute_name == LWM2M_LT || item->attribute_name == LWM2M_ST ||
       item->attribute_name == LWM2M_HIGH_STEP || item->attribute_name == LWM2M_LOW_STEP) {
        item->value.float_value = attribute->value.float_value;
    } else if(item->attribute_name == LWM2M_PMIN || item->attribute_name == LWM2M_PMAX) {
        item->value.int_value = attribute->value.int_value;
    } else if(item->attribute_name == LWM2M_PMIN_TIMER|| item->attribute_name == LWM2M_PMAX_TIMER) {
        item->value.void_value = attribute->value.void_value;
    }

    item++;
    item->attribute_name = LWM2M_END;
    item->value.void_value = NULL;
    return true;
}

lwm2m_item_s::lwm2m_value* M2MReportHandler::lwm2m_get_attribute(lwm2m_attribute_t attribute_name)
{
    lwm2m_item_s::lwm2m_value *value = NULL;
    lwm2m_item_s *item = _lwm2m_item;
    while (item != NULL && item->attribute_name != LWM2M_END) {
        if (item->attribute_name == attribute_name) {
            value = &item->value;
            break;
        }
        item++;
    }
    return value;
}

bool M2MReportHandler::lwm2m_remove_attribute(lwm2m_attribute_t attribute_name)
{
    bool found = false;
    lwm2m_item_s *item = _lwm2m_item;
    while (item != NULL) {
        if (item->attribute_name == LWM2M_END) {
            break;
        }
        // Remove if attribute name matches
        if (item->attribute_name == attribute_name) {
            // Currently only pointer values, need to free and set as NOP
            lwm2m_free_attribute(item);
            item->attribute_name = LWM2M_NOP;
            found = true;
            break;
        }
        item++;
    }
    return found;
}

void M2MReportHandler::lwm2m_free_attribute(lwm2m_item_s *attribute)
{
    switch (attribute->attribute_name) {
        case LWM2M_PMIN_TIMER:
        case LWM2M_PMAX_TIMER: {
            M2MTimer *timer = (M2MTimer*)(attribute->value.void_value);
            if(timer) {
                delete timer;
            }
            attribute->value.void_value = NULL;
        }
        break;
        case LWM2M_PMAX:
        case LWM2M_PMIN:
        case LWM2M_HIGH_STEP:
        case LWM2M_LOW_STEP:
        case LWM2M_LT:
        case LWM2M_ST:
        case LWM2M_GT:
        case LWM2M_NOP:
        case LWM2M_END:
        default:
            break;
    }
}

