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
    class Function : public NoCopyable
    {
    public:
        virtual std::string name() = 0;
        virtual bool deterministic() = 0;
        
        virtual void func(sqlite3_context* context, std::vector<sqlite3_value *> &argv) = 0;
        
        virtual Encoding encoding() {
            return Encoding::UTF8;
        }
        
        virtual int argumentsCount() {
            return -1;
        }
        
    public:
        static void xFunc(sqlite3_context* context, int argc, sqlite3_value** argv) {
            Function *obj = static_cast<Function *>(sqlite3_user_data(context));
            if(!obj) {
                return;
            }
            
            auto list = makeArguments(argc, argv);
            obj->func(context, list);
        }
        
        static void xDestroy(void *p) {
            Function *obj = static_cast<Function *>(p);
            delete obj;
        }
        
    protected:
        static std::vector<sqlite3_value *> makeArguments(int argc, sqlite3_value** argv) {
            std::vector<sqlite3_value *> list;
            for (int i = 0; i < argc; ++i) {
                list.push_back(argv[i]);
            }
            
            return list;
        }
    };
    
    class AggregateFunction : public Function
    {
    public:
        void func(sqlite3_context* context, std::vector<sqlite3_value *> &argv) final {}
        
        virtual void step(sqlite3_context* context, std::vector<sqlite3_value *> &argv) = 0;
        virtual void final(sqlite3_context* context) = 0;
        
    public:
        static void xStep(sqlite3_context* context, int argc, sqlite3_value** argv) {
            AggregateFunction *obj = static_cast<AggregateFunction *>(sqlite3_user_data(context));
            if(!obj) {
                return;
            }
            
            auto list = makeArguments(argc, argv);
            obj->step(context, list);
        }
        
        static void xFinal(sqlite3_context* context) {
            AggregateFunction *obj = static_cast<AggregateFunction *>(sqlite3_user_data(context));
            if(!obj) {
                return;
            }
            
            obj->final(context);
        }
    };
    
    class FunctionHelper
    {
    private:
        typedef tr1::function<void(sqlite3_context*, std::vector<sqlite3_value *> &)> sqlite_function;
        typedef tr1::function<void(sqlite3_context*)> sqlite_final_function;
        
        class _Function : public Function
        {
        public:
            _Function(const std::string &name, int argumentCount, bool deterministic, sqlite_function func)
            : _name(name), _deterministic(deterministic), _func(func), _argumentsCount(argumentCount) {
                
            }
            
            std::string name() override {
                return _name;
            }
            
            int argumentsCount() override {
                return _argumentsCount;
            }
            
            bool deterministic() override {
                return _deterministic;
            }
            
            void func(sqlite3_context* context, std::vector<sqlite3_value *> &argv) override {
                if (_func) {
                    _func(context, argv);
                }
            }
            
        private:
            std::string _name = "";
            bool _deterministic = false;
            int _argumentsCount = -1;
            sqlite_function _func;
        };
        
        class _Aggregate : public AggregateFunction
        {
        public:
            _Aggregate(const std::string &name, int argumentCount, bool deterministic, sqlite_function step, sqlite_final_function final)
            : _name(name), _deterministic(deterministic), _step(step), _final(final), _argumentsCount(argumentCount) {
                
            }
            
            std::string name() override {
                return _name;
            }
            
            int argumentsCount() override {
                return _argumentsCount;
            }
            
            bool deterministic() override {
                return _deterministic;
            }
            
            void step(sqlite3_context* context, std::vector<sqlite3_value *> &argv) override {
                if (_step) {
                    _step(context, argv);
                }
            }
            
            void final(sqlite3_context* context) override {
                if (_final) {
                    _final(context);
                }
            }
            
        private:
            std::string _name = "";
            bool _deterministic = false;
            int _argumentsCount = -1;
            sqlite_function _step;
            sqlite_final_function _final;
        };
        
    public:
        static Function *createFunction(const std::string &name, sqlite_function func, int argumentCount = -1, bool deterministic = false) {
            if (name.empty()) {
                return nullptr;
            }
            
            return new _Function(name, argumentCount, deterministic, func);
        }
        
        static AggregateFunction *createAggregateFunction(const std::string &name, sqlite_function step, sqlite_final_function final, int argumentCount = -1, bool deterministic = false) {
            if (name.empty()) {
                return nullptr;
            }
            
            return new _Aggregate(name, argumentCount, deterministic, step, final);
        }
    };
}

#endif /* Function_hpp */
