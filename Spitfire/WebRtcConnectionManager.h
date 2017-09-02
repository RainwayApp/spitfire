#pragma once
#include "Rtc.h"
#include "webrtc/api/mediastreaminterface.h"
#include "webrtc/api/peerconnectioninterface.h"
#include "webrtc/rtc_base/thread.h"
#include "webrtc/p2p/base/basicpacketsocketfactory.h"
#include "webrtc/api/peerconnectioninterface.h"
#include "webrtc/api/test/fakeconstraints.h"
#include "webrtc/media/engine/webrtcvideocapturerfactory.h"

namespace Spitfire
{
	typedef void(__stdcall *OnErrorCallbackNative)();
	typedef void(__stdcall *OnSuccessCallbackNative)(const char * type, const char * sdp);
	typedef void(__stdcall *OnFailureCallbackNative)(const char * error);
	typedef void(__stdcall *OnIceCandidateCallbackNative)(const char * sdp_mid, int sdp_mline_index, const char * sdp);
	typedef void(__stdcall *OnDataMessageCallbackNative)(const char * msg);
	typedef void(__stdcall *OnDataBinaryMessageCallbackNative)(const uint8_t * msg, uint32_t size);

	class WebRtcConnectionManager : public webrtc::PeerConnectionObserver,
		public webrtc::CreateSessionDescriptionObserver,
		public webrtc::SetSessionDescriptionObserver,
		public webrtc::DataChannelObserver {

	public:
		WebRtcConnectionManager();
		~WebRtcConnectionManager();

		bool InitializePeerConnection();
		void CreateOffer();
		void OnOfferReply(const std::string & type, const std::string & sdp);
		void OnOfferRequest(const std::string & sdp);
		bool AddIceCandidate(const std::string & sdp_mid, int sdp_mlineindex, const std::string & sdp);
		void AddServerConfig(const std::string & uri, const std::string & username, const std::string & password);
		void CreateDataChannel(const std::string & label);
		void DataChannelSendText(const std::string & text);
		void DataChannelSendData(const webrtc::DataBuffer & data);

		OnIceCandidateCallbackNative onIceCandidate;
		OnErrorCallbackNative onError;
		OnSuccessCallbackNative onSuccess;
		OnFailureCallbackNative onFailure;
		OnDataMessageCallbackNative onDataMessage;
		OnDataBinaryMessageCallbackNative onDataBinaryMessage;

		virtual void webrtc::SetSessionDescriptionObserver::OnSuccess()
		{
			LOG(INFO) << __FUNCTION__;
		}

		virtual void OnAddStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) {
			LOG(INFO) << __FUNCTION__ << " ";
		}
		virtual void OnRemoveStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) {
			LOG(INFO) << __FUNCTION__ << " ";
		}

		virtual void webrtc::CreateSessionDescriptionObserver::OnSuccess(webrtc::SessionDescriptionInterface* desc);
		virtual void OnFailure(const std::string& error);
		virtual void OnError();
		virtual void OnIceCandidate(const webrtc::IceCandidateInterface* candidate);
		virtual void OnIceChange();
		virtual void OnSignalingChange(webrtc::PeerConnectionInterface::SignalingState state);
		virtual void OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState state);
		virtual void OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState state);
		virtual void OnStateChange(webrtc::PeerConnectionObserver::StateType state_changed);
		virtual void OnRenegotiationNeeded();

		virtual void OnDataChannel(rtc::scoped_refptr<webrtc::DataChannelInterface> channel);
		virtual void OnStateChange();
		virtual void OnMessage(const webrtc::DataBuffer& buffer);
		virtual void OnBufferedAmountChange(uint64_t previous_amount);

		//bool ProcessMessages(int delay)
		//{
		//	return rtc::Thread::Current()->ProcessMessages(delay);
		//}

		int AddRef() const
		{
			return 0;
		};
		int Release() const
		{
			return 0;
		};

		private:

		bool CreatePeerConnection(bool dtlsEnabled);
		void DeletePeerConnection();
		rtc::scoped_refptr<webrtc::PeerConnectionInterface> _peerConnection;
		rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> _pcFactory;
		rtc::scoped_refptr<webrtc::DataChannelInterface> _dataChannel;
		std::vector<webrtc::PeerConnectionInterface::IceServer> _serverConfigs;
	};
}