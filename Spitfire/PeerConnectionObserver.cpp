#include "PeerConnectionObserver.h"
#include "RtcConductor.h"

void Spitfire::Observers::PeerConnectionObserver::OnSignalingChange(webrtc::PeerConnectionInterface::SignalingState new_state)
{
	LOG(INFO) << __FUNCTION__ << " ";
}


void Spitfire::Observers::PeerConnectionObserver::OnDataChannel(rtc::scoped_refptr<webrtc::DataChannelInterface> channel)
{
	if (_manager->dataObservers.find(channel->label()) == _manager->dataObservers.end())
	{
		LOG(INFO) << __FUNCTION__ << " " << channel->label();
		_manager->dataObservers[channel->label()] = new DataChannelObserver(_manager);
		_manager->dataObservers[channel->label()]->dataChannel = channel.get();
		_manager->dataObservers[channel->label()]->dataChannel->RegisterObserver(_manager->dataObservers[channel->label()]);
	}
}

void Spitfire::Observers::PeerConnectionObserver::OnRenegotiationNeeded()
{
	LOG(INFO) << __FUNCTION__ << " ";
}


void Spitfire::Observers::PeerConnectionObserver::OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState new_state)
{
	if (_manager->onIceStateChange != nullptr) {

		_manager->onIceStateChange(new_state);
	}
}

void Spitfire::Observers::PeerConnectionObserver::OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState new_state)
{
	LOG(INFO) << __FUNCTION__ << " ";

}
void Spitfire::Observers::PeerConnectionObserver::OnIceCandidate(const webrtc::IceCandidateInterface * candidate)
{
	LOG(INFO) << __FUNCTION__ << " " << candidate->sdp_mline_index();

	std::string sdp;
	if (!candidate->ToString(&sdp))
	{
		LOG(LS_ERROR) << "Failed to serialize candidate";
		return;
	}
	if (_manager->onIceCandidate != nullptr)
	{
		_manager->onIceCandidate(candidate->sdp_mid().c_str(), candidate->sdp_mline_index(), sdp.c_str());
	}
}

Spitfire::Observers::PeerConnectionObserver::~PeerConnectionObserver()
{
	LOG(INFO) << __FUNCTION__ << " ";

}



Spitfire::Observers::PeerConnectionObserver::PeerConnectionObserver(RtcConductor * manager)
{
	this->_manager = manager;
}