#include "CreateSessionDescriptionObserver.h"
#include "RtcConductor.h"

namespace Spitfire 
{
namespace Observers
{

void CreateSessionDescriptionObserver::OnSuccess(webrtc::SessionDescriptionInterface* desc)
{
	RTC_DCHECK(desc);
	if (!conductor_->peer_observer_->peer_connection_.get())
	{
		return;
	}
	conductor_->peer_observer_->peer_connection_->SetLocalDescription(conductor_->set_session_observer_.get(), desc);
	std::string sdp;
	desc->ToString(&sdp);
	RTC_LOG_F(LS_INFO) << desc->type() << "; SDP: " << sdp;
	if (conductor_->on_success_)
	{
		conductor_->on_success_(desc->type().c_str(), sdp.c_str());
	}
}

void CreateSessionDescriptionObserver::OnFailure(webrtc::RTCError error)
{
	RTC_LOG_F(LS_ERROR) << error.message();
	if (conductor_->on_failure_)
	{
		conductor_->on_failure_(error.message());
	}
}

}
}
