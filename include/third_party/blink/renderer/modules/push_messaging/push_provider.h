// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef THIRD_PARTY_BLINK_RENDERER_MODULES_PUSH_MESSAGING_PUSH_PROVIDER_H_
#define THIRD_PARTY_BLINK_RENDERER_MODULES_PUSH_MESSAGING_PUSH_PROVIDER_H_

#include <stdint.h>
#include <memory>

#include "base/macros.h"
#include "base/memory/scoped_refptr.h"
#include "base/single_thread_task_runner.h"
#include "third_party/blink/public/mojom/push_messaging/push_messaging.mojom-blink.h"
#include "third_party/blink/public/mojom/push_messaging/push_messaging_status.mojom-blink.h"
#include "third_party/blink/public/platform/modules/push_messaging/web_push_subscription.h"
#include "third_party/blink/renderer/modules/service_worker/service_worker_registration.h"
#include "third_party/blink/renderer/platform/supplementable.h"
#include "third_party/blink/renderer/platform/wtf/vector.h"

namespace blink {

namespace mojom {
enum class PushGetRegistrationStatus;
enum class PushRegistrationStatus;
}  // namespace mojom

class KURL;

class PushProvider final : public GarbageCollectedFinalized<PushProvider>,
                           public Supplement<ServiceWorkerRegistration> {
  USING_GARBAGE_COLLECTED_MIXIN(PushProvider);

 public:
  static const char kSupplementName[];

  explicit PushProvider(ServiceWorkerRegistration& registration);
  ~PushProvider() = default;

  static PushProvider* From(ServiceWorkerRegistration* registration);

  void Subscribe(const WebPushSubscriptionOptions& options,
                 bool user_gesture,
                 std::unique_ptr<WebPushSubscriptionCallbacks> callbacks);
  void Unsubscribe(std::unique_ptr<WebPushUnsubscribeCallbacks> callbacks);
  void GetSubscription(std::unique_ptr<WebPushSubscriptionCallbacks> callbacks);

 private:
  static void GetInterface(mojom::blink::PushMessagingRequest request);

  void DidSubscribe(std::unique_ptr<WebPushSubscriptionCallbacks> callbacks,
                    mojom::blink::PushRegistrationStatus status,
                    const base::Optional<KURL>& endpoint,
                    mojom::blink::PushSubscriptionOptionsPtr options,
                    const base::Optional<WTF::Vector<uint8_t>>& p256dh,
                    const base::Optional<WTF::Vector<uint8_t>>& auth);

  void DidUnsubscribe(std::unique_ptr<WebPushUnsubscribeCallbacks> callbacks,
                      mojom::blink::PushErrorType error_type,
                      bool did_unsubscribe,
                      const WTF::String& error_message);

  void DidGetSubscription(
      std::unique_ptr<WebPushSubscriptionCallbacks> callbacks,
      mojom::blink::PushGetRegistrationStatus status,
      const base::Optional<KURL>& endpoint,
      mojom::blink::PushSubscriptionOptionsPtr options,
      const base::Optional<WTF::Vector<uint8_t>>& p256dh,
      const base::Optional<WTF::Vector<uint8_t>>& auth);

  mojom::blink::PushMessagingPtr push_messaging_manager_;

  DISALLOW_COPY_AND_ASSIGN(PushProvider);
};

}  // namespace blink

#endif  // THIRD_PARTY_BLINK_RENDERER_MODULES_PUSH_MESSAGING_PUSH_PROVIDER_H_
