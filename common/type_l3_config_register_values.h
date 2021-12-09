/*========================== begin_copyright_notice ============================

Copyright (C) 2020 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef GUARD_common_type_l3_config_register_values_h
#define GUARD_common_type_l3_config_register_values_h
struct L3ConfigRegisterValues
{
    L3ConfigRegisterValues(): configRegister0(0),
                              configRegister1(0),
                              configRegister2(0),
                              configRegister3(0) {}

    L3ConfigRegisterValues(uint32_t registerValue0,
                           uint32_t registerValue1,
                           uint32_t registerValue2,
                           uint32_t registerValue3):
        configRegister0(registerValue0),
        configRegister1(registerValue1),
        configRegister2(registerValue2),
        configRegister3(registerValue3) {}

    uint32_t configRegister0;
    uint32_t configRegister1;
    uint32_t configRegister2;
    uint32_t configRegister3;
};

#endif // GUARD_common_type_l3_config_register_values_h
