#pragma once

#include "api/peer_connection_interface.h"
#include "api/data_channel_interface.h"

namespace Spitfire 
{
	class RtcConductor;

	namespace Observers
	{
		class DataChannelObserver : public webrtc::DataChannelObserver
		{
		public:
			explicit DataChannelObserver(RtcConductor* conductor) :
				conductor_(conductor)
			{
			}
			~DataChannelObserver() = default;

			// The data channel state have changed.
			void OnStateChange() override;

			//  A data buffer was successfully received.
			void OnMessage(const webrtc::DataBuffer & buffer) override;

			// The data channel's buffered_amount has changed.
			void OnBufferedAmountChange(uint64_t previous_amount) override;

			rtc::scoped_refptr<webrtc::DataChannelInterface> dataChannel;

			int AddRef() const
			{
				return 0;
			};
			int Release() const
			{
				return 0;
			};

		private:
			RtcConductor* conductor_;
		};
	}
}