/*========================== begin_copyright_notice ============================

Copyright (C) 2022 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#include "driver.h"
#include "device.h"

namespace shim {
namespace ze {

Driver::Driver() {}

Driver &Driver::Instance() {
  static Driver driver;
  return driver;
}

} // namespace ze
} // namespace shim

extern "C" {
SHIM_EXPORT(zeInit);
SHIM_EXPORT(zeDriverGet);
SHIM_EXPORT(zeDriverGetApiVersion);
SHIM_EXPORT(zeDriverGetProperties);
SHIM_EXPORT(zeDriverGetIpcProperties);
SHIM_EXPORT(zeDriverGetExtensionProperties);

ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeInit)(ze_init_flags_t flags) {
  return ZE_RESULT_SUCCESS;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
SHIM_CALL(zeDriverGet)(uint32_t *pCount, ze_driver_handle_t *phDrivers) {
  if (pCount == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
  }

  if (*pCount == 0) {
    *pCount = 1;
  }

  if (phDrivers != nullptr) {
    auto &driver = shim::ze::Driver::Instance();
    *phDrivers = reinterpret_cast<ze_driver_handle_t>(&driver);
  }

  return ZE_RESULT_SUCCESS;
}

ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeDriverGetApiVersion)(
    ze_driver_handle_t hDriver, ze_api_version_t *version) {
  if (hDriver == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }

  if (version == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
  }

  *version = shim::ze::Driver::Instance().version;

  return ZE_RESULT_SUCCESS;
}

ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeDriverGetProperties)(
    ze_driver_handle_t hDriver, ze_driver_properties_t *pDriverProperties) {
  if (hDriver == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }

  if (pDriverProperties == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
  }

  pDriverProperties->uuid = {{0xff}};
  pDriverProperties->driverVersion = shim::ze::Driver::Instance().version;

  return ZE_RESULT_SUCCESS;
}

ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeDriverGetIpcProperties)(
    ze_driver_handle_t hDriver, ze_driver_ipc_properties_t *pIpcProperties) {
  if (hDriver == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }

  pIpcProperties->flags = 0;

  return ZE_RESULT_SUCCESS;
}

ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeDriverGetExtensionProperties)(
    ze_driver_handle_t hDriver, uint32_t *pCount,
    ze_driver_extension_properties_t *pExtensionProperties) {
  if (hDriver == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }

  if (*pCount == 0) {
    *pCount = 0;
  }

  return ZE_RESULT_SUCCESS;
}
}
