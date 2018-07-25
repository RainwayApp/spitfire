using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace Example
{
    public static class WebRtcManager
    {
        private static readonly ConcurrentDictionary<string, WebRtcSession> Sessions =
            new ConcurrentDictionary<string, WebRtcSession>();


        /// <summary>
        /// A remote session has sent over its ice candidate, add it to your local session.
        /// </summary>
        /// <param name="id"></param>
        /// <param name="sdpMid"></param>
        /// <param name="sdpMLineIndex"></param>
        /// <param name="candidate"></param>
        /// <returns></returns>
        public static bool AddIceCandidate(string id, string sdpMid, int sdpMLineIndex, string candidate)
        {
            if (!Sessions.ContainsKey(id))
            {
               Console.WriteLine("Attempted to add candidate to invalid session.");
                return false;
            }
            Sessions[id].Spitfire.AddIceCandidate(sdpMid, sdpMLineIndex, candidate);
            return true;
        }

        /// <summary>
        /// A remote session has sent over its SDP, create a local session based on it.
        /// </summary>
        /// <param name="id"></param>
        /// <param name="sdp"></param>
        public static WebRtcSession AddSession(string id, string sdp)
        {
            var session = Sessions[id] = new WebRtcSession(id);
            using (var go = new ManualResetEvent(false))
            {
                Task.Factory.StartNew(() =>
                {

                    using (session.Spitfire)
                    {
                       Console.WriteLine($"Starting WebRTC Loop for {id}");
                        session.BeginLoop(go);
                    }
                }, session.Token.Token, TaskCreationOptions.LongRunning, TaskScheduler.Default);
                if (go.WaitOne(9999))
                    session.Setup(sdp);
            }
            return session;
        }

        /// <summary>
        /// A remote session has sent over its SDP, create a local session based on it.
        /// </summary>
        /// <param name="id"></param>
        /// <param name="sdp"></param>
        public static WebRtcSession AddSession(string id)
        {
            var session = Sessions[id] = new WebRtcSession(id);
            using (var go = new ManualResetEvent(false))
            {
                Task.Factory.StartNew(() =>
                {
                    using (session.Spitfire)
                    {
                        Console.WriteLine($"Starting WebRTC Loop for {id}");
                        session.BeginLoop(go);
                    }
                }, session.Token.Token, TaskCreationOptions.LongRunning, TaskScheduler.Default);
                if (go.WaitOne(9999))
                    session.Setup();
            }
            return session;
        }
    }
}
