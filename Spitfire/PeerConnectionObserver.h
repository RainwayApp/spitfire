#pragma once

#include "api/media_stream_interface.h"
#include "api/peer_connection_interface.h"

namespace Spitfire 
{
	class RtcConductor;

	namespace Observers
	{
		class PeerConnectionObserver : public webrtc::PeerConnectionObserver
		{
		public:
			explicit PeerConnectionObserver(RtcConductor* conductor) :
				conductor_(conductor)
			{
			}
			~PeerConnectionObserver()
			{
				RTC_LOG(INFO) << __FUNCTION__ << " ";
			}

			// Triggered when the SignalingState changed.
			void OnSignalingChange(webrtc::PeerConnectionInterface::SignalingState new_state) override;

			// Triggered when media is received on a new stream from remote peer.
			void OnAddStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) override
			{
				RTC_LOG(INFO) << __FUNCTION__ << " ";
			}
			void OnRemoveStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) override
			{
				RTC_LOG(INFO) << __FUNCTION__ << " ";
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
			RtcConductor* conductor_;
		};
	}
}