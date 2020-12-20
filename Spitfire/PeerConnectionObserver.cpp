#include "PeerConnectionObserver.h"
#include "RtcConductor.h"

namespace Spitfire
{
namespace Observers
{

void PeerConnectionObserver::OnDataChannel(rtc::scoped_refptr<webrtc::DataChannelInterface> channel)
{
	const auto label = channel->label();
	if(conductor_->data_observers_.find(label) != conductor_->data_observers_.end())
		return;
	RTC_LOG(INFO) << __FUNCTION__ << ": " << label;
	auto data_observer = std::make_unique<DataChannelObserver>(conductor_, channel);
	data_observer->RegisterObserver();
	conductor_->data_observers_[label] = std::move(data_observer);
}

void PeerConnectionObserver::OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState new_state)
{
	if(conductor_->onIceStateChange)
		conductor_->onIceStateChange(new_state);
}

void PeerConnectionObserver::OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState new_state)
{
	if(conductor_->onIceGatheringStateChange) 
		conductor_->onIceGatheringStateChange(new_state);
}

void PeerConnectionObserver::OnIceCandidate(const webrtc::IceCandidateInterface* candidate)
{
	RTC_LOG(INFO) << __FUNCTION__ << " " << candidate->sdp_mline_index();
	std::string sdp;
	if(candidate->ToString(&sdp))
	{
		if(conductor_->onIceCandidate)
			conductor_->onIceCandidate(candidate->sdp_mid().c_str(), candidate->sdp_mline_index(), sdp.c_str());
	} else
		RTC_LOG(LS_ERROR) << "Failed to serialize candidate";
}

}
}