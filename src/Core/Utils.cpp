/**
 Copyright (c) 2015, 2coding
 All rights reserved.
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
 
 * Redistributions of source code must retain the above copyright notice, this
 list of conditions and the following disclaimer.
 
 * Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following disclaimer in the documentation
 and/or other materials provided with the distribution.
 
 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 **/

#include "Utils.hpp"
#include "USQLDefs.hpp"

namespace usql {
    std::time_t Utils::str2tm(const std::string &str) {
        if (str.empty()) {
            return USQL_ERROR_DATATIME;
        }
        
        int year = 0, mouth = 0, day = 0;
        int hour = 0, minute = 0;
        double second = 0.0f;
        std::sscanf(str.c_str(), "%d-%d-%d %d:%d:%lf", &year, &mouth, &day, &hour, &minute, &second);
        
        if (year <= 1900
            || (mouth < 1 || mouth > 12)
            || (day < 1 || day > 31)
            || (hour < 0 || hour > 23)
            || (minute < 0 || minute > 59)
            || (second < 0 || second > 60)) {
            return USQL_ERROR_DATATIME;
        }
        
        std::tm tm;
        tm.tm_sec = static_cast<int>(second);
        tm.tm_min = minute;
        tm.tm_hour = hour;
        tm.tm_mday = day;
        tm.tm_mon = mouth - 1;
        tm.tm_year = year - 1900;
        
        return mktime(&tm);
    }
}