#pragma once

#include <api/peer_connection_interface.h>
#include <p2p/client/relay_port_factory_interface.h>
#include <p2p/base/basic_packet_socket_factory.h>
#include <rtc_base/logging.h>
#include <rtc_base/log_sinks.h>

#include "DataChannelObserver.h"
#include "PeerConnectionObserver.h"
#include "CreateSessionDescriptionObserver.h"
#include "SetSessionDescriptionObserver.h"

namespace Spitfire
{
	struct ProcessingThread // Obsolete
	{
		std::unique_ptr<rtc::Thread> thread;
		rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> factory;
	};

	struct RtcDataChannelInfo 
	{
		uint64_t currentBuffer;
		uint64_t bytesSent;
		uint64_t bytesReceived;

		bool reliable;
		bool ordered;
		bool negotiated;
		int32_t id;

		uint32_t messagesSent;
		uint32_t messagesReceived;
		int16_t maxRetransmits;
		int16_t maxRetransmitTime;

		std::string protocol;
		
		webrtc::DataChannelInterface::DataState state;
	};

	typedef void (__stdcall *OnSuccessCallbackNative)(const char* type, const char* sdp);
	typedef void (__stdcall *OnFailureCallbackNative)(const char* error);
	typedef void (__stdcall *OnIceCandidateCallbackNative)(const char* sdpMid, int32_t sdpIndex, const char* sdp);
	typedef void (__stdcall *OnMessageCallbackNative)(const char* label, const uint8_t* msg, uint32_t size, bool is_binary);
	typedef void (__stdcall *OnIceStateChangeCallbackNative)(webrtc::PeerConnectionInterface::IceConnectionState state);
	typedef void (__stdcall *OnIceGatheringStateCallbackNative)(webrtc::PeerConnectionInterface::IceGatheringState state);
	typedef void (__stdcall *OnDataChannelStateCallbackNative)(const char* label, webrtc::DataChannelInterface::DataState state);
	typedef void (__stdcall *OnBufferAmountCallbackNative)(const char* label, uint64_t previousAmount, uint64_t currentAmount, uint64_t bytesSent, uint64_t bytesReceived);

	class RtcConductor
	{
	public:
		RtcConductor();
		~RtcConductor();

		bool InitializePeerConnection(uint16_t min_port, uint16_t max_port);
		void DeletePeerConnection();
		void CreateOffer();
		void OnOfferReply(const std::string& type, const std::string& sdp);
		void OnOfferRequest(const std::string& sdp);
		bool AddIceCandidate(const std::string& sdp_mid, int32_t sdp_mlineindex, const std::string& sdp);
		bool ProcessMessages(int32_t delay)
		{
			return rtc::ThreadManager::Instance()->WrapCurrentThread()->ProcessMessages(delay);
		}
		void AddServerConfig(const std::string& uri, const std::string& username, const std::string& password);

		void CreateDataChannel(const std::string& label, webrtc::DataChannelInit config);
		void CloseDataChannel(const std::string& label);
		absl::optional<RtcDataChannelInfo> GetDataChannelInfo(const std::string& label);
		absl::optional<webrtc::DataChannelInterface::DataState> GetDataChannelState(const std::string& label);
		void SendToDataChannel(const std::string& label, const webrtc::DataBuffer& buffer);

		void HandleDataChannel(rtc::scoped_refptr<webrtc::DataChannelInterface> channel);
		void HandleIceCandidate(const webrtc::IceCandidateInterface* candidate);

		OnSuccessCallbackNative on_success_ { nullptr };
		OnFailureCallbackNative on_failure_ { nullptr };
		OnMessageCallbackNative on_message_ { nullptr };
		OnIceStateChangeCallbackNative on_ice_state_change_ { nullptr };
		OnIceGatheringStateCallbackNative on_ice_gathering_state_change_ { nullptr };
		OnIceCandidateCallbackNative on_ice_candidate_ { nullptr };
		OnDataChannelStateCallbackNative on_data_channel_state_change_ { nullptr };
		OnBufferAmountCallbackNative on_buffer_amount_change_ { nullptr };

		std::unique_ptr<Observers::PeerConnectionObserver> peer_observer_;
		rtc::scoped_refptr<Observers::CreateSessionDescriptionObserver> session_observer_;
		rtc::scoped_refptr<Observers::SetSessionDescriptionObserver> set_session_observer_;

	private:
		bool CreatePeerConnection(uint16_t minPort, uint16_t maxPort);

		std::unordered_map<std::string, std::unique_ptr<Observers::DataChannelObserver>> data_observers_;

		std::unique_ptr<rtc::Thread> worker_thread_;
		std::unique_ptr<rtc::Thread> signaling_thread_;
		std::unique_ptr<rtc::Thread> network_thread_;
		std::unique_ptr<rtc::BasicNetworkManager> network_manager_;
		std::unique_ptr<rtc::BasicPacketSocketFactory> socket_factory_;

		rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> pc_factory_;
		webrtc::PeerConnectionInterface::IceServers servers_;
		std::unique_ptr<cricket::RelayPortFactoryInterface> relay_port_factory_;
	};
}
