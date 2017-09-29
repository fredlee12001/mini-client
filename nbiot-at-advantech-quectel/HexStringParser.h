/* Copyright (c) 2017 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HEXSTRING_PARSER_H
#define HEXSTRING_PARSER_H

#include <string>
#include <stdio.h>
#include "string.h"

using namespace std;

/* TBD. TMP SOLUTION, Need to find out if there is better way in mbed os */

typedef struct Char2Decimal_S {
    char c;
    unsigned char bValue;
} CHAR2DECIMAL_T, *PCHAR2DECIMAL_T;

const CHAR2DECIMAL_T c2d[22] = {
    {'0', 0},{'1', 1},
    {'2', 2},{'3', 3},
    {'4', 4},{'5', 5},
    {'6', 6},{'7', 7},
    {'8', 8},{'9', 9},
    {'a', 10},{'A', 10},
    {'b', 11},{'B', 11},
    {'c', 12},{'C', 12},
    {'d', 13},{'D', 13},
    {'e', 14},{'E', 14},
    {'f', 15},{'F', 15}
};

class HexStringParser
{
public:
    HexStringParser() {};
    virtual ~HexStringParser() {};
public:
    unsigned char GetCharValue(char cAscii);
    void ConverToHexString(char *strHexData, const char* pszSrc, int nLen);
    void HexStringToString(unsigned char* strDest,  int nDestLen, const char* pszHexSrc);
};

#endif
