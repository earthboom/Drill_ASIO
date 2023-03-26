#pragma once

#include <iostream>
#include <vector>
#include <deque>
#include <algorithm>
#include <string>

#include <boost/bind.hpp>
#include <boost/asio.hpp>

#include "Protocol.h"

class ServerSession_TCP;

class ChatServer_TCP
{
public:
	ChatServer_TCP(boost::asio::io_context& io_service);
	~ChatServer_TCP();

public:
	void Init(const int nMaxSessionCount);
	void Start();
	void CloseSession(const int nSessionID);
	void ProcessPacket(const int nSessionID, const char* pData);

private:
	bool PostAccept();
	void handle_accept(ServerSession_TCP* pSession, const boost::system::error_code& error);

private:
	int nSeqNumber;
	bool _bIsAccepting;

	boost::asio::ip::tcp::acceptor _acceptor;

	std::vector<ServerSession_TCP*> _sessionList;
	std::deque<int> _sessionQueue;
};