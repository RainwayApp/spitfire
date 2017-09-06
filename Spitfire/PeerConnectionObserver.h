#pragma once

#include "webrtc/api/mediastreaminterface.h"
#include "webrtc/api/peerconnectioninterface.h"

namespace Spitfire {

	class RtcConductor;

	namespace Observers
	{
		class PeerConnectionObserver : public webrtc::PeerConnectionObserver
		{
		public:

			// Triggered when the SignalingState changed.
			void OnSignalingChange(webrtc::PeerConnectionInterface::SignalingState new_state) override;

			virtual void OnStateChange(StateType state_changed) { /* Obsolete. Ignore. */ }

			// Triggered when media is received on a new stream from remote peer.
			void OnAddStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) override
			{
				LOG(INFO) << __FUNCTION__ << " ";
			}

			void OnRemoveStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) override
			{
				LOG(INFO) << __FUNCTION__ << " ";
			}


			// Triggered when a remote peer open a data channel.
			void OnDataChannel(rtc::scoped_refptr<webrtc::DataChannelInterface> channel) override;

			// Triggered when renegotiation is needed, for example the ICE has restarted.
			void OnRenegotiationNeeded() override;

			// Called any time the IceConnectionState changes
			void OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState new_state) override;

			// Called any time the IceGatheringState changes
			void OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState new_state) override;

			// New Ice candidate have been found.
			void OnIceCandidate(const webrtc::IceCandidateInterface* candidate) override;

			virtual void OnIceComplete() { /* Obsolete. Ignore. */ }

			// Called when the ICE connection receiving status changes.
			void OnIceConnectionReceivingChange(bool receiving) override { /* Not Implemented */ };

			explicit PeerConnectionObserver(RtcConductor* manager);
			~PeerConnectionObserver();

			rtc::scoped_refptr<webrtc::PeerConnectionInterface> peerConnection;

			int AddRef() const
			{
				return 0;
			};
			int Release() const
			{
				return 0;
			};

		private:
			RtcConductor * _manager;
		};
	}
}