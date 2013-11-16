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
	
	Server <= Server [label="sendHandshake()", URL="\ref BFG::Network::Server::sendHandshake()"];
	Server <- Server [label="writeHandshakeHandler()", URL="\ref BFG::Network::Server::writeHandshakeHandler()"];
	Server => TcpModule [label="startReading()", URL="\ref BFG::Network::TcpModule::startReading()"];
	Server => TcpModule [label="startSending()", URL="\ref BFG::Network::TcpModule::startSending()"];
	Server -> User [label="NE_CONNECTED"];
	
	Client <= Client [label="readHandshake()", URL="\ref BFG::Network::Client::readHandshake()"];
	Client <- Client [label="readHandshakeHandler()", URL="\ref BFG::Network::Client::readHandshakeHandler()"];
	Client => TcpModule [label="startReading()", URL="\ref BFG::Network::TcpModule::startReading()"];
	Client => TcpModule [label="startSending()", URL="\ref BFG::Network::TcpModule::startSending()"];
	
	Client abox Client [label="UDP connect", textbgcolour="#C1DAD6"];
	Client => UdpReadModule [label="startReading()", URL="\ref BFG::Network::UdpReadModule::startReading()"];
	Client => UdpWriteModule [label="pingRemote()", URL="\ref BFG::Network::UdpWriteModule::pingRemote()"];
	Client => UdpWriteModule [label="startSending()", URL="\ref BFG::Network::UdpWriteModule::startSending()"];
	Client -> User [label="NE_CONNECTED"];
	Client => TcpModule [label="sendTimesyncRequest()", URL="\ref BFG::Network::TcpModule::sendTimesyncRequest()"];

	UdpReadModule <- UdpReadModule [label="readHandler()", URL="\ref BFG::Network::UdpReadModule::readHandler()"];
	UdpReadModule => PeerIdentificator [label="identify()"];
	UdpReadModule << PeerIdentificator [label="senderId"];
	UdpReadModule => UdpWriteModule [label="startSending()", URL="\ref BFG::Network::UdpWriteModule::startSending()"];
\endmsc

\note
\ref BFG::Network::TcpModule "TcpModule", \ref BFG::Network::UdpReadModule
"UdpReadModule" and \ref BFG::Network::UdpWriteModule "UdpWriteModule" are
written in a generic way. They may behave differently though. The Client only
has to manage one TCP and one UDP connection to the Server, but the Server has
to handle many connections UDP connections on the same port.

*/
