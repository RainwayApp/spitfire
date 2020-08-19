// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_MEMORY_CHECKED_PTR_H_
#define BASE_MEMORY_CHECKED_PTR_H_

#include <stddef.h>
#include <stdint.h>

#include <utility>

#include "base/compiler_specific.h"
#include "base/logging.h"
#include "build/build_config.h"

// TEST: We can't use protection in the real code (yet) because it may lead to
// crashes in absence of PartitionAlloc support. Setting it to 0 will disable
// the protection, while preserving all calculations.
#define CHECKED_PTR2_PROTECTION_ENABLED 0

#define CHECKED_PTR2_USE_NO_OP_WRAPPER 0

// Set it to 1 to avoid branches when checking if per-pointer protection is
// enabled.
#define CHECKED_PTR2_AVOID_BRANCH_WHEN_CHECKING_ENABLED 0
// Set it to 1 to avoid branches when dereferencing the pointer.
// Must be 1 if the above is 1.
#define CHECKED_PTR2_AVOID_BRANCH_WHEN_DEREFERENCING 0

namespace base {

// NOTE: All methods should be ALWAYS_INLINE. CheckedPtr is meant to be a
// lightweight replacement of a raw pointer, hence performance is critical.

namespace internal {
// These classes/structures are part of the CheckedPtr implementation.
// DO NOT USE THESE CLASSES DIRECTLY YOURSELF.

struct CheckedPtrNoOpImpl {
  // Wraps a pointer, and returns its uintptr_t representation.
  // Use |const volatile| to prevent compiler error. These will be dropped
  // anyway when casting to uintptr_t and brought back upon pointer extraction.
  static ALWAYS_INLINE uintptr_t WrapRawPtr(const volatile void* cv_ptr) {
    return reinterpret_cast<uintptr_t>(cv_ptr);
  }

  // Returns equivalent of |WrapRawPtr(nullptr)|. Separated out to make it a
  // constexpr.
  static constexpr ALWAYS_INLINE uintptr_t GetWrappedNullPtr() {
    // This relies on nullptr and 0 being equal in the eyes of reinterpret_cast,
    // which apparently isn't true in all environments.
    return 0;
  }

  // Unwraps the pointer's uintptr_t representation, while asserting that memory
  // hasn't been freed. The function is allowed to crash on nullptr.
  static ALWAYS_INLINE void* SafelyUnwrapPtrForDereference(
      uintptr_t wrapped_ptr) {
    return reinterpret_cast<void*>(wrapped_ptr);
  }

  // Unwraps the pointer's uintptr_t representation, while asserting that memory
  // hasn't been freed. The function must handle nullptr gracefully.
  static ALWAYS_INLINE void* SafelyUnwrapPtrForExtraction(
      uintptr_t wrapped_ptr) {
    return reinterpret_cast<void*>(wrapped_ptr);
  }

  // Unwraps the pointer's uintptr_t representation, without making an assertion
  // on whether memory was freed or not.
  static ALWAYS_INLINE void* UnsafelyUnwrapPtrForComparison(
      uintptr_t wrapped_ptr) {
    return reinterpret_cast<void*>(wrapped_ptr);
  }

  // Advance the wrapped pointer by |delta| bytes.
  static ALWAYS_INLINE uintptr_t Advance(uintptr_t wrapped_ptr, size_t delta) {
    return wrapped_ptr + delta;
  }

  // This is for accounting only, used by unit tests.
  static ALWAYS_INLINE void IncrementSwapCountForTest() {}
};

#if defined(ARCH_CPU_64_BITS)

constexpr int kValidAddressBits = 48;
constexpr uintptr_t kAddressMask = (1ull << kValidAddressBits) - 1;
constexpr int kGenerationBits = sizeof(uintptr_t) * 8 - kValidAddressBits;
constexpr uintptr_t kGenerationMask = ~kAddressMask;
constexpr int kTopBitShift = 63;
constexpr uintptr_t kTopBit = 1ull << kTopBitShift;
static_assert(kTopBit << 1 == 0, "kTopBit should really be the top bit");
static_assert((kTopBit & kGenerationMask) > 0,
              "kTopBit bit must be inside the generation region");

// TEST: Use volatile so that the read isn't optimized out.
static volatile bool g_enabled = true;

struct CheckedPtr2Impl {
  static_assert(sizeof(uintptr_t) == 8,
                "only 64-bit architectures are supported");

  // Wraps a pointer, and returns its uintptr_t representation.
  static ALWAYS_INLINE uintptr_t WrapRawPtr(const volatile void* cv_ptr) {
    void* ptr = const_cast<void*>(cv_ptr);
    uintptr_t addr = reinterpret_cast<uintptr_t>(ptr);
#if CHECKED_PTR2_USE_NO_OP_WRAPPER
    static_assert(!CHECKED_PTR2_PROTECTION_ENABLED, "");
#else
    // Make sure that the address bits that will be used for generation are 0.
    // Otherwise the logic may fail.
    DCHECK_EQ(ExtractGeneration(addr), 0ull);

    // TEST: |g_enabled| should be replaced with a check if the allocation is on
    // PartitionAlloc. There could be also a Finch check added.
    if (ptr == nullptr || !g_enabled) {
      return addr;
    }

    // TEST: It should be |size = base::PartitionAllocGetSize(ptr)|, however
    // |PartitionAllocGetSize()| will likely crash if used an a non-PA pointer.
    // For now, replacing it with something that always passes.
    //
    // TEST: There shouldn't be |volatile|; that's to prevent optimization of %.
    volatile size_t size = (addr & (addr - 1)) ^ addr;
    if (addr % size != 0) {
      DCHECK(false);
      return addr;
    }

    // Read the generation from 16 bits before the allocation. Then place it in
    // the top bits of the address.
    //
    // TODO(bartekn): Consider if casting to |volatile*| is needed. I
    // believe it's needed when dereferencing, not sure about here.
    static_assert(sizeof(uint16_t) * 8 == kGenerationBits, "");
#if CHECKED_PTR2_PROTECTION_ENABLED
    uintptr_t generation = *(static_cast<volatile uint16_t*>(ptr) - 1);
#else
    // TEST: Reading from offset -1 may crash without PA support.
    // Just read from offset 0 to attain the same perf characteristics as the
    // expected production solution.
    // This generation will be ignored anyway either when unwrapping or below
    // (depending on the algorithm variant), on the
    // !CHECKED_PTR2_PROTECTION_ENABLED path.
    uintptr_t generation = *(static_cast<volatile uint16_t*>(ptr));
#endif  // #else CHECKED_PTR2_PROTECTION_ENABLED
    generation <<= kValidAddressBits;
    addr |= generation;
#if CHECKED_PTR2_AVOID_BRANCH_WHEN_CHECKING_ENABLED
    // Always set top bit to 1, to indicated that the protection is enabled.
    addr |= kTopBit;
#if !CHECKED_PTR2_PROTECTION_ENABLED
    // TEST: Clear the generation, or else it could crash without PA support.
    // If the top bit was set, the unwrapper would read from before the address
    // address, but with it cleared, it'll read from the address itself.
    addr &= kAddressMask;
#endif  // #if !CHECKED_PTR2_PROTECTION_ENABLED
#endif  // #if CHECKED_PTR2_AVOID_BRANCH_WHEN_CHECKING_ENABLED
#endif  // #if CHECKED_PTR2_USE_NO_OP_WRAPPER
    return addr;
  }

  // Returns equivalent of |WrapRawPtr(nullptr)|. Separated out to make it a
  // constexpr.
  static constexpr ALWAYS_INLINE uintptr_t GetWrappedNullPtr() {
    return kWrappedNullPtr;
  }

  static ALWAYS_INLINE uintptr_t
  SafelyUnwrapPtrInternal(uintptr_t wrapped_ptr) {
#if CHECKED_PTR2_AVOID_BRANCH_WHEN_CHECKING_ENABLED
    // Top bit tells if the protection is enabled. Use it to decide whether to
    // read the word before the allocation, which exists only if the protection
    // is enabled. Otherwise it may crash, in which case read the data from the
    // beginning of the allocation instead and ignore it later. All this magic
    // is to avoid a branch, for performance reasons.
    //
    // A couple examples, assuming 64-bit system (continued below):
    //   Ex.1: wrapped_ptr=0x8442000012345678
    //           => enabled=0x8000000000000000
    //           => offset=1
    //   Ex.2: wrapped_ptr=0x0000000012345678
    //           => enabled=0x0000000000000000
    //           => offset=0
    uintptr_t enabled = wrapped_ptr & kTopBit;
    // We can't have protection disabled and generation set in the same time.
    DCHECK(!(enabled == 0 && (ExtractGeneration(wrapped_ptr)) != 0));
    uintptr_t offset = enabled >> kTopBitShift;  // 0 or 1
    // Use offset to decide if the generation should be read at the beginning or
    // before the allocation.
    // TODO(bartekn): Do something about 1-byte allocations. Reading 2-byte
    // generation at the allocation could crash. This case is executed
    // specifically for non-PartitionAlloc pointers, so we can't make
    // assumptions about alignment.
    //
    // Cast to volatile to ensure memory is read. E.g. in a tight loop, the
    // compiler could cache the value in a register and thus could miss that
    // another thread freed memory and cleared generation.
    //
    // Examples (continued):
    //   Ex.1: generation_ptr=0x0000000012345676
    //     a) if pointee wasn't freed, read e.g. generation=0x0442 (could be
    //        also 0x8442, the top bit is overwritten later)
    //     b) if pointee was freed, read e.g. generation=0x1234 (could be
    //        anything)
    //   Ex.2: generation_ptr=0x0000000012345678, read e.g. 0x2345 (doesn't
    //         matter what we read, as long as this read doesn't crash)
    volatile uint16_t* generation_ptr =
        reinterpret_cast<volatile uint16_t*>(ExtractAddress(wrapped_ptr)) -
        offset;
    uintptr_t generation = *generation_ptr;
    // Shift generation into the right place and add back the enabled bit.
    //
    // Examples (continued):
    //   Ex.1:
    //     a) generation=0x8442000000000000
    //     a) generation=0x9234000000000000
    //   Ex.2: generation=0x2345000000000000
    generation <<= kValidAddressBits;
    generation |= enabled;

    // If the protection isn't enabled, clear top bits. Casting to a signed
    // type makes >> sign extend the last bit.
    //
    // Examples (continued):
    //   Ex.1: mask=0xffff000000000000
    //     a) generation=0x8442000000000000
    //     b) generation=0x9234000000000000
    //   Ex.2: mask=0x0000000000000000 => generation=0x0000000000000000
    uintptr_t mask = static_cast<intptr_t>(enabled) >> (kGenerationBits - 1);
    generation &= mask;

    // Use hardware to detect generation mismatch. CPU will crash if top bits
    // aren't all 0 (technically it won't if all bits are 1, but that's a kernel
    // mode address, which isn't allowed either... also, top bit will be always
    // zeroed out).
    //
    // Examples (continued):
    //   Ex.1:
    //     a) returning 0x0000000012345678
    //     b) returning 0x1676000012345678 (this will generate a desired crash)
    //   Ex.2: returning 0x0000000012345678
    static_assert(CHECKED_PTR2_AVOID_BRANCH_WHEN_DEREFERENCING, "");
    return generation ^ wrapped_ptr;
#else  // #if CHECKED_PTR2_AVOID_BRANCH_WHEN_CHECKING_ENABLED
    uintptr_t ptr_generation = wrapped_ptr >> kValidAddressBits;
    if (ptr_generation > 0) {
      // Read generation from before the allocation.
      //
      // Cast to volatile to ensure memory is read. E.g. in a tight loop, the
      // compiler could cache the value in a register and thus could miss that
      // another thread freed memory and cleared generation.
#if CHECKED_PTR2_PROTECTION_ENABLED
      uintptr_t read_generation =
          *(reinterpret_cast<volatile uint16_t*>(ExtractAddress(wrapped_ptr)) -
            1);
#else
      // TEST: Reading from before the pointer may crash. See more above...
      uintptr_t read_generation =
          *(reinterpret_cast<volatile uint16_t*>(ExtractAddress(wrapped_ptr)));
#endif
#if CHECKED_PTR2_AVOID_BRANCH_WHEN_DEREFERENCING
      // Use hardware to detect generation mismatch. CPU will crash if top bits
      // aren't all 0 (technically it won't if all bits are 1, but that's a
      // kernel mode address, which isn't allowed either).
      read_generation <<= kValidAddressBits;
      return read_generation ^ wrapped_ptr;
#else
#if CHECKED_PTR2_PROTECTION_ENABLED
      if (UNLIKELY(ptr_generation != read_generation))
        IMMEDIATE_CRASH();
#else
      // TEST: Use volatile to prevent optimizing out the calculations leading
      // to this point.
      volatile bool x = false;
      if (ptr_generation != read_generation)
        x = true;
#endif  // #else CHECKED_PTR2_PROTECTION_ENABLED
      return wrapped_ptr & kAddressMask;
#endif  // #else CHECKED_PTR2_AVOID_BRANCH_WHEN_DEREFERENCING
    }
    return wrapped_ptr;
#endif  // #else CHECKED_PTR2_AVOID_BRANCH_WHEN_CHECKING_ENABLED
  }

  // Unwraps the pointer's uintptr_t representation, while asserting that memory
  // hasn't been freed. The function is allowed to crash on nullptr.
  static ALWAYS_INLINE void* SafelyUnwrapPtrForDereference(
      uintptr_t wrapped_ptr) {
#if CHECKED_PTR2_PROTECTION_ENABLED
    return reinterpret_cast<void*>(SafelyUnwrapPtrInternal(wrapped_ptr));
#else
    // TEST: Use volatile to prevent optimizing out the calculations leading to
    // this point.
    // |SafelyUnwrapPtrInternal| was separated out solely for this purpose.
    volatile uintptr_t addr = SafelyUnwrapPtrInternal(wrapped_ptr);
    return reinterpret_cast<void*>(addr);
#endif
  }

  // Unwraps the pointer's uintptr_t representation, while asserting that memory
  // hasn't been freed. The function must handle nullptr gracefully.
  static ALWAYS_INLINE void* SafelyUnwrapPtrForExtraction(
      uintptr_t wrapped_ptr) {
#if CHECKED_PTR2_AVOID_BRANCH_WHEN_CHECKING_ENABLED
    // In this implementation SafelyUnwrapPtrForDereference doesn't tolerate
    // nullptr, because it reads unconditionally to avoid branches. Handle the
    // nullptr case here.
    if (wrapped_ptr == kWrappedNullPtr)
      return nullptr;
    return reinterpret_cast<void*>(SafelyUnwrapPtrForDereference(wrapped_ptr));
#else
    // In this implementation SafelyUnwrapPtrForDereference handles nullptr case
    // well.
    return reinterpret_cast<void*>(SafelyUnwrapPtrForDereference(wrapped_ptr));
#endif
  }

  // Unwraps the pointer's uintptr_t representation, without making an assertion
  // on whether memory was freed or not.
  static ALWAYS_INLINE void* UnsafelyUnwrapPtrForComparison(
      uintptr_t wrapped_ptr) {
    return reinterpret_cast<void*>(ExtractAddress(wrapped_ptr));
  }

  // Advance the wrapped pointer by |delta| bytes.
  static ALWAYS_INLINE uintptr_t Advance(uintptr_t wrapped_ptr, size_t delta) {
    // Mask out the generation to disable the protection. It's not supported for
    // pointers inside an allocation.
    return ExtractAddress(wrapped_ptr) + delta;
  }

  // This is for accounting only, used by unit tests.
  static ALWAYS_INLINE void IncrementSwapCountForTest() {}

 private:
  static ALWAYS_INLINE uintptr_t ExtractAddress(uintptr_t wrapped_ptr) {
    return wrapped_ptr & kAddressMask;
  }

  static ALWAYS_INLINE uintptr_t ExtractGeneration(uintptr_t wrapped_ptr) {
    return wrapped_ptr & kGenerationMask;
  }

  // This relies on nullptr and 0 being equal in the eyes of reinterpret_cast,
  // which apparently isn't true in some rare environments.
  static constexpr uintptr_t kWrappedNullPtr = 0;
};

#endif  // #if defined(ARCH_CPU_64_BITS)

template <typename T>
struct DereferencedPointerType {
  using Type = decltype(*std::declval<T*>());
};
// This explicitly doesn't define any type aliases, since dereferencing void is
// invalid.
template <>
struct DereferencedPointerType<void> {};

}  // namespace internal

// DO NOT USE! EXPERIMENTAL ONLY! This is helpful for local testing!
//
// CheckedPtr is meant to be a pointer wrapper, that will crash on
// Use-After-Free (UaF) to prevent security issues. This is very much in the
// experimental phase. More context in:
// https://docs.google.com/document/d/1pnnOAIz_DMWDI4oIOFoMAqLnf_MZ2GsrJNb_dbQ3ZBg
//
// For now, CheckedPtr is a no-op wrapper to aid local testing.
//
// Goals for this API:
// 1. Minimize amount of caller-side changes as much as physically possible.
// 2. Keep this class as small as possible, while still satisfying goal #1 (i.e.
//    we aren't striving to maximize compatibility with raw pointers, merely
//    adding support for cases encountered so far).
template <typename T,
#if defined(ARCH_CPU_64_BITS)
          typename Impl = internal::CheckedPtr2Impl>
#else
          typename Impl = internal::CheckedPtrNoOpImpl>
#endif
class CheckedPtr {
 public:
  // CheckedPtr can be trivially default constructed (leaving |wrapped_ptr_|
  // uninitialized).  This is needed for compatibility with raw pointers.
  //
  // TODO(lukasza): Always initialize |wrapped_ptr_|.  Fix resulting build
  // errors.  Analyze performance impact.
  constexpr CheckedPtr() noexcept = default;

  // Deliberately implicit, because CheckedPtr is supposed to resemble raw ptr.
  // NOLINTNEXTLINE(runtime/explicit)
  constexpr ALWAYS_INLINE CheckedPtr(nullptr_t) noexcept
      : wrapped_ptr_(Impl::GetWrappedNullPtr()) {}

  // Deliberately implicit, because CheckedPtr is supposed to resemble raw ptr.
  // NOLINTNEXTLINE(runtime/explicit)
  ALWAYS_INLINE CheckedPtr(T* p) noexcept : wrapped_ptr_(Impl::WrapRawPtr(p)) {}

  // In addition to nullptr_t ctor above, CheckedPtr needs to have these
  // as |=default| or |constexpr| to avoid hitting -Wglobal-constructors in
  // cases like this:
  //     struct SomeStruct { int int_field; CheckedPtr<int> ptr_field; };
  //     SomeStruct g_global_var = { 123, nullptr };
  CheckedPtr(const CheckedPtr&) noexcept = default;
  CheckedPtr(CheckedPtr&&) noexcept = default;
  CheckedPtr& operator=(const CheckedPtr&) noexcept = default;
  CheckedPtr& operator=(CheckedPtr&&) noexcept = default;

  ALWAYS_INLINE CheckedPtr& operator=(T* p) noexcept {
    wrapped_ptr_ = Impl::WrapRawPtr(p);
    return *this;
  }
  ALWAYS_INLINE CheckedPtr& operator=(std::nullptr_t) noexcept {
    wrapped_ptr_ = Impl::GetWrappedNullPtr();
    return *this;
  }

  ~CheckedPtr() = default;

  // Avoid using. The goal of CheckedPtr is to be as close to raw pointer as
  // possible, so use it only if absolutely necessary (e.g. for const_cast).
  ALWAYS_INLINE T* get() const { return GetForExtraction(); }

  explicit ALWAYS_INLINE operator bool() const {
    return wrapped_ptr_ != Impl::GetWrappedNullPtr();
  }

  // Use SFINAE to avoid defining |operator*| for T=void, which wouldn't compile
  // due to |void&|.
  template <typename U = T,
            typename V = typename internal::DereferencedPointerType<U>::Type>
  ALWAYS_INLINE V& operator*() const {
    return *GetForDereference();
  }
  ALWAYS_INLINE T* operator->() const { return GetForDereference(); }
  // Deliberately implicit, because CheckedPtr is supposed to resemble raw ptr.
  // NOLINTNEXTLINE(runtime/explicit)
  ALWAYS_INLINE operator T*() const { return GetForExtraction(); }
  template <typename U>
  explicit ALWAYS_INLINE operator U*() const {
    return static_cast<U*>(GetForExtraction());
  }

  ALWAYS_INLINE CheckedPtr& operator++() {
    wrapped_ptr_ = Impl::Advance(wrapped_ptr_, sizeof(T));
    return *this;
  }
  ALWAYS_INLINE CheckedPtr& operator--() {
    wrapped_ptr_ = Impl::Advance(wrapped_ptr_, -sizeof(T));
    return *this;
  }
  ALWAYS_INLINE CheckedPtr operator++(int /* post_increment */) {
    CheckedPtr result = *this;
    ++(*this);
    return result;
  }
  ALWAYS_INLINE CheckedPtr operator--(int /* post_decrement */) {
    CheckedPtr result = *this;
    --(*this);
    return result;
  }
  ALWAYS_INLINE CheckedPtr& operator+=(ptrdiff_t delta_elems) {
    wrapped_ptr_ = Impl::Advance(wrapped_ptr_, delta_elems * sizeof(T));
    return *this;
  }
  ALWAYS_INLINE CheckedPtr& operator-=(ptrdiff_t delta_elems) {
    return *this += -delta_elems;
  }

  // Be careful to cover all cases with CheckedPtr being on both sides, left
  // side only and right side only. If any case is missed, a more costly
  // |operator T*()| will get called, instead of |operator==|.
  friend ALWAYS_INLINE bool operator==(const CheckedPtr& lhs,
                                       const CheckedPtr& rhs) {
    return lhs.GetForComparison() == rhs.GetForComparison();
  }
  friend ALWAYS_INLINE bool operator!=(const CheckedPtr& lhs,
                                       const CheckedPtr& rhs) {
    return !(lhs == rhs);
  }
  friend ALWAYS_INLINE bool operator==(const CheckedPtr& lhs, T* rhs) {
    return lhs.GetForComparison() == rhs;
  }
  friend ALWAYS_INLINE bool operator!=(const CheckedPtr& lhs, T* rhs) {
    return !(lhs == rhs);
  }
  friend ALWAYS_INLINE bool operator==(T* lhs, const CheckedPtr& rhs) {
    return rhs == lhs;  // Reverse order to call the operator above.
  }
  friend ALWAYS_INLINE bool operator!=(T* lhs, const CheckedPtr& rhs) {
    return rhs != lhs;  // Reverse order to call the operator above.
  }
  // Needed for cases like |derived_ptr == base_ptr|. Without these, a more
  // costly |operator T*()| will get called, instead of |operator==|.
  template <typename U>
  friend ALWAYS_INLINE bool operator==(const CheckedPtr& lhs,
                                       const CheckedPtr<U, Impl>& rhs) {
    // Add |const volatile| when casting, in case |U| has any. Even if |T|
    // doesn't, comparison between |T*| and |const volatile T*| is fine.
    return lhs.GetForComparison() ==
           static_cast<std::add_cv_t<T>*>(rhs.GetForComparison());
  }
  template <typename U>
  friend ALWAYS_INLINE bool operator!=(const CheckedPtr& lhs,
                                       const CheckedPtr<U, Impl>& rhs) {
    return !(lhs == rhs);
  }
  template <typename U>
  friend ALWAYS_INLINE bool operator==(const CheckedPtr& lhs, U* rhs) {
    // Add |const volatile| when casting, in case |U| has any. Even if |T|
    // doesn't, comparison between |T*| and |const volatile T*| is fine.
    return lhs.GetForComparison() == static_cast<std::add_cv_t<T>*>(rhs);
  }
  template <typename U>
  friend ALWAYS_INLINE bool operator!=(const CheckedPtr& lhs, U* rhs) {
    return !(lhs == rhs);
  }
  template <typename U>
  friend ALWAYS_INLINE bool operator==(U* lhs, const CheckedPtr& rhs) {
    return rhs == lhs;  // Reverse order to call the operator above.
  }
  template <typename U>
  friend ALWAYS_INLINE bool operator!=(U* lhs, const CheckedPtr& rhs) {
    return rhs != lhs;  // Reverse order to call the operator above.
  }
  // Needed for comparisons against nullptr. Without these, a slightly more
  // costly version would be called that extracts wrapped pointer, as opposed
  // to plain comparison against 0.
  friend ALWAYS_INLINE bool operator==(const CheckedPtr& lhs, nullptr_t) {
    return !lhs;
  }
  friend ALWAYS_INLINE bool operator!=(const CheckedPtr& lhs, nullptr_t) {
    return !!lhs;  // Use !! otherwise the costly implicit cast will be used.
  }
  friend ALWAYS_INLINE bool operator==(nullptr_t, const CheckedPtr& rhs) {
    return !rhs;
  }
  friend ALWAYS_INLINE bool operator!=(nullptr_t, const CheckedPtr& rhs) {
    return !!rhs;  // Use !! otherwise the costly implicit cast will be used.
  }

  friend ALWAYS_INLINE void swap(CheckedPtr& lhs, CheckedPtr& rhs) noexcept {
    Impl::IncrementSwapCountForTest();
    std::swap(lhs.wrapped_ptr_, rhs.wrapped_ptr_);
  }

 private:
  // This getter is meant for situations where the pointer is meant to be
  // dereferenced. It is allowed to crash on nullptr (it may or may not),
  // because it knows that the caller will crash on nullptr.
  ALWAYS_INLINE T* GetForDereference() const {
    return static_cast<T*>(Impl::SafelyUnwrapPtrForDereference(wrapped_ptr_));
  }
  // This getter is meant for situations where the raw pointer is meant to be
  // extracted outside of this class, but not necessarily with an intention to
  // dereference. It mustn't crash on nullptr.
  ALWAYS_INLINE T* GetForExtraction() const {
    return static_cast<T*>(Impl::SafelyUnwrapPtrForExtraction(wrapped_ptr_));
  }
  // This getter is meant *only* for situations where the pointer is meant to be
  // compared (guaranteeing no dereference or extraction outside of this class).
  // Any verifications can and should be skipped for performance reasons.
  ALWAYS_INLINE T* GetForComparison() const {
    return static_cast<T*>(Impl::UnsafelyUnwrapPtrForComparison(wrapped_ptr_));
  }

  // Store the pointer as |uintptr_t|, because depending on implementation, its
  // unused bits may be re-purposed to store extra information.
  uintptr_t wrapped_ptr_;

  template <typename U, typename V>
  friend class CheckedPtr;
};

}  // namespace base

using base::CheckedPtr;

#endif  // BASE_MEMORY_CHECKED_PTR_H_
