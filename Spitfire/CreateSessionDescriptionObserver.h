#pragma once

#include "api/peer_connection_interface.h"
#include "api/jsep.h"

namespace Spitfire 
{
	class RtcConductor;

	namespace Observers
	{
		class CreateSessionDescriptionObserver : public webrtc::CreateSessionDescriptionObserver
		{
		public:
			explicit CreateSessionDescriptionObserver(RtcConductor* conductor) :
				conductor_(conductor)
			{
			}
			~CreateSessionDescriptionObserver() = default;

			void OnSuccess(webrtc::SessionDescriptionInterface* desc) override;
			void OnFailure(webrtc::RTCError error) override;

			void AddRef() const override
			{
				ref_count_.IncRef();
			};
			rtc::RefCountReleaseStatus Release() const override
			{
				if (ref_count_.HasOneRef()) {
					delete this;
					return rtc::RefCountReleaseStatus::kDroppedLastRef;
				}
				ref_count_.DecRef();
				return rtc::RefCountReleaseStatus::kOtherRefsRemained;
			};

		private:
			mutable webrtc::webrtc_impl::RefCounter ref_count_{ 0 };
			RtcConductor* conductor_;
		};
	}
}