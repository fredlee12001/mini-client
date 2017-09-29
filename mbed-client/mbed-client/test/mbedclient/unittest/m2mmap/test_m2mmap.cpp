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
#include "CppUTest/TestHarness.h"
#include <stdio.h>
#include <string.h>
#include <test/mbedclient/unittest/m2mmap/test_m2mmap.h>

Test_M2MMap::Test_M2MMap()
{

}

Test_M2MMap::~Test_M2MMap()
{
}

void Test_M2MMap::test_operator_array() {


    String s0("s0");
    String s1("s1");
    String s2("s2");


    // create a small map instance, fill and check we get data back
    m2m::Map<String> tmap(2);

    tmap[std::string("test0")] = s0;
    tmap[std::string("test1")] = s1;

    CHECK(tmap[std::string("test0")] == s0);
    CHECK(tmap[std::string("test1")] == s1);

    // over capacity, realloc should occur and data should still come back
    tmap[std::string("test2")] = s2;
    CHECK(tmap[std::string("test0")] == s0);
    CHECK(tmap[std::string("test2")] == s2);

}

void Test_M2MMap::test_size() {

    m2m::Map<String> tmap(3);
    String s3("s4");
    String s4("s5");
    String s5("s6");
    tmap[std::string("test0")] = s3;
    tmap[std::string("test1")] = s4;
    tmap[std::string("test2")] = s5;

    CHECK(tmap.size() == 3);
}

void Test_M2MMap::test_empty() {
    m2m::Map<String> tmap;
    CHECK(tmap.empty());
}

void Test_M2MMap::test_capacity() {
    m2m::Map<String> tmap(20);
    CHECK(tmap.capacity() == 20);
}

void Test_M2MMap::test_erase() {
    m2m::Map<String> tmap(10);
    tmap[std::string("test0")] = String("s0");
    CHECK(!tmap.empty());
    tmap.erase(std::string("test0"));
    CHECK(tmap.empty());
}

void Test_M2MMap::test_find() {

    m2m::Map<String> tmap(5);

    CHECK(tmap.find(std::string("test0")) == tmap.end());
    tmap[std::string("test0")] = String("s0");
    tmap[std::string("test1")] = String("s1");
    tmap[std::string("test2")] = String("s2");
    CHECK(tmap.find(std::string("test0")) != tmap.end());
    tmap.erase(std::string("test0"));
    CHECK(tmap.find(std::string("test0")) == tmap.end());


}
