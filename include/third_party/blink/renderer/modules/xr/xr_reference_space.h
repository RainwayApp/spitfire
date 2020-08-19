// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef THIRD_PARTY_BLINK_RENDERER_MODULES_XR_XR_REFERENCE_SPACE_H_
#define THIRD_PARTY_BLINK_RENDERER_MODULES_XR_XR_REFERENCE_SPACE_H_

#include <memory>

#include "device/vr/public/mojom/vr_service.mojom-blink.h"
#include "third_party/blink/renderer/modules/xr/xr_space.h"
#include "third_party/blink/renderer/platform/transforms/transformation_matrix.h"

namespace blink {

class XRRigidTransform;

class XRReferenceSpace : public XRSpace {
  DEFINE_WRAPPERTYPEINFO();

 public:
  static device::mojom::blink::XRReferenceSpaceType StringToReferenceSpaceType(
      const String& reference_space_type);

  XRReferenceSpace(XRSession* session,
                   device::mojom::blink::XRReferenceSpaceType type);
  XRReferenceSpace(XRSession* session,
                   XRRigidTransform* origin_offset,
                   device::mojom::blink::XRReferenceSpaceType type);
  ~XRReferenceSpace() override;

  base::Optional<TransformationMatrix> NativeFromMojo() override;
  base::Optional<TransformationMatrix> NativeFromViewer(
      const base::Optional<TransformationMatrix>& mojo_from_viewer) override;

  // MojoFromNative is final to enforce that children should be returning
  // NativeFromMojo, since this is simply written to always provide the inverse
  // of NativeFromMojo
  base::Optional<TransformationMatrix> MojoFromNative() final;

  bool IsStationary() const override;

  TransformationMatrix NativeFromOffsetMatrix() override;
  TransformationMatrix OffsetFromNativeMatrix() override;

  // We override getPose to ensure that the viewer pose in viewer space returns
  // the identity pose instead of the result of multiplying inverse matrices.
  XRPose* getPose(XRSpace* other_space) override;

  device::mojom::blink::XRReferenceSpaceType GetType() const;

  XRReferenceSpace* getOffsetReferenceSpace(XRRigidTransform* transform);

  DEFINE_ATTRIBUTE_EVENT_LISTENER(reset, kReset)

  base::Optional<device::mojom::blink::XRNativeOriginInformation> NativeOrigin()
      const final;

  void Trace(Visitor*) const override;

  virtual void OnReset();

 private:
  virtual XRReferenceSpace* cloneWithOriginOffset(
      XRRigidTransform* origin_offset);

  // Updates the floor_from_mojo_ transform to match the one present in the
  // latest display parameters of a session.
  void SetFloorFromMojo();

  unsigned int display_info_id_ = 0;

  // Floor from mojo (aka local-floor_from_mojo) transform.
  std::unique_ptr<TransformationMatrix> floor_from_mojo_;
  Member<XRRigidTransform> origin_offset_;
  device::mojom::blink::XRReferenceSpaceType type_;
};

}  // namespace blink

#endif  // THIRD_PARTY_BLINK_RENDERER_MODULES_XR_XR_REFERENCE_SPACE_H_
