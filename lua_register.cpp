#include <iostream>

extern "C"
{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

#include "lua_register.h"

void lua_register::init(lua_State* L)
{
    registerSigned64(L);
    registerUnsigned64(L);
}

static int signed64ToString(lua_State* L)
{
    char temp[64];
    sprintf(temp, "%lld", *(long long*)lua_topointer(L, 1));
    lua_pushstring(L, temp);
    return 1;
}

static int signed64Equal(lua_State* L)
{
    long long num1 = *(long long*)lua_topointer(L, 1);
    long long num2 = *(long long*)lua_topointer(L, 2);
    lua_pushboolean(L, num1 == num2);

    return 1;
}

static int signed64LessThan(lua_State* L)
{
    long long num1 = *(long long*)lua_topointer(L, 1);
    long long num2 = *(long long*)lua_topointer(L, 2);
    lua_pushboolean(L, num1 < num2);

    return 1;
}

static int signed64LessEqual(lua_State* L)
{
    long long num1 = *(long long*)lua_topointer(L, 1);
    long long num2 = *(long long*)lua_topointer(L, 2);
    lua_pushboolean(L, num1 <= num2);

    return 1;
}

void lua_register::registerSigned64(lua_State* L)
{
    const char* name = "__Signed64";
    lua_newtable(L);

    lua_pushstring(L, "__name");
    lua_pushstring(L, name);
    lua_rawset(L, -3);

    lua_pushstring(L, "__tostring");
    lua_pushcclosure(L, signed64ToString, 0);
    lua_rawset(L, -3);

    lua_pushstring(L, "__eq");
    lua_pushcclosure(L, signed64Equal, 0);
    lua_rawset(L, -3);

    lua_pushstring(L, "__lt");
    lua_pushcclosure(L, signed64LessThan, 0);
    lua_rawset(L, -3);

    lua_pushstring(L, "__le");
    lua_pushcclosure(L, signed64LessEqual, 0);
    lua_rawset(L, -3);

    lua_setglobal(L, name);
}

static int unsigned64ToString(lua_State* L)
{
    char temp[64];
    sprintf(temp, "%llu", *(unsigned long long*)lua_topointer(L, 1));
    lua_pushstring(L, temp);

    return 1;
}

static int unsigned64Equal(lua_State* L)
{
    unsigned long long num1 = *(unsigned long long*)lua_topointer(L, 1);
    unsigned long long num2 = *(unsigned long long*)lua_topointer(L, 2);
    lua_pushboolean(L, num1 == num2);

    return 1;
}

static int unsigned64LessThan(lua_State* L)
{
    unsigned long long num1 = *(unsigned long long*)lua_topointer(L, 1);
    unsigned long long num2 = *(unsigned long long*)lua_topointer(L, 2);
    lua_pushboolean(L, num1 < num2);

    return 1;
}

static int unsigned64LessEqual(lua_State* L)
{
    unsigned long long num1 = *(unsigned long long*)lua_topointer(L, 1);
    unsigned long long num2 = *(unsigned long long*)lua_topointer(L, 2);
    lua_pushboolean(L, num1 <= num2);

    return 1;
}

void lua_register::registerUnsigned64(lua_State* L)
{
    const char* name = "__Unsigned64";

    lua_newtable(L);

    lua_pushstring(L, "__name");
    lua_pushstring(L, name);
    lua_rawset(L, -3);

    lua_pushstring(L, "__tostring");
    lua_pushcclosure(L, unsigned64ToString, 0);
    lua_rawset(L, -3);

    lua_pushstring(L, "__eq");
    lua_pushcclosure(L, unsigned64Equal, 0);
    lua_rawset(L, -3);

    lua_pushstring(L, "__lt");
    lua_pushcclosure(L, unsigned64LessThan, 0);
    lua_rawset(L, -3);

    lua_pushstring(L, "__le");
    lua_pushcclosure(L, unsigned64LessEqual, 0);
    lua_rawset(L, -3);

    lua_setglobal(L, name);
}

void lua_register::dofile(lua_State* L, const char* filename)
{
    lua_pushcclosure(L, on_error, 0);
    int errfunc = lua_gettop(L);

    if(luaL_loadfile(L, filename) == 0)
    {
        lua_pcall(L, 0, 1, errfunc);
    }
    else
    {
        print_error(L, "%s", lua_tostring(L, -1));
    }

    lua_remove(L, errfunc);
    lua_pop(L, 1);
}

void lua_register::dostring(lua_State* L, const char* buff)
{
    lua_register::dobuffer(L, buff, strlen(buff));
}

void lua_register::dobuffer(lua_State* L, const char* buff, size_t len)
{
    lua_pushcclosure(L, on_error, 0);
    int errfunc = lua_gettop(L);

    if(luaL_loadbuffer(L, buff, len, "lua_register::dobuffer()") == 0)
    {
        lua_pcall(L, 0, 1, errfunc);
    }
    else
    {
        print_error(L, "%s", lua_tostring(L, -1));
    }

    lua_remove(L, errfunc);
    lua_pop(L, 1);
}

static void call_stack(lua_State* L, int n)
{
    lua_Debug debugInfo;
    if(lua_getstack(L, n, &debugInfo) == 1)
    {
        lua_getinfo(L, "nSlu", &debugInfo);

        const char* indent;
        if(n == 0)
        {
            indent = "->\t";
            lua_register::print_error(L, "\t<call stack>");
        }
        else
        {
            indent = "\t";
        }

        if(debugInfo.name)
            lua_register::print_error(L, "%s%s() : line %d [%s : line %d]", indent, debugInfo.name, debugInfo.currentline, debugInfo.source, debugInfo.linedefined);
        else

            lua_register::print_error(L, "%sunknow : line %d [%s : line %d]", indent,  debugInfo.currentline, debugInfo.source, debugInfo.linedefined);
        call_stack(L, n+1);
    }
}

int lua_register::on_error(lua_State* L)
{
    print_error(L, "%s", lua_tostring(L, -1));

    call_stack(L, 0);

    return 0;
}

void lua_register::print_error(lua_State* L, const char* fmt, ...)
{
    char text[10240];

    va_list args;
    va_start(args, fmt);
    vsprintf(text, fmt, args);
    va_end(args);

    lua_getglobal(L, "_ALERT");
    if(lua_isfunction(L, -1))
    {
        lua_pushstring(L, text);
        lua_call(L, 1, 0);
    }
    else
    {
        printf("%s\n", text);
        lua_pop(L, 1);
    }
}

void lua_register::enum_stack(lua_State *L)
{
    int top = lua_gettop(L);
    print_error(L, "Type:%d", top);
    for(int i=1; i<=lua_gettop(L); ++i)
    {
        switch(lua_type(L, i))
        {
        case LUA_TNIL:
            print_error(L, "\t%s", lua_typename(L, lua_type(L, i)));
            break;
        case LUA_TBOOLEAN:
            print_error(L, "\t%s	%s", lua_typename(L, lua_type(L, i)), lua_toboolean(L, i)?"true":"false");
            break;
        case LUA_TLIGHTUSERDATA:
            print_error(L, "\t%s	0x%08p", lua_typename(L, lua_type(L, i)), lua_topointer(L, i));
            break;
        case LUA_TNUMBER:
            print_error(L, "\t%s	%f", lua_typename(L, lua_type(L, i)), lua_tonumber(L, i));
            break;
        case LUA_TSTRING:
            print_error(L, "\t%s	%s", lua_typename(L, lua_type(L, i)), lua_tostring(L, i));
            break;
        case LUA_TTABLE:
            print_error(L, "\t%s	0x%08p", lua_typename(L, lua_type(L, i)), lua_topointer(L, i));
            break;
        case LUA_TFUNCTION:
            print_error(L, "\t%s()	0x%08p", lua_typename(L, lua_type(L, i)), lua_topointer(L, i));
            break;
        case LUA_TUSERDATA:
            print_error(L, "\t%s	0x%08p", lua_typename(L, lua_type(L, i)), lua_topointer(L, i));
            break;
        case LUA_TTHREAD:
            print_error(L, "\t%s", lua_typename(L, lua_type(L, i)));
            break;
		}
	}
}

template<>
char* lua_register::getFromLuaStack(lua_State* L, int index)
{
    return (char*)lua_tostring(L, index);
}

template<>
const char* lua_register::getFromLuaStack(lua_State* L, int index)
{
    return (const char*)lua_tostring(L, index);
}

template<>
char lua_register::getFromLuaStack(lua_State* L, int index)
{
    return (char)lua_tonumber(L, index);
}

template<>
unsigned char lua_register::getFromLuaStack(lua_State* L, int index)
{
    return (unsigned char)lua_tonumber(L, index);
}

template<>
short lua_register::getFromLuaStack(lua_State* L, int index)
{
    return (short)lua_tonumber(L, index);
}

template<>
unsigned short lua_register::getFromLuaStack(lua_State* L, int index)
{
    return (unsigned short)lua_tonumber(L, index);
}

template<>
long lua_register::getFromLuaStack(lua_State* L, int index)
{
    return (long)lua_tonumber(L, index);
}

template<>
unsigned long lua_register::getFromLuaStack(lua_State* L, int index)
{
    return (unsigned long)lua_tonumber(L, index);
}

template<>
int lua_register::getFromLuaStack(lua_State* L, int index)
{
    printf("getFromLuaStack<int> %d\n", (int)lua_tonumber(L, index));
    return (int)lua_tonumber(L, index);
}

template<>
unsigned int lua_register::getFromLuaStack(lua_State* L, int index)
{
    return (unsigned int)lua_tonumber(L, index);
}

template<>
float lua_register::getFromLuaStack(lua_State* L, int index)
{
    return (float)lua_tonumber(L, index);
}

template<>
double lua_register::getFromLuaStack(lua_State* L, int index)
{
    return (double)lua_tonumber(L, index);
}

template<>
bool lua_register::getFromLuaStack(lua_State* L, int index)
{
    if(lua_isboolean(L, index))
        return lua_toboolean(L, index) != 0;
    else
        return lua_tonumber(L, index) != 0;
}

template<>
void lua_register::getFromLuaStack(lua_State* L, int index)
{
    return;
}

template<>
long long lua_register::getFromLuaStack(lua_State* L, int index)
{
    return (long long)lua_tonumber(L, index);
}

template<>
unsigned long long lua_register::getFromLuaStack(lua_State* L, int index)
{
    return (unsigned long long)lua_tonumber(L, index);
}

template<>
void lua_register::pushToLuaStack(lua_State* L, char val)
{
    lua_pushnumber(L, val);
}

template<>
void lua_register::pushToLuaStack(lua_State* L,unsigned char val)
{
    lua_pushnumber(L, val);
}

template<>
void lua_register::pushToLuaStack(lua_State* L, short val)
{
    lua_pushnumber(L, val);
}

template<>
void lua_register::pushToLuaStack(lua_State* L, unsigned short val)
{
    lua_pushnumber(L, val);
}

template<>
void lua_register::pushToLuaStack(lua_State* L, long val)
{
    lua_pushnumber(L, val);
}

template<>
void lua_register::pushToLuaStack(lua_State* L, unsigned long val)
{
    lua_pushnumber(L, val);
}

template<>
void lua_register::pushToLuaStack(lua_State* L, int val)
{
    lua_pushnumber(L, val);
}

template<>
void lua_register::pushToLuaStack(lua_State* L, unsigned int val)
{
    lua_pushnumber(L, val);
}

template<>
void lua_register::pushToLuaStack(lua_State* L, float val)
{
    lua_pushnumber(L, val);
}

template<>
void lua_register::pushToLuaStack(lua_State* L, double val)
{
    lua_pushnumber(L, val);
}

template<>
void lua_register::pushToLuaStack(lua_State* L, char* val)
{
    lua_pushstring(L, val);
}

template<>
void lua_register::pushToLuaStack(lua_State* L, const char* val)
{
    lua_pushstring(L, val);
}

template<>
void lua_register::pushToLuaStack(lua_State* L, bool val)
{
    lua_pushboolean(L, val);
}

template<>
void lua_register::pushToLuaStack(lua_State* L, long long val)
{
    *(long long*)lua_newuserdata(L, sizeof(long long)) = val;
    lua_getglobal(L, "__Signed64");
    lua_setmetatable(L, -2);
}

template<>
void lua_register::pushToLuaStack(lua_State* L, unsigned long long val)
{
    *(unsigned long long*)lua_newuserdata(L, sizeof(unsigned long long)) = val;
    lua_getglobal(L, "__Unsigned64");
    lua_setmetatable(L, -2);
}

static void invoke_parent(lua_State* L)
{
    lua_pushstring(L, "__parent");
    lua_rawget(L, -2);
    if(lua_istable(L, -1))
    {
        lua_pushvalue(L, 2);
        lua_rawget(L, -2);

        if(!lua_isnil(L, -1))
        {
            lua_remove(L, -2);
        }
        else
        {
            lua_remove(L, -1);
            invoke_parent(L);
            lua_remove(L, -2);
        }

    }
}

int lua_register::meta_get(lua_State* L)
{
    lua_getmetatable(L, 1);
    lua_pushvalue(L, 2);
    lua_rawget(L, -2);

    if(lua_isuserdata(L, -1))
    {
       LuaUserDataToType<VariableBase*>::convert(L, -1)->get(L);
       lua_remove(L, -2);
    }
    else if(lua_isnil(L, -1))
    {
        lua_remove(L, -1);
        invoke_parent(L);

        if(lua_isuserdata(L, -1))
        {
            LuaUserDataToType<VariableBase*>::convert(L, -1)->get(L);
            lua_remove(L, -2);
        }
        else
        {
            lua_pushfstring(L, "Can't find '%s' class variable. (forgot registering class variable?)", lua_tostring(L, 2));
            lua_error(L);
        }
    }

    lua_remove(L, -2);

    return 1;
}

int lua_register::meta_set(lua_State* L)
{
    lua_getmetatable(L, 1);
    lua_pushvalue(L, 2);
    lua_rawget(L, -2);

    if(lua_isuserdata(L, -1))
    {
        LuaUserDataToType<VariableBase*>::convert(L, -1)->set(L);
    }
    else if(lua_isnil(L, -1))
    {
       invoke_parent(L);

       if(lua_isuserdata(L, -1))
       {
           LuaUserDataToType<VariableBase*>::convert(L,-1)->set(L);
       }
       else
       {
       }
    }
    lua_settop(L, 3);

    return 0;
}

void lua_register::push_meta(lua_State* L, const char* name)
{
    lua_getglobal(L, name);
}
