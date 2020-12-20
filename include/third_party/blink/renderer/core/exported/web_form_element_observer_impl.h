// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef THIRD_PARTY_BLINK_RENDERER_CORE_EXPORTED_WEB_FORM_ELEMENT_OBSERVER_IMPL_H_
#define THIRD_PARTY_BLINK_RENDERER_CORE_EXPORTED_WEB_FORM_ELEMENT_OBSERVER_IMPL_H_

#include "base/macros.h"
#include "base/util/type_safety/pass_key.h"
#include "third_party/blink/public/web/modules/autofill/web_form_element_observer.h"
#include "third_party/blink/renderer/core/core_export.h"
#include "third_party/blink/renderer/platform/heap/heap_allocator.h"
#include "third_party/blink/renderer/platform/heap/member.h"
#include "third_party/blink/renderer/platform/heap/self_keep_alive.h"

namespace blink {

class HTMLElement;

class CORE_EXPORT WebFormElementObserverImpl final
    : public GarbageCollected<WebFormElementObserverImpl>,
      public WebFormElementObserver {
 public:
  WebFormElementObserverImpl(util::PassKey<WebFormElementObserver>,
                             HTMLElement&,
                             base::OnceClosure);
  ~WebFormElementObserverImpl() override;

  // WebFormElementObserver implementation.
  void Disconnect() override;

  virtual void Trace(Visitor*);

 private:
  class ObserverCallback;

  Member<ObserverCallback> mutation_callback_;

  // WebFormElementObserverImpl must remain alive until Disconnect() is called.
  SelfKeepAlive<WebFormElementObserverImpl> self_keep_alive_;

  DISALLOW_COPY_AND_ASSIGN(WebFormElementObserverImpl);
};

}  // namespace blink

#endif  // THIRD_PARTY_BLINK_RENDERER_CORE_EXPORTED_WEB_FORM_ELEMENT_OBSERVER_IMPL_H_
