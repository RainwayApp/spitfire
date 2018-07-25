// dllmain.cpp : Defines the entry point for the DLL application.
#include "internals.h"
#include "RtcConductor.h"

#include "rtc_base/checks.h"
#include "api/test/fakeconstraints.h"

// for servers
#include "p2p/base/relayserver.h"
#include "p2p/base/stunserver.h"
#include "p2p/base/basicpacketsocketfactory.h"
#include "p2p/base/turnserver.h"
#include "rtc_base/asyncudpsocket.h"
#include "rtc_base/optionsfile.h"
#include "rtc_base/stringencode.h"
#include "rtc_base/thread.h"

//#define L_LITTLE_ENDIAN
//#include "leptonica/allheaders.h"
//#include "turbojpeg/turbojpeg.h"
#include "atlsafe.h"


namespace Native
{
	bool CFG_quality_scaler_enabled_ = false;

	void InitializeSSL()
	{
		rtc::EnsureWinsockInit();
		rtc::InitializeSSL(NULL);
	}

	void CleanupSSL()
	{
		rtc::CleanupSSL();
	}
}

extern "C"
{
	__declspec(dllexport) void * WINAPI NewConductor()
	{
		return new Spitfire::RtcConductor();
	}

	__declspec(dllexport) void WINAPI DeleteConductor(Spitfire::RtcConductor * cd)
	{
		delete cd;
	}

	__declspec(dllexport) void WINAPI InitializeSSL()
	{
		Native::InitializeSSL();
	}

	__declspec(dllexport) void WINAPI CleanupSSL()
	{
		Native::CleanupSSL();
	}

	//------------------------------------------------

	//__declspec(dllexport) void WINAPI onRenderLocal(Spitfire::RtcConductor * cd, Spitfire::OnRenderCallbackNative callback)
	//{
	//	cd->onRenderLocal = callback;
	//}

	//__declspec(dllexport) void WINAPI onRenderRemote(Spitfire::RtcConductor * cd, Spitfire::OnRenderCallbackNative callback)
	//{
	//	cd->onRenderRemote = callback;
	//}

	__declspec(dllexport) void WINAPI onError(Spitfire::RtcConductor * cd, Spitfire::OnErrorCallbackNative callback)
	{
		cd->onError = callback;
	}

	__declspec(dllexport) void WINAPI onSuccess(Spitfire::RtcConductor * cd, Spitfire::OnSuccessCallbackNative callback)
	{
		cd->onSuccess = callback;
	}

	__declspec(dllexport) void WINAPI onFailure(Spitfire::RtcConductor * cd, Spitfire::OnFailureCallbackNative callback)
	{
		cd->onFailure = callback;
	}

	__declspec(dllexport) void WINAPI onDataMessage(Spitfire::RtcConductor * cd, Spitfire::OnDataMessageCallbackNative callback)
	{
		cd->onDataMessage = callback;
	}

	__declspec(dllexport) void WINAPI onDataBinaryMessage(Spitfire::RtcConductor * cd, Spitfire::OnDataBinaryMessageCallbackNative callback)
	{
		cd->onDataBinaryMessage = callback;
	}

	__declspec(dllexport) void WINAPI onIceCandidate(Spitfire::RtcConductor * cd, Spitfire::OnIceCandidateCallbackNative callback)
	{
		cd->onIceCandidate = callback;
	}

	__declspec(dllexport) void WINAPI onDataChannelStateChange(Spitfire::RtcConductor * cd, Spitfire::OnDataChannelStateCallbackNative callback)
	{
		cd->onDataChannelState = callback;
	}

	//------------------------------------------------

	__declspec(dllexport) bool WINAPI InitializePeerConnection(Spitfire::RtcConductor * cd, int minPort, int maxPort)
	{
		return cd->InitializePeerConnection(minPort, maxPort);
	}

	__declspec(dllexport) void WINAPI CreateOffer(Spitfire::RtcConductor * cd)
	{
		cd->CreateOffer();
	}

	__declspec(dllexport) bool WINAPI ProcessMessages(Spitfire::RtcConductor * cd, int delay)
	{
		return cd->ProcessMessages(delay);
	}

	__declspec(dllexport) void WINAPI SetOfferReply(Spitfire::RtcConductor * cd, const char * type, const char * sdp)
	{
		cd->OnOfferReply(type, sdp);
	}

	__declspec(dllexport) void WINAPI SetOfferRequest(Spitfire::RtcConductor * cd, const char * sdp)
	{
		cd->OnOfferRequest(sdp);
	}

	__declspec(dllexport) bool WINAPI AddIceCandidate(Spitfire::RtcConductor * cd, const char * sdp_mid, int sdp_mlineindex, const char * sdp)
	{
		return cd->AddIceCandidate(sdp_mid, sdp_mlineindex, sdp);
	}

	__declspec(dllexport) void WINAPI AddServerConfig(Spitfire::RtcConductor * cd, const char * uri, const char * username, const char * password)
	{
		cd->AddServerConfig(uri, username, password);
	}

	__declspec(dllexport) void WINAPI CreateDataChannel(Spitfire::RtcConductor * cd, const char * label)
	{
		webrtc::DataChannelInit dc_options;
		//dc_options.id = 1;
		dc_options.maxRetransmits = 1;
		dc_options.negotiated = false;
		dc_options.ordered = false;
		cd->CreateDataChannel(label, dc_options);
	}

	__declspec(dllexport) void WINAPI DataChannelSendText(Spitfire::RtcConductor * cd, const char * label, const char * text)
	{
		cd->DataChannelSendText(label, text);
	}

	__declspec(dllexport) void WINAPI DataChannelSendData(Spitfire::RtcConductor * cd, const char * label, uint8_t * array_data, int length)
	{
		rtc::CopyOnWriteBuffer writeBuffer(array_data, length);
		cd->DataChannelSendData(label, webrtc::DataBuffer(writeBuffer, true));
	}


	//__declspec(dllexport) bool WINAPI RunStunServer(Spitfire::RtcConductor * cd, const char * bindIp)
	//{
	//	return cd->RunStunServer(bindIp);
	//}

	//// File is stored as lines of <username>=<HA1>.
	//// Generate HA1 via "echo -n "<username>:<realm>:<password>" | md5sum"
	//__declspec(dllexport) bool WINAPI RunTurnServer(Spitfire::RtcConductor * cd, const char * bindIp, const char * ip, const char * realm, const char * authFile)
	//{
	//	return cd->RunTurnServer(bindIp, ip, realm, authFile);
	//}
}