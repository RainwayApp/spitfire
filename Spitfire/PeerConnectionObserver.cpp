#include "PeerConnectionObserver.h"
#include "RtcConductor.h"

namespace Spitfire
{
namespace Observers
{

void PeerConnectionObserver::OnDataChannel(rtc::scoped_refptr<webrtc::DataChannelInterface> channel)
{
	RTC_LOG(INFO) << __FUNCTION__;
	RTC_DCHECK(channel);
	RTC_DCHECK(conductor_);
	conductor_->HandleDataChannel(channel);
}

void PeerConnectionObserver::OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState new_state)
{
	RTC_LOG(INFO) << __FUNCTION__ << ": " << new_state;
	RTC_DCHECK(conductor_);
	if(conductor_->onIceStateChange)
		conductor_->onIceStateChange(new_state);
}

void PeerConnectionObserver::OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState new_state)
{
	RTC_LOG(INFO) << __FUNCTION__ << ": " << new_state;
	RTC_DCHECK(conductor_);
	if(conductor_->onIceGatheringStateChange) 
		conductor_->onIceGatheringStateChange(new_state);
}

void PeerConnectionObserver::OnIceCandidate(const webrtc::IceCandidateInterface* candidate)
{
	RTC_LOG(INFO) << __FUNCTION__ << ": " << candidate->sdp_mline_index();
	RTC_DCHECK(candidate);
	std::string sdp;
	if(candidate->ToString(&sdp))
	{
		RTC_DCHECK(conductor_);
		if(conductor_->onIceCandidate)
			conductor_->onIceCandidate(candidate->sdp_mid().c_str(), candidate->sdp_mline_index(), sdp.c_str());
	} else
		RTC_LOG(LS_ERROR) << "Failed to serialize candidate";
}

}
}