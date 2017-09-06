# Spitfire

![](https://i.imgur.com/XFsqa6I.png)

# Installing

For projects targeting x64 

```
Install-Package Spitfirex64
```

For projects targeting x86
```
Install-Package Spitfirex86
```

To install the utilities 

```
Install-Package SpitfireUtils
```


# What is this?

Spitfire is a wrapper around the WebRTC native code that allows .NET applications to take advantage of data channels. The goal of this is to allow people to build awesome P2P applications and to make it easier to use WebRTC is a server-like fashion. 

# What is a data channel?

A WebRTC data channel lets you send text or binary data over an active connection to a peer. In the context of a game, this lets players send data to each other, whether text chat or game status information. Data channels come in two flavors.

Reliable channels guarantee that messages you send arrive at the other peer and in the same order in which they're sent. This is analogous to a TCP socket.

Unreliable channels make no such guarantees; messages aren't guaranteed to arrive in any particular order and, in fact, aren't guaranteed to arrive at all. This is analogous to a UDP socket.


# What about audio/video?

This library does not currently support audio and video, however if there is enough demand we may consider adding it. To ensure this code runs in all environments we implement fake audio/video devices during initialization.  


# Size limitations 

Data channels only support sending tiny fragments of data, while it is possible to send complete files through it, they must first be chunked. We provide some functions that will allow you to do this quickly without unnecessary copying in ```DataChannelUtils```. It is recommended you chunk all messages larger than 10KB to avoid hitting the 16 KB limit. 

# Signaling 


![](https://i.imgur.com/tY0yv7M.png)

Signaling is the process of coordinating communication. In order for a WebRTC application to set up a data channel, its clients need to exchange information. Spitfire does not currently provide a signaling server, however this isn't a complex process and you can find more information on it here [here](https://www.html5rocks.com/en/tutorials/webrtc/infrastructure/). 


# Messaging 

If you're looking to maximize speed for your application, we recommend pairing Spitfire with [Sachiel](https://github.com/RainwayApp/sachiel-net), our fast network messaging framework.

# Contributing & Building

If you wish to contribute documentation, code examples or fixes we are more than happy to accept pull request.

To build the C++, you can find the precompiled WebRTC libraries on the release page [here](https://github.com/RainwayApp/spitfire/releases). Building WebRTC itself can be quite the headache so we provide scripts for that as well.




