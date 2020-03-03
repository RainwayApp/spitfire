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

[assembly:System::Runtime::Versioning::TargetFrameworkAttribute(L".NETFramework,Version=v4.0", FrameworkDisplayName = L".NET Framework 4")];

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
		Connecting,

		/// <summary>
		/// The underlying data transport is established and communication is possible. 
		/// This is the initial state of a RTCDataChannel object dispatched as a part of a RTCDataChannelEvent.
		/// </summary>
		Open,

		/// <summary>
		/// The procedure to close down the underlying data transport has started.
		/// </summary>
		Closing,

		/// <summary>
		/// The underlying data transport has been closed or could not be established.
		/// </summary>
		Closed
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

	public ref class DataMessage
	{
	public:
		bool IsBinary;
		bool IsText;
		array<System::Byte>^ RawData;
		String^ Data;
	};

	public ref class DataChannelInfo
	{
	public:
		unsigned long CurrentBuffer;
		unsigned long BytesSent;
		unsigned long BytesReceived;

		bool Reliable;
		bool Ordered;
		bool Negotiated;

		unsigned int MessagesSent;
		unsigned int MessagesReceived;

		int MaxRetransmits;
		int MaxRetransmitTime;

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
		Nullable<int> MaxRetransmitTime;

		/* 
		 * The max number of retransmissions. -1 if unset.
		 * Cannot be set along with |MaxRetransmitTime|.
		 */
		 /// <summary>
		 /// The max number of retransmissions. -1 if unset.
		 /// Cannot be set along with |MaxRetransmitTime|.
		 /// </summary>
		Nullable<int> MaxRetransmits;
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
		int Id = -1;
	};

	public ref class SpitfireIceCandidate
	{
	public:
		String^ SdpMid;
		int SdpIndex;
		String^ Sdp;
	};

	public ref class SpitfireRtc
	{
	private:
		std::unique_ptr<Spitfire::RtcConductor>* conductor_;

		bool disposed_;
		int min_port_;
		int max_port_;

		delegate void _OnErrorCallback();
		_OnErrorCallback^ onError;
		GCHandle^ onErrorHandle;

		delegate void _OnSuccessCallback(String^ type, String^ sdp);
		_OnSuccessCallback^ onSuccess;
		GCHandle^ onSuccessHandle;

		delegate void _OnFailureCallback(String^ error);
		_OnFailureCallback^ onFailure;
		GCHandle^ onFailureHandle;

		delegate void _OnDataMessageCallback(String^ label, String^ msg);
		_OnDataMessageCallback^ onDataMessage;
		GCHandle^ onDataMessageHandle;

		delegate void _OnDataBinaryMessageCallback(String^ label, uint8_t* msg, uint32_t size);
		_OnDataBinaryMessageCallback^ onDataBinaryMessage;
		GCHandle^ onDataBinaryMessageHandle;

		delegate void _OnIceCandidateCallback(String^ sdp_mid, Int32 sdp_mline_index, String^ sdp);
		_OnIceCandidateCallback^ onIceCandidate;
		GCHandle^ onIceCandidateHandle;

		delegate void _OnDataChannelStateCallback(String^ label, webrtc::DataChannelInterface::DataState state);
		_OnDataChannelStateCallback^ onDataChannelStateChange;
		GCHandle^ onDataChannelStateHandle;

		delegate void _OnBufferChangeCallback(String^ label, uint64_t previousAmount, uint64_t currentAmount, uint64_t bytesSent, uint64_t bytesReceived);
		_OnBufferChangeCallback^ onBufferAmountChange;
		GCHandle^ onBufferAmountChangeHandle;

		delegate void _OnIceStateCallback(webrtc::PeerConnectionInterface::IceConnectionState state);
		_OnIceStateCallback^ onIceStateChange;
		GCHandle^ onIceStateCallbackHandle;

		delegate void _OnIceGatheringStateCallback(webrtc::PeerConnectionInterface::IceGatheringState state);
		_OnIceGatheringStateCallback^ onIceGatheringStateChange;
		GCHandle^ onIceGatheringStateCallbackHandle;

		void FreeGCHandle(GCHandle^% g)
		{
			if(g != nullptr)
			{
				g->Free();
				g = nullptr;
			}
		}

		void _OnError()
		{
			OnError();
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

		void _OnIceCandidate(String^ sdp_mid, Int32 sdp_mline_index, String^ sdp)
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

		void _OnDataMessage(String^ label, String^ msg)
		{
			auto message = gcnew Spitfire::DataMessage();
			message->IsBinary = false;
			message->RawData = nullptr;
			message->IsText = true;
			message->Data = gcnew String(msg);
			OnDataMessage(label, message);
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

		void _OnBufferAmountChange(String^ label, uint64_t previousAmount, uint64_t currentAmount, uint64_t bytesSent, uint64_t bytesReceived)
		{
			OnBufferAmountChange(label, static_cast<long>(previousAmount), static_cast<long>(currentAmount), static_cast<long>(bytesSent), static_cast<long>(bytesReceived));
		}

		void _OnDataChannelState(String^ label, webrtc::DataChannelInterface::DataState state)
		{
			if(state == webrtc::DataChannelInterface::DataState::kOpen)
			{
				OnDataChannelOpen(label);
			}
			else if(state == webrtc::DataChannelInterface::DataState::kClosed)
			{
				OnDataChannelClose(label);
			}
		}

		void _OnDataBinaryMessage(String^ label, uint8_t* data, uint32_t size)
		{
			array<Byte>^ data_array = gcnew array<Byte>(size);
			IntPtr src(data);
			Marshal::Copy(src, data_array, 0, size);
			auto message = gcnew Spitfire::DataMessage();
			message->IsBinary = true;
			message->Data = nullptr;
			message->IsText = false;
			message->RawData = data_array;
			OnDataMessage(label, message);
		}

		void Initialize(int min_port, int max_port)
		{
			disposed_ = false;
			conductor_ = new std::unique_ptr<Spitfire::RtcConductor>(new Spitfire::RtcConductor());
			min_port_ = min_port;
			max_port_ = max_port;

			onError = gcnew _OnErrorCallback(this, &SpitfireRtc::_OnError);
			onErrorHandle = GCHandle::Alloc(onError);
			conductor_->get()->onError = static_cast<Spitfire::OnErrorCallbackNative>(Marshal::GetFunctionPointerForDelegate(onError).ToPointer());

			onSuccess = gcnew _OnSuccessCallback(this, &SpitfireRtc::_OnSuccess);
			onSuccessHandle = GCHandle::Alloc(onSuccess);
			conductor_->get()->onSuccess = static_cast<Spitfire::OnSuccessCallbackNative>(Marshal::GetFunctionPointerForDelegate(onSuccess).ToPointer());

			onFailure = gcnew _OnFailureCallback(this, &SpitfireRtc::_OnFailure);
			onFailureHandle = GCHandle::Alloc(onFailure);
			conductor_->get()->onFailure = static_cast<Spitfire::OnFailureCallbackNative>(Marshal::GetFunctionPointerForDelegate(onFailure).ToPointer());

			onDataMessage = gcnew _OnDataMessageCallback(this, &SpitfireRtc::_OnDataMessage);
			onDataMessageHandle = GCHandle::Alloc(onDataMessage);
			conductor_->get()->onDataMessage = static_cast<Spitfire::OnDataMessageCallbackNative>(Marshal::GetFunctionPointerForDelegate(onDataMessage).ToPointer());

			onDataBinaryMessage = gcnew _OnDataBinaryMessageCallback(this, &SpitfireRtc::_OnDataBinaryMessage);
			onDataBinaryMessageHandle = GCHandle::Alloc(onDataBinaryMessage);
			conductor_->get()->onDataBinaryMessage = static_cast<Spitfire::OnDataBinaryMessageCallbackNative>(Marshal::GetFunctionPointerForDelegate(onDataBinaryMessage).ToPointer());

			onIceCandidate = gcnew _OnIceCandidateCallback(this, &SpitfireRtc::_OnIceCandidate);
			onIceCandidateHandle = GCHandle::Alloc(onIceCandidate);
			conductor_->get()->onIceCandidate = static_cast<Spitfire::OnIceCandidateCallbackNative>(Marshal::GetFunctionPointerForDelegate(onIceCandidate).ToPointer());

			onDataChannelStateChange = gcnew _OnDataChannelStateCallback(this, &SpitfireRtc::_OnDataChannelState);
			onDataChannelStateHandle = GCHandle::Alloc(onDataChannelStateChange);
			conductor_->get()->onDataChannelState = static_cast<Spitfire::OnDataChannelStateCallbackNative>(Marshal::GetFunctionPointerForDelegate(onDataChannelStateChange).ToPointer());

			onIceStateChange = gcnew _OnIceStateCallback(this, &SpitfireRtc::_OnIceState);
			onIceStateCallbackHandle = GCHandle::Alloc(onIceStateChange);
			conductor_->get()->onIceStateChange = static_cast<Spitfire::OnIceStateChangeCallbackNative>(Marshal::GetFunctionPointerForDelegate(onIceStateChange).ToPointer());

			onIceGatheringStateChange = gcnew _OnIceGatheringStateCallback(this, &SpitfireRtc::_OnIceGatheringState);
			onIceGatheringStateCallbackHandle = GCHandle::Alloc(onIceGatheringStateChange);
			conductor_->get()->onIceGatheringStateChange = static_cast<Spitfire::OnIceGatheringStateCallbackNative>(Marshal::GetFunctionPointerForDelegate(onIceGatheringStateChange).ToPointer());

			onBufferAmountChange = gcnew _OnBufferChangeCallback(this, &SpitfireRtc::_OnBufferAmountChange);
			onBufferAmountChangeHandle = GCHandle::Alloc(onBufferAmountChange);
			conductor_->get()->onBufferAmountChange = static_cast<Spitfire::OnBufferAmountCallbackNative>(Marshal::GetFunctionPointerForDelegate(onBufferAmountChange).ToPointer());
		}

	public:
		delegate void OnCallbackSdp(SpitfireSdp^ sdp);
		event OnCallbackSdp^ OnSuccessOffer;
		event OnCallbackSdp^ OnSuccessAnswer;

		delegate void OnCallbackIceCandidate(SpitfireIceCandidate^ iceCandidate);
		event OnCallbackIceCandidate^ OnIceCandidate;

		event Action^ OnError;

		delegate void DataChannelOpen(String^ label);
		/// <summary>
		/// Signals that the data channel is opened and ready for interaction
		/// </summary>
		event DataChannelOpen^ OnDataChannelOpen;

		delegate void DataChannelClose(String^ label);
		/// <summary>
		/// Signals that the data channel has been closed, this is not
		/// Guaranteed to fire at all, so trust it as far as you can throw it.
		/// Best paired with OnIceStateChange for best results.
		/// </summary>
		event DataChannelClose^ OnDataChannelClose;

		delegate void OnCallbackError(String^ error);
		event OnCallbackError^ OnFailure;

		delegate void OnCallbackDataMessage(String^ label, Spitfire::DataMessage^ msg);
		event OnCallbackDataMessage^ OnDataMessage;

		delegate void IceStateChange(IceConnectionState msg);
		/// <summary>
		/// Informs you have the latest changes to the active ICE candidates state. 
		/// This will always provide you the best information of if a peer has been lost (albeit delayed).
		/// </summary>
		event IceStateChange^ OnIceStateChange;

		delegate void IceGatheringStateChange(IceGatheringState msg);
		/// <summary>
		/// When ICE firststarts to gather connection candidates, the value changes from new to gathering to indicate that the process of collecting candidate 
		/// configurations for the connection has begun. When the value changes to complete, 
		/// all of the transports that make up the RTCPeerConnection have finished gathering ICE candidates.
		/// </summary>
		event IceGatheringStateChange^ OnIceGatheringStateChange;

		delegate void BufferChange(String^ label, long previousBufferAmount, long currentBufferAmount, long bytesSent, long bytesReceived);
		/// <summary>
		/// Lets you know the buffer has changed and gives a snapshot of the current buffer
		/// Along with the current amount of data that has been sent/received. 
		/// </summary>
		event BufferChange^ OnBufferAmountChange;

		SpitfireRtc()
		{
			Initialize(1025, 65535);
		}
		SpitfireRtc(int MinPort, int MaxPort)
		{
			Initialize(MinPort, MaxPort);
		}
		~SpitfireRtc()
		{
			if(disposed_)
				return;

			// dispose managed data
			FreeGCHandle(onErrorHandle);
			FreeGCHandle(onSuccessHandle);
			FreeGCHandle(onFailureHandle);
			FreeGCHandle(onIceCandidateHandle);
			FreeGCHandle(onDataMessageHandle);
			FreeGCHandle(onIceGatheringStateCallbackHandle);
			FreeGCHandle(onDataBinaryMessageHandle);
			FreeGCHandle(onDataChannelStateHandle);
			if(conductor_)
			{
				conductor_->get()->DeletePeerConnection();
			}

			this->!SpitfireRtc(); // call finalizer

			disposed_ = true;
		}

		/// <summary>
		/// Enables logging of WebRTC verbosely 
		/// You'll likely want to do this for debugging WebRTC itself.
		/// </summary>
		static void EnableLogging()
		{
			rtc::LogMessage::LogTimestamps();
			rtc::LogMessage::LogThreads();
			rtc::LogMessage::LogToDebug(rtc::LS_VERBOSE);
			rtc::LogMessage::SetLogToStderr(true);
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

		bool AddIceCandidate(String^ sdp_mid, Int32 sdp_mlineindex, String^ sdp)
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
			auto rtcInfo = conductor_->get()->GetDataChannelInfo(marshal_as<std::string>(label));
			if(rtcInfo.protocol != "unknown")
			{
				auto managedInfo = gcnew Spitfire::DataChannelInfo();
				managedInfo->CurrentBuffer = static_cast<unsigned long>(rtcInfo.currentBuffer);
				managedInfo->BytesSent = static_cast<unsigned long>(rtcInfo.bytesSent);
				managedInfo->BytesReceived = static_cast<unsigned long>(rtcInfo.bytesReceived);

				managedInfo->Reliable = rtcInfo.reliable;
				managedInfo->Ordered = rtcInfo.ordered;
				managedInfo->Negotiated = rtcInfo.negotiated;

				managedInfo->MessagesSent = rtcInfo.messagesSent;
				managedInfo->MessagesReceived = rtcInfo.messagesReceived;
				managedInfo->MaxRetransmits = rtcInfo.maxRetransmits;
				managedInfo->MaxRetransmitTime = rtcInfo.maxRetransmitTime;

				if(!rtcInfo.protocol.empty())
				{
					managedInfo->Protocol = gcnew String(rtcInfo.protocol.c_str());
				}

				managedInfo->State = static_cast<DataChannelState>(rtcInfo.state);
				return managedInfo;
			}
			return nullptr;
		}

		/// <summary>
		/// Returns the current state of a given data channel
		/// </summary>
		Spitfire::DataChannelState GetDataChannelState(String^ label)
		{
			auto state = conductor_->get()->GetDataChannelState(marshal_as<std::string>(label));
			DataChannelState managedState = static_cast<DataChannelState>(state);
			return managedState;
		}

		/// <summary>
		/// Send your binary data through the data channel
		/// Be aware that channels have a 16KB limit and you should take advantage 
		/// Of the provided utilties to chunk messages quickly.
		/// </summary>
		void DataChannelSendData(String^ label, Byte* array_data, int length)
		{
			rtc::CopyOnWriteBuffer writeBuffer(array_data, length);
			conductor_->get()->DataChannelSendData(marshal_as<std::string>(label), webrtc::DataBuffer(writeBuffer, true));
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
