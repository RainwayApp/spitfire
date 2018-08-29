using System;
using System.Collections.Generic;
using System.Threading;
using System.Threading.Tasks;
using Spitfire.Net;


namespace Example
{
    class Program
    {
        static void Main(string[] args)
        {
            //This is a bare bones example that shows the logic in which one might
            //Implement Spitfire into their application
            //TODO a full fledged example
            //SpitfireRtc.EnableLogging();

            Thread th = new Thread(() =>
            {
                SecondSession();
            });
            th.Start();

            Thread th2 = new Thread(() =>
            {
                SecondSession2();
            });
            th2.Start();


            Console.Read();
        }

        public class EventMessageEventArgs : EventArgs
        {
            public Message Msg;
            public EventMessageEventArgs(Message msg)
            {
                this.Msg = msg;
            }
        }

        public static event EventHandler<EventMessageEventArgs> EventMessage;

        private static void SecondSession()
        {
            Guid id = Guid.NewGuid();

            var session = WebRtcManager.AddSession(id.ToString());

            ManualResetEvent offerEvt = new ManualResetEvent(false);
            string offer = string.Empty;

            EventMessage += (s, ex) =>
            {
                if (ex.Msg.Source == id)
                {
                    return;
                }
                else if (ex.Msg.Sdp.Type == SdpTypes.Answer)
                {                
                    session.Spitfire.SetOfferReply(ex.Msg.Sdp.Type.ToString().ToLower(), ex.Msg.Sdp.Sdp);
                }
            };
            session.OnIceCandidateFound += (s, ex) =>
            {
                EventMessage.Invoke(null, new EventMessageEventArgs(new Message(id, null, null, ex.IceCandidate)));
            };
            session.Spitfire.OnSuccessOffer += (ex) =>
            {
                offer = ex.Sdp;
                offerEvt.Set();
            };
            session.DataChannelOpened += (s, ex) =>
            {
                session.Spitfire.DataChannelSendText("default", "Hello World!!! Data channel is open");
            };
            session.Spitfire.CreateOffer();
            offerEvt.WaitOne();
            EventMessage.Invoke(null, new EventMessageEventArgs(new Message(id, offer)));
            offerEvt.Reset();
            offerEvt.WaitOne();       
        }

        private static void SecondSession2()
        {
            Guid id = Guid.NewGuid();

            ManualResetEvent offerEvt = new ManualResetEvent(false);
            ManualResetEvent waitEvt = new ManualResetEvent(false);

            string offer = string.Empty;
            WebRtcSession session = null;
            List<SpitfireIceCandidate> candidates = new List<SpitfireIceCandidate>();
            EventMessage += (s, ex) =>
            {
                if (ex.Msg.Source == id)
                {
                    return;
                }

                if (session == null && ex.Msg.IceCandidate != null)
                {
                    candidates.Add(ex.Msg.IceCandidate);
                }
                else if (session == null)
                {
                    offerEvt.Set();
                    offer = ex.Msg.Msg;
                }
                else if (ex.Msg.IceCandidate != null)
                {
                    if (!session.Spitfire.AddIceCandidate(ex.Msg.IceCandidate.SdpMid, ex.Msg.IceCandidate.SdpIndex, ex.Msg.IceCandidate.Sdp) )
                    {
                        Console.WriteLine("Unable to add");
                    }
                }
            };
            offerEvt.WaitOne();

            session = WebRtcManager.AddSession(id.ToString(), offer);
            session.Spitfire.OnDataMessage += (s, ex) =>
            {
                Console.WriteLine("Meesage from peer:" + ex);
            };
            while (session.AnswerSdp == null)
            {
                Thread.Sleep(100);
            }
            EventMessage.Invoke(null, new EventMessageEventArgs(new Message(id, "Answer", session.AnswerSdp)));

            if (candidates.Count > 0)
            {
                foreach(var c in candidates)
                {
                    if (!session.Spitfire.AddIceCandidate(c.SdpMid, c.SdpIndex, c.Sdp))
                    {
                        Console.WriteLine("Unable to add");
                    }
                }
            }

            while (true)
            {
                if (waitEvt.WaitOne(100))
                {
                    break;
                }
            }
        }

        public class Message
        {
            public Guid Source;
            public string Msg;
            public SpitfireSdp Sdp;
            public SpitfireIceCandidate IceCandidate;
            public Message(Guid id, string message, SpitfireSdp sdp = null, SpitfireIceCandidate iceCandidate = null)
            {
                this.Source = id;
                this.Msg = message;
                this.Sdp = sdp;
                this.IceCandidate = iceCandidate;
            }
        }
    }
}
