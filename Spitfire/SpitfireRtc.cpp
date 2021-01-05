#pragma unmanaged

#pragma warning(disable: 4635) // warning C4635: XML document comment applied to 'rtc.EventBasedExponentialMovingAverage': badly-formed XML: A semi colon character was expected.

#include <rtc_base\ssl_adapter.h>
#include <rtc_base\win32_socket_init.h>
#include <rtc_base\win32_socket_server.h>
#include <rtc_base\logging.h>
#include <rtc_base\time_utils.h>
#include <rtc_base\helpers.h>

#include "RtcConductor.h"

#pragma managed

#include <msclr\marshal_cppstd.h>

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
		Spitfire::RtcConductor* conductor_ = nullptr;

		bool disposed_ = false;
		uint16_t min_port_;
		uint16_t max_port_;

		delegate void _OnSuccessCallback(String^ type, String^ sdp);
		_OnSuccessCallback^ on_success_;
		GCHandle^ on_success_handle_;

		delegate void _OnFailureCallback(String^ error);
		_OnFailureCallback^ on_failure_;
		GCHandle^ on_failure_handle_;
		
		delegate void _OnMessageCallback(String^ label, uint8_t* msg, uint32_t size, bool is_binary);
		_OnMessageCallback^ on_message_;
		GCHandle^ on_message_handle_;

		delegate void _OnIceCandidateCallback(String^ sdp_mid, int32_t sdp_mline_index, String^ sdp);
		_OnIceCandidateCallback^ on_ice_candidate_;
		GCHandle^ on_ice_candidate_handle_;

		delegate void _OnDataChannelStateCallback(String^ label, webrtc::DataChannelInterface::DataState state);
		_OnDataChannelStateCallback^ on_data_channel_state_change_;
		GCHandle^ on_data_channel_state_change_handle_;

		delegate void _OnBufferChangeCallback(String^ label, uint64_t previousAmount, uint64_t currentAmount, uint64_t bytesSent, uint64_t bytesReceived);
		_OnBufferChangeCallback^ on_buffer_amount_change_;
		GCHandle^ on_buffer_amount_change_handle_;

		delegate void _OnIceStateCallback(webrtc::PeerConnectionInterface::IceConnectionState state);
		_OnIceStateCallback^ on_ice_state_change_;
		GCHandle^ on_ice_state_change_handle_;

		delegate void _OnIceGatheringStateCallback(webrtc::PeerConnectionInterface::IceGatheringState state);
		_OnIceGatheringStateCallback^ on_ice_gathering_state_change_;
		GCHandle^ on_ice_gathering_state_change_handle_;

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
		void _OnFailure(String^ error)
		{
			OnFailure(error);
		}
		void _OnMessage(String^ label, uint8_t* data, uint32_t size, bool is_binary)
		{
			//auto buffer = gcnew array<Byte>(size);
			//IntPtr src(data);
			//Marshal::Copy(src, buffer, 0, size);
			IntPtr managedPointer(data);
			OnMessage(label, managedPointer, size, is_binary);
		}

		void _OnIceCandidate(String^ sdp_mid, int32_t sdp_mline_index, String^ sdp)
		{
			auto ice = gcnew SpitfireIceCandidate();
			ice->Sdp = gcnew String(sdp);
			ice->SdpMid = gcnew String(sdp_mid);
			ice->SdpIndex = sdp_mline_index;
			OnIceCandidate(ice);
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

		void Initialize(uint16_t min_port, uint16_t max_port)
		{
			#if defined(_DEBUG)
				//_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_CHECK_ALWAYS_DF);
			#endif

			assert(!conductor_);
			conductor_ = new Spitfire::RtcConductor();
			min_port_ = min_port;
			max_port_ = max_port;

			// NOTE: How to: Define and Use Delegates (C++/CLI) https://docs.microsoft.com/en-us/cpp/dotnet/how-to-define-and-use-delegates-cpp-cli

			#define B(on_xxx_, delegate_name_a, delegate_name_b, delegate_name_c) \
				on_xxx_ = gcnew _##delegate_name_a##Callback(this, &SpitfireRtc::_##delegate_name_b); \
				on_xxx_##handle_ = GCHandle::Alloc(on_xxx_); \
				conductor_->on_xxx_ = static_cast<delegate_name_c##CallbackNative>(Marshal::GetFunctionPointerForDelegate(on_xxx_).ToPointer());

			#define A(on_xxx_, delegate_name) B(on_xxx_, delegate_name, delegate_name, delegate_name)

			A(on_success_, OnSuccess);
			A(on_failure_, OnFailure);
			A(on_message_, OnMessage);

			A(on_ice_candidate_, OnIceCandidate);
			A(on_data_channel_state_change_, OnDataChannelState);
			B(on_ice_state_change_, OnIceState, OnIceState, OnIceStateChange);
			A(on_ice_gathering_state_change_, OnIceGatheringState);
			B(on_buffer_amount_change_, OnBufferChange, OnBufferAmountChange, OnBufferAmount);

			#undef A
			#undef B
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
			RTC_DLOG_F(LS_INFO);
			Initialize(1025, 65535);
		}
		SpitfireRtc(uint16_t min_port, uint16_t max_port)
		{
			RTC_DLOG_F(LS_INFO) << min_port << ", " << max_port;
			Initialize(min_port, max_port);
		}
		~SpitfireRtc()
		{
			RTC_DLOG_F(LS_INFO) << disposed_;
			if(disposed_)
				return;
			#pragma region Delegate Handle
			FreeGCHandle(on_success_handle_);
			FreeGCHandle(on_failure_handle_);
			FreeGCHandle(on_message_handle_);
			FreeGCHandle(on_ice_candidate_handle_);
			FreeGCHandle(on_data_channel_state_change_handle_);
			FreeGCHandle(on_buffer_amount_change_handle_);
			FreeGCHandle(on_ice_state_change_handle_);
			FreeGCHandle(on_ice_gathering_state_change_handle_);
			#pragma endregion
			this->!SpitfireRtc();
			disposed_ = true;
		}

		/// <summary>
		/// Enables logging of WebRTC.
		/// the max log size indicates how big a single log file can be before splitting.
		/// </summary>
		static void EnableLogging(RtcLogVerbosity verbosity, String^ log_directory, const uint64_t max_log_size, const uint16_t max_number_of_splits)
		{
			const auto directory = marshal_as<std::string>(log_directory);
			if (!directory.empty()) 
			{
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
			RTC_DCHECK(conductor_);
			return conductor_->InitializePeerConnection(min_port_, max_port_);
		}

		void CreateOffer()
		{
			RTC_DCHECK(conductor_);
			conductor_->CreateOffer();
		}

		/// <summary>
		/// Run this within a loop to process signaling messages for your peer.
		/// </summary>
		bool ProcessMessages(Int32 delay)
		{
			RTC_DCHECK(conductor_);
			return conductor_->ProcessMessages(delay);
		}

		void SetOfferReply(String^ type, String^ sdp)
		{
			RTC_DCHECK(type && sdp);
			RTC_DCHECK(conductor_);
			conductor_->OnOfferReply(marshal_as<std::string>(type), marshal_as<std::string>(sdp));
		}

		/// <summary>
		/// Provides an offer to your peer connection from a remote peer
		/// This is used to setup the data channel between two peers. 
		/// </summary>
		void SetOfferRequest(String^ sdp)
		{
			RTC_DCHECK(sdp);
			RTC_DCHECK(conductor_);
			conductor_->OnOfferRequest(marshal_as<std::string>(sdp));
		}

		bool AddIceCandidate(String^ sdp_mid, int32_t sdp_mlineindex, String^ sdp)
		{
			RTC_DCHECK(sdp_mid && sdp);
			RTC_DCHECK(conductor_);
			return conductor_->AddIceCandidate(marshal_as<std::string>(sdp_mid), sdp_mlineindex, marshal_as<std::string>(sdp));
		}

		void AddServerConfig(ServerConfig^ config)
		{
			//RTC_DCHECK(config);
			RTC_DCHECK(conductor_);
			if(!config)
				throw gcnew ArgumentException(gcnew String("Missing mandatory argument"), gcnew String("config"));
			if(String::IsNullOrEmpty(config->Host))
				throw gcnew ArgumentException(gcnew String("Missing mandatory host argument"));
			char uri[256];
			sprintf_s(uri, "%hs:%hs:%u", config->Type == ServerType::Stun ? "stun" : "turn", From(config->Host, "").c_str(), config->Port);
			conductor_->AddServerConfig(uri, From(config->Username, ""), From(config->Password, ""));
		}

		/// <summary>
		/// Creates a data channel from within the application.
		/// Only call if your application is setting up the connection and preparing to offer.
		/// </summary>
		void CreateDataChannel(DataChannelOptions^ dataChannelOptions)
		{
			RTC_DCHECK(dataChannelOptions);
			RTC_DCHECK(conductor_);
			auto label = dataChannelOptions->Label;
			auto protocol = dataChannelOptions->Protocol;
			webrtc::DataChannelInit dc_options;
			dc_options.id = dataChannelOptions->Id;
			if(dataChannelOptions->MaxRetransmits.HasValue)
				dc_options.maxRetransmits.emplace(dataChannelOptions->MaxRetransmits.Value);
			if(dataChannelOptions->MaxRetransmitTime.HasValue)
				dc_options.maxRetransmitTime.emplace(dataChannelOptions->MaxRetransmitTime.Value);
			dc_options.negotiated = dataChannelOptions->Negotiated;
			dc_options.ordered = dataChannelOptions->Ordered;
			if(!String::IsNullOrWhiteSpace(protocol))
				dc_options.protocol = marshal_as<std::string>(protocol);
			dc_options.reliable = dataChannelOptions->Reliable;
			conductor_->CreateDataChannel(marshal_as<std::string>(label), dc_options);
		}

		/// <summary>
		/// Send your text through the data channel
		/// </summary>
		void DataChannelSendText(String^ label, String^ text)
		{
			RTC_DCHECK(label);
			RTC_DCHECK(text);
			RTC_DCHECK(conductor_);
			conductor_->SendToDataChannel(marshal_as<std::string>(label), webrtc::DataBuffer(marshal_as<std::string>(text)));
		}

		/// <summary>
		/// Returns a snapshot of information on the target data channel, including its state and structure.
		/// </summary>
		Spitfire::DataChannelInfo^ GetDataChannelInfo(String^ label)
		{
			RTC_DCHECK(label);
			RTC_DCHECK(conductor_);
			const auto optional_info = conductor_->GetDataChannelInfo(marshal_as<std::string>(label));
			if(optional_info.has_value())
			{
				const auto& info = optional_info.value();
				const auto managed_info = gcnew Spitfire::DataChannelInfo();
				managed_info->CurrentBuffer = info.currentBuffer;
				managed_info->BytesSent = info.bytesSent;
				managed_info->BytesReceived = info.bytesReceived;

				managed_info->Reliable = info.reliable;
				managed_info->Ordered = info.ordered;
				managed_info->Negotiated = info.negotiated;

				managed_info->MessagesSent = info.messagesSent;
				managed_info->MessagesReceived = info.messagesReceived;
				managed_info->MaxRetransmits = info.maxRetransmits;
				managed_info->MaxRetransmitTime = info.maxRetransmitTime;

				if(!info.protocol.empty())
				{
					managed_info->Protocol = gcnew String(info.protocol.c_str());
				}
				managed_info->State = static_cast<DataChannelState>(info.state);
				return managed_info;
			}
			return nullptr;
		}

		/// <summary>
		/// Returns the current state of a given data channel
		/// </summary>
		Spitfire::DataChannelState GetDataChannelState(String^ label)
		{
			RTC_DCHECK(label);
			RTC_DCHECK(conductor_);
			const auto state = conductor_->GetDataChannelState(marshal_as<std::string>(label));
			return static_cast<DataChannelState>(state.value_or(webrtc::DataChannelInterface::DataState::kClosed));
		}
		
		/// <summary>
		/// Closes a data channel and disposes it's observers
		/// </summary>
		void CloseDataChannel(String^ label)
		{
			RTC_DCHECK(label);
			RTC_DCHECK(conductor_);
			conductor_->CloseDataChannel(marshal_as<std::string>(label));
		}

		/// <summary>
		/// Send your binary data through the data channel
		/// Be aware that channels have a 16KB limit and you should take advantage 
		/// Of the provided utilties to chunk messages quickly.
		/// </summary>
		void DataChannelSendData(String^ label, Byte* array_data, uint32_t length)
		{
			RTC_DCHECK(label && array_data);
			RTC_DCHECK(conductor_);
			conductor_->SendToDataChannel(marshal_as<std::string>(label), webrtc::DataBuffer(rtc::CopyOnWriteBuffer(array_data, length), true));
		}

	protected:
		!SpitfireRtc()
		{
			RTC_DLOG_F(LS_INFO) << static_cast<bool>(conductor_ != nullptr);
			if(conductor_)
			{
				RtcConductor* conductor = conductor_;
				conductor_ = nullptr;
				delete conductor;
			}
		}
		static void FreeGCHandle(GCHandle^% handle)
		{
			if(!handle)
				return;
			handle->Free();
			handle = nullptr;
		}

	private:
		static absl::optional<std::string> From(String^ value)
		{
			if(!value)
				return absl::nullopt;
			return msclr::interop::marshal_as<std::string>(value);
		}
		static std::string From(String^ value, const char* default_value)
		{
			if(!value)
				return default_value;
			return msclr::interop::marshal_as<std::string>(value);
		}
	};
}
