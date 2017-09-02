using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Spitfire;

namespace ConsoleApp1
{
    class Program
    {
        static void HandleMessage(string message)
        {
       
            Console.WriteLine(message);
        }
        static void Main(string[] args)
        {
            using (var rtc = new WebRtc())
            {
                Action<string> displayMessage = HandleMessage;
                rtc.SetCallback(displayMessage);
                rtc.InitializeSSL();
            }
            
            Console.Read();
        }
    }
}
