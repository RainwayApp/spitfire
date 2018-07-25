using System;
using System.Collections.Generic;
using System.Text;

namespace Spitfire.Net
{
    /// <summary>
	/// <seealso href="http://w3c.github.io/webrtc-pc/#idl-def-RTCDataChannelState"/>
	/// </summary>
	public enum DataChannelState
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
    public enum IceConnectionState
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

    public enum SdpTypes
    {
        Answer,
		Offer
    }

    public enum ServerType
    {
        Stun,
		Turn
    }

    public class SpitfireSdp
    {
        public string Sdp { get; set; }
        public SdpTypes Type { get; set; }
    }

    public class SpitfireIceCandidate
    {
        public string SdpMid { get; set; }
		public int SdpIndex { get; set; }
        public string Sdp { get; set; }
    }

    public class DataMessage
    {
        public bool IsBinary;
        public bool IsText;
        public byte[] RawData;
		public string Data;
	}

    public class DataChannelOptions
    {        
		 /// <summary>
		 /// The name of your data channel
		 /// </summary>
		public string Label;

		 /// <summary>
		 /// Deprecated. Reliability is assumed, and channel will be unreliable if
		 /// MaxRetransmitTime or MaxRetransmits is set.
		 /// </summary>
		public bool Reliable = false;

        /// <summary>
        /// True if ordered delivery is required.
        /// </summary>
        public bool Ordered = true;

        /// <summary>
        /// The max period of time in milliseconds in which retransmissions will be
        /// sent. After this time, no more retransmissions will be sent. -1 if unset.
        /// Cannot be set along with |MaxRetransmits|.
        /// </summary>
        public int MaxRetransmitTime = -1;

        /* 
		 * The max number of retransmissions. -1 if unset.
		 * Cannot be set along with |MaxRetransmitTime|.
		 */
        /// <summary>
        /// The max number of retransmissions. -1 if unset.
        /// Cannot be set along with |MaxRetransmitTime|.
        /// </summary>
        public int MaxRetransmits = -1;
        /// <summary>
        /// This is set by the application and opaque to the WebRTC implementation.
        /// </summary>
        public string Protocol;
		 /// <summary>
		 /// True if the channel has been externally negotiated and we do not send an
		 /// in-band signalling in the form of an "open" message. If this is true, Id
		 /// must be set; otherwise it should be unset and will be negotiated
		 /// </summary>
		public bool Negotiated = false;

        /// <summary>
        ///  The stream id, or SID, for SCTP data channels. -1 if unset (see Negotiated).
        /// </summary>
        public int Id = -1;
    }

    public class ServerConfig
    {
        public ServerType Type;
        public string Host;
		public ushort Port;
        public string Username;
		public string Password;
	}
}
