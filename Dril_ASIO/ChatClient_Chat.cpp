#include "ChatClient_Chat.h"

#include <iostream>

ChatClient_Chat::ChatClient_Chat(boost::asio::io_context& io_service)
	: _IOService(io_service)
	, _socket(io_service)
{
	_bIsLogin = false;
	InitializeCriticalSectionAndSpinCount(&_lock, 4000);
}

ChatClient_Chat::~ChatClient_Chat()
{
	EnterCriticalSection(&_lock);
	
	while (_sendDataQueue.empty() == false) {
		delete[] _sendDataQueue.front();
		_sendDataQueue.pop_front();
	}

	LeaveCriticalSection(&_lock);

	DeleteCriticalSection(&_lock);
}

void ChatClient_Chat::Connect(boost::asio::ip::tcp::endpoint endpoint)
{
	_nPacketBufferMark = 0;

	_socket.async_connect(endpoint, boost::bind(&ChatClient_Chat::handle_connect, this, boost::asio::placeholders::error));
}

void ChatClient_Chat::Close()
{
	if (_socket.is_open()) {
		_socket.close();
	}
}

void ChatClient_Chat::PostSend(const bool bImmediately, const int nSize, char* pData)
{
	char* sendData = nullptr;
	
	EnterCriticalSection(&_lock);

	if (bImmediately== false) {
		sendData = new char[nSize];
		memcpy(sendData, pData, nSize);

		_sendDataQueue.emplace_back(sendData);
	}
	else {
		sendData = pData;
	}

	if (bImmediately || _sendDataQueue.size() < 2) {
		boost::asio::async_write(_socket,
			boost::asio::buffer(sendData, nSize),
			boost::bind(&ChatClient_Chat::handle_wrtie, this,
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred)
		);
	}

	LeaveCriticalSection(&_lock);
}

void ChatClient_Chat::PostReceive()
{
	memset(&_receiveBuffer, '\0', sizeof(_receiveBuffer));

	_socket.async_read_some(boost::asio::buffer(_receiveBuffer),
		boost::bind(&ChatClient_Chat::handle_receive, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
}

void ChatClient_Chat::handle_connect(const boost::system::error_code& error)
{
	if (!error) {
		std::cout << "Server Connect Success!" << std::endl;
		std::cout << "Input your name!!" << std::endl;

		PostReceive();
	}
	else {
		std::cout << "Server connect failed. error No : " << error.value() << " error Message : " << error.message() << std::endl;
	}
}

void ChatClient_Chat::handle_wrtie(const boost::system::error_code& error, size_t bytes_transferred)
{
	EnterCriticalSection(&_lock);

	delete[] _sendDataQueue.front();
	_sendDataQueue.pop_front();

	char* data = nullptr;
	if (_sendDataQueue.empty() == false) {
		data = _sendDataQueue.front();
	}

	LeaveCriticalSection(&_lock);

	if (data != nullptr) {
		PACKET_HEADER* header = (PACKET_HEADER*)data;
		PostSend(true, header->nSize, data);
	}
}

void ChatClient_Chat::handle_receive(const boost::system::error_code& error, size_t bytes_transferred)
{
	if (error) {
		if (error == boost::asio::error::eof) {
			std::cout << "disconnected with Client!" << std::endl;
		}
		else {
			std::cout << "error No : " << error.value() << " error Message : " << error.message() << std::endl;
		}

		Close();
	}
	else {
		memcpy(&_packetBuffer[_nPacketBufferMark], _receiveBuffer.data(), bytes_transferred);

		int packetData = _nPacketBufferMark + bytes_transferred;
		int readData = 0;

		while (packetData > 0) {
			if (packetData < sizeof(PACKET_HEADER)) {
				break;
			}

			PACKET_HEADER* header = (PACKET_HEADER*)&_packetBuffer[readData];

			if (header->nSize <= packetData) {
				ProcessPacket(&_packetBuffer[readData]);

				packetData -= header->nSize;
				readData += header->nSize;
			} else{
				break;
			}

			if (packetData > 0) {
				char tempBuffer[MAX_RECEIVE_BUFFER_LEN] = { 0, };
				memcpy(&tempBuffer[0], &_packetBuffer[readData], packetData);
				memcpy(&_packetBuffer[0], &tempBuffer[0], packetData);
			}

			_nPacketBufferMark = packetData;

			PostReceive();
		}
	}
}

void ChatClient_Chat::ProcessPacket(const char* pData)
{
	PACKET_HEADER* header = (PACKET_HEADER*)pData;

	switch (header->nID) {
	case RES_IN:
		{
			PKT_RES_IN* packet = (PKT_RES_IN*)pData;
			LoginOk();
			std::cout << "Client login success ? : " << packet->bIsSuccess << std::endl;
		}
		break;

	case NOTICE_CHAT:
		{
			PKT_NOTICE_CHAT* packet = (PKT_NOTICE_CHAT*)pData;

			std::cout << packet->szName << " : " << packet->szMessage<< std::endl;
		}
		break;
	}
}
