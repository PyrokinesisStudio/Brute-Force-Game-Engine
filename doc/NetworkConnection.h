/**
\page NetworkConnection The Network Connection Process

\msc
	hscale="1.6";
	User, Client, Server, TcpModule, UdpReadModule, UdpWriteModule, PeerIdentificator;
	|||;
	Client <- Client [label="NE_CONNECT"];
	Client <= Client [label="startConnecting()", URL="\ref BFG::Network::Client::startConnecting()"];
	Client <- Client [label="resolveHandler()", URL="\ref BFG::Network::Client::resolveHandler()"];
	Client abox Client [label="DNS query", textbgcolour="#C1DAD6"];
	Client <- Client [label="connectHandler()", URL="\ref BFG::Network::Client::connectHandler()"];
	Client abox Client [label="TCP connect", textbgcolour="#C1DAD6"];
	
	Server <= Client [label="sendHandshake()"];
	Server <- Server [label="writeHandshakeHandler()"];
	Server => TcpModule [label="startReading()"];
	Server => TcpModule [label="startSending()"];
	Server -> User [label="NE_CONNECTED"];
	
	Client <= Client [label="readHandshake()"];
	Client <- Client [label="readHandshakeHandler()"];
	Client => TcpModule [label="startReading()"];
	Client => TcpModule [label="startSending()"];
	
	Client abox Client [label="UDP connect", textbgcolour="#C1DAD6"];
	Client => UdpReadModule [label="startReading()"];
	Client => UdpWriteModule [label="pingRemote()"];
	Client => UdpWriteModule [label="startSending()"];
	Client -> User [label="NE_CONNECTED"];
	Client => TcpModule [label="sendTimesyncRequest()"];

	UdpReadModule <- UdpReadModule [label="readHandler()"];
	UdpReadModule => PeerIdentificator [label="identify()"];
	UdpReadModule << PeerIdentificator [label="senderId"];
	UdpReadModule => UdpWriteModule [label="startSending()"];
\endmsc

\note
\ref BFG::Network::TcpModule "TcpModule", \ref BFG::Network::UdpReadModule
"UdpReadModule" and \ref BFG::Network::UdpWriteModule "UdpWriteModule" are
written in a generic way. They may behave differently though. The Client only
has to manage one TCP and one UDP connection to the Server, but the Server has
to handle many connections UDP connections on the same port.

*/
