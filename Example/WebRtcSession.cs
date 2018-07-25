using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using Spitfire.Net;

namespace Example
{
    public class WebRtcSession
    {
        public WebRtcSession(string id)
        {
            Id = id;
            Spitfire = new SpitfireRtc(44110, 44113);
            Token = new CancellationTokenSource();
        }

        public string Id { get; set; }
        public bool IsConnected { get; set; }
        public SpitfireRtc Spitfire { get; set; }
        public readonly CancellationTokenSource Token;
        public event EventHandler<SpitfireIceCandidateEventArgs> OnIceCandidateFound;
        public event EventHandler DataChannelOpened;

        public class SpitfireIceCandidateEventArgs : EventArgs
        {
            public SpitfireIceCandidate IceCandidate;
            public SpitfireIceCandidateEventArgs(SpitfireIceCandidate iceCandidate)
            {
                this.IceCandidate = iceCandidate;
            }
        }

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
            Spitfire.OnFailure += Spitfire_OnFailure;
            Spitfire.OnDataChannelOpen += DataChannelOpen;
            Spitfire.OnDataChannelClose += SpitfireOnOnDataChannelClose;
            Spitfire.OnDataChannelConnecting += Spitfire_OnDataChannelConnecting;
            Spitfire.OnDataChannelClosing += Spitfire_OnDataChannelClosing;
            //Spitfire.OnBufferAmountChange += SpitfireOnOnBufferAmountChange;
            Spitfire.OnDataMessage += Spitfire_OnDataMessage;
            Spitfire.OnDataBinaryMessage += Spitfire_OnDataBinaryMessage;
            //Spitfire.OnIceStateChange += IceStateChange;
            Spitfire.OnSuccessAnswer += OnSuccessAnswer;
            Spitfire.OnIceCandidate += SpitfireOnOnIceCandidate;
            //Gives your newly created peer connection the remote clients SDP
            Spitfire.SetOfferRequest(sdp);
        }

        private void Spitfire_OnDataBinaryMessage(string label, byte[] data)
        {
            var msg = new DataMessage();
            msg.IsBinary = true;
            msg.RawData = data;
            HandleMessage("test", msg);
        }

        private void Spitfire_OnDataMessage(string label, string message)
        {
            var msg = new DataMessage();
            msg.IsText = true;
            msg.Data = message;
            HandleMessage("test", msg);
        }

        public void Setup()
        {
            Spitfire.OnFailure += Spitfire_OnFailure;
            Spitfire.OnSuccessOffer += Spitfire_OnSuccessOffer;
            Spitfire.OnDataChannelOpen += DataChannelOpen;
            Spitfire.OnDataChannelClose += SpitfireOnOnDataChannelClose;
            Spitfire.OnDataChannelConnecting += Spitfire_OnDataChannelConnecting;
            Spitfire.OnDataChannelClosing += Spitfire_OnDataChannelClosing;
            //Spitfire.OnBufferAmountChange += SpitfireOnOnBufferAmountChange;
            Spitfire.OnDataMessage += Spitfire_OnDataMessage;
            Spitfire.OnDataBinaryMessage += Spitfire_OnDataBinaryMessage;
            //Spitfire.OnDataMessage += HandleMessage;
            //Spitfire.OnIceStateChange += IceStateChange;
            Spitfire.OnSuccessAnswer += OnSuccessAnswer;
            Spitfire.OnIceCandidate += SpitfireOnOnIceCandidate;
            //Gives your newly created peer connection the remote clients SDP
            //Spitfire.SetOfferRequest(sdp);
            Spitfire.CreateDataChannel(new DataChannelOptions()
            {
                Label = "default"
            });
            Spitfire.CreateOffer();
        }

        private void Spitfire_OnDataChannelClosing(string label)
        {
            
        }

        private void Spitfire_OnDataChannelConnecting(string label)
        {
            
        }

        private void Spitfire_OnError()
        {
            
        }

        private void Spitfire_OnFailure(string error)
        {
            
        }

        private void Spitfire_OnSuccessOffer(SpitfireSdp sdp)
        {
            this.OfferStp = sdp;
        }

        private void SpitfireOnOnIceCandidate(SpitfireIceCandidate iceCandidate)
        {
            //var parsed = IceParser.Parse(iceCandidate.Sdp);
            OnIceCandidateFound?.Invoke(this, new SpitfireIceCandidateEventArgs(iceCandidate));
            //Reply to the remote client with your ICE information (sdp, sdpMid, sdpIndex)
        }

        private void OnSuccessAnswer(SpitfireSdp sdp)
        {
            AnswerSdp = sdp;
        }

        public SpitfireSdp OfferStp;
        public SpitfireSdp AnswerSdp;

        private void IceStateChange(IceConnectionState state)
        {
            if (state == IceConnectionState.Disconnected)
            {
                Console.WriteLine("ICE has left the building.");
            }
            else
            {
                Console.WriteLine("Ice State:" + state);
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
            IsConnected = false;
            Console.WriteLine("Data Channel Closed!");
        }

        private void DataChannelOpen(string label)
        {
            DataChannelOpened?.Invoke(this, EventArgs.Empty);
            IsConnected = true;
            Console.WriteLine("$Data Channel Opened!");
            //Console.WriteLine(Spitfire.GetDataChannelInfo(label).Reliable);
        }
    }
}