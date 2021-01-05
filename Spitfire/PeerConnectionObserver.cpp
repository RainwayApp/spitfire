#include "PeerConnectionObserver.h"
#include "RtcConductor.h"

namespace Spitfire
{
namespace Observers
{

void PeerConnectionObserver::OnDataChannel(rtc::scoped_refptr<webrtc::DataChannelInterface> channel)
{
	RTC_DCHECK(channel);
	RTC_DLOG_F(LS_INFO);
	RTC_DCHECK(conductor_);
	conductor_->HandleDataChannel(channel);
}

void PeerConnectionObserver::OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState new_state)
{
	RTC_DLOG_F(LS_INFO) << new_state;
	RTC_DCHECK(conductor_);
	if(conductor_->on_ice_state_change_)
		conductor_->on_ice_state_change_(new_state);
}

void PeerConnectionObserver::OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState new_state)
{
	RTC_DLOG_F(LS_INFO) << new_state;
	RTC_DCHECK(conductor_);
	if(conductor_->on_ice_gathering_state_change_) 
		conductor_->on_ice_gathering_state_change_(new_state);
}

void PeerConnectionObserver::OnIceCandidate(const webrtc::IceCandidateInterface* candidate)
{
	RTC_DCHECK(candidate);
	RTC_DLOG_F(LS_INFO) << candidate->sdp_mline_index();
	RTC_DCHECK(conductor_);
	conductor_->HandleIceCandidate(candidate);
}

}
}