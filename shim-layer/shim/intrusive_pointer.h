/*===================== begin_copyright_notice ==================================

 Copyright (c) 2021, Intel Corporation


 Permission is hereby granted, free of charge, to any person obtaining a
 copy of this software and associated documentation files (the "Software"),
 to deal in the Software without restriction, including without limitation
 the rights to use, copy, modify, merge, publish, distribute, sublicense,
 and/or sell copies of the Software, and to permit persons to whom the
 Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included
 in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 OTHER DEALINGS IN THE SOFTWARE.
======================= end_copyright_notice ==================================*/

#ifndef CM_EMU_SHIM_INTRUSIVE_POINTER_H
#define CM_EMU_SHIM_INTRUSIVE_POINTER_H

#include <atomic>

namespace shim {

template <typename DerivedT>
class IntrusiveRefCounter {
 private:
  using Counter = std::atomic<unsigned>;
 public:
  IntrusiveRefCounter() noexcept = default;
  IntrusiveRefCounter(const IntrusiveRefCounter &) noexcept = default;
  IntrusiveRefCounter &operator=(const IntrusiveRefCounter &) noexcept = default;
  ~IntrusiveRefCounter() = default;

  unsigned UseCount() const noexcept { return counter_; }

  Counter counter_ = 0;
};

template <typename T>
void IntrusivePtrAddRef(IntrusiveRefCounter<T> *ptr) {
  if (ptr) {
    ptr->counter_++;
  }
}

template <typename T>
void IntrusivePtrRelease(T *ptr) {
  IntrusiveRefCounter<T> *p = ptr;
  if (ptr && ptr->counter_-- == 1) {
    delete ptr;
  }
}

template <typename T>
class IntrusivePtr {
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
  }

  template <typename U>
  IntrusivePtr &operator=(IntrusivePtr<U> r) noexcept {
    std::swap(ptr_, r.ptr_);
  }

  IntrusivePtr &operator=(T * r) noexcept {
    ptr_ = r;
    IntrusivePtrAddRef(ptr_);
  }

  operator bool() const noexcept {
    return bool(ptr_);
  }

  bool operator!() const noexcept {
    return !ptr_;
  }

  T &operator*() const noexcept {
    return *ptr_;
  }

  T *operator->() const noexcept {
    return ptr_;
  }

  T *get() const noexcept {
    return ptr_;
  }

  void reset(T *r, bool add_ref = true) {
    auto *old = ptr_;
    if (add_ref) {
      IntrusivePtrAddRef(ptr_ = r);
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

} // namespace shim

#endif // CM_EMU_SHIM_INTRUSIVE_POINTER_H
