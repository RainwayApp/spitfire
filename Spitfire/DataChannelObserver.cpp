#include "DataChannelObserver.h"
#include "RtcConductor.h"

void Spitfire::Observers::DataChannelObserver::OnStateChange()
{
	const auto state = dataChannel->state();
	if (conductor_->onDataChannelState)
	{
		conductor_->onDataChannelState(dataChannel->label().c_str(), state);
	}
}

void Spitfire::Observers::DataChannelObserver::OnBufferedAmountChange(uint64_t previous_amount)
{
	if (conductor_->onBufferAmountChange)
	{
		conductor_->onBufferAmountChange(dataChannel->label().c_str(), previous_amount, dataChannel->buffered_amount(), dataChannel->bytes_sent(), dataChannel->bytes_received());
	}
}

void Spitfire::Observers::DataChannelObserver::OnMessage(const webrtc::DataBuffer & buffer)
{
	if (conductor_->onMessage)
	{
		conductor_->onMessage(dataChannel->label().c_str(), buffer.data.data(), static_cast<uint32_t>(buffer.size()), buffer.binary);
	}
}
