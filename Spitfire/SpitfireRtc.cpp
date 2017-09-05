
#pragma unmanaged
#include "RtcUtils.h"
#include "RtcConductor.h"
#pragma managed

#include "msclr\marshal_cppstd.h"

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

	public enum class SdpTypes
	{
		Answer,
		Offer
	};

	public ref class DataMessage
	{
	public:
		bool IsBinary;
		bool IsText;
		array<System::Byte>^ RawData;
		String^ Data;
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

			bool m_isDisposed;
			Spitfire::RtcConductor * _conductor;

			delegate void _OnErrorCallback();
			_OnErrorCallback ^ onError;
			GCHandle ^ onErrorHandle;

			delegate void _OnSuccessCallback(String ^ type, String ^ sdp);
			_OnSuccessCallback ^ onSuccess;
			GCHandle ^ onSuccessHandle;

			delegate void _OnFailureCallback(String ^ error);
			_OnFailureCallback ^ onFailure;
			GCHandle ^ onFailureHandle;

			delegate void _OnDataMessageCallback(String ^ msg);
			_OnDataMessageCallback ^ onDataMessage;
			GCHandle ^ onDataMessageHandle;

			delegate void _OnDataBinaryMessageCallback(uint8_t * msg, uint32_t size);
			_OnDataBinaryMessageCallback ^ onDataBinaryMessage;
			GCHandle ^ onDataBinaryMessageHandle;

			delegate void _OnIceCandidateCallback(String ^ sdp_mid, Int32 sdp_mline_index, String ^ sdp);
			_OnIceCandidateCallback ^ onIceCandidate;
			GCHandle ^ onIceCandidateHandle;


			delegate void _OnDataChannelStateCallback(webrtc::DataChannelInterface::DataState state);
			_OnDataChannelStateCallback ^ onDataChannelStateChange;
			GCHandle ^ onDataChannelStateHandle;


			delegate void _OnIceStateCallback(webrtc::PeerConnectionInterface::IceConnectionState state);
			_OnIceStateCallback ^ onIceStateChange;
			GCHandle ^ onIceStateCallbackHandle;





			void FreeGCHandle(GCHandle ^% g)
			{
				if (g != nullptr)
				{
					g->Free();
					g = nullptr;
				}
			}

			void _OnError()
			{
				OnError();
			}

			void _OnSuccess(String ^ type, String ^ sdp)
			{
				auto sdpModel = gcnew SpitfireSdp();
				sdpModel->Sdp = sdp;

				if (type == "offer")
				{
					sdpModel->Type = SdpTypes::Offer;
					OnSuccessOffer(sdpModel);
				}
				else if (type == "answer")
				{
					sdpModel->Type = SdpTypes::Answer;
					OnSuccessAnswer(sdpModel);
				}
			}

			void _OnIceCandidate(String ^ sdp_mid, Int32 sdp_mline_index, String ^ sdp)
			{
				auto ice = gcnew SpitfireIceCandidate();
				ice->Sdp = gcnew String(sdp);
				ice->SdpMid = gcnew String(sdp_mid);
				ice->SdpIndex = sdp_mline_index;
				OnIceCandidate(ice);
			}

			void _OnFailure(String ^ error)
			{
				OnFailure(error);
			}

			void _OnDataMessage(String ^ msg)
			{
				auto message = gcnew Spitfire::DataMessage();
				message->IsBinary = false;
				message->RawData = nullptr;
				message->IsText = true;
				message->Data = gcnew String(msg);
				OnDataMessage(message);
			}


			void _OnIceState(webrtc::PeerConnectionInterface::IceConnectionState state)
			{
			
				
				IceConnectionState managedState = static_cast<IceConnectionState>(state);
				OnIceStateChange(managedState);
			}

			void _OnDataChannelState(webrtc::DataChannelInterface::DataState state)
			{
				if (state == webrtc::DataChannelInterface::DataState::kOpen)
				{
					OnDataChannelOpen();
				}
				else if (state == webrtc::DataChannelInterface::DataState::kClosed)
				{
					OnDataChannelClose();
				}
			}

			void _OnDataBinaryMessage(uint8_t * data, uint32_t size)
			{
				array<Byte>^ data_array = gcnew array<Byte>(size);
				IntPtr src(data);
				Marshal::Copy(src, data_array, 0, size);
				auto message = gcnew Spitfire::DataMessage();
				message->IsBinary = true;
				message->Data = nullptr;
				message->IsText = false;
				message->RawData = data_array;
				OnDataMessage(message);
			}

		public:

			delegate void OnCallbackSdp(SpitfireSdp ^ sdp);
			event OnCallbackSdp ^ OnSuccessOffer;
			event OnCallbackSdp ^ OnSuccessAnswer;

			delegate void OnCallbackIceCandidate(SpitfireIceCandidate ^ iceCandidate);
			event OnCallbackIceCandidate ^ OnIceCandidate;

			event Action ^ OnError;

			event Action ^ OnDataChannelOpen;

			event Action ^ OnDataChannelClose;

			delegate void OnCallbackError(String ^ error);
			event OnCallbackError ^ OnFailure;

			delegate void OnCallbackDataMessage(Spitfire::DataMessage^ msg);
			event OnCallbackDataMessage ^ OnDataMessage;

			delegate void IceStateChange(IceConnectionState msg);
			event IceStateChange ^ OnIceStateChange;

			SpitfireRtc()
			{
				m_isDisposed = false;
				_conductor = new Spitfire::RtcConductor();


				onError = gcnew _OnErrorCallback(this, &SpitfireRtc::_OnError);
				onErrorHandle = GCHandle::Alloc(onError);
				_conductor->onError = static_cast<Spitfire::OnErrorCallbackNative>(Marshal::GetFunctionPointerForDelegate(onError).ToPointer());

				onSuccess = gcnew _OnSuccessCallback(this, &SpitfireRtc::_OnSuccess);
				onSuccessHandle = GCHandle::Alloc(onSuccess);
				_conductor->onSuccess = static_cast<Spitfire::OnSuccessCallbackNative>(Marshal::GetFunctionPointerForDelegate(onSuccess).ToPointer());

				onFailure = gcnew _OnFailureCallback(this, &SpitfireRtc::_OnFailure);
				onFailureHandle = GCHandle::Alloc(onFailure);
				_conductor->onFailure = static_cast<Spitfire::OnFailureCallbackNative>(Marshal::GetFunctionPointerForDelegate(onFailure).ToPointer());

				onDataMessage = gcnew _OnDataMessageCallback(this, &SpitfireRtc::_OnDataMessage);
				onDataMessageHandle = GCHandle::Alloc(onDataMessage);
				_conductor->onDataMessage = static_cast<Spitfire::OnDataMessageCallbackNative>(Marshal::GetFunctionPointerForDelegate(onDataMessage).ToPointer());

				onDataBinaryMessage = gcnew _OnDataBinaryMessageCallback(this, &SpitfireRtc::_OnDataBinaryMessage);
				onDataBinaryMessageHandle = GCHandle::Alloc(onDataBinaryMessage);
				_conductor->onDataBinaryMessage = static_cast<Spitfire::OnDataBinaryMessageCallbackNative>(Marshal::GetFunctionPointerForDelegate(onDataBinaryMessage).ToPointer());

				onIceCandidate = gcnew _OnIceCandidateCallback(this, &SpitfireRtc::_OnIceCandidate);
				onIceCandidateHandle = GCHandle::Alloc(onIceCandidate);
				_conductor->onIceCandidate = static_cast<Spitfire::OnIceCandidateCallbackNative>(Marshal::GetFunctionPointerForDelegate(onIceCandidate).ToPointer());


				onDataChannelStateChange = gcnew _OnDataChannelStateCallback(this, &SpitfireRtc::_OnDataChannelState);
				onDataChannelStateHandle = GCHandle::Alloc(onDataChannelStateChange);
				_conductor->onDataChannelState = static_cast<Spitfire::OnDataChannelStateCallbackNative>(Marshal::GetFunctionPointerForDelegate(onDataChannelStateChange).ToPointer());

				onIceStateChange = gcnew _OnIceStateCallback(this, &SpitfireRtc::_OnIceState);
				onIceStateCallbackHandle = GCHandle::Alloc(onIceStateChange);
				_conductor->onIceStateChange = static_cast<Spitfire::OnIceStateChangeCallbackNative>(Marshal::GetFunctionPointerForDelegate(onIceStateChange).ToPointer());
			}

			~SpitfireRtc()
			{
				if (m_isDisposed)
					return;

				

				// dispose managed data
				FreeGCHandle(onErrorHandle);
				FreeGCHandle(onSuccessHandle);
				FreeGCHandle(onFailureHandle);
				FreeGCHandle(onIceCandidateHandle);
				FreeGCHandle(onDataMessageHandle);
				FreeGCHandle(onDataBinaryMessageHandle);
				FreeGCHandle(onDataChannelStateHandle);

				this->!SpitfireRtc(); // call finalizer

				m_isDisposed = true;
			}

			static void EnableLogging()
			{
				Spitfire::EnableLogging();
			}

			static void InitializeSSL()
			{
				Spitfire::InitializeSSL();
			}

			static void CleanupSSL()
			{
				Spitfire::CleanupSSL();
			}

			bool InitializePeerConnection()
			{
				return _conductor->InitializePeerConnection();
			}

			void CreateOffer()
			{
				_conductor->CreateOffer();
			}

			bool ProcessMessages(Int32 delay)
			{
				return _conductor->ProcessMessages(delay);
			}

			void OnOfferReply(String ^ type, String ^ sdp)
			{
				_conductor->OnOfferReply(marshal_as<std::string>(type), marshal_as<std::string>(sdp));
			}

			void OnOfferRequest(String ^ sdp)
			{
				_conductor->OnOfferRequest(marshal_as<std::string>(sdp));
			}

			bool AddIceCandidate(String ^ sdp_mid, Int32 sdp_mlineindex, String ^ sdp)
			{
				return _conductor->AddIceCandidate(marshal_as<std::string>(sdp_mid), sdp_mlineindex, marshal_as<std::string>(sdp));
			}

			void AddServerConfig(String ^ uri, String ^ username, String ^ password)
			{
				_conductor->AddServerConfig(marshal_as<std::string>(uri), marshal_as<std::string>(username), marshal_as<std::string>(password));
			}

			void CreateDataChannel(String ^ label)
			{
				_conductor->CreateDataChannel(marshal_as<std::string>(label));
			}

			void DataChannelSendText(String ^ text)
			{
				_conductor->DataChannelSendText(marshal_as<std::string>(text));
			}

			void DataChannelSendData(array<Byte>^ array_data)
			{
				pin_ptr<uint8_t> thePtr = &array_data[0];
				uint8_t * bPtr = thePtr;
				rtc::CopyOnWriteBuffer writeBuffer(bPtr, array_data->Length);
				_conductor->DataChannelSendData(webrtc::DataBuffer(writeBuffer, true));
			}

		protected:

			!SpitfireRtc()
			{
				// free unmanaged data
				if (_conductor != NULL)
				{
					delete _conductor;
				}
				_conductor = NULL;
			}
		};
	}
