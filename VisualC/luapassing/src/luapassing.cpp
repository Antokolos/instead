// luapassing.cpp : Defines the exported functions for the DLL application.
// See http://www.wellho.net/mouth/1844_Calling-functions-in-C-from-your-Lua-script-a-first-HowTo.html

#include <stdio.h>
#include "luapassing.h"
#include "adapter.h"

#ifdef _WINDOWS
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
#include <Shellapi.h>
#include <Objbase.h>
#else
#include <dlfcn.h>
#endif

#ifdef _LINUX
#include <stdlib.h>
#endif

static int init(lua_State *L) {
    log("Initializing API...\n");
    if (checkInitFunc()) {
        log("Already initialized!\n");
        lua_pushnumber(L, 0.0);
    } else {
        initFunc();
#ifdef _WINDOWS
        CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
#endif
        log("API initialized.\n");
        lua_pushnumber(L, 1.0);
    }
    return 1;
}

static void checkInit() {
    if (!checkInitFunc()) {
        initFunc();
        log("Warning: init while processing.\n");
    }
}

static int setAchievement(lua_State *L) {
    const char* achievementName = lua_tostring(L, 1);
    bool storeImmediately = lua_toboolean(L, 2);
    checkInit();
    log("Setting achievement '%s'...\n", achievementName);
    setAchievementFunc(achievementName);
    if (storeImmediately) {
        storeFunc();
    }
    log("Achievement set.\n");
    lua_pushnumber(L, 0.0);
    return 1;
}

static int setAchievementProgress(lua_State *L) {
	const char* achievementName = lua_tostring(L, 1);
	int current = (int) lua_tonumber(L, 2);
	int max = (int) lua_tonumber(L, 3);
	bool storeImmediately = lua_toboolean(L, 4);
	checkInit();
	log("Setting achievement progress '%s', %d of %d...\n", achievementName, current, max);
	setAchievementProgressFunc(achievementName, current, max);
	if (storeImmediately) {
		storeFunc();
	}
	log("Achievement progress set.\n");
	lua_pushnumber(L, 0.0);
	return 1;
}

static int setStat(lua_State *L) {
	const char* statName = lua_tostring(L, 1);
	int val = (int)lua_tonumber(L, 2);
	bool storeImmediately = lua_toboolean(L, 3);
	checkInit();
	log("Setting stat '%s' to %d...\n", statName, val);
	setStatFunc(statName, val);
	if (storeImmediately) {
		storeFunc();
	}
	log("Stat set.\n");
	lua_pushnumber(L, 0.0);
	return 1;
}

static int store(lua_State *L) {
    checkInit();
    log("Storing...\n");
    storeFunc();
    log("Done.\n");
    lua_pushnumber(L, 0.0);
    return 1;
}

static int clearAchievement(lua_State *L) {
    const char* achievementName = lua_tostring(L, 1);
    bool storeImmediately = lua_toboolean(L, 2);
    checkInit();
    log("Clearing achievement '%s'...\n", achievementName);
    clearAchievementFunc(achievementName);
    if (storeImmediately) {
        storeFunc();
    }
    log("Achievement cleared.\n");
    lua_pushnumber(L, 0.0);
    return 1;
}

static int resetAll(lua_State *L) {
    log("Resetting all achievements...\n");
    checkInit();
    resetAllFunc();
    log("Done.\n");
    lua_pushnumber(L, 0.0);
    return 1;
}

#ifdef _WINDOWS
static int openURL(lua_State *L) {
    const char* url = lua_tostring(L, 1);
    log("Opening URL '%s'...\n", url);
    ShellExecuteA(NULL, "open", url, NULL, NULL, SW_SHOWNORMAL);
    lua_pushnumber(L, 0.0);
    return 1;
}

extern "C" __declspec(dllexport) int luaopen_luapassing(lua_State *L) {
#elif defined(_LINUX)
static int openURL(lua_State *L) {
    const char* url = lua_tostring(L, 1);
    char buffer[256];
    log("Opening URL '%s'...\n", url);
    sprintf(buffer, "xdg-open %s", (char*) url);
    system(buffer);
    lua_pushnumber(L, 0.0);
    return 1;
}

extern "C" int luaopen_luapassing ( lua_State *L) {
#elif defined(_MACOSX)
static int openURL(lua_State *L) {
    const char* url = lua_tostring(L, 1);
    char buffer[256];
    log("Opening URL '%s'...\n", url);
    sprintf(buffer, "open %s", (char*) url);
    system(buffer);
    lua_pushnumber(L, 0.0);
    return 1;
}

extern "C" int luaopen_luapassing ( lua_State *L) {
#endif
    static const luaL_Reg Map [] = {
        {"init", init},
        {"setAchievement", setAchievement},
		{"setAchievementProgress", setAchievementProgress},
		{"setStat", setStat},
        {"clearAchievement", clearAchievement},
        {"store", store},
        {"resetAll", resetAll},
        {"openURL", openURL},
        {NULL,NULL}
    };
    new_lib(L, Map);
	lua_pushvalue(L, -1);
	lua_setglobal(L, "statsAPI");
	return 1;
}

/*
Interesting code to enum loaded process modules on Windows:
#include <Psapi.h>
HMODULE hMods[1024];
DWORD cbNeeded;
int i;
if (EnumProcessModules(GetCurrentProcess(), hMods, sizeof(hMods), &cbNeeded)) {
    for (i = 0; i < (cbNeeded / sizeof(HMODULE)); i++) {
        TCHAR szModName[MAX_PATH];
        // Get the full path to the module's file.

        if (GetModuleFileName(hMods[i], szModName, MAX_PATH)) {
            int wlength = GetShortPathNameW(szModName, 0, 0);
            LPWSTR shortp = (LPWSTR)calloc(wlength, sizeof(WCHAR));
            GetShortPathNameW(szModName, shortp, wlength);
            int clength = WideCharToMultiByte(CP_OEMCP, WC_NO_BEST_FIT_CHARS, shortp, wlength, 0, 0, 0, 0);
            LPSTR cpath = (LPSTR)calloc(clength, sizeof(CHAR));
            WideCharToMultiByte(CP_OEMCP, WC_NO_BEST_FIT_CHARS, shortp, wlength, cpath, clength, 0, 0);
            // Print the module name and handle value.
            printf("\tz%sz (0x%08X)\n", cpath, hMods[i]);
        }
    }
}
*/
