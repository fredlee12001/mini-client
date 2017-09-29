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

#include "HexStringParser.h"

/*
*
 "AV" to "4156"
 */
void HexStringParser::ConverToHexString(char *strHexData, const char* pszSrc, int srcLen)
{
    int destPos = 0;
    for (int i = 0; i < srcLen; i++) {
        sprintf(strHexData + destPos, "%02X", (unsigned char)pszSrc[i]);
        destPos += 2;
    }
}

/*
*
 "4156" to "AV"
 */
void HexStringParser::HexStringToString(unsigned char *strDest, int nDestLen, const char* pszHexSrc)
{
    for (int i = 0; i < nDestLen; i++) {
        strDest[i] = (HexStringParser::GetCharValue(pszHexSrc[i*2]) << 4) +
                     HexStringParser::GetCharValue(pszHexSrc[i*2+1]);
    }
}

/*
 *  'A' -> 10 'b'-> 11
 */
unsigned char HexStringParser::GetCharValue(char cAscii)
{
    unsigned char cRt = 0;
    for (int i = 0 ; i < 22; i++) {
        if (c2d[i].c == cAscii) {
            return c2d[i].bValue;
        }

    }
    return cRt;
}
