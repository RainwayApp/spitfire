// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef THIRD_PARTY_BLINK_RENDERER_MODULES_XR_XR_CUBE_MAP_H_
#define THIRD_PARTY_BLINK_RENDERER_MODULES_XR_XR_CUBE_MAP_H_

#include "base/util/type_safety/pass_key.h"
#include "device/vr/public/mojom/vr_service.mojom-blink-forward.h"
#include "third_party/blink/renderer/core/typed_arrays/dom_typed_array.h"
#include "third_party/blink/renderer/platform/bindings/script_wrappable.h"

namespace blink {

class WebGL2RenderingContextBase;
class WebGLTexture;
class ExceptionState;

class XRCubeMap : public ScriptWrappable {
  DEFINE_WRAPPERTYPEINFO();

 public:
  explicit XRCubeMap(const device::mojom::blink::XRCubeMap& cube_map);

  WebGLTexture* updateWebGLEnvironmentCube(
      WebGL2RenderingContextBase* context,
      WebGLTexture* texture,
      ExceptionState& exception_state) const;

 private:
  uint32_t width_and_height_ = 0;
  WTF::Vector<device::RgbaTupleF16> positive_x_;
  WTF::Vector<device::RgbaTupleF16> negative_x_;
  WTF::Vector<device::RgbaTupleF16> positive_y_;
  WTF::Vector<device::RgbaTupleF16> negative_y_;
  WTF::Vector<device::RgbaTupleF16> positive_z_;
  WTF::Vector<device::RgbaTupleF16> negative_z_;
};

}  // namespace blink

#endif  // THIRD_PARTY_BLINK_RENDERER_MODULES_XR_XR_CUBE_MAP_H_
