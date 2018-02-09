#pragma once


#include "api/peerconnectioninterface.h"
#include "api/jsep.h"



namespace Spitfire {

	class RtcConductor;

	namespace Observers
	{
		class CreateSessionDescriptionObserver : public webrtc::CreateSessionDescriptionObserver
		{
		public:


			void OnSuccess(webrtc::SessionDescriptionInterface * desc) override;
			void OnFailure(const std::string & error) override;
			explicit CreateSessionDescriptionObserver(RtcConductor* manager);
			~CreateSessionDescriptionObserver();

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