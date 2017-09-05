#pragma once


#include "webrtc/api/peerconnectioninterface.h"
#include "webrtc/api/datachannelinterface.h"


namespace Spitfire {


	class RtcConductor;

	namespace Observers
	{


		class DataChannelObserver : public webrtc::DataChannelObserver
		{
		public:
			// The data channel state have changed.
			virtual void OnStateChange();

			//  A data buffer was successfully received.
			virtual void OnMessage(const webrtc::DataBuffer & buffer);

			// The data channel's buffered_amount has changed.
			virtual void OnBufferedAmountChange(uint64_t previous_amount)
			{

			}

			explicit DataChannelObserver(RtcConductor* manager);
			~DataChannelObserver();
			rtc::scoped_refptr<webrtc::DataChannelInterface> dataChannel;
			//gcroot<WebRtcInterop::RtcDataChannel ^> _dataChannel;
			//rtc::scoped_refptr<webrtc::DataChannelInterface> _nativeDataChannel;

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
