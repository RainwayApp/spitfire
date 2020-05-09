#pragma unmanaged

#include "rtc_base\ssl_adapter.h"
#include "rtc_base\win32_socket_init.h"
#include "rtc_base\win32_socket_server.h"
#include "rtc_base\logging.h"
#include "rtc_base\time_utils.h"
#include "rtc_base\helpers.h"

#include "RtcConductor.h"

FILE _iob[] { *stdin, *stdout, *stderr };

extern "C" FILE * __cdecl __iob_func()
{
	return _iob;
}

#pragma managed

#include "msclr/marshal_cppstd.h"

using namespace System::Runtime::InteropServices;
using namespace System::Reflection;
using namespace System;
using namespace System::IO;
using namespace System::Diagnostics;
using namespace msclr::interop;

namespace Spitfire
{

	

	/// <summary>
	/// <seealso href="http://w3c.github.io/webrtc-pc/#idl-def-RTCDataChannelState"/>
	/// </summary>
	public enum class DataChannelState
	{
		/// <summary>
		/// Attempting to establish the underlying data transport. 
		/// This is the initial state of a RTCDataChannel object created with createDataChannel().
		/// </summary>
		Connecting = 0,

		/// <summary>
		/// The underlying data transport is established and communication is possible. 
		/// This is the initial state of a RTCDataChannel object dispatched as a part of a RTCDataChannelEvent.
		/// </summary>
		Open = 1,

		/// <summary>
		/// The procedure to close down the underlying data transport has started.
		/// </summary>
		Closing = 2,

		/// <summary>
		/// The underlying data transport has been closed or could not be established.
		/// </summary>
		Closed = 3
	};

	/// <summary>
	/// WebRTC logging verbosity
	/// </summary>
	public enum class RtcLogVerbosity
	{
		Verbose = 0,
		Info = 1,
		Warning = 2,
		Error = 3,
		None = 4
	};

	/// <summary>
	/// <seealso href="http://www.w3.org/TR/webrtc/#rtciceconnectionstate-enum"/>
	/// </summary>
	public enum class IceConnectionState
	{
		/// <summary>
		/// The ICE Agent is gathering addresses and / or waiting 
		/// for remote candidates to be supplied.
		/// </summary>
		New = 0,

		/// <summary>
		/// The ICE Agent has received remote candidates on at least one component, 
		/// and is checking candidate pairs but has not yet found a connection.
		/// In addition to checking, it may also still be gathering.
		/// </summary>
		Checking = 1,

		/// <summary>
		/// The ICE Agent has found a usable connection for all components but is 
		/// still checking other candidate pairs to see if there is a better 
		/// connection.It may also still be gathering.
		/// </summary>
		Connected = 2,

		/// <summary>
		/// The ICE Agent has finished gathering and checking and found a connection for all components.
		/// </summary>
		Completed = 3,

		/// <summary>
		/// The ICE Agent is finished checking all candidate pairs and failed to find a connection for at least one component.
		/// </summary>
		Failed = 4,

		/// <summary>
		/// Liveness checks have failed for one or more components.
		/// This is more aggressive than failed, and may trigger 
		/// intermittently(and resolve itself without action) on a flaky network.
		/// </summary>
		Disconnected = 5,

		/// <summary>
		/// The ICE Agent has shut down and is no longer responding to STUN requests.
		/// </summary>
		Closed = 6,
		// TODO: description
		ConnectionMax = 7
	};

	/// <summary>
	/// <seealso href="https://developer.mozilla.org/en-US/docs/Web/API/RTCPeerConnection/iceGatheringState"/>
	/// </summary>
	public enum class IceGatheringState
	{
		/// <summary>
		/// The peer connection was just created and hasn't done any networking yet.
		/// </summary>
		New = 0,

		/// <summary>
		/// The ICE agent is in the process of gathering candidates for the connection.
		/// </summary>
		Gathering = 1,

		/// <summary>
		/// The ICE agent has finished gathering candidates. 
		/// If something happens that requires collecting new candidates, such as a new interface being added or the addition of a new ICE server, 
		/// the state will revert to "gathering" to gather those candidates.
		/// </summary>
		Complete = 2
	};

	public enum class SdpTypes
	{
		Answer,
		Offer
	};

	public enum class ServerType
	{
		Stun,
		Turn
	};

	public ref class ServerConfig
	{
	public:
		ServerType Type;
		String^ Host;
		unsigned short Port;
		String^ Username;
		String^ Password;
	};


	public ref class DataChannelInfo
	{
	public:
		uint64_t CurrentBuffer;
		uint64_t BytesSent;
		uint64_t BytesReceived;

		bool Reliable;
		bool Ordered;
		bool Negotiated;

		uint32_t MessagesSent;
		uint32_t MessagesReceived;

		int32_t MaxRetransmits;
		int32_t MaxRetransmitTime;

		String^ Protocol;

		DataChannelState^ State;
	};

	public ref class SpitfireSdp
	{
	public:
		String^ Sdp;
		SdpTypes Type;
	};

	public ref class SpitfireFailure
	{
	public:
		String^ Error;
	};

	public ref class DataChannelOptions
	{
	public:
		 /// <summary>
		 /// The name of your data channel
		 /// </summary>
		String^ Label;

		 /// <summary>
		 /// Deprecated. Reliability is assumed, and channel will be unreliable if
		 /// MaxRetransmitTime or MaxRetransmits is set.
		 /// </summary>
		bool Reliable = false;

		 /// <summary>
		 /// True if ordered delivery is required.
		 /// </summary>
		bool Ordered = true;

		 /// <summary>
		 /// The max period of time in milliseconds in which retransmissions will be
		 /// sent. After this time, no more retransmissions will be sent. -1 if unset.
		 /// Cannot be set along with |MaxRetransmits|.
		 /// </summary>
		Nullable<int32_t> MaxRetransmitTime;

		/* 
		 * The max number of retransmissions. -1 if unset.
		 * Cannot be set along with |MaxRetransmitTime|.
		 */
		 /// <summary>
		 /// The max number of retransmissions. -1 if unset.
		 /// Cannot be set along with |MaxRetransmitTime|.
		 /// </summary>
		Nullable<int32_t> MaxRetransmits;
		 /// <summary>
		 /// This is set by the application and opaque to the WebRTC implementation.
		 /// </summary>
		String^ Protocol;
		 /// <summary>
		 /// True if the channel has been externally negotiated and we do not send an
		 /// in-band signalling in the form of an "open" message. If this is true, Id
		 /// must be set; otherwise it should be unset and will be negotiated
		 /// </summary>
		bool Negotiated = false;

		 /// <summary>
		 ///  The stream id, or SID, for SCTP data channels. -1 if unset (see Negotiated).
		 /// </summary>
		int32_t Id = -1;
	};

	public ref class SpitfireIceCandidate
	{
	public:
		String^ SdpMid;
		int32_t SdpIndex;
		String^ Sdp;
	};

	public ref class SpitfireRtc
	{
	private:
		std::unique_ptr<Spitfire::RtcConductor>* conductor_;

		bool disposed_;
		uint16_t min_port_;
		uint16_t max_port_;

		delegate void _OnSuccessCallback(String^ type, String^ sdp);
		_OnSuccessCallback^ onSuccess;
		GCHandle^ on_success_handle_;

		delegate void _OnFailureCallback(String^ error);
		_OnFailureCallback^ onFailure;
		GCHandle^ on_failure_handle_;
		
		delegate void _OnMessageCallback(String^ label, uint8_t* msg, uint32_t size, bool is_binary);
		_OnMessageCallback^ onMessage;
		GCHandle^ on_message_handle_;

		delegate void _OnIceCandidateCallback(String^ sdp_mid, int32_t sdp_mline_index, String^ sdp);
		_OnIceCandidateCallback^ onIceCandidate;
		GCHandle^ on_ice_candidate_handle_;

		delegate void _OnDataChannelStateCallback(String^ label, webrtc::DataChannelInterface::DataState state);
		_OnDataChannelStateCallback^ onDataChannelStateChange;
		GCHandle^ on_data_channel_state_handle_;

		delegate void _OnBufferChangeCallback(String^ label, uint64_t previousAmount, uint64_t currentAmount, uint64_t bytesSent, uint64_t bytesReceived);
		_OnBufferChangeCallback^ onBufferAmountChange;
		GCHandle^ on_buffer_amount_change_handle_;

		delegate void _OnIceStateCallback(webrtc::PeerConnectionInterface::IceConnectionState state);
		_OnIceStateCallback^ onIceStateChange;
		GCHandle^ on_ice_state_callback_handle_;

		delegate void _OnIceGatheringStateCallback(webrtc::PeerConnectionInterface::IceGatheringState state);
		_OnIceGatheringStateCallback^ onIceGatheringStateChange;
		GCHandle^ on_ice_gathering_state_callback_handle_;

		void FreeGCHandle(GCHandle^% g)
		{
			if(g != nullptr)
			{
				g->Free();
			}
		}

		void _OnSuccess(String^ type, String^ sdp)
		{
			auto sdpModel = gcnew SpitfireSdp();
			sdpModel->Sdp = sdp;

			if(type == "offer")
			{
				sdpModel->Type = SdpTypes::Offer;
				OnSuccessOffer(sdpModel);
			}
			else if(type == "answer")
			{
				sdpModel->Type = SdpTypes::Answer;
				OnSuccessAnswer(sdpModel);
			}
		}

		void _OnIceCandidate(String^ sdp_mid, int32_t sdp_mline_index, String^ sdp)
		{
			auto ice = gcnew SpitfireIceCandidate();
			ice->Sdp = gcnew String(sdp);
			ice->SdpMid = gcnew String(sdp_mid);
			ice->SdpIndex = sdp_mline_index;
			OnIceCandidate(ice);
		}

		void _OnFailure(String^ error)
		{
			OnFailure(error);
		}

		
		void _OnIceState(webrtc::PeerConnectionInterface::IceConnectionState state)
		{

			IceConnectionState managedState = static_cast<IceConnectionState>(state);
			OnIceStateChange(managedState);
		}

		void _OnIceGatheringState(webrtc::PeerConnectionInterface::IceGatheringState state)
		{
			IceGatheringState managedState = static_cast<IceGatheringState>(state);
			OnIceGatheringStateChange(managedState);
		}

		void _OnBufferAmountChange(String^ label, const uint64_t previous_amount, const uint64_t current_amount, const uint64_t bytes_sent, const uint64_t bytes_received)
		{
			OnBufferAmountChange(label, previous_amount, current_amount, bytes_sent, bytes_received);
		}

		void _OnDataChannelState(String^ label, webrtc::DataChannelInterface::DataState state)
		{
			DataChannelState managedState = static_cast<DataChannelState>(state);
			OnDataChannelStateChange(label, managedState);
		}

		void _OnMessage(String^ label, uint8_t* data, const uint32_t size, const bool is_binary)
		{
			//auto buffer = gcnew array<Byte>(size);
			//IntPtr src(data);
			//Marshal::Copy(src, buffer, 0, size);
			IntPtr managedPointer(data);
			OnMessage(label, managedPointer, size, is_binary);
		}

		void Initialize(uint16_t min_port, uint16_t max_port)
		{
			disposed_ = false;
			conductor_ = new std::unique_ptr<Spitfire::RtcConductor>(new Spitfire::RtcConductor());
			min_port_ = min_port;
			max_port_ = max_port;

			onSuccess = gcnew _OnSuccessCallback(this, &SpitfireRtc::_OnSuccess);
			on_success_handle_ = GCHandle::Alloc(onSuccess);
			conductor_->get()->onSuccess = static_cast<Spitfire::OnSuccessCallbackNative>(Marshal::GetFunctionPointerForDelegate(onSuccess).ToPointer());

			onFailure = gcnew _OnFailureCallback(this, &SpitfireRtc::_OnFailure);
			on_failure_handle_ = GCHandle::Alloc(onFailure);
			conductor_->get()->onFailure = static_cast<Spitfire::OnFailureCallbackNative>(Marshal::GetFunctionPointerForDelegate(onFailure).ToPointer());

			onMessage = gcnew _OnMessageCallback(this, &SpitfireRtc::_OnMessage);
			on_message_handle_ = GCHandle::Alloc(onMessage);
			conductor_->get()->onMessage = static_cast<Spitfire::OnMessageCallbackNative>(Marshal::GetFunctionPointerForDelegate(onMessage).ToPointer());

			
			onIceCandidate = gcnew _OnIceCandidateCallback(this, &SpitfireRtc::_OnIceCandidate);
			on_ice_candidate_handle_ = GCHandle::Alloc(onIceCandidate);
			conductor_->get()->onIceCandidate = static_cast<Spitfire::OnIceCandidateCallbackNative>(Marshal::GetFunctionPointerForDelegate(onIceCandidate).ToPointer());

			onDataChannelStateChange = gcnew _OnDataChannelStateCallback(this, &SpitfireRtc::_OnDataChannelState);
			on_data_channel_state_handle_ = GCHandle::Alloc(onDataChannelStateChange);
			conductor_->get()->onDataChannelState = static_cast<Spitfire::OnDataChannelStateCallbackNative>(Marshal::GetFunctionPointerForDelegate(onDataChannelStateChange).ToPointer());

			onIceStateChange = gcnew _OnIceStateCallback(this, &SpitfireRtc::_OnIceState);
			on_ice_state_callback_handle_ = GCHandle::Alloc(onIceStateChange);
			conductor_->get()->onIceStateChange = static_cast<Spitfire::OnIceStateChangeCallbackNative>(Marshal::GetFunctionPointerForDelegate(onIceStateChange).ToPointer());

			onIceGatheringStateChange = gcnew _OnIceGatheringStateCallback(this, &SpitfireRtc::_OnIceGatheringState);
			on_ice_gathering_state_callback_handle_ = GCHandle::Alloc(onIceGatheringStateChange);
			conductor_->get()->onIceGatheringStateChange = static_cast<Spitfire::OnIceGatheringStateCallbackNative>(Marshal::GetFunctionPointerForDelegate(onIceGatheringStateChange).ToPointer());

			onBufferAmountChange = gcnew _OnBufferChangeCallback(this, &SpitfireRtc::_OnBufferAmountChange);
			on_buffer_amount_change_handle_ = GCHandle::Alloc(onBufferAmountChange);
			conductor_->get()->onBufferAmountChange = static_cast<Spitfire::OnBufferAmountCallbackNative>(Marshal::GetFunctionPointerForDelegate(onBufferAmountChange).ToPointer());
		}
	
		
	public:
		
		/*delegate void DataChannelStateChange(String^ label, Spitfire::DataChannelState state);
		delegate void OnCallbackError(String^ error);
		delegate void OnCallbackMessage(String^ label, array<System::Byte>^ data, bool isBinary);
		delegate void IceStateChange(IceConnectionState msg);
		delegate void IceGatheringStateChange(IceGatheringState msg);
		delegate void BufferChange(String^ label, long previousBufferAmount, long currentBufferAmount, long bytesSent, long bytesReceived);*/


		delegate void OnCallbackSdp(SpitfireSdp^ sdp);
		event OnCallbackSdp^ OnSuccessOffer;
		event OnCallbackSdp^ OnSuccessAnswer;
		
		/// <summary>
		/// This happens whenever the local ICE agent needs to deliver a message to the other peer through the signaling server.
		/// This lets the ICE agent perform negotiation with the remote peer without the browser itself needing to know any specifics
		/// about the technology being used for signaling; simply implement this method to use whatever messaging technology you choose to send the ICE candidate to the remote peer.
		/// </summary>
		delegate void OnCallbackIceCandidate(SpitfireIceCandidate^ iceCandidate);
		event OnCallbackIceCandidate^ OnIceCandidate;

		/// <summary>
		/// indicates the state of the data channel's underlying data connection.
		/// </summary>
		delegate void DataChannelStateChange(String^ label, Spitfire::DataChannelState state);
		event DataChannelStateChange^ OnDataChannelStateChange;
		
		delegate void OnCallbackError(String^ error);
		event OnCallbackError^ OnFailure;

		/// <summary>
		/// Property stores an EventHandler which specifies a function to be called when the message event is fired on the channel.
		/// </summary>
		delegate void OnCallbackMessage(String^ label, IntPtr data, uint32_t length, bool isBinary);
		event OnCallbackMessage^ OnMessage;

		
		
		/// <summary>
		/// Informs you have the latest changes to the active ICE candidates state. 
		/// This will always provide you the best information of if a peer has been lost (albeit delayed).
		/// </summary>
		delegate void IceStateChange(IceConnectionState msg);
		event IceStateChange^ OnIceStateChange;

	
		
		/// <summary>
		/// When ICE firststarts to gather connection candidates, the value changes from new to gathering to indicate that the process of collecting candidate 
		/// configurations for the connection has begun. When the value changes to complete, 
		/// all of the transports that make up the RTCPeerConnection have finished gathering ICE candidates.
		/// </summary>
		delegate void IceGatheringStateChange(IceGatheringState^ msg);
		event IceGatheringStateChange^ OnIceGatheringStateChange;
		
		/// <summary>
		/// Lets you know the buffer has changed and gives a snapshot of the current buffer
		/// Along with the current amount of data that has been sent/received. 
		/// </summary>
		delegate void BufferChange(String^ label, uint64_t previous_buffer_amount, uint64_t current_buffer_amount, uint64_t bytes_sent, uint64_t bytes_received);
		event BufferChange^ OnBufferAmountChange;

		SpitfireRtc()
		{
			Initialize(1025, 65535);
		}
		SpitfireRtc(const uint16_t min_port, const uint16_t max_port)
		{
			Initialize(min_port, max_port);
		}
		~SpitfireRtc()
		{
			if(disposed_)
				return;

			// dispose managed data
			FreeGCHandle(on_success_handle_);
			FreeGCHandle(on_failure_handle_);
			FreeGCHandle(on_message_handle_);
			FreeGCHandle(on_ice_candidate_handle_);
			FreeGCHandle(on_data_channel_state_handle_);
			FreeGCHandle(on_buffer_amount_change_handle_);
			FreeGCHandle(on_ice_state_callback_handle_);
			FreeGCHandle(on_ice_gathering_state_callback_handle_);

		
			if(conductor_)
			{
				conductor_->get()->DeletePeerConnection();
			}

			this->!SpitfireRtc(); // call finalizer

			disposed_ = true;
		}

		/// <summary>
		/// Enables logging of WebRTC.
		/// the max log size indicates how big a single log file can be before splitting.
		/// </summary>
		static void EnableLogging(RtcLogVerbosity verbosity, String^ log_directory, const uint64_t max_log_size, const uint16_t max_number_of_splits)
		{
			const auto directory = marshal_as<std::string>(log_directory);
			if (!directory.empty()) {
				static rtc::FileRotatingLogSink sink(directory, "spitfire", max_log_size, max_number_of_splits);
				sink.Init();
				rtc::LogMessage::LogTimestamps();
				rtc::LogMessage::LogThreads();
				const auto log_severity = static_cast<rtc::LoggingSeverity>(verbosity);
				rtc::LogMessage::AddLogToStream(&sink, log_severity);
			}
		}

		/// <summary>
		/// Setups and ensures SSL is working.
		/// </summary>
		static void InitializeSSL()
		{
			rtc::WinsockInitializer();
			rtc::InitializeSSL();
			rtc::InitRandom(static_cast<int>(rtc::Time()));
		}
		
		/// <summary>
		/// Attempts to clean up SSL threads.
		/// </summary>
		static void CleanupSSL()
		{
			rtc::CleanupSSL();
		}

		/// <summary>
		/// Creates a peer connection, call InitializeSSL before calling this.
		/// </summary>
		bool InitializePeerConnection()
		{
			return conductor_->get()->InitializePeerConnection(min_port_, max_port_);
		}

		void CreateOffer()
		{
			conductor_->get()->CreateOffer();
		}

		/// <summary>
		/// Run this within a loop to process signaling messages for your peer.
		/// </summary>
		bool ProcessMessages(Int32 delay)
		{
			return conductor_->get()->ProcessMessages(delay);
		}

		void SetOfferReply(String^ type, String^ sdp)
		{
			conductor_->get()->OnOfferReply(marshal_as<std::string>(type), marshal_as<std::string>(sdp));
		}

		/// <summary>
		/// Provides an offer to your peer connection from a remote peer
		/// This is used to setup the data channel between two peers. 
		/// </summary>
		void SetOfferRequest(String^ sdp)
		{
			conductor_->get()->OnOfferRequest(marshal_as<std::string>(sdp));
		}

		bool AddIceCandidate(String^ sdp_mid, int32_t sdp_mlineindex, String^ sdp)
		{
			return conductor_->get()->AddIceCandidate(marshal_as<std::string>(sdp_mid), sdp_mlineindex, marshal_as<std::string>(sdp));
		}

		void AddServerConfig(ServerConfig^ config)
		{
			String^ type = config->Type == ServerType::Stun ? "stun" : "turn";
			auto hostUri = marshal_as<std::string>(type + ":" + config->Host + ":" + config->Port);
			auto u = config->Username;
			auto username = String::IsNullOrWhiteSpace(u) ? "" : marshal_as<std::string>(u);
			auto p = config->Password;
			auto password = String::IsNullOrWhiteSpace(p) ? "" : marshal_as<std::string>(p);
			conductor_->get()->AddServerConfig(hostUri, username, password);
		}
		/// <summary>
		/// Creates a data channel from within the application.
		/// Only call if your application is setting up the connection and preparing to offer.
		/// </summary>
		void CreateDataChannel(DataChannelOptions^ dataChannelOptions)
		{
			auto label = dataChannelOptions->Label;
			auto protocol = dataChannelOptions->Protocol;

			webrtc::DataChannelInit dc_options;
			dc_options.id = dataChannelOptions->Id;
			if(dataChannelOptions->MaxRetransmits.HasValue) {
				dc_options.maxRetransmits.emplace(dataChannelOptions->MaxRetransmits.Value);
			}
			if(dataChannelOptions->MaxRetransmitTime.HasValue) {
				dc_options.maxRetransmitTime.emplace(dataChannelOptions->MaxRetransmitTime.Value);
			}
			dc_options.negotiated = dataChannelOptions->Negotiated;
			dc_options.ordered = dataChannelOptions->Ordered;
			if(!String::IsNullOrWhiteSpace(protocol))
			{
				dc_options.protocol = marshal_as<std::string>(protocol);
			}
			dc_options.reliable = dataChannelOptions->Reliable;
			conductor_->get()->CreateDataChannel(marshal_as<std::string>(label), dc_options);
		}
		/// <summary>
		/// Send your text through the data channel
		/// </summary>
		void DataChannelSendText(String^ label, String^ text)
		{
			conductor_->get()->DataChannelSendText(marshal_as<std::string>(label), marshal_as<std::string>(text));
		}

		/// <summary>
		/// Returns a snapshot of information on the target data channel, including its state and structure.
		/// </summary>
		Spitfire::DataChannelInfo^ GetDataChannelInfo(String^ label)
		{
			auto rtc_info = conductor_->get()->GetDataChannelInfo(marshal_as<std::string>(label));
			if(rtc_info.protocol != "unknown")
			{
				const auto managed_info = gcnew Spitfire::DataChannelInfo();
				managed_info->CurrentBuffer = rtc_info.currentBuffer;
				managed_info->BytesSent = rtc_info.bytesSent;
				managed_info->BytesReceived = rtc_info.bytesReceived;

				managed_info->Reliable = rtc_info.reliable;
				managed_info->Ordered = rtc_info.ordered;
				managed_info->Negotiated = rtc_info.negotiated;

				managed_info->MessagesSent = rtc_info.messagesSent;
				managed_info->MessagesReceived = rtc_info.messagesReceived;
				managed_info->MaxRetransmits = rtc_info.maxRetransmits;
				managed_info->MaxRetransmitTime = rtc_info.maxRetransmitTime;

				if(!rtc_info.protocol.empty())
				{
					managed_info->Protocol = gcnew String(rtc_info.protocol.c_str());
				}
				managed_info->State = static_cast<DataChannelState>(rtc_info.state);
				return managed_info;
			}
			return nullptr;
		}

		/// <summary>
		/// Returns the current state of a given data channel
		/// </summary>
		Spitfire::DataChannelState GetDataChannelState(String^ label)
		{
			auto state = conductor_->get()->GetDataChannelState(marshal_as<std::string>(label));
			const auto managed_state = static_cast<DataChannelState>(state);
			return managed_state;
		}
		
		/// <summary>
		/// Closes a data channel and disposes it's observers
		/// </summary>
		void CloseDataChannel(String^ label)
		{
			conductor_->get()->CloseDataChannel(marshal_as<std::string>(label));
		}
		/// <summary>
		/// Send your binary data through the data channel
		/// Be aware that channels have a 16KB limit and you should take advantage 
		/// Of the provided utilties to chunk messages quickly.
		/// </summary>
		void DataChannelSendData(String^ label, Byte* array_data, uint32_t length)
		{
			conductor_->get()->DataChannelSendData(marshal_as<std::string>(label), array_data, length);
		}

	protected:
		!SpitfireRtc()
		{
			// free unmanaged data
			if(conductor_)
			{
				conductor_->release();
				delete conductor_;
			}
		}
	};
}
