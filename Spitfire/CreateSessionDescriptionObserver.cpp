#include "CreateSessionDescriptionObserver.h"
#include "RtcConductor.h"

Spitfire::Observers::CreateSessionDescriptionObserver::~CreateSessionDescriptionObserver()
{
}

Spitfire::Observers::CreateSessionDescriptionObserver::CreateSessionDescriptionObserver(RtcConductor * manager)
{
	this->_manager = manager;
}

void Spitfire::Observers::CreateSessionDescriptionObserver::OnSuccess(webrtc::SessionDescriptionInterface * desc)
{
	if (!_manager->peerObserver->peerConnection.get())
	{
		return;
	}
	_manager->peerObserver->peerConnection->SetLocalDescription(_manager->setSessionObserver.get(), desc);
	std::string sdp;
	desc->ToString(&sdp);
	if (_manager->onSuccess != nullptr)
	{
		_manager->onSuccess(desc->type().c_str(), sdp.c_str());
	}

}

void Spitfire::Observers::CreateSessionDescriptionObserver::OnFailure(const std::string & error)
{
	RTC_LOG(LERROR) << error;
	if (_manager->onFailure != nullptr)
	{
		_manager->onFailure(error.c_str());
	}
}