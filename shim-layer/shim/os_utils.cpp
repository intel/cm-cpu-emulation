/*===================== begin_copyright_notice ==================================

 Copyright (c) 2020, Intel Corporation


 Permission is hereby granted, free of charge, to any person obtaining a
 copy of this software and associated documentation files (the "Software"),
 to deal in the Software without restriction, including without limitation
 the rights to use, copy, modify, merge, publish, distribute, sublicense,
 and/or sell copies of the Software, and to permit persons to whom the
 Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included
 in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 OTHER DEALINGS IN THE SOFTWARE.
======================= end_copyright_notice ==================================*/

#include <stdio.h>
#include <algorithm>

#include <dlfcn.h>
#include <link.h>
#include <stdlib.h>
#include <limits.h>

#include "os_utils.h"

// os-dependent functionality
namespace os
{

bool IsLibHandleValid(SharedLibHandle h)
{
    return h != nullptr;
}

SharedLibHandle LoadSharedLib(const std::string& name)
{
    SharedLibHandle result;
    result = dlopen(name.c_str(), RTLD_NOW | RTLD_GLOBAL);
    return result;
}

bool FreeSharedLib(SharedLibHandle h)
{
    return dlclose(h) == 0;
}

void *GetSharedSymbolAddress(SharedLibHandle h, const std::string& name)
{
    return dlsym(h, name.c_str());
}

std::string GetSharedLibLocation(SharedLibHandle h)
{
    link_map *map;
    dlinfo(h, RTLD_DI_LINKMAP, &map);
    if(!map) {
        return std::string();
    }
    char *path = realpath(map->l_name, NULL);
    return std::string(path);
}

/// Create a temporary file and write "bytes" to it. Return filename.
std::string CreateTempFile(const unsigned char* const bytes, size_t num_bytes)
{

    char tmpl[] = "tmpXXXXXX";
    int fd = mkstemp(tmpl);
    FILE *dll = fdopen(fd, "wb");
    size_t written = fwrite(bytes, 1, num_bytes, dll);
    fclose(dll);
    if (num_bytes != written)
    {
        // something bad happened...
        // should we delete the file?..
    }
    char *path = realpath(tmpl, NULL);
    return std::string(path);
    return std::string();
}

bool DeleteFile(const std::string& name)
{
    return remove(name.c_str()) == 0;
}

std::string GetEnvVarValue(const std::string& name)
{
    char* var = getenv(name.c_str());
    return var ? std::string(var) : std::string();
}

} /* namespace os */
