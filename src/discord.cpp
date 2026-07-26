#define DISCORDPP_IMPLEMENTATION
#include <discordsdk/discord.h>

static std::string librarySearchPath;

void discordpp::SetLibrarySearchPath(const std::string_view path)
{
    librarySearchPath = path;
}

std::string_view discordpp::GetLibrarySearchPath()
{
    return librarySearchPath;
}

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <delayimp.h>

FARPROC WINAPI DelayLoadRedirectHook(unsigned dliNotify, PDelayLoadInfo pdli)
{
    if (dliNotify == dliNotePreLoadLibrary && !librarySearchPath.empty())
    {
        if (_stricmp(pdli->szDll, "discord_partner_sdk.dll") == 0)
        {
            std::string customPath = librarySearchPath + "\\" + pdli->szDll;

            HMODULE hMod = LoadLibraryA(customPath.c_str());
            if (hMod)
            {
                return (FARPROC)hMod;
            }
        }
    }
    return NULL; // default behavior
}

extern "C" const PfnDliHook __pfnDliNotifyHook2 = DelayLoadRedirectHook;

LONG WINAPI DelayLoadFilter(EXCEPTION_POINTERS *pep)
{
    DWORD excCode = pep->ExceptionRecord->ExceptionCode;
    if (excCode == VcppException(ERROR_SEVERITY_ERROR, ERROR_MOD_NOT_FOUND) ||
        excCode == VcppException(ERROR_SEVERITY_ERROR, ERROR_PROC_NOT_FOUND))
    {
        return EXCEPTION_EXECUTE_HANDLER;
    }
    return EXCEPTION_CONTINUE_SEARCH;
}

bool discordpp::IsAvailable()
{
    __try
    {
        Discord_RunCallbacks();
    }
    __except (DelayLoadFilter(GetExceptionInformation()))
    {
        return false;
    }

    return true;
}

#elif defined(__APPLE__)

bool discordpp::IsAvailable()
{
    return Discord_RunCallbacks != nullptr;
}

#elif defined(__linux__)
#include <dlfcn.h>

extern "C" void *linux_dlopen_callback(const char *lib_name)
{
    if (!librarySearchPath.empty())
    {
        std::string customPath = librarySearchPath + "/" + lib_name;
        void *h = dlopen(customPath.c_str(), RTLD_LAZY | RTLD_LOCAL);
        if (h)
        {
            return h;
        }
    }

    void *h = dlopen(lib_name, RTLD_LAZY | RTLD_LOCAL);
    if (h)
    {
        return h;
    }

    return nullptr;
}

bool discordpp::IsAvailable()
{
    static bool checked = false;
    static void *handle = nullptr;
    if (!checked)
    {
        handle = linux_dlopen_callback("libdiscord_partner_sdk.so");
        checked = true;
    }

    return handle != nullptr;
}

#endif