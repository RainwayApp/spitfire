#pragma once

#include <api/media_stream_interface.h>
#include <api/peer_connection_interface.h>

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
				RTC_DCHECK(conductor);
			}
			~PeerConnectionObserver() = default;

			// webrtc::PeerConnectionObserver
			void OnSignalingChange(webrtc::PeerConnectionInterface::SignalingState new_state) override
			{
				RTC_DLOG_F(LS_INFO);
			}
			void OnAddStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) override
			{
				RTC_DLOG_F(LS_INFO);
			}
			void OnRemoveStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) override
			{
				RTC_DLOG_F(LS_INFO);
			}
			void OnDataChannel(rtc::scoped_refptr<webrtc::DataChannelInterface> channel) override;
			void OnRenegotiationNeeded() override
			{
				RTC_DLOG_F(LS_INFO);
			}
			void OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState new_state) override;
			void OnStandardizedIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState new_state) override
			{
				RTC_DLOG_F(LS_INFO);
			}
			void OnConnectionChange(webrtc::PeerConnectionInterface::PeerConnectionState new_state) override
			{
				RTC_DLOG_F(LS_INFO);
			}
			void OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState new_state) override;
			void OnIceCandidate(const webrtc::IceCandidateInterface* candidate) override;
			void OnIceCandidateError(const std::string& host_candidate, const std::string& url, int error_code, const std::string& error_text) override
			{
				RTC_DLOG_F(LS_INFO);
			}
			void OnIceCandidateError(const std::string& address, int port, const std::string& url, int error_code, const std::string& error_text) override
			{
				RTC_DLOG_F(LS_INFO);
			}
			void OnIceCandidatesRemoved(const std::vector<cricket::Candidate>& candidates) override
			{
				RTC_DLOG_F(LS_INFO);
			}
			void OnIceConnectionReceivingChange(bool receiving) override
			{
				RTC_DLOG_F(LS_INFO);
			}
			void OnIceSelectedCandidatePairChanged(const cricket::CandidatePairChangeEvent& event) override
			{
				RTC_DLOG_F(LS_INFO);
			}
			void OnAddTrack(rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver, const std::vector<rtc::scoped_refptr<webrtc::MediaStreamInterface>>& streams) override
			{
				RTC_DLOG_F(LS_INFO);
			}
			void OnTrack(rtc::scoped_refptr<webrtc::RtpTransceiverInterface> transceiver) override
			{
				RTC_DLOG_F(LS_INFO);
			}
			void OnRemoveTrack(rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver) override
			{
				RTC_DLOG_F(LS_INFO);
			}
			void OnInterestingUsage(int usage_pattern) override
			{
				RTC_DLOG_F(LS_INFO);
			}

			rtc::scoped_refptr<webrtc::PeerConnectionInterface> peer_connection_;

		private:
			RtcConductor* conductor_;
		};
	}
}