#pragma once

#include <api/peer_connection_interface.h>
#include <api/data_channel_interface.h>

namespace Spitfire 
{
	class RtcConductor;

	namespace Observers
	{
		class DataChannelObserver : public webrtc::DataChannelObserver
		{
		public:
			explicit DataChannelObserver(RtcConductor* conductor, rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel) :
				conductor_(conductor),
				data_channel_(data_channel)
			{
			}
			~DataChannelObserver()
			{
				#if defined(_DEBUG)
					RTC_DCHECK(!registered_);
				#endif
			}

			void RegisterObserver()
			{
				RTC_DCHECK(data_channel_);
				data_channel_->RegisterObserver(this);
				#if defined(_DEBUG)
					RTC_DCHECK(!registered_);
					registered_ = true;
				#endif
			}
			void UnregisterObserver()
			{
				RTC_DCHECK(data_channel_);
				// NOTE: The channel is assumed to be provided in constructor with immediate following observer registration, hence it's lifetime is directly
				//       related to this observer; we close the channel along with observer termination as a part of cleanup
				data_channel_->Close();
				data_channel_->UnregisterObserver();
				#if defined(_DEBUG)
					RTC_DCHECK(registered_);
					registered_ = false;
				#endif
			}

			// webrtc::DataChannelObserver
			void OnStateChange() override;
			void OnMessage(const webrtc::DataBuffer & buffer) override;
			void OnBufferedAmountChange(uint64_t previous_amount) override;

			rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel_;

		private:
			RtcConductor* conductor_;
			#if defined(_DEBUG)
				bool registered_ = false;
			#endif
		};
	}
}