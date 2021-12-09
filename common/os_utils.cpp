/*========================== begin_copyright_notice ============================

Copyright (C) 2019 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#include <stdio.h>
#include <algorithm>
#include <cerrno>

#if defined(_WIN32)
#include <windows.h>
#else /* _WIN32 */
#include <dlfcn.h>
#include <link.h>
#include <stdlib.h>
#include <limits.h>
#endif /* _WIN32 */

#include "emu_log.h"
#include "os_utils.h"

// os-dependent functionality
namespace os
{

bool IsLibHandleValid(SharedLibHandle h)
{
#if defined(_WIN32)
    return h != NULL;
#else /* _WIN32 */
    return h != nullptr;
#endif /* _WIN32 */
}

SharedLibHandle LoadSharedLib(const std::string& name)
{
#if defined(_WIN32)
    SharedLibHandle result;
    result = ::LoadLibrary(name.c_str());
    return result;
#else /* _WIN32 */
    SharedLibHandle result;
    result = dlopen(name.c_str(), RTLD_NOW | RTLD_GLOBAL);
    return result;
#endif /* _WIN32 */
}

bool FreeSharedLib(SharedLibHandle h)
{
#if defined(_WIN32)
    return ::FreeLibrary(h) == TRUE;
#else /* _WIN32 */
    return dlclose(h) == 0;
#endif /* _WIN32 */
}

void *GetSharedSymbolAddress(SharedLibHandle h, const std::string& name)
{
#if defined(_WIN32)
    return ::GetProcAddress(h, name.c_str());
#else /* _WIN32 */
    return dlsym(h, name.c_str());
#endif /* _WIN32 */
}

std::string GetSharedLibLocation(SharedLibHandle h)
{
#if defined(_WIN32)
    char buffer[MAX_PATH + 1] = { 0 };
    ::GetModuleFileName(h, buffer, sizeof(buffer));
    return std::string(buffer);
#else /* _WIN32 */
    link_map *map = nullptr;

    dlinfo(h, RTLD_DI_LINKMAP, &map);

    if(!map) {
      GFX_EMU_FAIL_WITH_MESSAGE("Cannot get a DL link map");
    }

    char *path = realpath(map->l_name, nullptr);

    if (!path) {
      GFX_EMU_FAIL_WITH_MESSAGE("Cannot get a shared library path");
    }

    return std::string(path);
#endif /* _WIN32 */
}

/// Create a temporary file and write "bytes" to it. Return filename.
std::string CreateTempFile(const unsigned char* const bytes, size_t num_bytes)
{
#if defined(_WIN32)
    char filename[MAX_PATH + 1] = { 0 };
    UINT success = ::GetTempFileName(".", "tmp", 0, filename);
    if (success == 0)
    {
        // something bad happened...
        GFX_EMU_FAIL_WITH_MESSAGE("Cannot create a temporary file");
    }
    FILE *dll = fopen(filename, "wb");
    size_t written = fwrite(bytes, 1, num_bytes, dll);
    if (num_bytes != written)
    {
        // something bad happened...
        // should we delete the file?..
        GFX_EMU_FAIL_WITH_MESSAGE("Writing to a temporary file failed");
    }
    fclose(dll);
    return std::string(filename);
#else /* _WIN32 */
    char tmpl[] = "tmpXXXXXX";
    int fd = mkstemp(tmpl);

    FILE *dll = fdopen(fd, "wb");

    if (!dll) {
      GFX_EMU_FAIL_WITH_MESSAGE("Cannot create a temporary file");
    }

    size_t written = fwrite(bytes, 1, num_bytes, dll);
    if (num_bytes != written) {
      GFX_EMU_FAIL_WITH_MESSAGE("Temporary file write error: %s", std::strerror(errno));
    }
    fclose(dll);

    char *path = realpath(tmpl, NULL);

    if (!path) {
      GFX_EMU_FAIL_WITH_MESSAGE("Cannot get an absolute path for a temporary file");
    }

    return std::string(path);
#endif /* _WIN32 */
}

bool DeleteFile(const std::string& name)
{
#if defined(_WIN32)
    return ::DeleteFile(name.c_str()) == TRUE;
#else /* _WIN32 */
    return remove(name.c_str()) == 0;
#endif /* _WIN32 */
}

std::string GetEnvVarValue(const std::string& name)
{
#if defined(_WIN32)
    char* var = getenv(name.c_str());
    return var ? std::string(var) : std::string();
#else /* _WIN32 */
    char* var = getenv(name.c_str());
    return var ? std::string(var) : std::string();
#endif /* _WIN32 */
}

} /* namespace os */
