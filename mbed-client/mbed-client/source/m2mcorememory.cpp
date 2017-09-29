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

#include <stdlib.h>
#include "mbed-client/m2mcorememory.h"
#include "mbed-trace/mbed_trace.h"

#define TRACE_GROUP "mClt"

#ifndef MBED_CLIENT_DYNAMIC_MEMORY_HEAP_SIZE
#define MBED_CLIENT_DYNAMIC_MEMORY_HEAP_SIZE 25000
#endif

#ifdef MBED_CLIENT_TRACE_PRINTS
#ifdef MBED_CLIENT_PASSTHROUGH
#include <malloc.h>
int M2MCoreMemory::memTotal = 0;
int M2MCoreMemory::memCount = 0;
#endif
#ifdef MBED_CLIENT_DYNMEM_LIB
mem_stat_t M2MCoreMemory::memInfo;
void M2MCoreMemory::memory_fail_callback(heap_fail_t fail) {
    tr_debug("\nM2M core memory failure: %u\n", fail);
}
#endif
#endif

#ifdef MBED_CLIENT_DYNMEM_LIB
/* nanoservices dynmemlib based implementation */

ns_mem_book_t * M2MCoreMemory::book = 0;

void * M2MCoreMemory::operator new (size_t size) {
    void *tmp;
    tmp=ns_mem_alloc(book, size);
    if(tmp == 0) {
#ifdef MBED_CLIENT_TRACE_PRINTS
    tr_debug("mn"); /* M2M new */
    print_heap_running_statistics();
#endif
    }
    return tmp;
}

void M2MCoreMemory::operator delete (void * ptr) {
    ns_mem_free(book, ptr);
#ifdef MBED_CLIENT_TRACE_PRINTS
    tr_debug("md"); /* M2M delete */
    print_heap_overall_statistics();
#endif
}
void * M2MCoreMemory::operator new[] (size_t size) {
    void *tmp;
    tmp=ns_mem_alloc(book, size);
    if(tmp == 0) {
#ifdef MBED_CLIENT_TRACE_PRINTS
    tr_debug("mn[]"); /* M2M new array */
#endif
    }
    return tmp;
}
void M2MCoreMemory::operator delete[] (void * ptr) {
    ns_mem_free(book, ptr);
#ifdef MBED_CLIENT_TRACE_PRINTS
    tr_debug("md[]"); /* M2M delete array */
#endif
}
#endif

#ifdef MBED_CLIENT_PASSTHROUGH
/* system malloc based implementation */
void * M2MCoreMemory::operator new (size_t size) {
    void *tmp;
#ifdef MBED_CLIENT_TRACE_PRINTS
    long int allocatedSize;
#endif
    tmp=malloc(size);
#ifdef MBED_CLIENT_TRACE_PRINTS
    allocatedSize=malloc_usable_size(tmp);
    memTotal+=allocatedSize; memCount++;
    tr_debug("mn:%lu:%lu:%d:%d:", size, allocatedSize, memTotal, memCount);
#endif
    return tmp;
}

void M2MCoreMemory::operator delete (void * ptr) {
#ifdef MBED_CLIENT_TRACE_PRINTS
    long int allocatedSize;
    allocatedSize=malloc_usable_size(ptr);
    memCount--;
    memTotal-=allocatedSize;
    tr_debug("md:%lu:%d:%d:", allocatedSize, memTotal, memCount);
#endif
    free(ptr);
}

void * M2MCoreMemory::operator new[] (size_t size) {
    void *tmp;
#ifdef MBED_CLIENT_TRACE_PRINTS
    long int allocatedSize;
#endif
    tmp=malloc(size);
#ifdef MBED_CLIENT_TRACE_PRINTS
    allocatedSize=malloc_usable_size(tmp);
    memTotal+=allocatedSize; memCount++;
    tr_debug("mn[]:%lu:%lu:%d:%d:", size, allocatedSize, memTotal, memCount);
#endif
    return tmp;
}
void M2MCoreMemory::operator delete[] (void * ptr) {
#ifdef MBED_CLIENT_TRACE_PRINTS
    long int allocatedSize;
    allocatedSize=malloc_usable_size(ptr);
    memCount--;
    memTotal-=allocatedSize;
    tr_debug("md[]:%lu:%d:%d:", allocatedSize, memTotal, memCount);
#endif
    free(ptr);
}
#endif

#ifdef MBED_CLIENT_DYNMEM_LIB
void M2MCoreMemory::init(void) {
    init(MBED_CLIENT_DYNAMIC_MEMORY_HEAP_SIZE);
}

void M2MCoreMemory::init(size_t heapSize) {
    void *heap=malloc( heapSize );
#ifdef MBED_CLIENT_TRACE_PRINTS
    tr_debug("Init allocated %lu bytes for cloud client heap at %p\n", heapSize, heap);
#endif
    init(heap, heapSize);
}

void M2MCoreMemory::init(void *heapAllocation, size_t heapSize) {
    heapSize=heapSize;
#ifdef MBED_CLIENT_TRACE_PRINTS
    book = ns_mem_init(heapAllocation, (ns_mem_heap_size_t)heapSize,
                                          &memory_fail_callback, &memInfo);
#else
    book = ns_mem_init(heapAllocation, (ns_mem_heap_size_t)heapSize,
                                                               NULL, NULL);
#endif
}
#endif

#ifdef MBED_CLIENT_TRACE_PRINTS
void M2MCoreMemory::print_heap_running_statistics() {
#ifdef MBED_CLIENT_DYNMEM_LIB
    tr_debug(":%d:%d:", memInfo.heap_sector_allocated_bytes, memInfo.heap_sector_alloc_cnt);
#else
    tr_debug(":%d:%d:", memTotal, memCount);
#endif
}
void M2MCoreMemory::print_heap_overall_statistics() {
#ifdef MBED_CLIENT_DYNMEM_LIB
    tr_debug(":%d:%d:%u:%u:", memInfo.heap_sector_size,
        memInfo.heap_sector_allocated_bytes_max,
        memInfo.heap_alloc_total_bytes, memInfo.heap_alloc_fail_cnt);
#endif
}
#endif

void* M2MCoreMemory::memory_alloc(size_t size){
    void *tmp;
    #ifdef MBED_CLIENT_DYNMEM_LIB
    tmp = ns_mem_alloc(book, size);
    #else
    if (size) {
        tmp = malloc(size);
    } else {
        tmp = 0;
    }
    #endif
    #ifdef MBED_CLIENT_TRACE_PRINTS
    #ifdef MBED_CLIENT_PASSTHROUGH
    memTotal+=size; memCount++;
    #endif
    tr_debug(":ma:%p", tmp);
    print_heap_running_statistics();
    #endif
    return tmp;
}

void* M2MCoreMemory::memory_temp_alloc(size_t size){
    void *tmp;
    #ifdef MBED_CLIENT_DYNMEM_LIB
    tmp = ns_mem_temporary_alloc(book, size);
    #else
    if (size) {
        tmp = malloc(size);
    } else {
        tmp = 0;
    }
    #endif
    #ifdef MBED_CLIENT_TRACE_PRINTS
    #ifdef MBED_CLIENT_PASSTHROUGH
    memTotal+=size; memCount++;
    #endif
    tr_debug(":ma:%p", tmp);
    print_heap_running_statistics();
    #endif
    return tmp;
}

void M2MCoreMemory::memory_free(void *ptr) {
    #ifdef MBED_CLIENT_DYNMEM_LIB
    ns_mem_free(book, ptr);
    #else
    free(ptr);
    #endif
    #ifdef MBED_CLIENT_TRACE_PRINTS
    tr_debug(":mf:%p", ptr);
    print_heap_overall_statistics();
    #endif
 }
