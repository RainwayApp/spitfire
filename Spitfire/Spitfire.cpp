// This is the main DLL file.

#include "Spitfire.h"

	
Spitfire::WebRtc::WebRtc()
{
	Console::WriteLine("Test!");
	 myCallback = nullptr;
	_connectionManager = new WebRtcConnectionManager();
	
}

Spitfire::WebRtc::~WebRtc()
{
	CleanupSSL();
}
void Spitfire::WebRtc::CleanupSSL()
{
	rtc::CleanupSSL();
	if (myCallback != nullptr)
	{
		myCallback("Clean up from Dispose via Callback!");
	}
}


void Spitfire::WebRtc::SetCallback(Action<String^>^ callback)
{
	if (myCallback == nullptr)
	{
		myCallback = callback;
	}
}


void Spitfire::WebRtc::InitializeSSL()
{
	rtc::EnsureWinsockInit();
	rtc::InitializeSSL(NULL);
}

void Spitfire::WebRtc::HelloWorld()
{
	Console::WriteLine("Kick!");
}
