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
//CppUTest includes should be after your and system includes
#include <test/mbedclient/unittest/m2mmap/test_m2mmap.h>

#include "CppUTest/TestHarness.h"

TEST_GROUP(M2MMap)
{
  Test_M2MMap* m2m_map;

  void setup()
  {
    m2m_map = new Test_M2MMap();
  }
  void teardown()
  {
    delete m2m_map;
  }
};

TEST(M2MMap, Create)
{
    CHECK(m2m_map != NULL);
}

TEST(M2MMap, test_operator_array)
{
    m2m_map->test_operator_array();
}

TEST(M2MMap, test_size)
{
    m2m_map->test_size();
}

TEST(M2MMap, test_empty)
{
    m2m_map->test_empty();
}

TEST(M2MMap, test_capacity)
{
    m2m_map->test_capacity();
}

TEST(M2MMap, test_erase)
{
    m2m_map->test_erase();
}

TEST(M2MMap, test_find)
{
    m2m_map->test_find();
}
