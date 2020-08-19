// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_ALLOCATOR_PARTITION_ALLOCATOR_PARTITION_ADDRESS_SPACE_H_
#define BASE_ALLOCATOR_PARTITION_ALLOCATOR_PARTITION_ADDRESS_SPACE_H_

#include "base/allocator/partition_allocator/address_pool_manager.h"
#include "base/allocator/partition_allocator/partition_alloc_constants.h"
#include "base/allocator/partition_allocator/partition_alloc_features.h"
#include "base/base_export.h"
#include "base/bits.h"
#include "base/feature_list.h"
#include "base/notreached.h"
#include "build/build_config.h"

namespace base {

namespace internal {

// The address space reservation is supported only on 64-bit architecture.
#if defined(ARCH_CPU_64_BITS)

// Reserves address space for PartitionAllocator.
class BASE_EXPORT PartitionAddressSpace {
 public:
  static ALWAYS_INLINE internal::pool_handle GetDirectMapPool() {
    return direct_map_pool_;
  }
  static ALWAYS_INLINE internal::pool_handle GetNormalBucketPool() {
    return normal_bucket_pool_;
  }

  static void Init();
  static void UninitForTesting();

  static ALWAYS_INLINE bool Contains(const void* address) {
    return (reinterpret_cast<uintptr_t>(address) &
            kReservedAddressSpaceBaseMask) == reserved_base_address_;
  }

  // PartitionAddressSpace is static_only class.
  PartitionAddressSpace() = delete;
  PartitionAddressSpace(const PartitionAddressSpace&) = delete;
  void* operator new(size_t) = delete;
  void* operator new(size_t, void*) = delete;

  // Partition Alloc Address Space
  // Reserves 64Gbytes address space for 1 direct map space(16G) and 1 normal
  // bucket space(16G). The remaining 32G is for padding, so that we can
  // guarantee a 32G alignment somewhere within the reserved region. Address
  // space is cheap and abundant on 64-bit systems.
  // TODO(tasak): release unused address space.
  //
  // +----------------+ reserved address start
  // |  (unused)      |
  // +----------------+ 32G-aligned reserved address: X
  // |                |
  // |  direct map    |
  // |    space       |
  // |                |
  // +----------------+ X + 16G bytes
  // | normal buckets |
  // |    space       |
  // +----------------+ X + 32G bytes
  // | (unused)       |
  // +----------------+ reserved address end
  //
  // The static member variables:
  // - reserved_address_starts_ points the "reserved address start" address, and
  // - reserved_base_address_ points the "32G-aligned reserved address: X".

  static constexpr size_t kGigaBytes = 1024 * 1024 * 1024;
  static constexpr size_t kDirectMapPoolSize = 16 * kGigaBytes;
  static constexpr size_t kNormalBucketPoolSize = 16 * kGigaBytes;
  // Reserves 32GB aligned address space.
  // Alignment should be the smallest power of two greater than or equal to the
  // desired size, so that we can check containment with a single bitmask
  // operation.
  static constexpr size_t kDesiredAddressSpaceSize =
      kDirectMapPoolSize + kNormalBucketPoolSize;
  static constexpr size_t kReservedAddressSpaceAlignment =
      kDesiredAddressSpaceSize;
  static constexpr size_t kReservedAddressSpaceSize =
      kReservedAddressSpaceAlignment * 2;
  static constexpr uintptr_t kReservedAddressSpaceOffsetMask =
      static_cast<uintptr_t>(kReservedAddressSpaceAlignment) - 1;
  static constexpr uintptr_t kReservedAddressSpaceBaseMask =
      ~kReservedAddressSpaceOffsetMask;

 private:
  // See the comment describing the address layout above.
  static uintptr_t reserved_address_start_;
  static uintptr_t reserved_base_address_;

  static internal::pool_handle direct_map_pool_;
  static internal::pool_handle normal_bucket_pool_;
};

ALWAYS_INLINE internal::pool_handle GetDirectMapPool() {
  DCHECK(IsPartitionAllocGigaCageEnabled());
  return PartitionAddressSpace::GetDirectMapPool();
}

ALWAYS_INLINE internal::pool_handle GetNormalBucketPool() {
  DCHECK(IsPartitionAllocGigaCageEnabled());
  return PartitionAddressSpace::GetNormalBucketPool();
}

#else  // !defined(ARCH_CPU_64_BITS)

ALWAYS_INLINE internal::pool_handle GetDirectMapPool() {
  NOTREACHED();
  return 0;
}

ALWAYS_INLINE internal::pool_handle GetNormalBucketPool() {
  NOTREACHED();
  return 0;
}

#endif

}  // namespace internal

}  // namespace base

#endif  // BASE_ALLOCATOR_PARTITION_ALLOCATOR_PARTITION_ADDRESS_SPACE_H_
