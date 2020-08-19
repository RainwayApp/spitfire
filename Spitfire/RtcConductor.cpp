#include "RtcConductor.h"
#include "p2p/client/basic_port_allocator.h"
#include <iostream>

using cricket::MediaEngineInterface;

namespace Spitfire
{
	RtcConductor::RtcConductor()
	{
		onSuccess = nullptr;
		onFailure = nullptr;
		onIceCandidate = nullptr;
		onDataChannelState = nullptr;
		onMessage = nullptr;
		peerObserver = new Observers::PeerConnectionObserver(this);
		sessionObserver = new Observers::CreateSessionDescriptionObserver(this);
		setSessionObserver = new Observers::SetSessionDescriptionObserver(this);
	}

	RtcConductor::~RtcConductor()
	{
		DeletePeerConnection();
		RTC_DCHECK(!peerObserver || !peerObserver->peerConnection);
	}

	void RtcConductor::DeletePeerConnection()
	{
		if (peerObserver)
		{
			if (peerObserver->peerConnection)
			{
				peerObserver->peerConnection->Close();
			}
			delete peerObserver;
		}

		pc_factory_ = nullptr;
		delete default_socket_factory_.get();
		delete default_network_manager_.get();

		if (!dataObservers.empty())
		{
			// loop over all active data channel observers and close them
			for (auto itr = dataObservers.begin(); itr != dataObservers.end(); )
			{
				const auto copy_itr = itr;
				++itr;
				FinalizeDataChannelClose(copy_itr->first, copy_itr->second);
			}
			dataObservers.clear();
		}
		serverConfigs.clear();

		RTC_DCHECK(network_thread_ && worker_thread_ && signaling_thread_);
		network_thread_->Quit();
		worker_thread_->Quit();
		signaling_thread_->Quit();

		delete network_thread_.get();
		delete worker_thread_.get();;
		delete signaling_thread_.get();;


		delete sessionObserver;
		delete setSessionObserver;
		rtc::Thread* current_thread = rtc::ThreadManager::Instance()->CurrentThread();
		if (current_thread)
			current_thread->Quit();


	}

	void RtcConductor::FinalizeDataChannelClose(const std::string& label, Observers::DataChannelObserver* observer)
	{
		if (observer->dataChannel != nullptr)
		{
			// sends the close notification to the remote peer
			observer->dataChannel->Close();
			// unregisters the the observer which needs to be done before disposing 
			observer->dataChannel->UnregisterObserver();
			delete observer;
		}
		dataObservers.erase(label);

	}

	bool RtcConductor::InitializePeerConnection(uint16_t min_port, uint16_t max_port)
	{
		rtc::ThreadManager::Instance()->WrapCurrentThread();
		RTC_DCHECK(!pc_factory_);
		RTC_DCHECK(peerObserver && !peerObserver->peerConnection);

		RTC_DCHECK(!network_thread_ && !worker_thread_ && !signaling_thread_);

		network_thread_ = rtc::Thread::CreateWithSocketServer();
		network_thread_->SetName("network_thread", nullptr);
		RTC_CHECK(network_thread_->Start()) << "Failed to start network thread";

		worker_thread_ = rtc::Thread::Create();
		worker_thread_->SetName("worker_thread", nullptr);
		RTC_CHECK(worker_thread_->Start()) << "Failed to start worker thread";

		signaling_thread_ = rtc::Thread::Create();
		signaling_thread_->SetName("signaling_thread", nullptr);
		RTC_CHECK(signaling_thread_->Start()) << "Failed to start signaling thread";



		webrtc::PeerConnectionFactoryDependencies factory_deps;
		factory_deps.network_thread = network_thread_.get();
		factory_deps.worker_thread = worker_thread_.get();
		factory_deps.signaling_thread = signaling_thread_.get();

		pc_factory_ = CreateModularPeerConnectionFactory(std::move(factory_deps));
		if (pc_factory_)
		{
			default_network_manager_.reset(new rtc::BasicNetworkManager());
			if (default_network_manager_)
			{
				default_socket_factory_.reset(new rtc::BasicPacketSocketFactory(network_thread_.get()));
				if (default_socket_factory_)
				{
					default_relay_port_factory_.reset(new cricket::TurnPortFactory());
					if (default_relay_port_factory_)
					{
						webrtc::PeerConnectionFactoryInterface::Options opt;
						pc_factory_->SetOptions(opt);
						if (CreatePeerConnection(min_port, max_port))
						{
							RTC_DCHECK(peerObserver->peerConnection);
							if (peerObserver->peerConnection)
							{
								RTC_LOG(INFO) << "Peer connection created completed";
								return true;
							}
						}
					}
				}
			}
		}
		DeletePeerConnection();
		RTC_LOG(LS_ERROR) << "Unable to create peer connection";
		return false;
	}

	bool RtcConductor::CreatePeerConnection(uint16_t minPort, uint16_t maxPort)
	{
		RTC_DCHECK(pc_factory_);
		RTC_DCHECK(peerObserver && !peerObserver->peerConnection);

		webrtc::PeerConnectionInterface::RTCConfiguration config;

		config.tcp_candidate_policy = webrtc::PeerConnectionInterface::kTcpCandidatePolicyDisabled;
		//no clue why this was set to true
		config.disable_ipv6 = false;
		config.disable_ipv6_on_wifi = false;
		config.candidate_network_policy = webrtc::PeerConnectionInterface::kCandidateNetworkPolicyLowCost;
		config.network_preference = absl::optional<rtc::AdapterType>(rtc::AdapterType::ADAPTER_TYPE_ETHERNET);

		config.rtcp_mux_policy = webrtc::PeerConnectionInterface::kRtcpMuxPolicyRequire;

		for each (auto server in serverConfigs)
		{
			config.servers.push_back(server);
		}

		std::unique_ptr<cricket::PortAllocator> allocator = std::make_unique<cricket::BasicPortAllocator>(
			default_network_manager_.get(),
			default_socket_factory_.get(),
			config.turn_customizer,
			default_relay_port_factory_.get());

		allocator->set_flags(allocator->flags() | cricket::PORTALLOCATOR_DISABLE_TCP);
		allocator->set_allow_tcp_listen(false);
		allocator->SetPortRange(minPort, maxPort);
		peerObserver->peerConnection = pc_factory_->CreatePeerConnection(config, std::move(allocator), nullptr, peerObserver);
		return peerObserver->peerConnection != nullptr;
	}



	void RtcConductor::AddServerConfig(std::string uri, std::string username, std::string password)
	{
		webrtc::PeerConnectionInterface::IceServer server;
		server.uri = uri;
		server.username = username;
		server.password = password;
		serverConfigs.push_back(server);
		RTC_LOG(INFO) << "Added Ice Server " << uri;
	}

	void RtcConductor::CreateOffer()
	{
		if (!peerObserver->peerConnection)
			return;

		webrtc::PeerConnectionInterface::RTCOfferAnswerOptions options;
		options.offer_to_receive_audio = false;
		options.offer_to_receive_video = false;
		peerObserver->peerConnection->CreateOffer(sessionObserver, options);
		RTC_LOG(INFO) << "Created an offer";
	}

	void RtcConductor::OnOfferReply(std::string type, std::string sdp)
	{
		if (!peerObserver->peerConnection)
			return;

		webrtc::SdpParseError error;
		webrtc::SessionDescriptionInterface* session_description(CreateSessionDescription(type, sdp, &error));
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
		webrtc::SessionDescriptionInterface* session_description(CreateSessionDescription("offer", sdp, &error));
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
			o.offer_to_receive_video = false;
		}
		peerObserver->peerConnection->CreateAnswer(sessionObserver, o);
	}

	bool RtcConductor::AddIceCandidate(std::string sdp_mid, int32_t sdp_mlineindex, std::string sdp)
	{
		webrtc::SdpParseError error;
		webrtc::IceCandidateInterface* candidate = CreateIceCandidate(sdp_mid, sdp_mlineindex, sdp, &error);
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

	void RtcConductor::CreateDataChannel(const std::string& label, const webrtc::DataChannelInit dc_options)
	{
		if (!peerObserver->peerConnection)
			return;

		if (dataObservers.find(label) == dataObservers.end()) {
			dataObservers[label] = new Observers::DataChannelObserver(this);
			dataObservers[label]->dataChannel = peerObserver->peerConnection->CreateDataChannel(label, &dc_options);
			dataObservers[label]->dataChannel->RegisterObserver(dataObservers[label]);
			RTC_LOG(INFO) << "Created data channel " << label;
		}
	}

	void RtcConductor::DataChannelSendText(const std::string& label, const std::string& text)
	{
		const auto observer = dataObservers.find(label);
		if (observer != dataObservers.end()) {
			observer->second->dataChannel->Send(webrtc::DataBuffer(text));
		}
	}

	RtcDataChannelInfo RtcConductor::GetDataChannelInfo(const std::string& label)
	{
		auto info = RtcDataChannelInfo();

		const auto observer = dataObservers.find(label);

		if (observer != dataObservers.end()) {

			const auto data_channel = observer->second->dataChannel;

			info.id = data_channel->id();
			info.currentBuffer = data_channel->buffered_amount();
			info.bytesSent = data_channel->bytes_sent();
			info.bytesReceived = data_channel->bytes_received();

			info.reliable = data_channel->reliable();
			info.ordered = data_channel->ordered();
			info.negotiated = data_channel->negotiated();

			info.messagesSent = data_channel->messages_sent();
			info.messagesReceived = data_channel->messages_received();

			info.maxRetransmits = data_channel->maxRetransmits();
			info.maxRetransmitTime = data_channel->maxRetransmitTime();

			info.protocol = data_channel->protocol();
			info.state = data_channel->state();

			return info;
		}
		info.protocol = "unknown";
		return info;
	}

	webrtc::DataChannelInterface::DataState RtcConductor::GetDataChannelState(const std::string& label)
	{
		const auto observer = dataObservers.find(label);
		if (observer != dataObservers.end()) {
			return observer->second->dataChannel->state();
		}
		return {};
	}


	void RtcConductor::DataChannelSendData(const std::string& label, uint8_t* data, const uint32_t length)
	{
		const auto observer = dataObservers.find(label);
		if (observer != dataObservers.end()) {
			const rtc::CopyOnWriteBuffer write_buffer(data, length);
			observer->second->dataChannel->Send(webrtc::DataBuffer(write_buffer, true));
		}
	}

	void RtcConductor::CloseDataChannel(const std::string& label)
	{
		const auto observer = dataObservers.find(label);
		if (observer != dataObservers.end()) {
			FinalizeDataChannelClose(observer->first, observer->second);
			RTC_LOG(INFO) << "Closed data channel " << label;
		}
	}

}