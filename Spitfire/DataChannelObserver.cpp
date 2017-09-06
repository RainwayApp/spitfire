#include "DataChannelObserver.h"
#include "RtcConductor.h"

void Spitfire::Observers::DataChannelObserver::OnStateChange()
{
	auto state = dataChannel->state();
	if (_manager->onDataChannelState != nullptr)
	{
		_manager->onDataChannelState(state);
	}
}

void Spitfire::Observers::DataChannelObserver::OnBufferedAmountChange(uint64_t previous_amount)
{
	if (_manager->onBufferAmountChange != nullptr)
	{
		_manager->onBufferAmountChange(previous_amount, dataChannel->buffered_amount(), dataChannel->bytes_sent(), dataChannel->bytes_received());
	}
}

void Spitfire::Observers::DataChannelObserver::OnMessage(const webrtc::DataBuffer & buffer)
{
	if (buffer.binary)
	{

		if (_manager->onDataBinaryMessage != nullptr)
		{
			auto * data = buffer.data.data();
			_manager->onDataBinaryMessage(data, buffer.size());
		}
	}
	else
	{
		if (_manager->onDataMessage != nullptr)
		{
			std::string msg(buffer.data.data<char>(), buffer.size());
			_manager->onDataMessage(msg.c_str());
		}
	}
}

Spitfire::Observers::DataChannelObserver::~DataChannelObserver()
{

}

Spitfire::Observers::DataChannelObserver::DataChannelObserver(RtcConductor * manager)
{
	this->_manager = manager;
}

