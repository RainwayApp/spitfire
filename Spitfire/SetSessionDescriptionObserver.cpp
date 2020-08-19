#include "SetSessionDescriptionObserver.h"
#include "RtcConductor.h"


void Spitfire::Observers::SetSessionDescriptionObserver::OnFailure(webrtc::RTCError error)
{
	RTC_LOG(LS_ERROR) << __FUNCTION__ << error.message();
}
void Spitfire::Observers::SetSessionDescriptionObserver::OnSuccess()
{
	RTC_LOG(INFO) << __FUNCTION__;
}