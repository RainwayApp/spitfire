
#ifndef WEBRTC_NET_CONDUCTOR_H_
#define WEBRTC_NET_CONDUCTOR_H_
#pragma once
#include "DataChannelObserver.h"
#include "PeerConnectionObserver.h"
#include "CreateSessionDescriptionObserver.h"
#include "SetSessionDescriptionObserver.h"
#include "webrtc/api/peerconnectioninterface.h"

namespace Spitfire
{

	struct ProcessingThread {
		std::unique_ptr<rtc::Thread> thread;
		rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> factory;
	};

	typedef void(__stdcall *OnErrorCallbackNative)();
	typedef void(__stdcall *OnSuccessCallbackNative)(const char * type, const char * sdp);
	typedef void(__stdcall *OnFailureCallbackNative)(const char * error);
	typedef void(__stdcall *OnIceCandidateCallbackNative)(const char * sdp_mid, int sdp_mline_index, const char * sdp);
	typedef void(__stdcall *OnRenderCallbackNative)(uint8_t * frame_buffer, uint32_t w, uint32_t h);
	typedef void(__stdcall *OnDataMessageCallbackNative)(const char * msg);
	typedef void(__stdcall *OnDataBinaryMessageCallbackNative)(const uint8_t * msg, uint32_t size);
	typedef void(__stdcall *OnIceStateChangeCallbackNative)(webrtc::PeerConnectionInterface::IceConnectionState state);
	typedef void(__stdcall *OnDataChannelStateCallbackNative)(webrtc::DataChannelInterface::DataState state);


	class RtcConductor 
	{
	public:

		RtcConductor();
		~RtcConductor();

		bool InitializePeerConnection();
		void CreateOffer();
		void OnOfferReply(std::string type, std::string sdp);
		void OnOfferRequest(std::string sdp);
		bool AddIceCandidate(std::string sdp_mid, int sdp_mlineindex, std::string sdp);

		bool ProcessMessages(int delay)
		{
			return rtc::Thread::Current()->ProcessMessages(delay);
		}

		void AddServerConfig(std::string uri, std::string username, std::string password);


		void CreateDataChannel(const std::string & label);
		void DataChannelSendText(const std::string & text);
		void DataChannelSendData(const webrtc::DataBuffer & data);

		OnErrorCallbackNative onError;
		OnSuccessCallbackNative onSuccess;
		OnFailureCallbackNative onFailure;
		OnIceStateChangeCallbackNative onIceStateChange;
		OnIceCandidateCallbackNative onIceCandidate;
		OnDataChannelStateCallbackNative onDataChannelState;
		OnDataMessageCallbackNative onDataMessage;
		OnDataBinaryMessageCallbackNative onDataBinaryMessage;

		rtc::scoped_refptr<Observers::DataChannelObserver> dataObserver;
		rtc::scoped_refptr<Observers::PeerConnectionObserver> peerObserver;
		rtc::scoped_refptr<Observers::CreateSessionDescriptionObserver> sessionObserver;
		rtc::scoped_refptr<Observers::SetSessionDescriptionObserver> setSessionObserver;
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

		bool CreatePeerConnection(bool dtls);
	

		rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> pc_factory_;
		std::vector<webrtc::PeerConnectionInterface::IceServer> serverConfigs;
	};
}
#endif  // WEBRTC_NET_CONDUCTOR_H_
