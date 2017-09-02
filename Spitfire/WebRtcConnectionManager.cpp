#include "WebRtcConnectionManager.h"

namespace Spitfire
{
	Spitfire::WebRtcConnectionManager::WebRtcConnectionManager()
	{

		onError = nullptr;
		onSuccess = nullptr;
		onFailure = nullptr;
		onIceCandidate = nullptr;
		onDataMessage = nullptr;
		_dataChannel = nullptr;
		_peerConnection = nullptr;
		_pcFactory = nullptr;
		printf("The current value of i is %d\n", 1);
		fflush(stdout);
	}

	Spitfire::WebRtcConnectionManager::~WebRtcConnectionManager()
	{
		DeletePeerConnection();
		RTC_DCHECK(_peerConnection == nullptr);
	}

	void Spitfire::WebRtcConnectionManager::OnSuccess(webrtc::SessionDescriptionInterface* desc)
	{
		if (!_peerConnection)
			return;

		_peerConnection->SetLocalDescription(this, desc);
		std::string sdp;
		desc->ToString(&sdp);
		if (onSuccess != nullptr)
		{
			onSuccess(desc->type().c_str(), sdp.c_str());
		}
	}

	bool Spitfire::WebRtcConnectionManager::InitializePeerConnection()
	{
		RTC_DCHECK(_pcFactory == nullptr);
		RTC_DCHECK(_peerConnection == nullptr);
		_pcFactory = webrtc::CreatePeerConnectionFactory();

		if (!_pcFactory)
		{
			DeletePeerConnection();
			return false;
		}

		webrtc::PeerConnectionFactoryInterface::Options opt;
		{
			 //Not making this public because someone is going to be dumb enough to disable encryption.
			_pcFactory->SetOptions(opt);
		}
		if (!CreatePeerConnection(true))
		{
			DeletePeerConnection();
			return false;
		}
		return _peerConnection != nullptr;
	}

	void Spitfire::WebRtcConnectionManager::CreateOffer()
	{
		if (!_peerConnection)
			return;

		_peerConnection->CreateOffer(this, nullptr);
	}

	void Spitfire::WebRtcConnectionManager::OnOfferReply(const std::string & type, const std::string & sdp)
	{
		if (!_peerConnection)
			return;

		webrtc::SdpParseError error;
		webrtc::SessionDescriptionInterface* session_description(webrtc::CreateSessionDescription(type, sdp, &error));

		if (!session_description)
		{
			LOG(WARNING) << "Can't parse received session description message. " << "SdpParseError was: " << error.description;
			return;
		}
		_peerConnection->SetRemoteDescription(this, session_description);
	}

	void Spitfire::WebRtcConnectionManager::OnOfferRequest(const std::string & sdp)
	{
		if (!_peerConnection)
			return;
		webrtc::SdpParseError error;
		webrtc::SessionDescriptionInterface* session_description(webrtc::CreateSessionDescription("offer", sdp, &error));
		if (!session_description)
		{
			LOG(WARNING) << "Can't parse received session description message. " << "SdpParseError was: " << error.description;
			return;
		}
		_peerConnection->SetRemoteDescription(this, session_description);
		//Datachannel only.
		webrtc::PeerConnectionInterface::RTCOfferAnswerOptions o;
		{
			o.voice_activity_detection = false;
			o.offer_to_receive_audio = false;
			o.offer_to_receive_video = false;
		}
		_peerConnection->CreateAnswer(this, o);
	}

	bool Spitfire::WebRtcConnectionManager::AddIceCandidate(const std::string & sdp_mid, int sdp_mlineindex, const std::string & sdp)
	{
		webrtc::SdpParseError error;
		webrtc::IceCandidateInterface * candidate = webrtc::CreateIceCandidate(sdp_mid, sdp_mlineindex, sdp, &error);
		if (!candidate)
		{
			LOG(WARNING) << "Can't parse received candidate message. "
				<< "SdpParseError was: " << error.description;
			return false;
		}
		if (!_peerConnection)
			return false;
		if (!_peerConnection->AddIceCandidate(candidate))
		{
			LOG(WARNING) << "Failed to apply the received candidate";
			return false;
		}
		return true;
	}

	void Spitfire::WebRtcConnectionManager::AddServerConfig(const std::string & uri, const std::string & username, const std::string & password)
	{
		webrtc::PeerConnectionInterface::IceServer server;
		server.uri = uri;
		server.username = username;
		server.password = password;
		_serverConfigs.push_back(server);
	}

	void Spitfire::WebRtcConnectionManager::CreateDataChannel(const std::string & label)
	{
		if (!_peerConnection)
			return;
		webrtc::DataChannelInit dc_options;

		dc_options.maxRetransmits = 1;
		dc_options.negotiated = false;
		dc_options.ordered = false;

		_dataChannel = _peerConnection->CreateDataChannel(label, &dc_options);
		_dataChannel->RegisterObserver(this);
	}

	void Spitfire::WebRtcConnectionManager::OnDataChannel(rtc::scoped_refptr<webrtc::DataChannelInterface> channel)
	{
		LOG(INFO) << __FUNCTION__ << " " << channel->label();

		_dataChannel = channel.get();
		_dataChannel->RegisterObserver(this);
	}


	void Spitfire::WebRtcConnectionManager::DataChannelSendText(const std::string & text)
	{
		_dataChannel->Send(webrtc::DataBuffer(text));
	}

	void Spitfire::WebRtcConnectionManager::DataChannelSendData(const webrtc::DataBuffer & data)
	{
		_dataChannel->Send(data);
	}

	void Spitfire::WebRtcConnectionManager::OnFailure(const std::string & error)
	{
		LOG(LERROR) << error;

		if (onFailure != nullptr)
		{
			onFailure(error.c_str());
		}
	}

	void Spitfire::WebRtcConnectionManager::OnError()
	{
		if (onError != nullptr)
		{
			onError();
		}
	}

	void Spitfire::WebRtcConnectionManager::OnIceCandidate(const webrtc::IceCandidateInterface * candidate)
	{
	}

	void Spitfire::WebRtcConnectionManager::OnIceChange()
	{
	}

	void Spitfire::WebRtcConnectionManager::OnSignalingChange(webrtc::PeerConnectionInterface::SignalingState state)
	{
	}

	void Spitfire::WebRtcConnectionManager::OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState state)
	{
		if (state == 5)
		{
			//TODO
		}
	}

	void Spitfire::WebRtcConnectionManager::OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState state)
	{
	}

	void Spitfire::WebRtcConnectionManager::OnStateChange(webrtc::PeerConnectionObserver::StateType state_changed)
	{
		
	}

	void Spitfire::WebRtcConnectionManager::OnRenegotiationNeeded()
	{
	}


	void Spitfire::WebRtcConnectionManager::OnStateChange()
	{

	}

	void Spitfire::WebRtcConnectionManager::OnMessage(const webrtc::DataBuffer & buffer)
	{
		if (buffer.binary)
		{
			if (onDataBinaryMessage != nullptr)
			{
				auto * data = buffer.data.data();
				onDataBinaryMessage(data, buffer.size());
			}
		}
		else
		{
			if (onDataMessage != nullptr)
			{
				std::string msg(buffer.data.data<char>(), buffer.size());
				onDataMessage(msg.c_str());
			}
		}
	}

	void Spitfire::WebRtcConnectionManager::OnBufferedAmountChange(uint64_t previous_amount)
	{
		//TODO
	}

	bool Spitfire::WebRtcConnectionManager::CreatePeerConnection(bool dtlsEnabled)
	{
		RTC_DCHECK(_pcFactory != nullptr);
		RTC_DCHECK(_peerConnection == nullptr);

		webrtc::PeerConnectionInterface::RTCConfiguration config;
		config.tcp_candidate_policy = webrtc::PeerConnectionInterface::kTcpCandidatePolicyDisabled;
		config.enable_dtls_srtp = rtc::Optional<bool>(dtlsEnabled);
		config.rtcp_mux_policy = webrtc::PeerConnectionInterface::kRtcpMuxPolicyRequire;
		for each (auto server in _serverConfigs)
		{
			config.servers.push_back(server);
		}

		webrtc::FakeConstraints constraints;
		constraints.SetAllowDtlsSctpDataChannels();
		constraints.SetMandatoryReceiveVideo(false);
		constraints.SetMandatoryReceiveAudio(false);
		constraints.SetMandatoryIceRestart(true);
		constraints.SetMandatoryUseRtpMux(true);
		_peerConnection = _pcFactory->CreatePeerConnection(config, &constraints, NULL, NULL, this);
		return _peerConnection != nullptr;
	}

	void Spitfire::WebRtcConnectionManager::DeletePeerConnection()
	{
		if (_peerConnection.get())
		{
			_peerConnection->Close();
			_peerConnection = nullptr;
		}
		if (_dataChannel.get())
		{
			_dataChannel->Close();
			_dataChannel->UnregisterObserver();
			_dataChannel = nullptr;
		}
		_serverConfigs.clear();
		_pcFactory = nullptr;
		onError = nullptr;
		onSuccess = nullptr;
		onFailure = nullptr;
		onIceCandidate = nullptr;
		onDataMessage = nullptr;
		_peerConnection = nullptr;
	}
}