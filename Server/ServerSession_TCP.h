#pragma once
#include <sdkddkver.h>

#include <deque>

#include <boost/bind.hpp>
#include <boost/asio.hpp>

#include "Protocol.h"

class ChatServer_TCP;

class ServerSession_TCP
{
public:
	ServerSession_TCP(int nSessionID, boost::asio::io_context& io_service, ChatServer_TCP* pServer);
	~ServerSession_TCP();

	int SessionID() { return _nSessionID; }

	boost::asio::ip::tcp::socket& Socket() { return _socket; }

	void Init();
	void PostReceive();
	void PostSend(const bool bImmediately, const int nSize, char* pData);

	void SetName(const char* pszName){ _name = pszName; }
	const char* GetName() { return _name.c_str(); }

private:
	void handle_write(const boost::system::error_code& error, size_t bytes_transferred);
	void handle_receive(const boost::system::error_code& error, size_t bytes_transferred);

private:
	int _nSessionID;
	boost::asio::ip::tcp::socket _socket;

	std::array<char, MAX_RECEIVE_BUFFER_LEN> _receiverBuffer;

	int _nPacketBufferMark;
	char _packetBuffer[MAX_RECEIVE_BUFFER_LEN * 2];

	std::deque<char*> _sendDataQueue;

	std::string _name;

	ChatServer_TCP* _pServer;
};

