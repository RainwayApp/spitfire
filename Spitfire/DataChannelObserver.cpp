#include "DataChannelObserver.h"
#include "RtcConductor.h"

void Spitfire::Observers::DataChannelObserver::OnStateChange()
{
	auto state = dataChannel->state();
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
	if (buffer.binary)
	{
		if (conductor_->onDataBinaryMessage)
		{
			auto * data = buffer.data.data();
			conductor_->onDataBinaryMessage(dataChannel->label().c_str(), data, static_cast<uint32_t>(buffer.size()));
		}
	}
	else
	{
		if (conductor_->onDataMessage)
		{
			std::string msg(buffer.data.data<char>(), buffer.size());
			conductor_->onDataMessage(dataChannel->label().c_str(), msg.c_str());
		}
	}
}
