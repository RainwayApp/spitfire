#pragma once
#include "api/jsep.h"

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

			int AddRef() const override
			{
				return 0;
			};
			int Release() const override
			{
				return 0;
			};

		private:
			RtcConductor * _manager;
		};
	}
}