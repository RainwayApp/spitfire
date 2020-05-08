#pragma once

#ifndef WEBRTC_NET_CONDUCTOR_H_
#define WEBRTC_NET_CONDUCTOR_H_

#include "DataChannelObserver.h"
#include "PeerConnectionObserver.h"
#include "CreateSessionDescriptionObserver.h"
#include "SetSessionDescriptionObserver.h"
#include "api/peer_connection_interface.h"
#include "p2p/client/relay_port_factory_interface.h"
#include "p2p/base/basic_packet_socket_factory.h"

namespace Spitfire
{
	struct ProcessingThread 
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
		int id;

		uint32_t messagesSent;
		uint32_t messagesReceived;
		int16_t maxRetransmits;
		int16_t maxRetransmitTime;

		std::string protocol;

		webrtc::DataChannelInterface::DataState state;
	};

	typedef void(__stdcall *OnErrorCallbackNative)();
	typedef void(__stdcall *OnSuccessCallbackNative)(const char * type, const char * sdp);
	typedef void(__stdcall *OnFailureCallbackNative)(const char * error);
	typedef void(__stdcall *OnIceCandidateCallbackNative)(const char * sdpMid, int sdpIndex, const char * sdp);
	typedef void(__stdcall *OnRenderCallbackNative)(uint8_t * frameBuffer, uint32_t w, uint32_t h);
	typedef void(__stdcall *OnMessageCallbackNative)(const char* label, const uint8_t* msg, uint32_t size, bool is_binary);
	typedef void(__stdcall *OnIceStateChangeCallbackNative)(webrtc::PeerConnectionInterface::IceConnectionState state);
	typedef void(__stdcall* OnIceGatheringStateCallbackNative)(webrtc::PeerConnectionInterface::IceGatheringState state);
	typedef void(__stdcall *OnDataChannelStateCallbackNative)(const char * label, webrtc::DataChannelInterface::DataState state);
	typedef void(__stdcall *OnBufferAmountCallbackNative)(const char * label, uint64_t previousAmount, uint64_t currentAmount, uint64_t bytesSent, uint64_t bytesReceived);

	class RtcConductor
	{
	public:
		RtcConductor();
		~RtcConductor();

		bool InitializePeerConnection(int min_port, int max_port);
		void CreateOffer();
		void OnOfferReply(std::string type, std::string sdp);
		void OnOfferRequest(std::string sdp);
		bool AddIceCandidate(std::string sdp_mid, int sdp_mlineindex, std::string sdp);

		bool ProcessMessages(int delay)
		{
			return rtc::ThreadManager::Instance()->WrapCurrentThread()->ProcessMessages(delay);
		}

		void AddServerConfig(std::string uri, std::string username, std::string password);

		void CreateDataChannel(const std::string & label, webrtc::DataChannelInit dc_options);
		void DataChannelSendText(const std::string & label, const std::string & text);
		RtcDataChannelInfo GetDataChannelInfo(const std::string& label);
		webrtc::DataChannelInterface::DataState GetDataChannelState(const std::string& label);
		void CloseDataChannel(const std::string& label);
		void DataChannelSendData(const std::string& label, unsigned char* data, int length);

		
		OnSuccessCallbackNative onSuccess;
		OnFailureCallbackNative onFailure;
		OnIceStateChangeCallbackNative onIceStateChange{};
		OnIceGatheringStateCallbackNative onIceGatheringStateChange{};
		OnIceCandidateCallbackNative onIceCandidate;
		OnDataChannelStateCallbackNative onDataChannelState;
		OnBufferAmountCallbackNative onBufferAmountChange{};
		OnMessageCallbackNative onMessage;

		//rtc::scoped_refptr<Observers::DataChannelObserver> dataObserver;
		rtc::scoped_refptr<Observers::PeerConnectionObserver> peerObserver;
		rtc::scoped_refptr<Observers::CreateSessionDescriptionObserver> sessionObserver;
		rtc::scoped_refptr<Observers::SetSessionDescriptionObserver> setSessionObserver;

		std::unordered_map<std::string, Observers::DataChannelObserver*> dataObservers;

		
		void DeletePeerConnection();

	protected:
		int AddRef() const
		{
			return 0;
		};
		int Release() const
		{
			return 0;
		};

	private:
		std::unique_ptr<rtc::Thread> worker_thread_;
		std::unique_ptr<rtc::Thread> signaling_thread_;
		std::unique_ptr<rtc::Thread> network_thread_;
		std::unique_ptr<rtc::BasicNetworkManager> default_network_manager_;
		std::unique_ptr<rtc::BasicPacketSocketFactory> default_socket_factory_;

		bool CreatePeerConnection(int minPort, int maxPort);
		void FinalizeDataChannelClose(const std::string& label, Observers::DataChannelObserver* observer);

		rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> pc_factory_;
		std::vector<webrtc::PeerConnectionInterface::IceServer> serverConfigs;
		std::unique_ptr<cricket::RelayPortFactoryInterface> default_relay_port_factory_;
	};
}
#endif  // WEBRTC_NET_CONDUCTOR_H_