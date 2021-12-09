/*========================== begin_copyright_notice ============================

Copyright (C) 2019 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#pragma once

#if defined(_WIN32)
#define SHIM_API_EXPORT __declspec(dllexport)
#else /* _WIN32 */
#define SHIM_API_EXPORT
#endif /* _WIN32 */
