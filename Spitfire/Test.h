#pragma once

namespace Native
{
	class Conductor : public webrtc::PeerConnectionObserver,
		public webrtc::CreateSessionDescriptionObserver,
		public webrtc::SetSessionDescriptionObserver,
		public webrtc::DataChannelObserver
	{

	}
}