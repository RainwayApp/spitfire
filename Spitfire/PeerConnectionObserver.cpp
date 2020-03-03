#include "PeerConnectionObserver.h"
#include "RtcConductor.h"

void Spitfire::Observers::PeerConnectionObserver::OnSignalingChange(webrtc::PeerConnectionInterface::SignalingState new_state)
{
	RTC_LOG(INFO) << __FUNCTION__ << " ";
}

void Spitfire::Observers::PeerConnectionObserver::OnDataChannel(rtc::scoped_refptr<webrtc::DataChannelInterface> channel)
{
	if (conductor_->dataObservers.find(channel->label()) == conductor_->dataObservers.end())
	{
		RTC_LOG(INFO) << __FUNCTION__ << " " << channel->label();
		conductor_->dataObservers[channel->label()] = new DataChannelObserver(conductor_);
		conductor_->dataObservers[channel->label()]->dataChannel = channel.get();
		conductor_->dataObservers[channel->label()]->dataChannel->RegisterObserver(conductor_->dataObservers[channel->label()]);
	}
}

void Spitfire::Observers::PeerConnectionObserver::OnRenegotiationNeeded()
{
	RTC_LOG(INFO) << __FUNCTION__ << " ";
}

void Spitfire::Observers::PeerConnectionObserver::OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState new_state)
{
	if (conductor_->onIceStateChange)
	{
		conductor_->onIceStateChange(new_state);
	}
}

void Spitfire::Observers::PeerConnectionObserver::OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState new_state)
{
	if (conductor_->onIceGatheringStateChange) 
	{
		conductor_->onIceGatheringStateChange(new_state);
	}
}

void Spitfire::Observers::PeerConnectionObserver::OnIceCandidate(const webrtc::IceCandidateInterface * candidate)
{
	RTC_LOG(INFO) << __FUNCTION__ << " " << candidate->sdp_mline_index();

	std::string sdp;
	if (!candidate->ToString(&sdp))
	{
		RTC_LOG(LS_ERROR) << "Failed to serialize candidate";
		return;
	}
	if (conductor_->onIceCandidate)
	{
		conductor_->onIceCandidate(candidate->sdp_mid().c_str(), candidate->sdp_mline_index(), sdp.c_str());
	}
}
