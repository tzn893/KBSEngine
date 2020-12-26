#include "Protocol.h"
#include <string.h>

constexpr uint32_t magic_number = 0x11451419;
/*
Protocol format

POST = {PROTOCOL_HEAD 4 bytes|PROTOCOL COMMAND NUMBER 4 bytes|CONTINUE COMMAND 1 bytes|COMMANDS*}
COMMAND = {MAGIC_NUMBER = 0x11451419 4 bytes| COMMAND TYPE 4 bytes|COMMAND (type string ended with '\0')}
*/

PROTOCOL_PARSER_STATE ProtocolParser::Buffer2CommandList(ProtocolPost* post, NetBuffer* buffer) {
	size_t buffer_ptr = 0;
	post->head = *buffer->Get<PROTOCOL_HEAD>(0);
	buffer_ptr += 4;
	post->protocolCommands.resize(*buffer->Get<uint32_t>(buffer_ptr));
	buffer_ptr += 4;
	char continue_cmd = *buffer->Get<char>(buffer_ptr);
	buffer_ptr += 1;
	for (size_t i = 0; i != post->protocolCommands.size(); i++) {
		if (*buffer->Get<uint32_t>(buffer_ptr) != magic_number) {
			return PROTOCOL_PARSER_STATE_FAIL;
		}
		buffer_ptr += 4;
		ProtocolCommand cmd;
		cmd.type = *buffer->Get<PROTOCOL_COMMAND_TYPE>(buffer_ptr);
		buffer_ptr += 4;

		cmd.command = std::string(buffer->Get<char>(buffer_ptr));
		buffer_ptr += cmd.command.size() + 1;
		post->protocolCommands[i] = std::move(cmd);
	}

	if (buffer_ptr)
		return PROTOCOL_PARSER_STATE_FINISH;
	else
		return PROTOCOL_PARSER_STATE_CONTINUE;
}

PROTOCOL_PARSER_STATE ProtocolParser::CommandList2Buffer(NetBuffer* buffer, ProtocolPost* post, size_t& protocolOffset) {
	size_t bufferSize = 9;
	size_t newOffset = 0;
	for (size_t i = protocolOffset; i < post->protocolCommands.size(); i++) {
		size_t chunck_size = 8 + post->protocolCommands[i].command.size() + 1;
		if (bufferSize + chunck_size <= buffer->GetSize()) {
			*buffer->Get<uint32_t>(bufferSize) = magic_number;
			*buffer->Get<PROTOCOL_COMMAND_TYPE>(bufferSize + 4) = post->protocolCommands[i].type;
			char* str = buffer->Get<char>(bufferSize + 8);
			memcpy(str, post->protocolCommands[i].command.data(), post->protocolCommands[i].command.size());
			*buffer->Get<char>(bufferSize + chunck_size - 1) = '\0';
		}
		else {
			break;
		}
		bufferSize += chunck_size;
		newOffset++;
	}
	*buffer->Get<PROTOCOL_HEAD>(0) = post->head;
	*buffer->Get<uint32_t>(4) = newOffset;
	protocolOffset += newOffset;
	char needContinue = protocolOffset != post->protocolCommands.size();
	*buffer->Get<char>(8) = needContinue;

	return needContinue ? PROTOCOL_PARSER_STATE_CONTINUE : PROTOCOL_PARSER_STATE_FINISH;
}