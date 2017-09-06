using System.Collections.Generic;
using System.Net;
using System.Text.RegularExpressions;
namespace SpitfireUtils { 

    public enum IceType
    {
        Host,
        Srflx,
        Prflx,
        Relay,
        Unknown
    }

    public enum IceTransport
    {
        Tcp,
        Udp,
        Unknown
    }
    /// <summary>
    /// Contains information on a parse ICE candidate.
    /// </summary>
    public class IceCandidate
    {
        public string Raw { get; set; }
        public ulong Foundation { get; set; }
        public uint ComponentId { get; set; }
        public IceTransport Transport { get; set; }
        public ulong Priority { get; set; }

        public IPAddress LocalIp { get; set; }

        public ushort LocalPort { get; set; }
        public IceType Type { get; set; }

        public IPAddress RemoteIp { get; set; }

        public ushort RemotePort { get; set; }
        public uint Generation { get; set; }

        public bool Valid => Type != IceType.Unknown;

        public string UFrag { get; set; }
        public int NetworkCost { get; set; }
        public int NetworkId { get; set; }
    }

    public static class IceParser
    {
        #region static members

        private static readonly Dictionary<string, IceType> IceTypeMap = new Dictionary<string, IceType>
        {
            {"host", IceType.Host},
            {"srflx", IceType.Srflx},
            {"prflx", IceType.Prflx},
            {"relay", IceType.Relay}
        };

        private static readonly Dictionary<string, IceTransport> IceTransportMap =
            new Dictionary<string, IceTransport>
            {
                {"udp", IceTransport.Udp},
                {"tcp", IceTransport.Tcp}
            };

        private static readonly Regex TokenRegex = new Regex("[0-9a-zA-Z\\-\\.!\\%\\*_\\+\\`\\\'\\~]+");
        private static readonly Regex IceRegex = new Regex("[a-zA-Z0-9\\+\\/]+");
        private static readonly Regex ComponentIdRegex = new Regex("[0-9]{1,5}");
        private static readonly Regex PriorityRegex = new Regex("[0-9]{1,10}");
        private static readonly Regex Ipv4AddressRegex = new Regex("[0-9]{1,3}.[0-9]{1,3}.[0-9]{1,3}.[0-9]{1,3}");
        private static readonly Regex Ipv6AdresssRegex = new Regex(":?(?:[0-9a-fA-F]{0,4}:?)+");

        private static readonly Regex ConnectAddressRegex =
            new Regex($"(?:{Ipv4AddressRegex})|(?:{Ipv6AdresssRegex})");

        private static readonly Regex PortRegex = new Regex("[0-9]{1,5}");

        private static readonly Regex MasterRegex =
                new Regex(
                    $"(?:a=)?candidate:({IceRegex})\\s({ComponentIdRegex})\\s({TokenRegex})\\s" +
                    $"({PriorityRegex})\\s({ConnectAddressRegex})\\s({PortRegex})" +
                    $"\\styp\\s({TokenRegex})(?:\\sraddr\\s({ConnectAddressRegex})\\srport\\s({PortRegex}))?(?:\\sgeneration\\s(\\d+))?(?:\\sufrag\\s({IceRegex}))?(?:\\snetwork-id\\s({PriorityRegex}))?(?:\\snetwork-cost\\s({PriorityRegex}))?")
            ;
        #endregion

        /// <summary>
        ///     Checks the transport type Dictionary for the connection type.
        /// </summary>
        /// <param name="transport"></param>
        /// <returns>The connection type</returns>
        private static IceTransport GetTransportType(string transport)
        {
            transport = transport.ToLower();
            return IceTransportMap.ContainsKey(transport) ? IceTransportMap[transport] : IceTransport.Unknown;
        }


        /// <summary>
        ///     Parse an ice candidate string and extract information on the connection
        /// </summary>
        /// <param name="candidate"></param>
        /// <returns>A parse ice candidate</returns>
        public static IceCandidate Parse(string candidate)
        {
            var iceCandidate = new IceCandidate();
            var parsed = MasterRegex.Match(candidate);
            //we should only ever have 11 results, even if generation is missing.
            if (!parsed.Success || parsed.Groups.Count != 14)
            {
                return null;
            }
            for (var i = 0; i < parsed.Groups.Count; i++)
            {
                try
                {
                    var value = parsed.Groups[i].Value;
                    if (string.IsNullOrWhiteSpace(value))
                    {
                        continue;
                    }
                    switch (i)
                    {
                        case 0:
                            iceCandidate.Raw = value;

                            break;
                        case 1:
                            iceCandidate.Foundation = ulong.Parse(value);
                            break;
                        case 2:
                            iceCandidate.ComponentId = uint.Parse(value);
                            break;
                        case 3:
                            iceCandidate.Transport = GetTransportType(value);
                            break;
                        case 4:
                            iceCandidate.Priority = ulong.Parse(value);
                            break;
                        case 5:
                            iceCandidate.LocalIp = IPAddress.Parse(value);
                            break;
                        case 6:
                            iceCandidate.LocalPort = ushort.Parse(value);
                            break;
                        case 7:
                            iceCandidate.Type = GetIceType(value);
                            break;
                        case 8:
                            iceCandidate.RemoteIp = IPAddress.Parse(value);
                            break;
                        case 9:
                            iceCandidate.RemotePort = ushort.Parse(value);
                            break;
                        case 10:
                            iceCandidate.Generation = uint.Parse(value);
                            break;
                        case 11:
                            iceCandidate.UFrag = value;
                            break;
                        case 12:
                            iceCandidate.NetworkId = int.Parse(value);
                            break;
                        case 13:
                            iceCandidate.NetworkCost = int.Parse(value);
                            break;
                    }
                }
                catch (System.Exception ex)
                {
                    return null;
                }
            }
            return iceCandidate;
        }

        /// <summary>
        ///     Checks the ice type against the type Dictionary.
        /// </summary>
        /// <param name="value"></param>
        /// <returns>The ice type</returns>
        private static IceType GetIceType(string value)
        {
            value = value.ToLower();
            return IceTypeMap.ContainsKey(value) ? IceTypeMap[value] : IceType.Unknown;
        }
    }
}