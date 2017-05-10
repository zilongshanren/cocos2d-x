#pragma once

#include "../config.hpp"

#ifdef SCRIPT_ENGINE_V8

#include "Base.h"
#include "../Ref.hpp"
#include "../Value.hpp"
#include "ObjectWrap.h"

namespace se {

    class Class;

    class Object : public Ref
    {
    private:
        Object();
        bool init(v8::Local<v8::Object> obj, bool rooted);

    public:
        virtual ~Object();

        static Object* createPlainObject(bool rooted);

        static Object* createObject(const char* clsName, bool rooted);
        static Object* getObjectWithPtr(void* ptr);
        static Object* getOrCreateObjectWithPtr(void* ptr, const char* clsName, bool rooted);

        static Object* _createJSObject(v8::Local<v8::Object> obj, bool rooted);
        
        // --- Getter/Setter
        bool getProperty(const char *name, Value* data);
        void setProperty(const char *name, const Value& data);

        // --- Function
        bool isFunction() const;
        bool _isNativeFunction() const;
        bool call(const ValueArray& args, Object* thisObject, Value* rval = nullptr);

        bool defineFunction(const char *funcName, v8::FunctionCallback func);


        // --- TypedArrays
        bool isTypedArray() const;
        void getAsFloat32Array(float **ptr, unsigned int *length);

        void getAsUint8Array(unsigned char **ptr, unsigned int *length);

        void getAsUint16Array(unsigned short **ptr, unsigned int *length);

        void getAsUint32Array(unsigned int **ptr, unsigned int *length);

        // --- Arrays
        bool isArray() const;
        void getArrayLength(unsigned int *length);

        void getArrayElement(unsigned int index, Value *data);

        // --- Private
        void setPrivateData(void* data);
        void* getPrivateData() const;

        void switchToUnrooted();
        bool isRooted() const;

        bool isSame(Object* o) const;
        bool attachChild(Object* child);
        bool detachChild(Object* child);

        v8::Local<v8::Object> _getJSObject() const;
        Class* _getClass() const;

    private:
        static void nativeObjectFinalizeHook(void* nativeObj);
        static void setIsolate(v8::Isolate* isolate);

        Class* _cls;
        ObjectWrap _obj;
        bool _isRooted;
        bool _hasPrivateData;

        friend class ScriptEngine;
    };

    extern std::unordered_map<void* /*native*/, Object* /*jsobj*/> __nativePtrToObjectMap;

} // namespace se {

#endif // SCRIPT_ENGINE_V8
