// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef THIRD_PARTY_BLINK_RENDERER_CORE_DOM_IDLE_DEADLINE_H_
#define THIRD_PARTY_BLINK_RENDERER_CORE_DOM_IDLE_DEADLINE_H_

#include "third_party/blink/renderer/core/core_export.h"
#include "third_party/blink/renderer/platform/bindings/script_wrappable.h"
#include "third_party/blink/renderer/platform/heap/handle.h"
#include "third_party/blink/renderer/platform/wtf/time.h"

namespace base {
class TickClock;
}

namespace blink {

class CORE_EXPORT IdleDeadline : public ScriptWrappable {
  DEFINE_WRAPPERTYPEINFO();

 public:
  // These values are persisted to logs. Entries should not be renumbered and
  // numeric values should never be reused.
  enum class CallbackType {
    kCalledWhenIdle = 0,
    kCalledByTimeout = 1,
    kMaxValue = kCalledByTimeout
  };

  IdleDeadline(TimeTicks deadline, CallbackType);

  double timeRemaining() const;

  bool didTimeout() const {
    return callback_type_ == CallbackType::kCalledByTimeout;
  }

  // The caller is the owner of the |clock|. The |clock| must outlive the
  // IdleDeadline.
  void SetTickClockForTesting(const base::TickClock* clock) { clock_ = clock; }

 private:
  TimeTicks deadline_;
  CallbackType callback_type_;
  const base::TickClock* clock_;
};

}  // namespace blink

#endif  // THIRD_PARTY_BLINK_RENDERER_CORE_DOM_IDLE_DEADLINE_H_
