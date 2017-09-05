#pragma once
#include "webrtc/api/jsep.h"

namespace Spitfire {

	class RtcConductor;

	namespace Observers
	{
		 class SetSessionDescriptionObserver : public webrtc::SetSessionDescriptionObserver
		{
		public:

			~SetSessionDescriptionObserver();
			SetSessionDescriptionObserver(RtcConductor* manager);

			virtual void OnSuccess();
			virtual void OnFailure(const std::string& error);

			int AddRef() const
			{
				return 0;
			};
			int Release() const
			{
				return 0;
			};

		private:
			RtcConductor * _manager;
		};
	}
}