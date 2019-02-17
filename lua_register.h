//
//
//
//
//
//
//

#if !defined(_LUA_REGISTER_H)
#define _LUA_REGISTER_H

#include <new>
#include <string.h>

namespace lua_register
{
    void init(lua_State* L);
    void registerSigned64(lua_State* L);
    void registerUnsigned64(lua_State* L);

    void dofile(lua_State* L, const char* filename);
    void dostring(lua_State* L, const char* buff);
    void dobuffer(lua_State* L, const char* buff, size_t len);
    void print_error(lua_State* L, const char* fmt, ...);
    int on_error(lua_State* L);
    void enum_stack(lua_State* L);

    // Selector
    template<bool C, typename A, typename B> struct Selector{};

    template<typename A, typename B> struct Selector<true, A, B> 
    {
        typedef A Type;
    };

    template<typename A, typename B> struct Selector<false, A, B>
    {
        typedef B Type;
    };

    // Ptr
    template<typename T>
    struct IsPtr { static const bool Result = false; };

    template<typename T>
    struct IsPtr<T*> { static const bool Result = true; };

    // Ref
    template<typename T>
    struct IsRef { static const bool Result = false; };

    template<typename T>
    struct IsRef<T&> { static const bool Result = true; };

    template<typename T>
    struct RemoveConst { typedef T Result; };

    template<typename T>
    struct RemoveConst<const T> {typedef T Result;};

    template<typename T>
    struct ClassName
    {
        static const char* name(const char* name = NULL)
        {
            static char temp[256] = "";
            if(name) strcpy(temp, name);
            return temp;
        }
    };
    // BaseType, T* -> T, T& -> T
    template<typename T>
    struct BaseType { typedef T Type; };

    template<typename T>
    struct BaseType<T*> { typedef T Type; };

    template<typename T>
    struct BaseType<T&> { typedef T TYpe; };

    template<typename T>
    struct ClassType
    {
        typedef typename BaseType<T>::Type Type1;
        typedef typename RemoveConst<Type1>::Result Type;
    };

    // Is Object 
    template<typename T>
    struct IsObject { static const bool Result = true; };

    template<>
    struct IsObject<char>
    {
        static const bool Result = false;
    };

    template<>
    struct IsObject<unsigned char>
    {
        static const bool Result = false;
    };
    template<>
    struct IsObject<short>
    {
        static const bool Result = false;
    };
    template<>
    struct IsObject<unsigned short>
    {
        static const bool Result = false;
    };
    template<>
    struct IsObject<int>
    {
        static const bool Result = false;
    };
    template<>
    struct IsObject<unsigned int>
    {
        static const bool Result = false;
    };
    template<>
    struct IsObject<long>
    {
        static const bool Result = false;
    };
    template<>
    struct IsObject<unsigned long>
    {
        static const bool Result = false;
    };
    template<>
    struct IsObject<float>
    {
        static const bool Result = false;
    };
    template<>
    struct IsObject<double>
    {
        static const bool Result = false;
    };
    template<>
    struct IsObject<char*>
    {
        static const bool Result = false;
    };
    template<>
    struct IsObject<const char*>
    {
        static const bool Result = false;
    };
    template<>
    struct IsObject<bool>
    {
        static const bool Result = false;
    };
    template<>
    struct IsObject<long long>
    {
        static const bool Result = false;
    };
    template<>
    struct IsObject<unsigned long long>
    {
        static const bool Result = false;
    };

    /////////////////////////////////////
    
    enum { no = 1, yes = 2 };

    typedef char (& NoType)[no];
    typedef char (& YesType)[yes];

    struct IntConvertableType
    {
        IntConvertableType(int);
    };

    NoType int_convert_test(...);
    YesType int_convert_test (IntConvertableType);

    NoType void_ptr_test(const volatile char*);
    NoType void_ptr_test(const volatile short*);
    NoType void_ptr_test(const volatile int*);
    NoType void_ptr_test(const volatile long*);
    NoType void_ptr_test(const volatile double*);
    NoType void_ptr_test(const volatile float*);
    NoType void_ptr_test(const volatile bool*);
    YesType void_ptr_test(const volatile void*);

    template <typename T> T* add_ptr(T&);

    template <bool C>
    struct BoolToYesNo
    {
        typedef NoType Type;
    };

    template<>
    struct BoolToYesNo<true>
    {
        typedef YesType Type;
    };

    // can convert to int, but not normal number Type
    template <typename T>
    struct IsEnum
    {
        static T arg;
        static const bool isIntConvertable = ( sizeof(int_convert_test(arg)) == sizeof(YesType));
        static const bool isPtrVoid = ( sizeof(void_ptr_test(add_ptr(arg))) == sizeof(YesType));
        static const bool Result = isIntConvertable && isPtrVoid;
    };
    ////////////////////////////////////////
    
    // form lua
    template<typename T>
    struct VoidToValue
    {
        static T convert(void* ptr) {return *(T*)ptr; }
    };

    template<typename T>
    struct VoidToPtr
    {
        static T* convert(void* ptr) {return (T*)ptr; }
    };

    template<typename T>
    struct VoidToRef
    {
        static T& convert(void* ptr) { return *(T*)ptr; }
    };

    template<typename T>
    struct VoidToType
    {
        static T convert(void* ptr)
        {
            typedef VoidToRef<typename BaseType<T>::Type> RefType;
            typedef VoidToValue<typename BaseType<T>::Type> ValueType;
            typedef VoidToPtr<typename BaseType<T>::Type> PtrType;
            typedef typename Selector<IsRef<T>::Result, RefType, ValueType>::Type RefOrValueType;
            typedef typename Selector<IsPtr<T>::Result, PtrType, RefOrValueType>::Type PtrOrRefOrValueType;

            return PtrOrRefOrValueType::convert(ptr);
        }
    };

    struct UserBase
    {
        UserBase(void* ptr) : m_p(ptr) {}
        virtual ~UserBase() {}
        void* m_p;
    };

    template<typename T>
    struct LuaUserDataToType
    {
        static T convert(lua_State* L, int index)
        {
            void* ptr = lua_touserdata(L, index);
            return VoidToType<T>::convert(ptr);
        }
    };

    template<typename T>
    struct LuaToEnum
    {
        static T convert(lua_State* L, int index)
        {
            return (T)(int)lua_tonumber(L, index);
        }
    };

    template<typename T>
    struct LuaToObject
    {
        static T convert(lua_State *L, int index)
        {
            if(!lua_isuserdata(L,index))
            {
                lua_pushstring(L, "no class at first argument. (forgot ':' expression?)");
                lua_error(L);
            }

            UserBase* user = LuaUserDataToType<UserBase*>::convert(L, index);
            return VoidToType<T>::convert(user->m_p);
        }
    };

    template<typename T>
    struct ClassDelegate : UserBase
    {
        template <typename ... Args>
        ClassDelegate(Args&&... args) : UserBase( new T(std::forward<Args>(args)...)) {}

        ~ClassDelegate() { delete ((T*)m_p); }
    };

    template<typename T>
    struct PtrToUser : UserBase
    {
        PtrToUser(T* t) : UserBase((void*)t) {}
    };

    template<typename T>
    struct RefToUser : UserBase
    {
        RefToUser(T& t) : UserBase(&t) {}
    };

    template<typename T>
    struct ValueToLua
    {
        static void push(lua_State* L, T& t)
        {
            new (lua_newuserdata(L, sizeof(ClassDelegate<T>))) ClassDelegate<T>(t);
        }
    };

    template<typename T>
    struct PtrToLua
    {
        static void push(lua_State* L, T* ptr)
        {
            if(ptr)
            {
                new (lua_newuserdata(L, sizeof(PtrToUser<T>))) PtrToUser<T>(ptr);
            }
            else
            {
                lua_pushnil(L);
            }

        }
    };

    template<typename T>
    struct RefToLua
    {
        static void push(lua_State* L, T& ref)
        {
            new(lua_newuserdata(L, sizeof(RefToUser<T>))) RefToUser<T>(ref);
        }
    };

    template<typename T>
    struct EnumToLua
    {
        static void push(lua_State* L, T value)
        {
            lua_pushnumber(L, (int)(value));
        }
    };

    template<typename T>
    struct ObjectToLua
    {
        static void push(lua_State* L, T t)
        {
            typedef PtrToLua<typename BaseType<T>::Type> Ptr;
            typedef RefToLua<typename BaseType<T>::Type> Ref;
            typedef ValueToLua<typename BaseType<T>::Type> Value;
            typedef typename Selector<IsRef<T>::Result, Ref, Value>::Type RefOrValue;
            typedef typename Selector<IsPtr<T>::Result, Ptr, RefOrValue>::Type PtrOrRefOrValue;

            PtrOrRefOrValue::push(L, t);

            push_meta(L, ClassName<typename ClassType<T>::Type>::name());

            lua_setmetatable(L, -2);
        }
    };

    // get upvalue from cclosure
    template<typename T>
    T getUpValue(lua_State* L)
    {
        return LuaUserDataToType<T>::convert(L, lua_upvalueindex(1));
    }

    template<typename T>
    T getFromLuaStack(lua_State* L, int index)
    {
        typedef typename Selector<IsEnum<T>::Result, LuaToEnum<T>, LuaToObject<T> >::Type Type;
        return Type::convert(L, index);
    }

    template<> char*              getFromLuaStack(lua_State* L, int index);
    template<> const char*        getFromLuaStack(lua_State* L, int index);
    template<> char               getFromLuaStack(lua_State* L, int index);
    template<> unsigned char      getFromLuaStack(lua_State* L, int index);
    template<> short              getFromLuaStack(lua_State* L, int index);
    template<> long               getFromLuaStack(lua_State* L, int index);
    template<> unsigned long      getFromLuaStack(lua_State* L, int index);
    template<> int                getFromLuaStack(lua_State* L, int index);
    template<> unsigned int       getFromLuaStack(lua_State* L, int index);
    template<> float              getFromLuaStack(lua_State* L, int index);
    template<> double             getFromLuaStack(lua_State* L, int index);
    template<> bool               getFromLuaStack(lua_State* L, int index);
    template<> void               getFromLuaStack(lua_State* L, int index);
    template<> long long          getFromLuaStack(lua_State* L, int index);
    template<> unsigned long long getFromLuaStack(lua_State* L, int index);
    //template<> table              getFromLuaStack(lua_State* L, int index);

    template<typename T>
    void pushToLuaStack(lua_State* L, T t)
    {
        typedef typename Selector<IsEnum<T>::Result, EnumToLua<T>, ObjectToLua<T>>::Type EnumOrObjectToLua;
        EnumOrObjectToLua::push(L, t);
    }

    template<> void pushToLuaStack(lua_State *L, char val);
    template<> void pushToLuaStack(lua_State *L, unsigned char val);
    template<> void pushToLuaStack(lua_State *L, short val);
    template<> void pushToLuaStack(lua_State *L, unsigned short val);
    template<> void pushToLuaStack(lua_State *L, long val);
    template<> void pushToLuaStack(lua_State *L, unsigned long val);
    template<> void pushToLuaStack(lua_State *L, int val);
    template<> void pushToLuaStack(lua_State *L, unsigned int val);
    template<> void pushToLuaStack(lua_State *L, float val);
    template<> void pushToLuaStack(lua_State *L, double val);
    template<> void pushToLuaStack(lua_State *L, char* ptr);
    template<> void pushToLuaStack(lua_State *L, const char* ptr);
    template<> void pushToLuaStack(lua_State *L, bool val);
    template<> void pushToLuaStack(lua_State *L, long long val);
    template<> void pushToLuaStack(lua_State *L, unsigned long long val);
    //template<> void pushToLuaStack(lua_State *L, table val);
    
    template<typename T>
    T popFromLuaStack(lua_State* L)
    {
        T t = getFromLuaStack<T>(L, -1);
        lua_pop(L, 1);
        return t;
    }

    // with return value
    template<typename RVal, typename ... Args>
    struct FunctionDelegate
    {
        static int call(lua_State* L)
        {
            typedef RVal(*F)(Args...);
            F fun = getUpValue<F>(L);
            int index = 1;
            pushToLuaStack(L, (*fun)(getFromLuaStack<Args>(L, index++)...) ); // Parameter pack
            enum_stack(L);
            printf("PushToLuaStack %d\n", index);
            return 1; // hint 1 return value in stack
        }
    };

    // without return value
    template<typename ... Args>
    struct FunctionDelegate<void, Args...>
    {
        static int call(lua_State* L)
        {
            typedef void(*F)(Args...);
            F fun = getUpValue<F>(L);
            int index = 1;
            
            (*fun)(getFromLuaStack<Args>(L, index++)...); // Parameter pack
            return 0; // hint 0 return value in stack
        }
    };

    template<typename RVal, typename ... Args>
    void pushFunctionDelegate(lua_State* L, RVal (*func)(Args ...))
    {
        lua_pushcclosure(L, FunctionDelegate<RVal, Args...>::call, 1);
    }

    struct VariableBase
    {
        virtual void get(lua_State* L) = 0;
        virtual void set(lua_State* L) = 0;
    };

    template<typename T, typename V>
    struct MemberVariable: VariableBase
    {
        V T::*_var;
        MemberVariable(V T::*val) : _var(val) {}

        void get(lua_State* L)
        {
            typedef typename Selector<IsObject<V>::Result, V&, V>::Type Type;
            pushToLuaStack<Type>(L, getFromLuaStack<T*>(L, 1)->*(_var));
        }

        void set(lua_State* L)
        {
            getFromLuaStack<T*>(L, 1)->*(_var) = getFromLuaStack<V>(L, 3);
        }
    };

    // with return value
    template<typename RVal, typename T, typename ... Args>
    struct MemberFunctionDelegate
    {
        static int call(lua_State* L)
        {
            typedef RVal(T::*MemFunc)(Args&& ...);
            MemFunc fun = getUpValue<MemFunc>(L);
            int index = 1;
            T* t = getFromLuaStack<T*>(L, index++);
            pushToLuaStack(L, t->*fun(getFromLuaStack<Args>(index++)...));
        
            return 1; // hint 1 return value in stack
        }
    };

    // without return value
    template<typename T, typename ... Args>
    struct MemberFunctionDelegate<void,T,Args ...>
    {
        static int call(lua_State* L)
        {
            typedef void(T::*MemFunc)(Args&& ...);
            MemFunc fun = getUpValue<MemFunc>(L);
            int index = 1;
            T* t = getFromLuaStack<T*>(L, index++);
            t->*fun(getFromLuaStack<Args>(index++)...);

            return 0; // hint 0 return value in stack
        }
    };

    template<typename T, typename RVal, typename ... Args>
    void pushFunctionDelegate(lua_State* L, RVal (T::*func)(Args&&...))
    {
        lua_pushcclosure(L, MemberFunctionDelegate<RVal,Args...>::call, 1);
    }

    template<typename T, typename ...Args>
    int constructor(lua_State* L)
    {
        int index = 2;
        new(lua_newuserdata(L, sizeof(ClassDelegate<T>))) ClassDelegate<Args ...>(getFromLuaStack<Args>(index++)...);
        push_meta(L, ClassName<typename ClassType<T>::Type>::Name());
        lua_setmetatable(L, -2);

        return 1;
    }

    template<typename T>
    void destroyer(lua_State* L)
    {
        ((UserBase*)lua_touserdata(L, 1))->~UserBase();
    }

    //C++ register global function to lua
    template <typename F>
    void registerFunction(lua_State* L, const char* name, F func)
    {
        lua_pushlightuserdata(L, (void*)func);
        pushFunctionDelegate(L, func);
        lua_setglobal(L, name);
    }

    // C++ register global variable to lua
    template<typename T>
    void registerVariable(lua_State* L, const char* name, T object)
    {
        pushToLuaStack<T>(L, object);
        lua_setglobal(L, name);
    }

    // C++ get Global Variable from lua
    template<typename T>
    void getVariable(lua_State* L, const char* name)
    {
        lua_getglobal(L, name);
        return popFromLuaStack<T>(L);
    }

    // C++ call funcion defined by lua
    template<typename RVal, typename ... Args>
    RVal callLuaFunction(lua_State* L, const char* name, Args&&... args)
    {
        lua_pushcclosure(L, on_error, 0);
        int errfunc = lua_gettop(L);

        lua_getglobal(L, name);
        
        if(lua_isfunction(L, -1))
        {
            int arrlist[] = {(pushToLuaStack(L, args), 0)...};

            lua_pcall(L, sizeof...(args), 1, errfunc);
        }
        else
        {
            printf("Attemp to call global '%s' not a function\n", name);
        }

        lua_remove(L, errfunc);
        return popFromLuaStack<RVal>(L);
    }

    int meta_get(lua_State* L);
    int meta_set(lua_State* L);
    void push_meta(lua_State* L, const char* name);

    template<typename T>
    void registerClass(lua_State* L, const char* name)
    {
        ClassName<T>::name(name);

        lua_newtable(L);

        lua_pushstring(L, "__name");
        lua_pushstring(L, name);
        lua_rawset(L, -3);

        lua_pushstring(L, "__index");
        lua_pushcclosure(L, meta_get, 0);
        lua_rawset(L, -3);

        lua_pushstring(L, "__newindex");
        lua_pushcclosure(L, meta_set, 0);
        lua_rawset(L, -3);

        lua_pushstring(L, "__gc");
        lua_pushcclosure(L, destroyer<T>, 0);
        lua_rawset(L, -3);

        lua_setglobal(L, name);

    }

    template<typename T, typename P>
    void inherientClass(lua_State* L)
    {
        push_meta(L, ClassName<T>::name());
        if(lua_istable(L, -1))
        {
            lua_pushstring(L, "__parent");
            push_meta(L, ClassName<P>::name());
            lua_rawset(L, -3);
        }
        lua_pop(L, 1);
    }

    template<typename T, typename F>
    void addClassConstructor(lua_State* L, F func)
    {
        push_meta(L, ClassName<T>::name());
        if(lua_istable(L, -1))
        {
            lua_newtable(L);
            lua_pushstring(L, "__call");
            lua_pushcclosure(L, func, 0);
            lua_rawset(L, -3);
            lua_setmetatable(L, -2);
        }
        lua_pop(L, 1);
    }

    // 需要做一个检查，判断是T的成员函数
    template<typename T, typename F>
    void registerClassFunction(lua_State* L, const char* name, F func)
    {
        push_meta(L, ClassName<T>::name());
        if(lua_istable(L, -1))
        {
            lua_pushstring(L, name);
            new(lua_newuserdata(L, sizeof(F))) F(func);
            pushFunctionDelegate<F>(L, func);
            lua_rawset(L, -3);
        }
        lua_pop(L, 1);
    }

    template<typename T, typename BASE, typename VAR>
    void registerClassMemberVariable(lua_State* L, const char* name, VAR BASE::*val)
    {
        push_meta(L, ClassName<T>::name());
        if(lua_istable(L, -1))
        {
            lua_pushstring(L, name);
            new(lua_newuserdata(L, sizeof(MemberVariable<BASE,VAR>))) MemberVariable<BASE, VAR>(val);
            lua_rawset(L, -3);
        }
        lua_pop(L, 1);
    }


}

#endif
