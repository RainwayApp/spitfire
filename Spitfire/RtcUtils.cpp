#include "targetver.h"
#include  "RtcUtils.h"

#include "rtc_base\ssl_adapter.h"
#include "rtc_base\win32_socket_init.h"
#include "rtc_base\win32_socket_server.h"
#include "rtc_base\logging.h"
#include <rtc_base\time_utils.h>
#include <rtc_base\helpers.h>

namespace Spitfire
{
	void InitializeSSL()
	{
		rtc::WinsockInitializer();
		rtc::InitializeSSL();
		rtc::InitRandom(static_cast<int>(rtc::Time()));
	}
	void EnableLogging()
	{
		rtc::LogMessage::LogTimestamps();
		rtc::LogMessage::LogThreads();
		rtc::LogMessage::LogToDebug(rtc::LS_VERBOSE);
		rtc::LogMessage::SetLogToStderr(true);
	}
	void CleanupSSL()
	{
		rtc::CleanupSSL();
	}
}

FILE _iob[] = { *stdin, *stdout, *stderr };

extern "C" FILE * __cdecl __iob_func()
{
	return _iob;
}