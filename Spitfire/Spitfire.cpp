// This is the main DLL file.

#include "Spitfire.h"
#include "webrtc/rtc_base/thread.h"
#include "webrtc/p2p/base/basicpacketsocketfactory.h"
#include "webrtc/api/peerconnectioninterface.h"
#include "webrtc/api/test/fakeconstraints.h"
#include "webrtc/media/engine/webrtcvideocapturerfactory.h"


void Spitfire::Class1::HelloWorld()
{
	rtc::EnsureWinsockInit();
	rtc::InitializeSSL(NULL);
	// logging
	rtc::LogMessage::LogToDebug(rtc::LS_VERBOSE); // LS_VERBOSE, LS_INFO, LERROR

	rtc::InitializeSSL();

	// something from base
	rtc::Thread* thread = rtc::Thread::Current();

	// something from p2p
	std::unique_ptr<rtc::BasicPacketSocketFactory> socket_factory(
		new rtc::BasicPacketSocketFactory());

	// something from api
	rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface>
		peer_connection_factory = webrtc::CreatePeerConnectionFactory();

	if (!peer_connection_factory)
	{
		Console::WriteLine("Docls");
	}

	// something from api/test
	webrtc::FakeConstraints constraints;

	// something from media/engine
	cricket::WebRtcVideoDeviceCapturerFactory factory;
	auto capturer = factory.Create(cricket::Device("", 0));
	// cricket::VideoCapturer* capturer = factory.Create(cricket::Device("", 0));

	Console::WriteLine("We Did It Reddit!");
}
