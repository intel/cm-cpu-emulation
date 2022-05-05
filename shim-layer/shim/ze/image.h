/*========================== begin_copyright_notice ============================

Copyright (C) 2022 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef CM_EMU_SHIM_ZE_IMAGE_H
#define CM_EMU_SHIM_ZE_IMAGE_H

#include <variant>

#include "ze.h"
#include "intrusive_pointer.h"

namespace shim {
namespace ze {
class Image : public IntrusiveRefCounter<Image> {
 private:
  class Deleter {
   public:
    Deleter(IntrusivePtr<CmDeviceEmu> dev) : dev_(dev) {}

    template <typename T>
    using EnableForCmSurface = std::void_t<decltype(std::declval<CmDeviceEmu*>()->DestroySurface(std::declval<T*&>()))>;

    template <typename T, typename = EnableForCmSurface<T>>
    void operator()(T *ptr) {
      if (ptr) {
        dev_->DestroySurface(ptr);
      }
    }

   private:
    IntrusivePtr<CmDeviceEmu> dev_;
  };

 public:
  enum class Type {
    k1D,
    k1DArray,
    k2D,
    k2DArray,
    k3D,
    kBuffer,
  };

  Image(IntrusivePtr<CmDeviceEmu> dev, uint32_t size);
  Image(IntrusivePtr<CmDeviceEmu> dev, uint32_t width, uint32_t height, CM_SURFACE_FORMAT format);
  Image(IntrusivePtr<CmDeviceEmu> dev, uint32_t width, uint32_t height, uint32_t depth, CM_SURFACE_FORMAT format);

  SurfaceIndex *GetIndex();

  Type type_;

  using BufferT = std::unique_ptr<CmBuffer, Deleter>;
  using Surface2DT = std::unique_ptr<CmSurface2D, Deleter>;
  using Surface3DT = std::unique_ptr<CmSurface3D, Deleter>;

  std::variant<BufferT, Surface2DT, Surface3DT> surface_;
};

} // namespace ze
} // namespace shim

extern "C" {
ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeImageGetProperties)(
    ze_device_handle_t hDevice, const ze_image_desc_t *desc,
    ze_image_properties_t *pImageProperties);
ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeImageCreate)(
    ze_context_handle_t hContext, ze_device_handle_t hDevice,
    const ze_image_desc_t *desc, ze_image_handle_t *phImage);
ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeImageDestroy)(
    ze_image_handle_t hImage);
} // extern "C"

#endif // CM_EMU_SHIM_ZE_IMAGE_H
