#pragma once
#include "api/jsep.h"
#include "rtc_base/ref_count.h"
#include "rtc_base/ref_counted_object.h"

namespace Spitfire {

	class RtcConductor;

	namespace Observers
	{
		 class SetSessionDescriptionObserver : public webrtc::SetSessionDescriptionObserver
		{
		public:

			~SetSessionDescriptionObserver();
			 explicit SetSessionDescriptionObserver(RtcConductor* manager);

			 void OnSuccess() override;
			 void OnFailure(const std::string& error) override;

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
			RtcConductor * _manager;
		};
	}
}