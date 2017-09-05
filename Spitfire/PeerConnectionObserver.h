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
			virtual void OnSignalingChange(webrtc::PeerConnectionInterface::SignalingState new_state);

			virtual void OnStateChange(StateType state_changed) { /* Obsolete. Ignore. */ }

			// Triggered when media is received on a new stream from remote peer.
			virtual void OnAddStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream)
			{
				LOG(INFO) << __FUNCTION__ << " ";
			}

			virtual void OnRemoveStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream)
			{
				LOG(INFO) << __FUNCTION__ << " ";
			}


			// Triggered when a remote peer open a data channel.
			virtual void OnDataChannel(rtc::scoped_refptr<webrtc::DataChannelInterface> channel);

			// Triggered when renegotiation is needed, for example the ICE has restarted.
			virtual void OnRenegotiationNeeded();

			// Called any time the IceConnectionState changes
			virtual void OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState new_state);

			// Called any time the IceGatheringState changes
			virtual void OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState new_state);

			// New Ice candidate have been found.
			virtual void OnIceCandidate(const webrtc::IceCandidateInterface* candidate);

			virtual void OnIceComplete() { /* Obsolete. Ignore. */ }

			// Called when the ICE connection receiving status changes.
			virtual void OnIceConnectionReceivingChange(bool receiving) { /* Not Implemented */ };

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