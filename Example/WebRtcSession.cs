using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using Spitfire;
using SpitfireUtils;

namespace Example
{
    class WebRtcSession
    {
        public WebRtcSession(string id)
        {
            Id = id;
            Spitfire = new SpitfireRtc();
            Token = new CancellationTokenSource();
        }

        public string Id { get; set; }

        public SpitfireRtc Spitfire { get; set; }
        public readonly CancellationTokenSource Token;

        public void BeginLoop(ManualResetEvent go)
        {
            //Call this before starting a peer connection
            SpitfireRtc.InitializeSSL();
            //Add a stun server
            Spitfire.AddServerConfig(new ServerConfig
            {
                Host = "stun.l.google.com",
                Port = 19302,
                Type = ServerType.Stun,
            });

            var started = Spitfire.InitializePeerConnection();
            if (started)
            {
                go.Set();
                //Keeps the RTC thread alive and active
                while (!Token.Token.IsCancellationRequested && Spitfire.ProcessMessages(1000))
                {
                    Spitfire.ProcessMessages(1000);
                }
                //Do cleanup here
                Console.WriteLine("WebRTC message loop is dead.");
            }
        }

        public void Setup(string sdp)
        {
            Spitfire.OnDataChannelOpen += DataChannelOpen;
            Spitfire.OnDataChannelClose += SpitfireOnOnDataChannelClose;
            Spitfire.OnBufferAmountChange += SpitfireOnOnBufferAmountChange;
            Spitfire.OnDataMessage += HandleMessage;
            Spitfire.OnIceStateChange += IceStateChange;
            Spitfire.OnSuccessAnswer += OnSuccessAnswer;
            Spitfire.OnIceCandidate += SpitfireOnOnIceCandidate;
            //Gives your newly created peer connection the remote clients SDP
            Spitfire.SetOfferRequest(sdp);
        }

        private void SpitfireOnOnIceCandidate(SpitfireIceCandidate iceCandidate)
        {
            var parsed = IceParser.Parse(iceCandidate.Sdp);
            //Reply to the remote client with your ICE information (sdp, sdpMid, sdpIndex)
        }

        private void OnSuccessAnswer(SpitfireSdp sdp)
        {
            //reply to the remote client with your SDP
        }

        private void IceStateChange(IceConnectionState state)
        {
            if (state == IceConnectionState.Disconnected)
            {
                Console.WriteLine("ICE has left the building.");
            }
        }

        private void HandleMessage(string label, DataMessage msg)
        {
            if (msg.IsBinary)
            {
                Console.WriteLine(msg.RawData.Length);
            }
            else
            {
                Console.WriteLine(msg.Data);
            }
        }

        private void SpitfireOnOnBufferAmountChange(string label, int previousBufferAmount, int currentBufferAmount,
            int bytesSent,
            int bytesReceived)
        {
        }

        private void SpitfireOnOnDataChannelClose(string label)
        {
            Console.WriteLine("Data Channel Closed!");
        }

        private void DataChannelOpen(string label)
        {
            Console.WriteLine("$Data Channel Opened!");
            Console.WriteLine(Spitfire.GetDataChannelInfo(label).Reliable);
        }
    }
}