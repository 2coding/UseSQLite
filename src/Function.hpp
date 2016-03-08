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

#ifndef Function_hpp
#define Function_hpp
#include "StdCpp.hpp"
#include "Object.hpp"
#include "USQLDefs.hpp"

namespace usql {
    typedef tr1::function<void(sqlite3_context*, std::vector<sqlite3_value *> &)> sqlite_step_func;
    typedef tr1::function<void(sqlite3_context *)> sqlite_final_func;
    
    class Function : public NoCopyable
    {
    public:
        static Function *create(const std::string &name) {
            return new Function(name);
        }
        
        static void xFunc(sqlite3_context* context, int argc, sqlite3_value** argv) {
            Function *obj = static_cast<Function *>(sqlite3_user_data(context));
            if(!obj) {
                return;
            }
            
            std::vector<sqlite3_value *> list;
            for (int i = 0; i < argc; ++i) {
                list.push_back(argv[i]);
            }
            
            if (obj->_func) {
                obj->_func(context, list);
            }
        }
        
        static void xDestroy(void *p) {
            Function *obj = static_cast<Function *>(p);
            delete obj;
        }
        
        void setFunction(sqlite_step_func func) {
            _func = func;
        }
        
        void setArgumentCount(int num) {
            _argumentCount = std::max(-1, num);
        }
        int argumentCount() const {
            return _argumentCount;
        }
        
        std::string name = "";
        Encoding encoding = Encoding::UTF8;
        bool deterministic = false;
        
    protected:
        Function(const std::string &n) : name(n) {}
        
        int _argumentCount = -1;
        sqlite_step_func _func;
    };
    
    class AggregateFunction : public Function
    {
    public:
        static AggregateFunction *create(const std::string &name) {
            return new AggregateFunction(name);
        }
        
        void setFinalFunction(sqlite_final_func func) {
            _final = func;
        }
        
        static void xFinal(sqlite3_context* context) {
            AggregateFunction *obj = static_cast<AggregateFunction *>(sqlite3_user_data(context));
            if(!obj) {
                return;
            }
            
            if (obj->_final) {
                obj->_final(context);
            }
        }
        
    protected:
        using Function::Function;
        
        sqlite_final_func _final;
    };
}

#endif /* Function_hpp */
