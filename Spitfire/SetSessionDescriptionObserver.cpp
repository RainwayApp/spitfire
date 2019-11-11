#include "SetSessionDescriptionObserver.h"
#include "RtcConductor.h"

Spitfire::Observers::SetSessionDescriptionObserver::~SetSessionDescriptionObserver()
{

}


Spitfire::Observers::SetSessionDescriptionObserver::SetSessionDescriptionObserver(RtcConductor * manager)
{
	this->_manager = manager;
}

void Spitfire::Observers::SetSessionDescriptionObserver::OnFailure(const std::string & error)
{
	//RTC_LOG(INFO) << __FUNCTION__;
}
void Spitfire::Observers::SetSessionDescriptionObserver::OnSuccess()
{
	//RTC_LOG(INFO) << __FUNCTION__;
}