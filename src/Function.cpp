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

#include "Function.hpp"

namespace usql {
//    class _Function : public Function
//    {
//    public:
//        _Function(const std::string &name, bool deterministic, sqlite_function func)
//        : _name(name), _deterministic(deterministic), _func(func) {
//            
//        }
//        
//        std::string name() override {
//            return _name;
//        }
//        
//        bool deterministic() override {
//            return _deterministic;
//        }
//        
//        void func(sqlite3_context* context, std::vector<sqlite3_value *> &argv) override {
//            if (_func) {
//                _func(context, argv);
//            }
//        }
//        
//    private:
//        std::string _name = "";
//        bool _deterministic = false;
//        sqlite_function _func;
//    };
//    
//    Function *FunctionHelper::createFunction(const std::string &name, bool deterministic, sqlite_function func) {
//        if (name.empty()) {
//            return nullptr;
//        }
//        
//        return new _Function(name, deterministic, func);
//    }
}