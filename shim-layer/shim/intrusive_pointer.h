/*========================== begin_copyright_notice ============================

Copyright (C) 2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef CM_EMU_SHIM_INTRUSIVE_POINTER_H
#define CM_EMU_SHIM_INTRUSIVE_POINTER_H

#include <atomic>

namespace shim {

template <typename DerivedT> class IntrusiveRefCounter {
private:
  using Counter = std::atomic<unsigned>;

public:
  IntrusiveRefCounter() noexcept = default;
  IntrusiveRefCounter(const IntrusiveRefCounter &) noexcept = delete;
  IntrusiveRefCounter(IntrusiveRefCounter &&) noexcept = delete;
  IntrusiveRefCounter &operator=(const IntrusiveRefCounter &) noexcept = delete;
  IntrusiveRefCounter &operator=(IntrusiveRefCounter &&) noexcept = delete;
  ~IntrusiveRefCounter() = default;

  unsigned UseCount() const noexcept { return counter_; }

  Counter counter_ = 0;
};

template <typename T> void IntrusivePtrAddRef(IntrusiveRefCounter<T> *ptr) {
  if (ptr) {
    ptr->counter_++;
  }
}

template <typename T>
std::enable_if_t<std::is_base_of_v<IntrusiveRefCounter<T>, T>>
IntrusivePtrRelease(T *ptr) {
  IntrusiveRefCounter<T> *p = ptr;
  if (p && p->counter_-- == 1) {
    delete ptr;
  }
}

template <typename T> class IntrusivePtr {
public:
  using element_type = T;

  IntrusivePtr() noexcept : ptr_(nullptr) {}
  IntrusivePtr(T *p, bool add_ref = true) noexcept : ptr_(p) {
    if (add_ref) {
      IntrusivePtrAddRef(ptr_);
    }
  }

  IntrusivePtr(const IntrusivePtr &r) noexcept : ptr_(r.ptr_) {
    IntrusivePtrAddRef(ptr_);
  }

  template <typename U>
  IntrusivePtr(const IntrusivePtr<U> &r) noexcept : ptr_(r.ptr_) {
    IntrusivePtrAddRef(ptr_);
  }

  ~IntrusivePtr() { IntrusivePtrRelease(ptr_); }

  IntrusivePtr &operator=(IntrusivePtr r) noexcept {
    std::swap(ptr_, r.ptr_);
    return *this;
  }

  template <typename U> IntrusivePtr &operator=(IntrusivePtr<U> r) noexcept {
    std::swap(ptr_, r.ptr_);
    return *this;
  }

  IntrusivePtr &operator=(std::nullptr_t) noexcept {
    IntrusivePtrRelease(ptr_);
    ptr_ = nullptr;
    return *this;
  }

  operator bool() const noexcept { return bool(ptr_); }

  bool operator!() const noexcept { return !ptr_; }

  T &operator*() const noexcept { return *ptr_; }

  T *operator->() const noexcept { return ptr_; }

  T *get() const noexcept { return ptr_; }

  void reset(T *r, bool add_ref = true) {
    auto *old = ptr_;
    ptr_ = r;
    if (add_ref) {
      IntrusivePtrAddRef(ptr_);
    }
    IntrusivePtrRelease(old);
  }

  T *ptr_;
};

template <typename T, typename U>
bool operator==(const IntrusivePtr<T> &a, const IntrusivePtr<U> &b) noexcept {
  return a.get() == b.get();
}

template <typename T, typename U>
bool operator!=(const IntrusivePtr<T> &a, const IntrusivePtr<U> &b) noexcept {
  return a.get() != b.get();
}

template <typename T>
bool operator==(const IntrusivePtr<T> &a, const T *b) noexcept {
  return a.get() == b;
}

template <typename T>
bool operator!=(const IntrusivePtr<T> &a, const T *b) noexcept {
  return a.get() != b;
}

template <typename T>
bool operator==(const T *a, const IntrusivePtr<T> &b) noexcept {
  return a == b.get();
}

template <typename T>
bool operator!=(const T *a, const IntrusivePtr<T> &b) noexcept {
  return a != b.get();
}

template <typename T, typename U>
void swap(IntrusivePtr<T> &a, IntrusivePtr<U> &b) noexcept {
  std::swap(a.ptr_, b.ptr_);
}

template <typename T, typename... Args>
IntrusivePtr<T> MakeIntrusive(Args &&...args) {
  return new T(std::forward<Args>(args)...);
}

} // namespace shim

#endif // CM_EMU_SHIM_INTRUSIVE_POINTER_H
