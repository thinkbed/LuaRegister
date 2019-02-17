

extern "C"
{
    #include "lua.h"
    #include "lualib.h"
    #include "lauxlib.h"
};

#include "lua_register.h"

int cpp_add(int arg1, int arg2)
{
    printf("cpp_add %d %d \n", arg1, arg2);
    return arg1 + arg2;
}

int main()
{
    lua_State* L = luaL_newstate();

    luaopen_base(L);

    lua_register::registerFunction(L, "cpp_add", cpp_add);

    lua_register::dofile(L, "example1.lua");

    int result = lua_register::callLuaFunction<int>(L, "lua_func", 3, 4);

    printf("lua_func(3,4) = %d\n", result);

    lua_close(L);

    return 0;
}
