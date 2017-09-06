

#include "RtcConductor.h"

#include "webrtc/media/engine/fakewebrtcvideoengine.h";
#include "webrtc/media/engine/webrtcmediaengine.h"
#include "webrtc/pc/test/fakeaudiocapturemodule.h"
#include "webrtc/api/test/fakeconstraints.h"

using cricket::MediaEngineInterface;
using cricket::FakeWebRtcVideoDecoderFactory;
using cricket::FakeWebRtcVideoEncoder;
using cricket::FakeWebRtcVideoEncoderFactory;

namespace Spitfire
{


	RtcConductor::RtcConductor()
	{
		onError = nullptr;
		onSuccess = nullptr;
		onFailure = nullptr;
		onIceCandidate = nullptr;
		onDataChannelState = nullptr;
		onDataMessage = nullptr;

		dataObserver = new Observers::DataChannelObserver(this);
		peerObserver = new Observers::PeerConnectionObserver(this);
		sessionObserver = new Observers::CreateSessionDescriptionObserver(this);
		setSessionObserver = new Observers::SetSessionDescriptionObserver(this);
	}

	RtcConductor::~RtcConductor()
	{
		DeletePeerConnection();
		RTC_DCHECK(peerObserver->peerConnection == nullptr);
	}

	void RtcConductor::DeletePeerConnection()
	{
		if (peerObserver->peerConnection.get())
		{
			peerObserver->peerConnection->Close();
			peerObserver->peerConnection = nullptr;
			peerObserver = nullptr;
		}

		pc_factory_ = nullptr;

		if (dataObserver->dataChannel.get())
		{
			dataObserver->dataChannel->UnregisterObserver();
			dataObserver->dataChannel = nullptr;
			dataObserver = nullptr;
		}
		serverConfigs.clear();

	    network_thread_->Stop();
	    worker_thread_->Stop();
	    signaling_thread_->Stop();
		rtc::Thread::Current()->Stop();


	}

	bool RtcConductor::InitializePeerConnection()
	{
		
		rtc::ThreadManager::Instance()->WrapCurrentThread();
		RTC_DCHECK(pc_factory_ == nullptr);
		RTC_DCHECK(peerObserver->peerConnection == nullptr);

		network_thread_ = rtc::Thread::CreateWithSocketServer().release();
		worker_thread_ = rtc::Thread::Create().release();
		signaling_thread_ = rtc::Thread::Create().release();


		network_thread_->Start();
		worker_thread_->Start();
		signaling_thread_->Start();

		pc_factory_ = webrtc::CreatePeerConnectionFactory(network_thread_, worker_thread_, signaling_thread_, FakeAudioCaptureModule::Create(),
			new FakeWebRtcVideoEncoderFactory(),
			new FakeWebRtcVideoDecoderFactory());

		if (!pc_factory_)
		{
			DeletePeerConnection();
			return false;
		}
		webrtc::PeerConnectionFactoryInterface::Options opt;
		{
			//opt.disable_encryption = true;
			//opt.disable_network_monitor = true;
			//opt.disable_sctp_data_channels = true;
			pc_factory_->SetOptions(opt);
		}

		if (!CreatePeerConnection(true))
		{
			DeletePeerConnection();
			return false;
		}
		return peerObserver->peerConnection != nullptr;
	}

	bool RtcConductor::CreatePeerConnection(bool dtls)
	{
		RTC_DCHECK(pc_factory_ != nullptr);
		RTC_DCHECK(peerObserver->peerConnection == nullptr);

		webrtc::PeerConnectionInterface::RTCConfiguration config;
		config.tcp_candidate_policy = webrtc::PeerConnectionInterface::kTcpCandidatePolicyDisabled;
		config.disable_ipv6 = true;
		config.enable_dtls_srtp = rtc::Optional<bool>(dtls);
		config.rtcp_mux_policy = webrtc::PeerConnectionInterface::kRtcpMuxPolicyRequire;

		for each (auto server in serverConfigs)
		{
			config.servers.push_back(server);
		}

		webrtc::FakeConstraints constraints;
		constraints.SetAllowDtlsSctpDataChannels();
		constraints.SetMandatoryReceiveVideo(false);
		constraints.SetMandatoryReceiveAudio(false);
		constraints.SetMandatoryIceRestart(true);
		constraints.SetMandatoryUseRtpMux(true);
		constraints.AddMandatory(webrtc::MediaConstraintsInterface::kVoiceActivityDetection, "false");

		peerObserver->peerConnection = pc_factory_->CreatePeerConnection(config, &constraints, nullptr, nullptr, peerObserver);
		return peerObserver->peerConnection != nullptr;
	}

	void RtcConductor::AddServerConfig(std::string uri, std::string username, std::string password)
	{
		webrtc::PeerConnectionInterface::IceServer server;
		server.uri = uri;
		server.username = username;
		server.password = password;

		serverConfigs.push_back(server);
	}

	void RtcConductor::CreateOffer()
	{
		if (!peerObserver->peerConnection)
			return;

		peerObserver->peerConnection->CreateOffer(sessionObserver, nullptr);
	}

	void RtcConductor::OnOfferReply(std::string type, std::string sdp)
	{
		if (!peerObserver->peerConnection)
			return;

		webrtc::SdpParseError error;
		webrtc::SessionDescriptionInterface* session_description(webrtc::CreateSessionDescription(type, sdp, &error));
		if (!session_description)
		{
			LOG(WARNING) << "Can't parse received session description message. " << "SdpParseError was: " << error.description;
			return;
		}
		peerObserver->peerConnection->SetRemoteDescription(setSessionObserver, session_description);
	}

	void RtcConductor::OnOfferRequest(std::string sdp)
	{
		if (!peerObserver->peerConnection)
			return;

		webrtc::SdpParseError error;
		webrtc::SessionDescriptionInterface* session_description(webrtc::CreateSessionDescription("offer", sdp, &error));
		if (!session_description)
		{
			LOG(WARNING) << "Can't parse received session description message. " << "SdpParseError was: " << error.description;
			return;
		}
		peerObserver->peerConnection->SetRemoteDescription(setSessionObserver, session_description);

		webrtc::PeerConnectionInterface::RTCOfferAnswerOptions o;
		{
			o.voice_activity_detection = false;
			o.offer_to_receive_audio = false;
			o.offer_to_receive_video = webrtc::PeerConnectionInterface::RTCOfferAnswerOptions::kOfferToReceiveMediaTrue;
		}
		peerObserver->peerConnection->CreateAnswer(sessionObserver, o);
	}

	bool RtcConductor::AddIceCandidate(std::string sdp_mid, int sdp_mlineindex, std::string sdp)
	{
		webrtc::SdpParseError error;
		webrtc::IceCandidateInterface * candidate = webrtc::CreateIceCandidate(sdp_mid, sdp_mlineindex, sdp, &error);
		if (!candidate)
		{
			LOG(WARNING) << "Can't parse received candidate message. "
				<< "SdpParseError was: " << error.description;
			return false;
		}

		if (!peerObserver->peerConnection)
			return false;

		if (!peerObserver->peerConnection->AddIceCandidate(candidate))
		{
			LOG(WARNING) << "Failed to apply the received candidate";
			return false;
		}
		return true;
	}




	void RtcConductor::CreateDataChannel(const std::string & label)
	{
		if (!peerObserver->peerConnection)
			return;

		webrtc::DataChannelInit dc_options;
		//dc_options.id = 1;
		dc_options.maxRetransmits = 1;
		dc_options.negotiated = false;
		dc_options.ordered = false;

		dataObserver->dataChannel = peerObserver->peerConnection->CreateDataChannel(label, &dc_options);
		dataObserver->dataChannel->RegisterObserver(dataObserver);
	}

	

	void RtcConductor::DataChannelSendText(const std::string & text)
	{
		dataObserver->dataChannel->Send(webrtc::DataBuffer(text));
	}

	void RtcConductor::DataChannelSendData(const webrtc::DataBuffer & data)
	{
		dataObserver->dataChannel->Send(data);
	}

}