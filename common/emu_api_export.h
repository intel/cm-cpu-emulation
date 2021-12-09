/*========================== begin_copyright_notice ============================

Copyright (C) 2019 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#pragma once
#ifdef _WIN32
#define GFX_EMU_API __declspec(dllexport)
#define GFX_EMU_API_IMPORT __declspec(dllimport)
#else
#define GFX_EMU_API
#define GFX_EMU_API_IMPORT
#endif
