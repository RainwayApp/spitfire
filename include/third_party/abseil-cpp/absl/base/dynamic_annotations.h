// Copyright 2017 The Abseil Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// This file defines dynamic annotations for use with dynamic analysis tool
// such as valgrind, PIN, etc.
//
// Dynamic annotation is a source code annotation that affects the generated
// code (that is, the annotation is not a comment). Each such annotation is
// attached to a particular instruction and/or to a particular object (address)
// in the program.
//
// The annotations that should be used by users are macros in all upper-case
// (e.g., ABSL_ANNOTATE_THREAD_NAME).
//
// Actual implementation of these macros may differ depending on the dynamic
// analysis tool being used.
//
// This file supports the following configurations:
// - Dynamic Annotations enabled (with static thread-safety warnings disabled).
//   In this case, macros expand to functions implemented by Thread Sanitizer,
//   when building with TSan. When not provided an external implementation,
//   dynamic_annotations.cc provides no-op implementations.
//
// - Static Clang thread-safety warnings enabled.
//   When building with a Clang compiler that supports thread-safety warnings,
//   a subset of annotations can be statically-checked at compile-time. We
//   expand these macros to static-inline functions that can be analyzed for
//   thread-safety, but afterwards elided when building the final binary.
//
// - All annotations are disabled.
//   If neither Dynamic Annotations nor Clang thread-safety warnings are
//   enabled, then all annotation-macros expand to empty.

#ifndef ABSL_BASE_DYNAMIC_ANNOTATIONS_H_
#define ABSL_BASE_DYNAMIC_ANNOTATIONS_H_

#include <stddef.h>

#include "absl/base/config.h"

// -------------------------------------------------------------------------
// Decide which features are enabled

#ifndef ABSL_DYNAMIC_ANNOTATIONS_ENABLED
#define ABSL_DYNAMIC_ANNOTATIONS_ENABLED 0
#endif

#if defined(__clang__) && !defined(SWIG)
#define ABSL_INTERNAL_IGNORE_READS_ATTRIBUTE_ENABLED 1
#else
#define ABSL_INTERNAL_IGNORE_READS_ATTRIBUTE_ENABLED 0
#endif

#if ABSL_DYNAMIC_ANNOTATIONS_ENABLED != 0

#define ABSL_INTERNAL_RACE_ANNOTATIONS_ENABLED 1
#define ABSL_INTERNAL_READS_ANNOTATIONS_ENABLED 1
#define ABSL_INTERNAL_WRITES_ANNOTATIONS_ENABLED 1
#define ABSL_INTERNAL_ANNOTALYSIS_ENABLED 0
#define ABSL_INTERNAL_READS_WRITES_ANNOTATIONS_ENABLED 1

#else

#define ABSL_INTERNAL_RACE_ANNOTATIONS_ENABLED 0
#define ABSL_INTERNAL_READS_ANNOTATIONS_ENABLED 0
#define ABSL_INTERNAL_WRITES_ANNOTATIONS_ENABLED 0

// Clang provides limited support for static thread-safety analysis through a
// feature called Annotalysis. We configure macro-definitions according to
// whether Annotalysis support is available. When running in opt-mode, GCC
// will issue a warning, if these attributes are compiled. Only include them
// when compiling using Clang.

// ABSL_ANNOTALYSIS_ENABLED == 1 when IGNORE_READ_ATTRIBUTE_ENABLED == 1
#define ABSL_INTERNAL_ANNOTALYSIS_ENABLED \
  ABSL_INTERNAL_IGNORE_READS_ATTRIBUTE_ENABLED
// Read/write annotations are enabled in Annotalysis mode; disabled otherwise.
#define ABSL_INTERNAL_READS_WRITES_ANNOTATIONS_ENABLED \
  ABSL_INTERNAL_ANNOTALYSIS_ENABLED
#endif

// Memory annotations are also made available to LLVM's Memory Sanitizer
#if defined(MEMORY_SANITIZER) && defined(__has_feature) && \
    !defined(__native_client__)
#if __has_feature(memory_sanitizer)
#define ABSL_INTERNAL_MEMORY_ANNOTATIONS_ENABLED 1
#endif
#endif

#ifndef ABSL_INTERNAL_MEMORY_ANNOTATIONS_ENABLED
#define ABSL_INTERNAL_MEMORY_ANNOTATIONS_ENABLED 0
#endif

#ifdef __cplusplus
#define ABSL_INTERNAL_BEGIN_EXTERN_C extern "C" {
#define ABSL_INTERNAL_END_EXTERN_C }  // extern "C"
#define ABSL_INTERNAL_GLOBAL_SCOPED(F) ::F
#define ABSL_INTERNAL_STATIC_INLINE inline
#else
#define ABSL_INTERNAL_BEGIN_EXTERN_C  // empty
#define ABSL_INTERNAL_END_EXTERN_C    // empty
#define ABSL_INTERNAL_GLOBAL_SCOPED(F) F
#define ABSL_INTERNAL_STATIC_INLINE static inline
#endif

// -------------------------------------------------------------------------
// Define race annotations.

#if ABSL_INTERNAL_RACE_ANNOTATIONS_ENABLED == 1

// -------------------------------------------------------------
// Annotations that suppress errors. It is usually better to express the
// program's synchronization using the other annotations, but these can be used
// when all else fails.

// Report that we may have a benign race at `pointer`, with size
// "sizeof(*(pointer))". `pointer` must be a non-void* pointer. Insert at the
// point where `pointer` has been allocated, preferably close to the point
// where the race happens. See also ABSL_ANNOTATE_BENIGN_RACE_STATIC.
#define ABSL_ANNOTATE_BENIGN_RACE(pointer, description) \
  ABSL_INTERNAL_GLOBAL_SCOPED(AbslAnnotateBenignRaceSized)  \
  (__FILE__, __LINE__, pointer, sizeof(*(pointer)), description)

// Same as ABSL_ANNOTATE_BENIGN_RACE(`address`, `description`), but applies to
// the memory range [`address`, `address`+`size`).
#define ABSL_ANNOTATE_BENIGN_RACE_SIZED(address, size, description) \
  ABSL_INTERNAL_GLOBAL_SCOPED(AbslAnnotateBenignRaceSized)              \
  (__FILE__, __LINE__, address, size, description)

// Enable (`enable`!=0) or disable (`enable`==0) race detection for all threads.
// This annotation could be useful if you want to skip expensive race analysis
// during some period of program execution, e.g. during initialization.
#define ABSL_ANNOTATE_ENABLE_RACE_DETECTION(enable)        \
  ABSL_INTERNAL_GLOBAL_SCOPED(AbslAnnotateEnableRaceDetection) \
  (__FILE__, __LINE__, enable)

// -------------------------------------------------------------
// Annotations useful for debugging.

// Report the current thread `name` to a race detector.
#define ABSL_ANNOTATE_THREAD_NAME(name) \
  ABSL_INTERNAL_GLOBAL_SCOPED(AbslAnnotateThreadName)(__FILE__, __LINE__, name)

// -------------------------------------------------------------
// Annotations useful when implementing locks. They are not normally needed by
// modules that merely use locks. The `lock` argument is a pointer to the lock
// object.

// Report that a lock has been created at address `lock`.
#define ABSL_ANNOTATE_RWLOCK_CREATE(lock) \
  ABSL_INTERNAL_GLOBAL_SCOPED(AbslAnnotateRWLockCreate)(__FILE__, __LINE__, lock)

// Report that a linker initialized lock has been created at address `lock`.
#ifdef THREAD_SANITIZER
#define ABSL_ANNOTATE_RWLOCK_CREATE_STATIC(lock)          \
  ABSL_INTERNAL_GLOBAL_SCOPED(AbslAnnotateRWLockCreateStatic) \
  (__FILE__, __LINE__, lock)
#else
#define ABSL_ANNOTATE_RWLOCK_CREATE_STATIC(lock) \
  ABSL_ANNOTATE_RWLOCK_CREATE(lock)
#endif

// Report that the lock at address `lock` is about to be destroyed.
#define ABSL_ANNOTATE_RWLOCK_DESTROY(lock) \
  ABSL_INTERNAL_GLOBAL_SCOPED(AbslAnnotateRWLockDestroy)(__FILE__, __LINE__, lock)

// Report that the lock at address `lock` has been acquired.
// `is_w`=1 for writer lock, `is_w`=0 for reader lock.
#define ABSL_ANNOTATE_RWLOCK_ACQUIRED(lock, is_w)     \
  ABSL_INTERNAL_GLOBAL_SCOPED(AbslAnnotateRWLockAcquired) \
  (__FILE__, __LINE__, lock, is_w)

// Report that the lock at address `lock` is about to be released.
// `is_w`=1 for writer lock, `is_w`=0 for reader lock.
#define ABSL_ANNOTATE_RWLOCK_RELEASED(lock, is_w)     \
  ABSL_INTERNAL_GLOBAL_SCOPED(AbslAnnotateRWLockReleased) \
  (__FILE__, __LINE__, lock, is_w)

// Apply ABSL_ANNOTATE_BENIGN_RACE_SIZED to a static variable `static_var`.
#define ABSL_ANNOTATE_BENIGN_RACE_STATIC(static_var, description)      \
  namespace {                                                          \
  class static_var##_annotator {                                       \
   public:                                                             \
    static_var##_annotator() {                                         \
      ABSL_ANNOTATE_BENIGN_RACE_SIZED(&static_var, sizeof(static_var), \
                                      #static_var ": " description);   \
    }                                                                  \
  };                                                                   \
  static static_var##_annotator the##static_var##_annotator;           \
  }  // namespace

// Function prototypes of annotations provided by the compiler-based sanitizer
// implementation.
ABSL_INTERNAL_BEGIN_EXTERN_C
void AbslAnnotateRWLockCreate(const char* file, int line,
                          const volatile void* lock);
void AbslAnnotateRWLockCreateStatic(const char* file, int line,
                                const volatile void* lock);
void AbslAnnotateRWLockDestroy(const char* file, int line,
                           const volatile void* lock);
void AbslAnnotateRWLockAcquired(const char* file, int line,
                            const volatile void* lock, long is_w);  // NOLINT
void AbslAnnotateRWLockReleased(const char* file, int line,
                            const volatile void* lock, long is_w);  // NOLINT
void AbslAnnotateBenignRace(const char* file, int line,
                        const volatile void* address, const char* description);
void AbslAnnotateBenignRaceSized(const char* file, int line,
                             const volatile void* address, size_t size,
                             const char* description);
void AbslAnnotateThreadName(const char* file, int line, const char* name);
void AbslAnnotateEnableRaceDetection(const char* file, int line, int enable);
ABSL_INTERNAL_END_EXTERN_C

#else  // ABSL_INTERNAL_RACE_ANNOTATIONS_ENABLED == 0

#define ABSL_ANNOTATE_RWLOCK_CREATE(lock)                            // empty
#define ABSL_ANNOTATE_RWLOCK_CREATE_STATIC(lock)                     // empty
#define ABSL_ANNOTATE_RWLOCK_DESTROY(lock)                           // empty
#define ABSL_ANNOTATE_RWLOCK_ACQUIRED(lock, is_w)                    // empty
#define ABSL_ANNOTATE_RWLOCK_RELEASED(lock, is_w)                    // empty
#define ABSL_ANNOTATE_BENIGN_RACE(address, description)              // empty
#define ABSL_ANNOTATE_BENIGN_RACE_SIZED(address, size, description)  // empty
#define ABSL_ANNOTATE_THREAD_NAME(name)                              // empty
#define ABSL_ANNOTATE_ENABLE_RACE_DETECTION(enable)                  // empty
#define ABSL_ANNOTATE_BENIGN_RACE_STATIC(static_var, description)    // empty

#endif  // ABSL_INTERNAL_RACE_ANNOTATIONS_ENABLED

// -------------------------------------------------------------------------
// Define memory annotations.

#if ABSL_INTERNAL_MEMORY_ANNOTATIONS_ENABLED == 1

#include <sanitizer/msan_interface.h>

#define ABSL_ANNOTATE_MEMORY_IS_INITIALIZED(address, size) \
  __msan_unpoison(address, size)

#define ABSL_ANNOTATE_MEMORY_IS_UNINITIALIZED(address, size) \
  __msan_allocated_memory(address, size)

#else  // ABSL_INTERNAL_MEMORY_ANNOTATIONS_ENABLED == 0

#if ABSL_DYNAMIC_ANNOTATIONS_ENABLED == 1
#define ABSL_ANNOTATE_MEMORY_IS_INITIALIZED(address, size) \
  do {                                                     \
    (void)(address);                                       \
    (void)(size);                                          \
  } while (0)
#define ABSL_ANNOTATE_MEMORY_IS_UNINITIALIZED(address, size) \
  do {                                                       \
    (void)(address);                                         \
    (void)(size);                                            \
  } while (0)
#else

#define ABSL_ANNOTATE_MEMORY_IS_INITIALIZED(address, size)    // empty
#define ABSL_ANNOTATE_MEMORY_IS_UNINITIALIZED(address, size)  // empty

#endif

#endif  // ABSL_INTERNAL_MEMORY_ANNOTATIONS_ENABLED

// -------------------------------------------------------------------------
// Define IGNORE_READS_BEGIN/_END attributes.

#if ABSL_INTERNAL_IGNORE_READS_ATTRIBUTE_ENABLED == 1

#define ABSL_INTERNAL_IGNORE_READS_BEGIN_ATTRIBUTE \
  __attribute((exclusive_lock_function("*")))
#define ABSL_INTERNAL_IGNORE_READS_END_ATTRIBUTE \
  __attribute((unlock_function("*")))

#else  // ABSL_INTERNAL_IGNORE_READS_ATTRIBUTE_ENABLED == 0

#define ABSL_INTERNAL_IGNORE_READS_BEGIN_ATTRIBUTE  // empty
#define ABSL_INTERNAL_IGNORE_READS_END_ATTRIBUTE    // empty

#endif  // ABSL_INTERNAL_IGNORE_READS_ATTRIBUTE_ENABLED

// -------------------------------------------------------------------------
// Define IGNORE_READS_BEGIN/_END annotations.

#if ABSL_INTERNAL_READS_ANNOTATIONS_ENABLED == 1

// Request the analysis tool to ignore all reads in the current thread until
// ABSL_ANNOTATE_IGNORE_READS_END is called. Useful to ignore intentional racey
// reads, while still checking other reads and all writes.
// See also ABSL_ANNOTATE_UNPROTECTED_READ.
#define ABSL_ANNOTATE_IGNORE_READS_BEGIN() \
  ABSL_INTERNAL_GLOBAL_SCOPED(AbslAnnotateIgnoreReadsBegin)(__FILE__, __LINE__)

// Stop ignoring reads.
#define ABSL_ANNOTATE_IGNORE_READS_END() \
  ABSL_INTERNAL_GLOBAL_SCOPED(AbslAnnotateIgnoreReadsEnd)(__FILE__, __LINE__)

// Function prototypes of annotations provided by the compiler-based sanitizer
// implementation.
ABSL_INTERNAL_BEGIN_EXTERN_C
void AbslAnnotateIgnoreReadsBegin(const char* file, int line)
    ABSL_INTERNAL_IGNORE_READS_BEGIN_ATTRIBUTE;
void AbslAnnotateIgnoreReadsEnd(const char* file,
                            int line) ABSL_INTERNAL_IGNORE_READS_END_ATTRIBUTE;
ABSL_INTERNAL_END_EXTERN_C

#elif defined(ABSL_INTERNAL_ANNOTALYSIS_ENABLED)

// When Annotalysis is enabled without Dynamic Annotations, the use of
// static-inline functions allows the annotations to be read at compile-time,
// while still letting the compiler elide the functions from the final build.
//
// TODO(delesley) -- The exclusive lock here ignores writes as well, but
// allows IGNORE_READS_AND_WRITES to work properly.

#define ABSL_ANNOTATE_IGNORE_READS_BEGIN() \
  ABSL_INTERNAL_GLOBAL_SCOPED(AbslInternalAnnotateIgnoreReadsBegin)()

#define ABSL_ANNOTATE_IGNORE_READS_END() \
  ABSL_INTERNAL_GLOBAL_SCOPED(AbslInternalAnnotateIgnoreReadsEnd)()

ABSL_INTERNAL_STATIC_INLINE void AbslInternalAnnotateIgnoreReadsBegin()
    ABSL_INTERNAL_IGNORE_READS_BEGIN_ATTRIBUTE {}

ABSL_INTERNAL_STATIC_INLINE void AbslInternalAnnotateIgnoreReadsEnd()
    ABSL_INTERNAL_IGNORE_READS_END_ATTRIBUTE {}

#else

#define ABSL_ANNOTATE_IGNORE_READS_BEGIN()  // empty
#define ABSL_ANNOTATE_IGNORE_READS_END()    // empty

#endif

// -------------------------------------------------------------------------
// Define IGNORE_WRITES_BEGIN/_END annotations.

#if ABSL_INTERNAL_WRITES_ANNOTATIONS_ENABLED == 1

// Similar to ABSL_ANNOTATE_IGNORE_READS_BEGIN, but ignore writes instead.
#define ABSL_ANNOTATE_IGNORE_WRITES_BEGIN() \
  ABSL_INTERNAL_GLOBAL_SCOPED(AbslAnnotateIgnoreWritesBegin)(__FILE__, __LINE__)

// Stop ignoring writes.
#define ABSL_ANNOTATE_IGNORE_WRITES_END() \
  ABSL_INTERNAL_GLOBAL_SCOPED(AbslAnnotateIgnoreWritesEnd)(__FILE__, __LINE__)

// Function prototypes of annotations provided by the compiler-based sanitizer
// implementation.
ABSL_INTERNAL_BEGIN_EXTERN_C
void AbslAnnotateIgnoreWritesBegin(const char* file, int line);
void AbslAnnotateIgnoreWritesEnd(const char* file, int line);
ABSL_INTERNAL_END_EXTERN_C

#else

#define ABSL_ANNOTATE_IGNORE_WRITES_BEGIN()  // empty
#define ABSL_ANNOTATE_IGNORE_WRITES_END()    // empty

#endif

// -------------------------------------------------------------------------
// Define the ABSL_ANNOTATE_IGNORE_READS_AND_WRITES_* annotations using the more
// primitive annotations defined above.
//
//     Instead of doing
//        ABSL_ANNOTATE_IGNORE_READS_BEGIN();
//        ... = x;
//        ABSL_ANNOTATE_IGNORE_READS_END();
//     one can use
//        ... = ABSL_ANNOTATE_UNPROTECTED_READ(x);

#if defined(ABSL_INTERNAL_READS_WRITES_ANNOTATIONS_ENABLED)

// Start ignoring all memory accesses (both reads and writes).
#define ABSL_ANNOTATE_IGNORE_READS_AND_WRITES_BEGIN() \
  do {                                                \
    ABSL_ANNOTATE_IGNORE_READS_BEGIN();               \
    ABSL_ANNOTATE_IGNORE_WRITES_BEGIN();              \
  } while (0)

// Stop ignoring both reads and writes.
#define ABSL_ANNOTATE_IGNORE_READS_AND_WRITES_END() \
  do {                                              \
    ABSL_ANNOTATE_IGNORE_WRITES_END();              \
    ABSL_ANNOTATE_IGNORE_READS_END();               \
  } while (0)

#ifdef __cplusplus
// ABSL_ANNOTATE_UNPROTECTED_READ is the preferred way to annotate racey reads.
#define ABSL_ANNOTATE_UNPROTECTED_READ(x) \
  absl::base_internal::AnnotateUnprotectedRead(x)

namespace absl {
ABSL_NAMESPACE_BEGIN
namespace base_internal {

template <typename T>
inline T AnnotateUnprotectedRead(const volatile T& x) {  // NOLINT
  ABSL_ANNOTATE_IGNORE_READS_BEGIN();
  T res = x;
  ABSL_ANNOTATE_IGNORE_READS_END();
  return res;
}

}  // namespace base_internal
ABSL_NAMESPACE_END
}  // namespace absl
#endif

#else

#define ABSL_ANNOTATE_IGNORE_READS_AND_WRITES_BEGIN()  // empty
#define ABSL_ANNOTATE_IGNORE_READS_AND_WRITES_END()    // empty
#define ABSL_ANNOTATE_UNPROTECTED_READ(x) (x)

#endif

ABSL_INTERNAL_BEGIN_EXTERN_C

// -------------------------------------------------------------------------
// Return non-zero value if running under valgrind.
//
//  If "valgrind.h" is included into dynamic_annotations.cc,
//  the regular valgrind mechanism will be used.
//  See http://valgrind.org/docs/manual/manual-core-adv.html about
//  RUNNING_ON_VALGRIND and other valgrind "client requests".
//  The file "valgrind.h" may be obtained by doing
//     svn co svn://svn.valgrind.org/valgrind/trunk/include
//
//  If for some reason you can't use "valgrind.h" or want to fake valgrind,
//  there are two ways to make this function return non-zero:
//    - Use environment variable: export RUNNING_ON_VALGRIND=1
//    - Make your tool intercept the function AbslRunningOnValgrind() and
//      change its return value.
//
int AbslRunningOnValgrind(void);

// AbslValgrindSlowdown returns:
//    * 1.0, if (AbslRunningOnValgrind() == 0)
//    * 50.0, if (AbslRunningOnValgrind() != 0 && getenv("VALGRIND_SLOWDOWN") ==
//    NULL)
//    * atof(getenv("VALGRIND_SLOWDOWN")) otherwise
//   This function can be used to scale timeout values:
//   EXAMPLE:
//   for (;;) {
//     DoExpensiveBackgroundTask();
//     SleepForSeconds(5 * AbslValgrindSlowdown());
//   }
//
double AbslValgrindSlowdown(void);

ABSL_INTERNAL_END_EXTERN_C

// -------------------------------------------------------------------------
// Address sanitizer annotations

#ifdef ADDRESS_SANITIZER
// Describe the current state of a contiguous container such as e.g.
// std::vector or std::string. For more details see
// sanitizer/common_interface_defs.h, which is provided by the compiler.
#include <sanitizer/common_interface_defs.h>

#define ABSL_ANNOTATE_CONTIGUOUS_CONTAINER(beg, end, old_mid, new_mid) \
  __sanitizer_annotate_contiguous_container(beg, end, old_mid, new_mid)
#define ABSL_ADDRESS_SANITIZER_REDZONE(name) \
  struct {                                   \
    char x[8] __attribute__((aligned(8)));   \
  } name

#else

#define ABSL_ANNOTATE_CONTIGUOUS_CONTAINER(beg, end, old_mid, new_mid)
#define ABSL_ADDRESS_SANITIZER_REDZONE(name) static_assert(true, "")

#endif  // ADDRESS_SANITIZER

// -------------------------------------------------------------------------
// Undefine the macros intended only for this file.

#undef ABSL_INTERNAL_RACE_ANNOTATIONS_ENABLED
#undef ABSL_INTERNAL_MEMORY_ANNOTATIONS_ENABLED
#undef ABSL_INTERNAL_READS_ANNOTATIONS_ENABLED
#undef ABSL_INTERNAL_WRITES_ANNOTATIONS_ENABLED
#undef ABSL_INTERNAL_ANNOTALYSIS_ENABLED
#undef ABSL_INTERNAL_READS_WRITES_ANNOTATIONS_ENABLED
#undef ABSL_INTERNAL_BEGIN_EXTERN_C
#undef ABSL_INTERNAL_END_EXTERN_C
#undef ABSL_INTERNAL_STATIC_INLINE

#endif  // ABSL_BASE_DYNAMIC_ANNOTATIONS_H_
