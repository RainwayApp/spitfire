#include "CreateSessionDescriptionObserver.h"
#include "RtcConductor.h"

void Spitfire::Observers::CreateSessionDescriptionObserver::OnSuccess(webrtc::SessionDescriptionInterface* desc)
{
	if (!conductor_->peerObserver->peerConnection.get())
	{
		return;
	}
	conductor_->peerObserver->peerConnection->SetLocalDescription(conductor_->setSessionObserver.get(), desc);
	std::string sdp;
	desc->ToString(&sdp);
	if (conductor_->onSuccess)
	{
		conductor_->onSuccess(desc->type().c_str(), sdp.c_str());
	}
}

void Spitfire::Observers::CreateSessionDescriptionObserver::OnFailure(webrtc::RTCError error)
{
	RTC_LOG(LERROR) << error.message();
	if (conductor_->onFailure)
	{
		conductor_->onFailure(error.message());
	}
}