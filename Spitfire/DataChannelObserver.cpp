#include "DataChannelObserver.h"
#include "RtcConductor.h"

namespace Spitfire
{
namespace Observers
{

void DataChannelObserver::OnStateChange()
{
	const auto state = data_channel_->state();
	RTC_LOG(INFO) << __FUNCTION__ << ": " << webrtc::DataChannelInterface::DataStateString(state);
	if (conductor_->onDataChannelState)
	{
		conductor_->onDataChannelState(data_channel_->label().c_str(), state);
	}
}

void DataChannelObserver::OnMessage(const webrtc::DataBuffer& buffer)
{
	RTC_LOG(INFO) << __FUNCTION__;
	if (conductor_->onMessage)
	{
		conductor_->onMessage(data_channel_->label().c_str(), buffer.data.data(), static_cast<uint32_t>(buffer.size()), buffer.binary);
	}
}

void DataChannelObserver::OnBufferedAmountChange(uint64_t previous_amount)
{
	RTC_LOG(INFO) << __FUNCTION__;
	if (conductor_->onBufferAmountChange)
	{
		conductor_->onBufferAmountChange(data_channel_->label().c_str(), previous_amount, data_channel_->buffered_amount(), data_channel_->bytes_sent(), data_channel_->bytes_received());
	}
}

}
}
