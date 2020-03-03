// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef THIRD_PARTY_BLINK_RENDERER_MODULES_VR_NAVIGATOR_VR_H_
#define THIRD_PARTY_BLINK_RENDERER_MODULES_VR_NAVIGATOR_VR_H_

#include "base/macros.h"
#include "third_party/blink/renderer/bindings/core/v8/script_promise.h"
#include "third_party/blink/renderer/core/frame/local_dom_window.h"
#include "third_party/blink/renderer/core/frame/navigator.h"
#include "third_party/blink/renderer/core/page/focus_changed_observer.h"
#include "third_party/blink/renderer/modules/modules_export.h"
#include "third_party/blink/renderer/modules/vr/vr_display.h"
#include "third_party/blink/renderer/modules/vr/vr_display_event.h"
#include "third_party/blink/renderer/platform/supplementable.h"

namespace blink {

class Document;
class VRController;

class MODULES_EXPORT NavigatorVR final
    : public GarbageCollected<NavigatorVR>,
      public Supplement<Navigator>,
      public LocalDOMWindow::EventListenerObserver,
      public FocusChangedObserver {
  USING_GARBAGE_COLLECTED_MIXIN(NavigatorVR);

 public:
  static const char kSupplementName[];

  // Returns whether WebVR has beeen used in the document.
  // If no supplement has been created, it returns false without creating one.
  // This allows it to be used in cases where creating objects is not allowed,
  // such as within NavigatorGamepad::DidAddEventListener().
  static bool HasWebVrBeenUsed(Document&);

  static NavigatorVR* From(Document&);
  static NavigatorVR& From(Navigator&);

  explicit NavigatorVR(Navigator&);
  ~NavigatorVR() override;

  // Legacy API
  static ScriptPromise getVRDisplays(ScriptState*, Navigator&);
  ScriptPromise getVRDisplays(ScriptState*);

  VRController* Controller();
  Document* GetDocument();
  bool IsFocused() const { return focused_; }

  // Queues up event to be fired soon.
  void EnqueueVREvent(VRDisplayEvent*);

  // Dispatches an event immediately.
  void DispatchVREvent(VRDisplayEvent*);

  // Inherited from FocusChangedObserver.
  void FocusedFrameChanged() override;

  // Inherited from LocalDOMWindow::EventListenerObserver.
  void DidAddEventListener(LocalDOMWindow*, const AtomicString&) override;
  void DidRemoveEventListener(LocalDOMWindow*, const AtomicString&) override;
  void DidRemoveAllEventListeners(LocalDOMWindow*) override;

  void Trace(blink::Visitor*) override;

 private:
  friend class VRDisplay;
  friend class VRGetDevicesCallback;

  void FireVRDisplayPresentChange(VRDisplay*);

  Member<VRController> controller_;

  // Whether this page is listening for vrdisplayactivate event.
  bool listening_for_activate_ = false;
  bool focused_ = false;

  bool did_use_webvr_ = false;

  // Metrics data - indicates whether we've already measured this data so we
  // don't do it every frame.
  bool did_log_getVRDisplays_ = false;

  DISALLOW_COPY_AND_ASSIGN(NavigatorVR);
};

}  // namespace blink

#endif  // THIRD_PARTY_BLINK_RENDERER_MODULES_VR_NAVIGATOR_VR_H_
