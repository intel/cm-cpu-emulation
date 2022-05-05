/*========================== begin_copyright_notice ============================

Copyright (C) 2022 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef CM_EMU_SHIM_ZE_DEVICE_H
#define CM_EMU_SHIM_ZE_DEVICE_H

#include "ze.h"

extern "C" {
ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeDeviceGet)(
    ze_driver_handle_t hDriver,
    uint32_t *pCount,
    ze_device_handle_t *phDevices);

ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeDeviceGetSubDevices)(
    ze_device_handle_t hDevice,
    uint32_t *pCount,
    ze_device_handle_t *phSubdevices);

ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeDeviceGetProperties)(
    ze_device_handle_t hDevice,
    ze_device_properties_t *pDeviceProperties);

ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeDeviceGetComputeProperties)(
    ze_device_handle_t hDevice,
    ze_device_compute_properties_t *pComputeProperties);

ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeDeviceGetModuleProperties)(
    ze_device_handle_t hDevice,
    ze_device_module_properties_t *pModuleProperties);

ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeDeviceGetCommandQueueGroupProperties)(
    ze_device_handle_t hDevice,
    uint32_t *pCount,
    ze_command_queue_group_properties_t *pCommandQueueGroupProperties);

ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeDeviceGetMemoryProperties)(
    ze_device_handle_t hDevice,
    uint32_t *pCount,
    ze_device_memory_properties_t *pMemProperties);

ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeDeviceGetMemoryAccessProperties)(
    ze_device_handle_t hDevice,
    ze_device_memory_access_properties_t *pMemAccessProperties);

ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeDeviceGetCacheProperties)(
    ze_device_handle_t hDevice,
    uint32_t *pCount,
    ze_device_cache_properties_t *pCacheProperties);

ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeDeviceGetImageProperties)(
    ze_device_handle_t hDevice,
    ze_device_image_properties_t *pImageProperties);

ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeDeviceGetExternalMemoryProperties)(
    ze_device_handle_t hDevice,
    ze_device_external_memory_properties_t *pExternalMemoryProperties);

ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeDeviceGetP2PProperties)(
    ze_device_handle_t hDevice,
    ze_device_handle_t hPeerDevice,
    ze_device_p2p_properties_t *pP2PProperties);

ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeDeviceCanAccessPeer)(
    ze_device_handle_t hDevice,
    ze_device_handle_t hPeerDevice,
    ze_bool_t *value);

ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeDeviceGetStatus)(
    ze_device_handle_t hDevice);
}
#endif // CM_EMU_SHIM_ZE_DEVICE_H
