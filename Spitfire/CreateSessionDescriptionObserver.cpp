#include "CreateSessionDescriptionObserver.h"
#include "RtcConductor.h"

namespace Spitfire 
{
namespace Observers
{

void CreateSessionDescriptionObserver::OnSuccess(webrtc::SessionDescriptionInterface* desc)
{
	if (!conductor_->peer_observer_->peerConnection.get())
	{
		return;
	}
	conductor_->peer_observer_->peerConnection->SetLocalDescription(conductor_->set_session_observer_.get(), desc);
	std::string sdp;
	desc->ToString(&sdp);
	RTC_LOG(LS_INFO) << __FUNCTION__ << ": " << desc->type() << "; SDP: " << sdp;
	if (conductor_->onSuccess)
	{
		conductor_->onSuccess(desc->type().c_str(), sdp.c_str());
	}
}

void CreateSessionDescriptionObserver::OnFailure(webrtc::RTCError error)
{
	RTC_LOG(LS_ERROR) << __FUNCTION__ << ": " << error.message();
	if (conductor_->onFailure)
	{
		conductor_->onFailure(error.message());
	}
}

}
}
