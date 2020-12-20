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

			// webrtc::DataChannelObserver
			void OnStateChange() override;
			void OnMessage(const webrtc::DataBuffer & buffer) override;
			void OnBufferedAmountChange(uint64_t previous_amount) override;

			rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel_;

		private:
			RtcConductor* conductor_;
		};
	}
}