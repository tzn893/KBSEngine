#pragma once
#include <vector>
#include <string>
#include "NetBuffer.h"

enum PROTOCOL_HEAD {
	PROTOCOL_HEAD_CLINET_CONNECT = 0,
	PROTOCOL_HEAD_CLINET_DISCONNECT = 1,
	PROTOCOL_HEAD_CLINET_MESSAGE = 2
};

enum PROTOCOL_COMMAND_TYPE {
	PROTOCOL_COMMAND_TYPE_HELLO = 0,
	PROTOCOL_COMMAND_TYPE_PLAYER_POSITION = 1,
	PROTOCOL_COMMAND_TYPE_SHOOT = 2,
	PROTOCOL_COMMAND_TYPE_BULLET_POSITION = 3,
	PROTOCOL_COMMAND_TYPE_SHOOTED = 4
};

struct ProtocolCommand {
	PROTOCOL_COMMAND_TYPE type;
	std::string command;
};

struct ProtocolPost {
	PROTOCOL_HEAD head;
	std::vector<ProtocolCommand> protocolCommands;
};

enum PROTOCOL_PARSER_STATE {
	PROTOCOL_PARSER_STATE_CONTINUE,
	PROTOCOL_PARSER_STATE_FAIL,
	PROTOCOL_PARSER_STATE_FINISH
};

class ProtocolParser {
public:
	static PROTOCOL_PARSER_STATE Buffer2CommandList(ProtocolPost* post, NetBuffer* buffer);
	static PROTOCOL_PARSER_STATE CommandList2Buffer(NetBuffer* buffer, ProtocolPost* post, size_t& offset);
};

