#include "SeApi.h"

#include <unistd.h>
#include <iostream>

void setTimeout(double seconds, const std::function<void()>& cb);

class Animal;
static std::vector<Animal*> __animals;

// --- C++ Class

class Animal
{
public:
    Animal()
    {
        type = "animal";
        isPrint = false;
        _func = nullptr;
        _target = nullptr;
        __animals.push_back(this);
    }

    virtual ~Animal()
    {
//        delete _func;
//        delete _target;

        auto iter = std::find(__animals.begin(), __animals.end(), this);
        if (iter != __animals.end())
        {
            __animals.erase(iter);
        }
    }

    virtual void eat()
    {
        printf("Animal eating ..., id: %d\n", id);
    }


    void setTimeout(JSContext* cx, float second, JS::HandleValue func, JS::HandleValue target, JS::HandleValue jsAnimal)
    {
        setFunc(cx, func);
        setTarget(cx, target);

        _jsAnimal = jsAnimal.toObjectOrNull();

        if (!isPrint)
        {
//            printf("func: %p, target: %p\n", _func.get(), _target.get());
            isPrint = true;
        }

        ::setTimeout(second, [=](){
            JS::AutoValueVector argv(cx);
            JS::RootedObject jstarget(cx, _target.get());
            JS::RootedValue jsfunc(cx, JS::ObjectValue(*_func));
            JS::RootedValue jsrval(cx);
//            printf("func: %p, target: %p\n", _func.get(), _target.get());
            JS_CallFunctionValue(cx, jstarget, jsfunc, argv, &jsrval);

            JS::AutoValueVector argv2(cx);
            JS::RootedObject rootedAnimal(cx, _jsAnimal);
            JS::RootedValue jsrval2(cx);
            JS_CallFunctionName(cx, rootedAnimal, "eat", argv2, &jsrval2);
        });
    }

    void setFunc(JSContext* cx, JS::HandleValue func)
    {/*
        if (_func != nullptr)
        {
            delete _func;
        }

        _func = new JS::PersistentRooted<JSObject*>(cx, func.get().toObjectOrNull());*/

        _func = func.toObjectOrNull();
    }

    void setTarget(JSContext* cx, JS::HandleValue target)
    {
/*        if (_target != nullptr)
        {
            delete _target;
        }

        _target = new JS::PersistentRooted<JSObject*>(cx, target.get().toObjectOrNull());*/
        _target = target.toObjectOrNull();
    }

    void updateAfterGC(JSContext* cx)
    {
        if (_func)
        {
            void* oldPtr = _func.get();
            JS_UpdateWeakPointerAfterGC(&_func);
            void* newPtr = _func.get();
            if (oldPtr != newPtr)
            {
                printf("JS_UpdateWeakPointerAfterGC Func: oldPtr: %p, newPtr: %p\n", oldPtr, newPtr);
            }
        }

        if (_target)
        {
            void* oldPtr = _target.get();
            JS_UpdateWeakPointerAfterGC(&_target);
            void* newPtr = _target.get();
            if (oldPtr != newPtr)
            {
                printf("JS_UpdateWeakPointerAfterGC Target: oldPtr: %p, newPtr: %p\n", oldPtr, newPtr);
            }
        }
    }

    void trace(JSTracer* trc, JSObject* obj)
    {
        JS::TraceEdge(trc, &_jsAnimal, "trace js animal");
    }

    bool isPrint;
    int id;
    std::string type;

//    JS::PersistentRooted<JSObject*>* _func;
//    JS::PersistentRooted<JSObject*>* _target;
    JS::Heap<JSObject*> _func;
    JS::Heap<JSObject*> _target;
    JS::Heap<JSObject*> _jsAnimal;
};



void myTracer(JSTracer* trc, JSObject* obj)
{
//    printf("myTracer: obj: %p\n", obj);
    Animal* animal = (Animal*)JS_GetPrivate(obj);
    if (animal != nullptr)
    {
        animal->trace(trc, obj);
    }
}

void myWeakPointerZoneGroupCallback(JSContext* cx, void* data)
{
    printf("myWeakPointerZoneGroupCallback, data: %p\n", data);
    for (auto& animal : __animals)
    {
        animal->updateAfterGC(cx);
    }
}

// ---

SE_CTOR_BEGIN(Animal_constructor, "Animal")
{
    // --- se::Arguments args; is already defined

//    std::cout << "Animal Constructor" << std::endl;

    Animal *animal=new Animal();
    SE_CTOR_SET_NATIVE_OBJ(animal);
}
SE_CTOR_END

SE_FUNC_BEGIN(eat)
{
    Animal *animal = (Animal *) SE_GET_NATIVE_OBJ();
    animal->eat();
}
SE_FUNC_END

SE_FUNC_BEGIN(setTimeout)
{
    Animal *animal = (Animal *) SE_GET_NATIVE_OBJ();
    float second = (float)args[0].toNumber();

    JS::RootedValue func(cx, _argv[1]);
    JS::RootedValue target(cx, _argv[2]);
    animal->setTimeout(cx, second, func, target, _argv[3]);

}
SE_FUNC_END


SE_GET_PROPERTY_BEGIN( GetAnimalProperty_id )
{
    Animal *animal=(Animal *) SE_GET_NATIVE_OBJ();

    se::Value data;
    data.setNumber( animal->id );
    SE_SET_RVAL( data )
}
SE_GET_PROPERTY_END

SE_SET_PROPERTY_BEGIN( SetAnimalProperty_id )
{
    // --- se::Data data; is already defined

    Animal *animal=(Animal *) SE_GET_NATIVE_OBJ();
    animal->id=data.toNumber();
}
SE_SET_PROPERTY_END

SE_FINALIZE_FUNC_BEGIN(Animal_finalize)
{
    Animal* animal = (Animal*)JS_GetPrivate(obj);
    if (animal == nullptr)
        return;

    printf("Animal finalized, id: %d\n", animal->id);
/*    if (animal->type == "dog")
    {
        printf("it's dog dead\n");
    }
    else
    {
        printf("animal dead\n");
    }*/

    delete animal;
}
SE_FINALIZE_FUNC_END

class Dog : public Animal
{
public:
    Dog(  ) { m_name="Freddy";
        type = "dog";
    }
    ~Dog() {
        printf("Dealloc Dog ...\n");
    }

    virtual void testFunc() {
        printf("Dog.testFunc, name: %s, age: %d\n", m_name.c_str(), m_age);
    }
    std::string m_name;
    int         m_age;
};

// ---

SE_CTOR_BEGIN( Dog_constructor, "Dog" )
{
    // --- se::Arguments args; is already defined

    std::cout << "Dog Constructor" << std::endl;

    Dog *dog=new Dog();
    SE_CTOR_SET_NATIVE_OBJ(dog);
}
SE_CTOR_END

SE_FUNC_BEGIN( testFunc )
{
    Dog *dog=(Dog *) SE_GET_NATIVE_OBJ();
    dog->testFunc();
}
SE_FUNC_END

SE_GET_PROPERTY_BEGIN( GetDogProperty_name )
{
    Dog *dog=(Dog *) SE_GET_NATIVE_OBJ();

    se::Value data;
    data.setString( dog->m_name );
    SE_SET_RVAL(data);
}
SE_GET_PROPERTY_END

SE_SET_PROPERTY_BEGIN( SetDogProperty_name )
{
    // --- se::Data data; is already defined

    Dog *dog=(Dog *) SE_GET_NATIVE_OBJ();
    dog->m_name=data.toString();
}
SE_SET_PROPERTY_END

SE_FINALIZE_FUNC_BEGIN(Dog_finalize)
{
    printf("Dog finalized ...\n");

    Animal* animal = (Animal*)JS_GetPrivate(obj);
    if (animal == nullptr)
        return;
    if (animal->type == "dog")
    {
        printf("it's dog dead\n");
    }
    else
    {
        printf("animal dead\n");
    }
    delete animal;
}
SE_FINALIZE_FUNC_END

int main_register_class(int argc, char** argv)
{
    se::ScriptEngine* se = new se::ScriptEngine( argv[0] );
    if ( !se->isValid()) {
        printf("Unable to Initialize JavaScript Wrapper.\n");
        return 1;
    }

    se::Object* global = se->globalObject();

    // --- Register Class

    se::Class* animalClass = se->createClass("Animal", global, nullptr, Animal_constructor);
    animalClass->registerFunction("eat", eat);
    animalClass->registerFunction("setTimeout", setTimeout);
    animalClass->registerProperty("id", GetAnimalProperty_id, SetAnimalProperty_id);
    animalClass->registerFinalizeFunction(Animal_finalize);
    se::Object* animalProto = animalClass->installAndReturnProto();

    se::Class *dogClass = se->createClass("Dog", global, animalProto, Dog_constructor);
    dogClass->registerFunction( "testFunc", testFunc );
    dogClass->registerProperty( "name", GetDogProperty_name, SetDogProperty_name );
    dogClass->registerFinalizeFunction(Dog_finalize);
    dogClass->installAndReturnProto();

    se->executeFile("/Users/james/tests/TestV8/ScriptEngine/Resources/test-gc.js");

//    delete global;

    return 0;
}
