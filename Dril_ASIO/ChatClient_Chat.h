#pragma once

#include <sdkddkver.h>

#include <deque>

#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>

#include "..\Server\Protocol.h"

class ChatClient_Chat
{
public:
	ChatClient_Chat(boost::asio::io_context& io_service);
	~ChatClient_Chat();

public:
	void Connect(boost::asio::ip::tcp::endpoint endpoint);
	void Close();
	void PostSend(const bool bImmediately, const int nSize, char* pData);

private:
	void PostReceive();
	void handle_connect(const boost::system::error_code& error);
	void handle_wrtie(const boost::system::error_code& error, size_t bytes_transferred);
	void handle_receive(const boost::system::error_code& error, size_t bytes_transferred);
	void ProcessPacket(const char* pData);

public:
	bool IsConnecting() { return _socket.is_open(); }
	void LoginOk() { _bIsLogin = true; }
	bool IsLogin() { return _bIsLogin; }

private:
	boost::asio::io_context& _IOService;
	boost::asio::ip::tcp::socket _socket;

	std::array<char, 512> _receiveBuffer;

	int _nPacketBufferMark;
	char _packetBuffer[MAX_RECEIVE_BUFFER_LEN * 2];

	CRITICAL_SECTION _lock;
	std::deque<char*> _sendDataQueue;

	bool _bIsLogin;
};

