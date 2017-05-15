#pragma once

#include "../config.hpp"

#ifdef SCRIPT_ENGINE_CHAKRACORE

#include "Base.h"

#include "../Value.hpp"

namespace se {

    extern const bool NEED_THIS;
    extern const bool DONT_NEED_THIS;

    namespace internal {

        struct PrivateData
        {
            void* data;
            JsFinalizeCallback finalizeCb;
        };
        
        void jsToSeArgs(int argc, const JsValueRef* argv, ValueArray* outArr);
        void seToJsArgs(const ValueArray& args, JsValueRef* outArr);
        void jsToSeValue(JsValueRef jsval, Value* v);
        void seToJsValue(const Value& v, JsValueRef* jsval);

        void forceConvertJsValueToStdString(JsValueRef jsval, std::string* ret);
        void jsStringToStdString(JsValueRef jsStr, std::string* ret);

        bool hasPrivate(JsValueRef obj);
        void setPrivate(JsValueRef obj, void* data, JsFinalizeCallback finalizeCb);
        void* getPrivate(JsValueRef obj);

    } // namespace internal {
} // namespace se {

#endif // #ifdef SCRIPT_ENGINE_CHAKRACORE
