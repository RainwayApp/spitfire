// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef THIRD_PARTY_BLINK_RENDERER_MODULES_WEBGPU_GPU_BUFFER_H_
#define THIRD_PARTY_BLINK_RENDERER_MODULES_WEBGPU_GPU_BUFFER_H_

#include "third_party/blink/renderer/bindings/core/v8/script_promise.h"
#include "third_party/blink/renderer/core/typed_arrays/array_buffer_view_helpers.h"
#include "third_party/blink/renderer/modules/webgpu/dawn_object.h"
#include "third_party/blink/renderer/platform/bindings/exception_state.h"

namespace blink {

class DOMArrayBuffer;
class GPUBufferDescriptor;
class ScriptPromiseResolver;

class GPUBuffer : public DawnObject<DawnBuffer> {
  DEFINE_WRAPPERTYPEINFO();

 public:
  static GPUBuffer* Create(GPUDevice* device,
                           const GPUBufferDescriptor* webgpu_desc);
  explicit GPUBuffer(GPUDevice* device, uint64_t size, DawnBuffer buffer);
  ~GPUBuffer() override;

  void Trace(blink::Visitor* visitor) override;

  // gpu_buffer.idl
  void setSubData(uint64_t dst_byte_offset,
                  const MaybeShared<DOMArrayBufferView>& src,
                  uint64_t src_byte_offset,
                  uint64_t byte_length,
                  ExceptionState& exception_state);
  ScriptPromise mapReadAsync(ScriptState* script_state,
                             ExceptionState& exception_state);
  ScriptPromise mapWriteAsync(ScriptState* script_state,
                              ExceptionState& exception_state);
  void unmap(ScriptState* script_state);
  void destroy(ScriptState* script_state);
  // TODO(crbug.com/877147): implement GPUBuffer.

 private:
  void OnMapAsyncCallback(ScriptPromiseResolver* resolver,
                          DawnBufferMapAsyncStatus status,
                          void* data,
                          uint64_t data_length);
  void DetachArrayBufferForCurrentMapping(ScriptState* script_state);

  uint64_t size_;
  Member<DOMArrayBuffer> mapped_buffer_;

  DISALLOW_COPY_AND_ASSIGN(GPUBuffer);
};

}  // namespace blink

#endif  // THIRD_PARTY_BLINK_RENDERER_MODULES_WEBGPU_GPU_BUFFER_H_
