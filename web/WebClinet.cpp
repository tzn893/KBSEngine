#include "WebClinet.h"		//add ws2_32.lib


constexpr int DEFAULT_PORT = 8000;
/*
int SocketBody(int argc, char* argv[])
{

	WORD	wVersionRequested;
	WSADATA wsaData;
	int		err, iLen;
	wVersionRequested = MAKEWORD(2, 2);//create 16bit data
	//(1)Load WinSock
	err = WSAStartup(wVersionRequested, &wsaData);	//load win socket
	if (err != 0)
	{
		cout << "Load WinSock Failed!";
		return -1;
	}
	//(2)create socket
	SOCKET sockClt = socket(AF_INET, SOCK_STREAM, 0);
	if (sockClt == INVALID_SOCKET) {
		cout << "socket() fail:" << WSAGetLastError() << endl;
		return -2;
	}
	//(3)IP
	SOCKADDR_IN addrSrv;
	addrSrv.sin_family = AF_INET;
	addrSrv.sin_addr.s_addr = inet_addr("127.0.0.1");
	addrSrv.sin_port = htons(DEFAULT_PORT);

	//(5)connect
	err = connect(sockClt, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR));

	if (err == INVALID_SOCKET)
	{
		cout << "connect() fail" << WSAGetLastError() << endl;
		return -1;
	}

	char sendBuf[1024], hostname[100];
	if (gethostname(hostname, 100) != 0)	//get host name
		strcpy(hostname, "None");
	strcpy(sendBuf, hostname);
	strcat(sendBuf, "have connected to you!");
	err = send(sockClt, sendBuf, strlen(sendBuf) + 1, 0);

	char recvBuf[1024] = "\0";
	iLen = recv(sockClt, recvBuf, 1024, 0);

	if (iLen == 0)
		return -3;
	else if (iLen == SOCKET_ERROR) {
		cout << "recv() fail:" << WSAGetLastError() << endl;
		return -4;
	}
	else
	{
		recvBuf[iLen] = '\0';
		cout << recvBuf << endl;
	}
	closesocket(sockClt);

	WSACleanup();
	system("PAUSE");
	return 0;
}
*/
constexpr size_t netBufferSize = 1 << 10;

bool WebClinet::Connent(const char* ip_addr,size_t port) {
	
	WORD	wVersionRequested;
	WSADATA wsaData;
	int		err, iLen;
	wVersionRequested = MAKEWORD(2, 2);//create 16bit data
	//(1)Load WinSock
	err = WSAStartup(wVersionRequested, &wsaData);	//load win socket
	if (err != 0)
	{
		cout << "Load WinSock Failed!";
		return false;
	}
	//(2)create socket
	socketClt = socket(AF_INET, SOCK_STREAM, 0);
	if (socketClt == INVALID_SOCKET) {
		cout << "socket() fail:" << WSAGetLastError() << endl;
		return false;
	}
	//(3)IP
	SOCKADDR_IN addrSrv;
	addrSrv.sin_family = AF_INET;
	addrSrv.sin_addr.s_addr = inet_addr(ip_addr);
	addrSrv.sin_port = htons(DEFAULT_PORT);

	//(5)connect
	err = connect(socketClt, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR));

	if (err == INVALID_SOCKET)
	{
		cout << "connect() fail" << WSAGetLastError() << endl;
		return false;
	}

	netBuffer.Resize(netBufferSize);
}

void WebClinet::Close() {
	closesocket(socketClt);
	WSACleanup();
}

bool WebClinet::Send(ProtocolPost* post) {

	size_t offset = 0;
	PROTOCOL_PARSER_STATE state = PROTOCOL_PARSER_STATE_CONTINUE;
	while (state == PROTOCOL_PARSER_STATE_CONTINUE) {
		state = ProtocolParser::CommandList2Buffer(&netBuffer,post,offset);
		int err = send(socketClt, netBuffer.Get<char>(0), netBuffer.GetSize(), 0);
		if (err != 0) {
			return false;
		}
	}
	if (state == PROTOCOL_PARSER_STATE_FAIL) return false;
	post->protocolCommands.clear();
	return true;
}