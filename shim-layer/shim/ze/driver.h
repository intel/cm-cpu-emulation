/*========================== begin_copyright_notice ============================

Copyright (C) 2022 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef CM_EMU_SHIM_ZE_DRIVER_H
#define CM_EMU_SHIM_ZE_DRIVER_H

#include "ze.h"

namespace shim {
namespace ze {

class Driver {
 public:
  static Driver &Instance();

  static constexpr ze_api_version_t version = ZE_API_VERSION_1_0;

 private:
  Driver();
  Driver(const Driver &) = delete;
  Driver(Driver &&) = delete;

  Driver &operator =(const Driver &) = delete;
  Driver &operator =(Driver &&) = delete;

  ~Driver() = default;
};

} // namespace ze
} // namespace shim

extern "C" {
ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeInit)(ze_init_flags_t flags);

ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeDriverGet)(
    uint32_t *pCount,
    ze_driver_handle_t *phDrivers);

ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeDriverGetApiVersion)(
    ze_driver_handle_t hDriver,
    ze_api_version_t *version);

ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeDriverGetProperties)(
    ze_driver_handle_t hDriver,
    ze_driver_properties_t *pDriverProperties);

ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeDriverGetIpcProperties)(
    ze_driver_handle_t hDriver,
    ze_driver_ipc_properties_t *pIpcProperties);

ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeDriverGetExtensionProperties)(
    ze_driver_handle_t hDriver,
    uint32_t *pCount,
    ze_driver_extension_properties_t *pExtensionProperties);
}

#endif // CM_EMU_SHIM_ZE_DRIVER_H
