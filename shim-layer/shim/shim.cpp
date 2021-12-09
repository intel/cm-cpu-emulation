/*========================== begin_copyright_notice ============================

Copyright (C) 2019 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#include <iostream>
#include <string>
#include <type_traits>

#include <cm_rt.h>

using std::cout;
using std::endl;
using std::string;

#if defined(_WIN32)
using Kernel = void(__cdecl)();
using KernelPointer = std::add_pointer<Kernel>::type;

static constexpr auto LIBRARY_NAME = "kernel.dll";
static constexpr auto KERNEL_NAME = "linear";

void print_last_error()
{
    DWORD err = GetLastError();
    LPSTR lpMsgBuf;
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        err,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf,
        0, NULL);
    cout << string(lpMsgBuf) << endl;
    LocalFree(lpMsgBuf);
}

KernelPointer load_kernel(const string& library_name, const string& kernel_name)
{
    HMODULE m = LoadLibrary(library_name.c_str());
    if (m == NULL)
    {
        print_last_error();
    }
    FARPROC p = GetProcAddress(m, kernel_name.c_str());
    if (p == NULL)
    {
        print_last_error();
    }
    return reinterpret_cast<KernelPointer>(p);
}

int test()
{
    int result = 0;

    CmDevice *device = nullptr;
    UINT version = 0;
    result = ::CreateCmDevice(device, version);
    if (result != CM_SUCCESS) {
        cout << "CmDevice creation error" << endl;
        return -1;
    }
    if (version < CM_1_0) {
        cout << "The runtime API version is later than runtime DLL version" << endl;
        return -1;
    }

    CmProgram *program = nullptr;
    result = device->LoadProgram(nullptr, 0, program);
    if (result != CM_SUCCESS) {
        cout << "CM LoadProgram error" << endl;
        return -1;
    }

    CmKernel *kernel = nullptr;
    KernelPointer kernel_func = load_kernel(LIBRARY_NAME, KERNEL_NAME);
    cout << "Kernel addr: " << kernel_func << endl;
    result = device->CreateKernel(program, "dummy", static_cast<void*>(kernel_func), kernel);
    if (result != CM_SUCCESS)
    {
        cout << "CM CreateKernel error" << endl;
    }

    CmQueue *queue = nullptr;
    result = device->CreateQueue(queue);
    if (result != CM_SUCCESS)
    {
        cout << "CM CreateQueue error" << endl;
    }

    CmTask *task = nullptr;
    result = device->CreateTask(task);
    if (result != CM_SUCCESS)
    {
        cout << "CM CreateTask error" << endl;
    }

    result = task->AddKernel(kernel);
    if (result != CM_SUCCESS)
    {
        cout << "CM AddKernel error" << endl;
    }

    CmThreadSpace *ts = nullptr;
    result = device->CreateThreadSpace(5, 1, ts);
    if (result != CM_SUCCESS)
    {
        cout << "CM CreateThreadSpace error" << endl;
    }

    CmEvent *e = nullptr;
    result = queue->Enqueue(task, e, ts);
    if (result != CM_SUCCESS)
    {
        cout << "CM Enqueue error" << endl;
    }

    result = e->WaitForTaskFinished();
    if (result != CM_SUCCESS)
    {
        cout << "CM Wait error" << endl;
    }

    result = device->DestroyTask(task);
    if (result != CM_SUCCESS)
    {
        cout << "CM DestroyTask error" << endl;
    }

    result = ::DestroyCmDevice(device);
    if (result != CM_SUCCESS)
    {
        cout << "CM DestroyDevice error" << endl;
    }

    return result;
}
#endif /* _WIN32 */

void run_shim_unit_tests()
{
#if defined(_WIN32)
    test();
#endif /* _WIN32 */
}

// currently not MP-safe
bool hwMode = false;
bool isHwMode() { return hwMode; }
void setHwMode(bool hw) { hwMode = hw; };
