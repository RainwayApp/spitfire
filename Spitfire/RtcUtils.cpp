#include "targetver.h"

#include  "RtcUtils.h"

#pragma comment(lib,"crypt32.lib")
#pragma comment(lib,"iphlpapi.lib")
#pragma comment(lib,"secur32.lib")
#pragma comment(lib,"winmm.lib")
#pragma comment(lib,"dmoguids.lib")
#pragma comment(lib,"wmcodecdspuuid.lib")
#pragma comment(lib,"amstrmid.lib")
#pragma comment(lib,"msdmo.lib")
#pragma comment(lib,"Strmiids.lib")

// common
#pragma comment(lib,"wininet.lib")
#pragma comment(lib,"dnsapi.lib")
#pragma comment(lib,"version.lib")
#pragma comment(lib,"msimg32.lib")
#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"usp10.lib")
#pragma comment(lib,"psapi.lib")
#pragma comment(lib,"dbghelp.lib")
#pragma comment(lib,"shlwapi.lib")
#pragma comment(lib,"kernel32.lib")
#pragma comment(lib,"gdi32.lib")
#pragma comment(lib,"winspool.lib")
#pragma comment(lib,"comdlg32.lib")
#pragma comment(lib,"advapi32.lib")
#pragma comment(lib,"shell32.lib")
#pragma comment(lib,"ole32.lib")
#pragma comment(lib,"oleaut32.lib")
#pragma comment(lib,"user32.lib")
#pragma comment(lib,"uuid.lib")
#pragma comment(lib,"odbc32.lib")
#pragma comment(lib,"odbccp32.lib")
#pragma comment(lib,"delayimp.lib")
#pragma comment(lib,"credui.lib")
#pragma comment(lib,"netapi32.lib")

// internal



#pragma comment(lib,"ana_config_proto.lib")
#pragma comment(lib,"ana_debug_dump_proto.lib")
#pragma comment(lib,"audio.lib")
#pragma comment(lib,"audioproc_debug_proto.lib")
#pragma comment(lib,"audioproc_protobuf_utils.lib")
#pragma comment(lib,"audioproc_unittest_proto.lib")
#pragma comment(lib,"audio_coder.lib")
#pragma comment(lib,"audio_coding.lib")
#pragma comment(lib,"audio_conference_mixer.lib")
#pragma comment(lib,"audio_decoder_g722.lib")
#pragma comment(lib,"audio_decoder_ilbc.lib")
#pragma comment(lib,"audio_decoder_opus.lib")
#pragma comment(lib,"audio_device.lib")
#pragma comment(lib,"audio_encoder_g722.lib")
#pragma comment(lib,"audio_encoder_ilbc.lib")
#pragma comment(lib,"audio_encoder_opus_config.lib")
#pragma comment(lib,"audio_format_conversion.lib")
#pragma comment(lib,"audio_frame_manipulator.lib")
#pragma comment(lib,"audio_frame_operations.lib")
#pragma comment(lib,"audio_level.lib")
#pragma comment(lib,"audio_mixer_impl.lib")
#pragma comment(lib,"audio_network_adaptor.lib")
#pragma comment(lib,"audio_processing.lib")
#pragma comment(lib,"audio_processing_sse2.lib")
#pragma comment(lib,"bitrate_controller.lib")
#pragma comment(lib,"boringssl.lib")
#pragma comment(lib,"boringssl_asm.lib")
#pragma comment(lib,"builtin_audio_decoder_factory.lib")
#pragma comment(lib,"builtin_audio_decoder_factory_internal.lib")
#pragma comment(lib,"builtin_audio_encoder_factory.lib")
#pragma comment(lib,"builtin_audio_encoder_factory_internal.lib")
#pragma comment(lib,"bwe_simulator_lib.lib")
#pragma comment(lib,"call.lib")
#pragma comment(lib,"chart_proto.lib")
#pragma comment(lib,"cng.lib")
#pragma comment(lib,"command_line_parser.lib")
#pragma comment(lib,"common_audio.lib")
#pragma comment(lib,"common_audio_sse2.lib")
#pragma comment(lib,"common_video.lib")
#pragma comment(lib,"congestion_controller.lib")
#pragma comment(lib,"create_pc_factory.lib")
#pragma comment(lib,"dl.lib")
#pragma comment(lib,"event_log_visualizer_utils.lib")
#pragma comment(lib,"field_trial_default.lib")
#pragma comment(lib,"file_player.lib")
#pragma comment(lib,"file_recorder.lib")
#pragma comment(lib,"frame_editing_lib.lib")
#pragma comment(lib,"g711.lib")
#pragma comment(lib,"g722.lib")
#pragma comment(lib,"gtest.lib")
#pragma comment(lib,"ilbc.lib")
#pragma comment(lib,"isac.lib")
#pragma comment(lib,"isac_c.lib")
#pragma comment(lib,"isac_common.lib")
#pragma comment(lib,"isac_fix.lib")
#pragma comment(lib,"legacy_encoded_audio_frame.lib")
#pragma comment(lib,"lib.lib")
#pragma comment(lib,"libjingle_peerconnection_api.lib")
#pragma comment(lib,"libjpeg.lib")
#pragma comment(lib,"libsrtp.lib")
#pragma comment(lib,"libstunprober.lib")
#pragma comment(lib,"libvpx.lib")
#pragma comment(lib,"libvpx_yasm.lib")
#pragma comment(lib,"libyuv_internal.lib")
#pragma comment(lib,"media_file.lib")
#pragma comment(lib,"metrics_default.lib")
#pragma comment(lib,"neteq.lib")
#pragma comment(lib,"neteq_unittest_proto.lib")
#pragma comment(lib,"network_tester.lib")
#pragma comment(lib,"network_tester_config_proto.lib")
#pragma comment(lib,"network_tester_packet_proto.lib")
#pragma comment(lib,"opus.lib")
#pragma comment(lib,"ortc.lib")
#pragma comment(lib,"pacing.lib")
#pragma comment(lib,"pcm16b.lib")
#pragma comment(lib,"peerconnection.lib")
#pragma comment(lib,"primitives.lib")
#pragma comment(lib,"protobuf_full.lib")
#pragma comment(lib,"protobuf_lite.lib")
#pragma comment(lib,"protoc_lib.lib")
#pragma comment(lib,"red.lib")
#pragma comment(lib,"reference_less_video_analysis_lib.lib")
#pragma comment(lib,"remote_bitrate_estimator.lib")
#pragma comment(lib,"rent_a_codec.lib")
#pragma comment(lib,"rtc_audio_video.lib")
#pragma comment(lib,"rtc_base.lib")
#pragma comment(lib,"rtc_base_approved.lib")
#pragma comment(lib,"rtc_data.lib")
#pragma comment(lib,"rtc_event_log_impl.lib")
#pragma comment(lib,"rtc_event_log_parser.lib")
#pragma comment(lib,"rtc_event_log_proto.lib")
#pragma comment(lib,"rtc_event_log_source.lib")
#pragma comment(lib,"rtc_media_base.lib")
#pragma comment(lib,"rtc_numerics.lib")
#pragma comment(lib,"rtc_p2p.lib")
#pragma comment(lib,"rtc_pc_base.lib")
#pragma comment(lib,"rtc_stats.lib")
#pragma comment(lib,"rtc_task_queue.lib")
#pragma comment(lib,"rtp_rtcp.lib")
#pragma comment(lib,"sequenced_task_checker.lib")
#pragma comment(lib,"simd.lib")
#pragma comment(lib,"simd_asm.lib")
#pragma comment(lib,"system_wrappers.lib")
#pragma comment(lib,"usrsctp.lib")
#pragma comment(lib,"utility.lib")
#pragma comment(lib,"video.lib")
#pragma comment(lib,"video_capture.lib")
#pragma comment(lib,"video_capture_internal_impl.lib")
#pragma comment(lib,"video_capture_module.lib")
#pragma comment(lib,"video_coding.lib")
#pragma comment(lib,"video_coding_utility.lib")
#pragma comment(lib,"video_processing.lib")
#pragma comment(lib,"video_processing_sse2.lib")
#pragma comment(lib,"video_quality_analysis.lib")
#pragma comment(lib,"voice_engine.lib")
#pragma comment(lib,"weak_ptr.lib")
#pragma comment(lib,"webrtc.lib")
#pragma comment(lib,"webrtc_common.lib")
#pragma comment(lib,"webrtc_h264.lib")
#pragma comment(lib,"webrtc_i420.lib")
#pragma comment(lib,"webrtc_opus.lib")
#pragma comment(lib,"webrtc_vp8.lib")
#pragma comment(lib,"webrtc_vp9.lib")
#pragma comment(lib,"winsdk_samples.lib")
#pragma comment(lib,"yasm_utils.lib")



#include "webrtc/base/ssladapter.h"
#include "webrtc/base/win32socketinit.h"
#include "webrtc/base/win32socketserver.h"

namespace Spitfire
{

	void InitializeSSL()
	{
		rtc::EnsureWinsockInit();
		rtc::InitializeSSL(nullptr);
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