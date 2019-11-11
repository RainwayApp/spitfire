#include "RtcConductor.h"
#include "p2p/client/basic_port_allocator.h"
#include <iostream>

using cricket::MediaEngineInterface;

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
		//dataObserver = new Observers::DataChannelObserver(this);
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
		default_socket_factory_ = nullptr;
		default_network_manager_ = nullptr;

		if (!dataObservers.empty())
		{
			for (auto const& x : dataObservers)
			{
				if (x.second->dataChannel.get())
				{
					x.second->dataChannel->UnregisterObserver();
					x.second->dataChannel = nullptr;
				}
			}
			dataObservers.empty();
		}
		serverConfigs.clear();

		network_thread_->Stop();
		worker_thread_->Stop();
		signaling_thread_->Stop();
		rtc::ThreadManager::Instance()->CurrentThread()->Stop();


	}

	bool RtcConductor::InitializePeerConnection(int minPort, int maxPort)
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

		webrtc::PeerConnectionFactoryDependencies factory_deps;
		factory_deps.network_thread = network_thread_;
		factory_deps.worker_thread = worker_thread_;
		factory_deps.signaling_thread = signaling_thread_;

		pc_factory_ = webrtc::CreateModularPeerConnectionFactory(std::move(factory_deps));

		if (!pc_factory_)
		{
			DeletePeerConnection();
			return false;
		}

		default_network_manager_.reset(new rtc::BasicNetworkManager());
		if (!default_network_manager_) {
			DeletePeerConnection();
			return false;
		}

		default_socket_factory_.reset(new rtc::BasicPacketSocketFactory(network_thread_));
		if (!default_socket_factory_) {
			DeletePeerConnection();
			return false;
		}

		default_relay_port_factory_.reset(new cricket::TurnPortFactory());
		if (!default_relay_port_factory_) {
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

		if (!CreatePeerConnection(true, minPort, maxPort))
		{
			DeletePeerConnection();
			return false;
		}
		return peerObserver->peerConnection != nullptr;
	}

	bool RtcConductor::CreatePeerConnection(bool dtls, int minPort, int maxPort)
	{
		RTC_DCHECK(pc_factory_ != nullptr);
		RTC_DCHECK(peerObserver->peerConnection == nullptr);

		webrtc::PeerConnectionInterface::RTCConfiguration config;
		
		config.tcp_candidate_policy = webrtc::PeerConnectionInterface::kTcpCandidatePolicyDisabled;
		//no clue why this was set to true
		config.disable_ipv6 = false;
		config.enable_dtls_srtp = absl::optional<bool>(dtls);
		config.rtcp_mux_policy = webrtc::PeerConnectionInterface::kRtcpMuxPolicyRequire;

		for each (auto server in serverConfigs)
		{
			config.servers.push_back(server);
		}	

		std::unique_ptr<cricket::PortAllocator> allocator = std::unique_ptr<cricket::PortAllocator>(new cricket::BasicPortAllocator(
			default_network_manager_.get(),
			default_socket_factory_.get(),
			config.turn_customizer,
			default_relay_port_factory_.get()));
		allocator->SetPortRange(minPort, maxPort);

		peerObserver->peerConnection = pc_factory_->CreatePeerConnection(config, std::move(allocator), nullptr, peerObserver.get());
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

		webrtc::PeerConnectionInterface::RTCOfferAnswerOptions options;
		options.offer_to_receive_audio = false;
		options.offer_to_receive_video = false;
		peerObserver->peerConnection->CreateOffer(sessionObserver, options);
	}

	void RtcConductor::OnOfferReply(std::string type, std::string sdp)
	{
		if (!peerObserver->peerConnection)
			return;

		webrtc::SdpParseError error;
		webrtc::SessionDescriptionInterface* session_description(webrtc::CreateSessionDescription(type, sdp, &error));
		if (!session_description)
		{
			RTC_LOG(WARNING) << "Can't parse received session description message. " << "SdpParseError was: " << error.description;
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
			RTC_LOG(WARNING) << "Can't parse received session description message. " << "SdpParseError was: " << error.description;
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
			RTC_LOG(WARNING) << "Can't parse received candidate message. "
				<< "SdpParseError was: " << error.description;
			return false;
		}

		if (!peerObserver->peerConnection)
			return false;

		if (!peerObserver->peerConnection->AddIceCandidate(candidate))
		{
			RTC_LOG(WARNING) << "Failed to apply the received candidate";
			return false;
		}
		return true;
	}




	void RtcConductor::CreateDataChannel(const std::string & label, const webrtc::DataChannelInit dc_options)
	{
		if (!peerObserver->peerConnection)
			return;

		if (dataObservers.find(label) == dataObservers.end()) {
			dataObservers[label] = new Observers::DataChannelObserver(this);
			dataObservers[label]->dataChannel = peerObserver->peerConnection->CreateDataChannel(label, &dc_options);
			dataObservers[label]->dataChannel->RegisterObserver(dataObservers[label]);
		}
	}



	void RtcConductor::DataChannelSendText(const std::string & label, const std::string & text)
	{
		auto observer = dataObservers.find(label);
		if (observer != dataObservers.end()) {
			observer->second->dataChannel->Send(webrtc::DataBuffer(text));
		}
	}

	RtcDataChannelInfo RtcConductor::GetDataChannelInfo(const std::string& label)
	{
		auto info = RtcDataChannelInfo();
		const auto observer = dataObservers.find(label);
		if (observer != dataObservers.end()) {

			const auto dataChannel = observer->second->dataChannel;

			info.id = dataChannel->id();
			info.currentBuffer = dataChannel->buffered_amount();
			info.bytesSent = dataChannel->bytes_sent();
			info.bytesReceived = dataChannel->bytes_received();

			info.reliable = dataChannel->reliable();
			info.ordered = dataChannel->ordered();
			info.negotiated = dataChannel->negotiated();

			info.messagesSent = dataChannel->messages_sent();
			info.messagesReceived = dataChannel->messages_received();

			info.maxRetransmits = dataChannel->maxRetransmits();
			info.maxRetransmitTime = dataChannel->maxRetransmitTime();

			info.protocol = dataChannel->protocol();
			info.state = dataChannel->state();

		

			return info;
		}
		info.protocol = "unknown";
		return info;
	}

	webrtc::DataChannelInterface::DataState RtcConductor::GetDataChannelState(const std::string& label)
	{
		const auto observer = dataObservers.find(label);
		if (observer != dataObservers.end()) {
			return dataObservers[label]->dataChannel->state();
		}
		return {};
	}

	void RtcConductor::DataChannelSendData(const std::string & label, const webrtc::DataBuffer & data)
	{
		auto observer = dataObservers.find(label);
		if (observer != dataObservers.end()) {
			dataObservers[label]->dataChannel->Send(data);
		}
	}

}