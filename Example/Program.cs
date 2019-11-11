using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using Spitfire;

namespace Example
{
    class Program
    {
        static void Main(string[] args)
        {
            //This is a bare bones example that shows the logic in which one might
            //Implement Spitfire into their application
            //TODO a full fledged example
            SpitfireRtc.EnableLogging();
            var t = new WebRtcSession("a");
            Console.WriteLine("dada");
            Console.Read();
        }
    }
}
