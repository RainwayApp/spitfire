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
			~PeerConnectionObserver() = default;

			// webrtc::PeerConnectionObserver
			void OnSignalingChange(webrtc::PeerConnectionInterface::SignalingState new_state) override
			{
				RTC_LOG(INFO) << __FUNCTION__ << " ";
			}
			void OnAddStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) override
			{
				RTC_LOG(INFO) << __FUNCTION__ << " ";
			}
			void OnRemoveStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) override
			{
				RTC_LOG(INFO) << __FUNCTION__ << " ";
			}
			void OnDataChannel(rtc::scoped_refptr<webrtc::DataChannelInterface> channel) override;
			void OnRenegotiationNeeded() override
			{
				RTC_LOG(INFO) << __FUNCTION__ << " ";
			}
			void OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState new_state) override;
			void OnStandardizedIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState new_state) override
			{
				RTC_LOG(INFO) << __FUNCTION__ << " ";
			}
			void OnConnectionChange(webrtc::PeerConnectionInterface::PeerConnectionState new_state) override
			{
				RTC_LOG(INFO) << __FUNCTION__ << " ";
			}
			void OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState new_state) override;
			void OnIceCandidate(const webrtc::IceCandidateInterface* candidate) override;
			void OnIceCandidateError(const std::string& host_candidate, const std::string& url, int error_code, const std::string& error_text) override
			{
				RTC_LOG(INFO) << __FUNCTION__ << " ";
			}
			void OnIceCandidateError(const std::string& address, int port, const std::string& url, int error_code, const std::string& error_text) override
			{
				RTC_LOG(INFO) << __FUNCTION__ << " ";
			}
			void OnIceCandidatesRemoved(const std::vector<cricket::Candidate>& candidates) override
			{
				RTC_LOG(INFO) << __FUNCTION__ << " ";
			}
			void OnIceConnectionReceivingChange(bool receiving) override
			{
				RTC_LOG(INFO) << __FUNCTION__ << " ";
			}
			void OnIceSelectedCandidatePairChanged(const cricket::CandidatePairChangeEvent& event) override
			{
				RTC_LOG(INFO) << __FUNCTION__ << " ";
			}
			void OnAddTrack(rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver, const std::vector<rtc::scoped_refptr<webrtc::MediaStreamInterface>>& streams) override
			{
				RTC_LOG(INFO) << __FUNCTION__ << " ";
			}
			void OnTrack(rtc::scoped_refptr<webrtc::RtpTransceiverInterface> transceiver) override
			{
				RTC_LOG(INFO) << __FUNCTION__ << " ";
			}
			void OnRemoveTrack(rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver) override
			{
				RTC_LOG(INFO) << __FUNCTION__ << " ";
			}
			void OnInterestingUsage(int usage_pattern) override
			{
				RTC_LOG(INFO) << __FUNCTION__ << " ";
			}

			rtc::scoped_refptr<webrtc::PeerConnectionInterface> peer_connection_;

		private:
			RtcConductor* conductor_;
		};
	}
}