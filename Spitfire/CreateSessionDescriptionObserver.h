#pragma once


#include "webrtc/api/peerconnectioninterface.h"
#include "webrtc/api/jsep.h"



namespace Spitfire {

	class RtcConductor;

	namespace Observers
	{
		class CreateSessionDescriptionObserver : public webrtc::CreateSessionDescriptionObserver
		{
		public:


			virtual void OnSuccess(webrtc::SessionDescriptionInterface * desc);
			virtual void OnFailure(const std::string & error);
			CreateSessionDescriptionObserver(RtcConductor* manager);
			~CreateSessionDescriptionObserver();

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