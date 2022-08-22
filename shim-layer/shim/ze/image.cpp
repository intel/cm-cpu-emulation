/*========================== begin_copyright_notice ============================

Copyright (C) 2022 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#include "image.h"
#include "context.h"

namespace shim::ze {
Image::Image(IntrusivePtr<CmDeviceEmu> dev, uint32_t size)
    : type_(Image::Type::kBuffer),
      surface_(std::in_place_type<BufferT>, nullptr, Deleter(dev)) {
  CmBuffer *surf = nullptr;
  if (auto r = dev->CreateBuffer(size, surf); r != CM_SUCCESS) {
    throw Error(::GetCmErrorString(r));
  }

  auto &s = std::get<BufferT>(surface_);
  s.reset(surf);
}

Image::Image(IntrusivePtr<CmDeviceEmu> dev, uint32_t width, uint32_t height,
             CM_SURFACE_FORMAT format)
    : type_(Image::Type::k2D),
      surface_(std::in_place_type<Surface2DT>, nullptr, Deleter(dev)) {
  CmSurface2D *surf = nullptr;
  if (auto r = dev->CreateSurface2D(width, height, format, surf);
      r != CM_SUCCESS) {
    throw Error(::GetCmErrorString(r));
  }

  auto &s = std::get<Surface2DT>(surface_);
  s.reset(surf);
}

Image::Image(IntrusivePtr<CmDeviceEmu> dev, uint32_t width, uint32_t height,
             uint32_t depth, CM_SURFACE_FORMAT format)
    : surface_(std::in_place_type<Surface3DT>, nullptr, Deleter(dev)) {
  CmSurface3D *surf = nullptr;
  if (auto r = dev->CreateSurface3D(width, height, depth, format, surf);
      r != CM_SUCCESS) {
    throw Error(::GetCmErrorString(r));
  }

  auto &s = std::get<Surface3DT>(surface_);
  s.reset(surf);
}

SurfaceIndex *Image::GetIndex() {
  return std::visit(
      [](auto &p) -> SurfaceIndex * {
        SurfaceIndex *index = nullptr;
        if (auto r = p->GetIndex(index); r != CM_SUCCESS) {
          return nullptr;
        }
        return index;
      },
      surface_);
}
} // namespace shim::ze

namespace {
CM_SURFACE_FORMAT GetCmFormat(ze_image_format_t format) {
  switch (format.layout) {
  case ZE_IMAGE_FORMAT_LAYOUT_8:
    if (format.type == ZE_IMAGE_FORMAT_TYPE_UINT &&
        format.x == ZE_IMAGE_FORMAT_SWIZZLE_R) {
      return CM_SURFACE_FORMAT_R8_UINT;
    } else if (format.type == ZE_IMAGE_FORMAT_TYPE_UNORM) {
      switch (format.x) {
      case ZE_IMAGE_FORMAT_SWIZZLE_R:
        return CM_SURFACE_FORMAT_R8_UNORM;
      case ZE_IMAGE_FORMAT_SWIZZLE_A:
        return CM_SURFACE_FORMAT_A8;
      }
    }

    break;
  case ZE_IMAGE_FORMAT_LAYOUT_16:
    if (format.x != ZE_IMAGE_FORMAT_SWIZZLE_R) {
      break;
    }

    switch (format.type) {
    case ZE_IMAGE_FORMAT_TYPE_UINT:
      return CM_SURFACE_FORMAT_R16_UINT;
    case ZE_IMAGE_FORMAT_TYPE_SINT:
      return CM_SURFACE_FORMAT_R16_SINT;
    case ZE_IMAGE_FORMAT_TYPE_UNORM:
      return CM_SURFACE_FORMAT_R16_UNORM;
    case ZE_IMAGE_FORMAT_TYPE_FLOAT:
      return CM_SURFACE_FORMAT_R16_FLOAT;
    }

    break;
  case ZE_IMAGE_FORMAT_LAYOUT_32:
    if (format.x != ZE_IMAGE_FORMAT_SWIZZLE_R) {
      break;
    }

    switch (format.type) {
    case ZE_IMAGE_FORMAT_TYPE_UINT:
      return CM_SURFACE_FORMAT_R32_UINT;
    case ZE_IMAGE_FORMAT_TYPE_SINT:
      return CM_SURFACE_FORMAT_R32_SINT;
    case ZE_IMAGE_FORMAT_TYPE_FLOAT:
      return CM_SURFACE_FORMAT_R32F;
    }

    break;
  case ZE_IMAGE_FORMAT_LAYOUT_8_8:
    if (format.x == ZE_IMAGE_FORMAT_SWIZZLE_R &&
        format.y == ZE_IMAGE_FORMAT_SWIZZLE_G &&
        format.type == ZE_IMAGE_FORMAT_TYPE_UNORM) {
      return CM_SURFACE_FORMAT_R8G8_UNORM;
    }
    break;
  case ZE_IMAGE_FORMAT_LAYOUT_8_8_8_8:
    if (format.type != ZE_IMAGE_FORMAT_TYPE_UNORM) {
      break;
    }

    if (format.x == ZE_IMAGE_FORMAT_SWIZZLE_R &&
        format.y == ZE_IMAGE_FORMAT_SWIZZLE_G &&
        format.z == ZE_IMAGE_FORMAT_SWIZZLE_B &&
        format.w == ZE_IMAGE_FORMAT_SWIZZLE_A) {
      return CM_SURFACE_FORMAT_A8B8G8R8;
    } else if (format.x == ZE_IMAGE_FORMAT_SWIZZLE_B &&
               format.y == ZE_IMAGE_FORMAT_SWIZZLE_G &&
               format.z == ZE_IMAGE_FORMAT_SWIZZLE_R) {
      switch (format.w) {
      case ZE_IMAGE_FORMAT_SWIZZLE_A:
        return CM_SURFACE_FORMAT_A8R8G8B8;
      case ZE_IMAGE_FORMAT_SWIZZLE_X:
        return CM_SURFACE_FORMAT_X8R8G8B8;
      }
    }

    break;
  case ZE_IMAGE_FORMAT_LAYOUT_16_16:
    if (format.x == ZE_IMAGE_FORMAT_SWIZZLE_R &&
        format.y == ZE_IMAGE_FORMAT_SWIZZLE_G &&
        format.type == ZE_IMAGE_FORMAT_TYPE_UNORM) {
      return CM_SURFACE_FORMAT_R16G16_UNORM;
    }
    break;
  case ZE_IMAGE_FORMAT_LAYOUT_16_16_16_16:
    if (format.x != ZE_IMAGE_FORMAT_SWIZZLE_R ||
        format.y != ZE_IMAGE_FORMAT_SWIZZLE_G ||
        format.z != ZE_IMAGE_FORMAT_SWIZZLE_B ||
        format.w != ZE_IMAGE_FORMAT_SWIZZLE_A) {
      break;
    }

    switch (format.type) {
    case ZE_IMAGE_FORMAT_TYPE_UNORM:
      return CM_SURFACE_FORMAT_A16B16G16R16;
    case ZE_IMAGE_FORMAT_TYPE_FLOAT:
      return CM_SURFACE_FORMAT_A16B16G16R16F;
    }

    break;
  case ZE_IMAGE_FORMAT_LAYOUT_32_32:
    break;
  case ZE_IMAGE_FORMAT_LAYOUT_32_32_32_32:
    if (format.x != ZE_IMAGE_FORMAT_SWIZZLE_R ||
        format.y != ZE_IMAGE_FORMAT_SWIZZLE_G ||
        format.z != ZE_IMAGE_FORMAT_SWIZZLE_B ||
        format.w != ZE_IMAGE_FORMAT_SWIZZLE_A ||
        format.type != ZE_IMAGE_FORMAT_TYPE_FLOAT) {
      break;
    }

    return CM_SURFACE_FORMAT_R32G32B32A32F;
  case ZE_IMAGE_FORMAT_LAYOUT_10_10_10_2:
    if (format.x != ZE_IMAGE_FORMAT_SWIZZLE_R ||
        format.y != ZE_IMAGE_FORMAT_SWIZZLE_G ||
        format.z != ZE_IMAGE_FORMAT_SWIZZLE_B ||
        format.w != ZE_IMAGE_FORMAT_SWIZZLE_A ||
        format.type != ZE_IMAGE_FORMAT_TYPE_UNORM) {
      break;
    }

    return CM_SURFACE_FORMAT_R10G10B10A2;
  case ZE_IMAGE_FORMAT_LAYOUT_11_11_10:
    break;
  case ZE_IMAGE_FORMAT_LAYOUT_5_6_5:
    break;
  case ZE_IMAGE_FORMAT_LAYOUT_5_5_5_1:
    break;
  case ZE_IMAGE_FORMAT_LAYOUT_4_4_4_4:
    break;
  case ZE_IMAGE_FORMAT_LAYOUT_Y8:
    return CM_SURFACE_FORMAT_Y8_UNORM;
  case ZE_IMAGE_FORMAT_LAYOUT_NV12:
    return CM_SURFACE_FORMAT_NV12;
  case ZE_IMAGE_FORMAT_LAYOUT_YUYV:
    return CM_SURFACE_FORMAT_YUY2;
  case ZE_IMAGE_FORMAT_LAYOUT_VYUY:
    break;
  case ZE_IMAGE_FORMAT_LAYOUT_YVYU:
    break;
  case ZE_IMAGE_FORMAT_LAYOUT_UYVY:
    return CM_SURFACE_FORMAT_UYVY;
  case ZE_IMAGE_FORMAT_LAYOUT_AYUV:
    return CM_SURFACE_FORMAT_AYUV;
  case ZE_IMAGE_FORMAT_LAYOUT_P010:
    return CM_SURFACE_FORMAT_P010;
#if defined(CM_SURFACE_FORMAT_Y410)
  case ZE_IMAGE_FORMAT_LAYOUT_Y410:
    return CM_SURFACE_FORMAT_Y410;
#endif // defined(CM_SURFACE_FORMAT_Y410)
  case ZE_IMAGE_FORMAT_LAYOUT_P012:
    break;
  case ZE_IMAGE_FORMAT_LAYOUT_Y16:
    break;
  case ZE_IMAGE_FORMAT_LAYOUT_P016:
    return CM_SURFACE_FORMAT_P016;
#if defined(CM_SURFACE_FORMAT_Y216)
  case ZE_IMAGE_FORMAT_LAYOUT_Y216:
    return CM_SURFACE_FORMAT_Y216;
#endif // defined(CM_SURFACE_FORMAT_Y216)
  case ZE_IMAGE_FORMAT_LAYOUT_P216:
    return CM_SURFACE_FORMAT_P216;
  }

  return CM_SURFACE_FORMAT_UNKNOWN;
}
} // namespace

extern "C" {
SHIM_EXPORT(zeImageGetProperties);
SHIM_EXPORT(zeImageCreate);
SHIM_EXPORT(zeImageDestroy);
} // extern "C"

ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeImageGetProperties)(
    ze_device_handle_t hDevice, const ze_image_desc_t *desc,
    ze_image_properties_t *pImageProperties) {
  if (hDevice == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }

  if (desc == nullptr || pImageProperties == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
  }

  if (desc->flags > 0x3 || desc->type > ZE_IMAGE_TYPE_BUFFER) {
    return ZE_RESULT_ERROR_INVALID_ENUMERATION;
  }

  pImageProperties->samplerFilterFlags = 0;

  switch (desc->type) {
  case ZE_IMAGE_TYPE_2D:
  case ZE_IMAGE_TYPE_3D:
    if (GetCmFormat(desc->format) == CM_SURFACE_FORMAT_UNKNOWN) {
      break;
    }
    pImageProperties->samplerFilterFlags = ZE_IMAGE_SAMPLER_FILTER_FLAG_POINT |
                                           ZE_IMAGE_SAMPLER_FILTER_FLAG_LINEAR;
    break;
  case ZE_IMAGE_TYPE_BUFFER:
    pImageProperties->samplerFilterFlags = ZE_IMAGE_SAMPLER_FILTER_FLAG_POINT;
    break;
  }

  return ZE_RESULT_SUCCESS;
}

ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeImageCreate)(
    ze_context_handle_t hContext, ze_device_handle_t hDevice,
    const ze_image_desc_t *desc, ze_image_handle_t *phImage) {
  if (hContext == nullptr || hDevice == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }

  if (desc == nullptr || phImage == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
  }

  auto fmt = GetCmFormat(desc->format);

  shim::IntrusivePtr<shim::ze::Context> ctx(
      reinterpret_cast<shim::ze::Context *>(hContext));
  shim::IntrusivePtr<CmDeviceEmu> dev(reinterpret_cast<CmDeviceEmu *>(hDevice));

  try {
    shim::IntrusivePtr<shim::ze::Image> image = nullptr;

    switch (desc->type) {
    case ZE_IMAGE_TYPE_2D:
      if (fmt == CM_SURFACE_FORMAT_UNKNOWN) {
        return ZE_RESULT_ERROR_UNSUPPORTED_IMAGE_FORMAT;
      }
      image = shim::MakeIntrusive<shim::ze::Image>(dev, desc->width,
                                                   desc->height, fmt);
      break;
    case ZE_IMAGE_TYPE_3D:
      if (fmt == CM_SURFACE_FORMAT_UNKNOWN) {
        return ZE_RESULT_ERROR_UNSUPPORTED_IMAGE_FORMAT;
      }
      image = shim::MakeIntrusive<shim::ze::Image>(
          dev, desc->width, desc->height, desc->depth, fmt);
      break;
    case ZE_IMAGE_TYPE_BUFFER:
      image = shim::MakeIntrusive<shim::ze::Image>(dev, desc->width);
      break;
    default:
      return ZE_RESULT_ERROR_UNSUPPORTED_IMAGE_FORMAT;
    }

    shim::IntrusivePtrAddRef(image.get());
    *phImage = reinterpret_cast<ze_image_handle_t>(image.get());
  } catch (shim::ze::Error &e) {
    return ZE_RESULT_ERROR_DEVICE_LOST;
  } catch (std::bad_alloc &e) {
    return ZE_RESULT_ERROR_OUT_OF_HOST_MEMORY;
  }

  return ZE_RESULT_SUCCESS;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
SHIM_CALL(zeImageDestroy)(ze_image_handle_t hImage) {
  if (hImage == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }

  shim::IntrusivePtr<shim::ze::Image> image(
      reinterpret_cast<shim::ze::Image *>(hImage), false);

  return ZE_RESULT_SUCCESS;
}
