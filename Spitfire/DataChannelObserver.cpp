#include "DataChannelObserver.h"
#include "RtcConductor.h"

namespace Spitfire
{
namespace Observers
{

void DataChannelObserver::OnStateChange()
{
	RTC_DCHECK(data_channel_);
	const auto state = data_channel_->state();
	RTC_DLOG_F(LS_INFO) << webrtc::DataChannelInterface::DataStateString(state);
	RTC_DCHECK(conductor_);
	if (conductor_->on_data_channel_state_change_)
	{
		conductor_->on_data_channel_state_change_(data_channel_->label().c_str(), state);
	}
}

void DataChannelObserver::OnMessage(const webrtc::DataBuffer& buffer)
{
	RTC_DLOG_F(LS_VERBOSE);
	RTC_DCHECK(conductor_);
	if (conductor_->on_message_)
	{
		conductor_->on_message_(data_channel_->label().c_str(), buffer.data.data(), static_cast<uint32_t>(buffer.size()), buffer.binary);
	}
}

void DataChannelObserver::OnBufferedAmountChange(uint64_t previous_amount)
{
	RTC_DLOG_F(LS_VERBOSE);
	RTC_DCHECK(conductor_);
	if (conductor_->on_buffer_amount_change_)
	{
		conductor_->on_buffer_amount_change_(data_channel_->label().c_str(), previous_amount, data_channel_->buffered_amount(), data_channel_->bytes_sent(), data_channel_->bytes_received());
	}
}

}
}
