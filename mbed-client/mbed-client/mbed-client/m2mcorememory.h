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

#ifndef M2M_CORE_MEMORY_H
#define M2M_CORE_MEMORY_H

#include <inttypes.h>
#include "nsdynmemLIB.h"
#include "mbed-client/m2mconfig.h"

class M2MCoreMemory {
public:
#if (defined(MBED_CLIENT_DYNMEM_LIB)||defined(MBED_CLIENT_PASSTHROUGH))
    void * operator new (size_t size);
    void operator delete (void * ptr);
    void * operator new[] (size_t size);
    void operator delete[] (void * ptr);
#endif
#ifdef MBED_CLIENT_DYNMEM_LIB
    static void init(void);
    static void init(size_t size);
    static void init(void *heapAllocation, size_t size);
#endif
#ifdef MBED_CLIENT_TRACE_PRINTS
#ifdef MBED_CLIENT_PASSTHROUGH
    static int memTotal;
    static int memCount;
#endif
    static void print_heap_running_statistics(void);
    static void print_heap_overall_statistics(void);
#endif
    static void *memory_alloc(size_t size);
    static void *memory_temp_alloc(size_t size);
    static void memory_free(void *ptr);
private:
#ifdef MBED_CLIENT_DYNMEM_LIB
    static ns_mem_book_t *book;
#ifdef MBED_CLIENT_TRACE_PRINTS
    static mem_stat_t memInfo;
    static void memory_fail_callback(heap_fail_t fail);
#endif
#endif
};
#endif
