#pragma once
#pragma warning(disable : 4996)

#include <winsock2.h>
#include <iostream>
#include <string.h>
using namespace std;

#pragma comment(lib, "ws2_32.lib")	

#include "Protocol.h"
#include "NetBuffer.h"

class WebClinet {
public:
	bool Connent(const char* ip_addr,size_t port);
	void Close();

	bool Send(ProtocolPost* post);
	bool Receive(ProtocolPost* post) { return false; }
private:
	SOCKET socketClt;
	NetBuffer netBuffer;
};


inline WebClinet gWebClinet;