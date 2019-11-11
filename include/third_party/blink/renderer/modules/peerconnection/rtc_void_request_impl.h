/*
 * Copyright (C) 2012 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer
 *    in the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name of Google Inc. nor the names of its contributors
 *    may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef THIRD_PARTY_BLINK_RENDERER_MODULES_PEERCONNECTION_RTC_VOID_REQUEST_IMPL_H_
#define THIRD_PARTY_BLINK_RENDERER_MODULES_PEERCONNECTION_RTC_VOID_REQUEST_IMPL_H_

#include "base/optional.h"
#include "third_party/blink/renderer/bindings/core/v8/v8_void_function.h"
#include "third_party/blink/renderer/bindings/modules/v8/v8_rtc_peer_connection_error_callback.h"
#include "third_party/blink/renderer/core/execution_context/context_lifecycle_observer.h"
#include "third_party/blink/renderer/modules/peerconnection/rtc_session_description_enums.h"
#include "third_party/blink/renderer/platform/heap/handle.h"
#include "third_party/blink/renderer/platform/peerconnection/rtc_void_request.h"

namespace blink {

class RTCPeerConnection;

// TODO(https://crbug.com/908468): Split up the operation-specific codepaths
// into separate request implementations and find a way to consolidate the
// shared code as to not repeat the majority of the implementations.
class RTCVoidRequestImpl final : public RTCVoidRequest,
                                 public ContextLifecycleObserver {
  USING_GARBAGE_COLLECTED_MIXIN(RTCVoidRequestImpl);

 public:
  RTCVoidRequestImpl(ExecutionContext*,
                     base::Optional<RTCSetSessionDescriptionOperation>,
                     RTCPeerConnection*,
                     V8VoidFunction*,
                     V8RTCPeerConnectionErrorCallback*);
  ~RTCVoidRequestImpl() override;

  // RTCVoidRequest
  void RequestSucceeded() override;
  void RequestFailed(const webrtc::RTCError&) override;

  // ContextLifecycleObserver
  void ContextDestroyed(ExecutionContext*) override;

  void Trace(blink::Visitor*) override;

 private:
  void Clear();

  base::Optional<RTCSetSessionDescriptionOperation> operation_;
  // This request object is held by WebRTCPeerConnectionHandler, which doesn't
  // support wrapper-tracing. Thus, this object holds the underlying callback
  // functions as persistent handles. This is acceptable because the request
  // object will be discarded in a limited time due to success, failure, or
  // destruction of the execution context.
  Member<V8PersistentCallbackFunction<V8VoidFunction>> success_callback_;
  Member<V8PersistentCallbackFunction<V8RTCPeerConnectionErrorCallback>>
      error_callback_;

  Member<RTCPeerConnection> requester_;
};

}  // namespace blink

#endif  // THIRD_PARTY_BLINK_RENDERER_MODULES_PEERCONNECTION_RTC_VOID_REQUEST_IMPL_H_
