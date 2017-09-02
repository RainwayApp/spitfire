// Spitfire.h

#include "WebRtcConnectionManager.h"

#pragma once


using namespace System;

namespace Spitfire {

	


	public ref class WebRtc  
	{
	public:
		void SetCallback(Action<String^>^ callback);
		WebRtc();
		~WebRtc();

		void HelloWorld();

		void InitializeSSL();

		void CleanupSSL();

	private:
		WebRtcConnectionManager* _connectionManager;
		Action<String^>^ myCallback;
	};
}
