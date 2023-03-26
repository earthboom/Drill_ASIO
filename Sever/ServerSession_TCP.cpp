#include "ServerSession_TCP.h"
#include "ChatServer_TCP.h"

ServerSession_TCP::ServerSession_TCP(int nSessionID, boost::asio::io_context& io_service, ChatServer_TCP* pServer)
	: _socket(io_service)
	, _nSessionID(nSessionID)
	, _pServer(pServer)
{
}

ServerSession_TCP::~ServerSession_TCP()
{
	while (_sendDataQueue.empty() == false) {
		delete[] _sendDataQueue.front();
		_sendDataQueue.pop_front();
	}
}

void ServerSession_TCP::Init()
{
	_nPacketBufferMark = 0;
}

void ServerSession_TCP::PostReceive()
{
	_socket.async_read_some(
		boost::asio::buffer(_receiverBuffer),
		boost::bind(&ServerSession_TCP::handle_receive, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred)
	);
}

void ServerSession_TCP::PostSend(const bool bImmediately, const int nSize, char* pData)
{
	char* sendData = nullptr;

	if (bImmediately == false) {
		sendData = new char[nSize];
		memcpy(sendData, pData, nSize);

		_sendDataQueue.emplace_back(sendData);
	} else{
		sendData = pData;
	}

	if (bImmediately == false && _sendDataQueue.size() > 1) {
		return;
	}

	boost::asio::async_write(_socket,
		boost::asio::buffer(sendData, nSize),
		boost::bind(&ServerSession_TCP::handle_write, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred)		
	);
}

void ServerSession_TCP::handle_write(const boost::system::error_code& error, size_t bytes_transferred)
{
	delete[] _sendDataQueue.front();
	_sendDataQueue.pop_front();

	if (_sendDataQueue.empty() == false) {
		char* data = _sendDataQueue.front();
		PACKET_HEADER* header = (PACKET_HEADER*)data;
		PostSend(true, header->nSize, data);
	}
}

void ServerSession_TCP::handle_receive(const boost::system::error_code& error, size_t bytes_transferred)
{
	if (error) {
		if (error == boost::asio::error::eof) {
			std::cout << "Connection is disconnected with Client" << std::endl;
		}
		else {
			std::cout << "error No : " << error.value() << " error Message : " << error.message() << std::endl;
		}

		_pServer->CloseSession(_nSessionID);
	}
	else {
		memcpy(&_packetBuffer[_nPacketBufferMark], _receiverBuffer.data(), bytes_transferred);

		int packetData = _nPacketBufferMark + bytes_transferred;
		int readData = 0;

		while (packetData > 0) {
			if (packetData < sizeof(PACKET_HEADER)) {
				break;
			}

			PACKET_HEADER* header = (PACKET_HEADER*)&_packetBuffer[readData];
			if (header->nSize <= packetData) {
				_pServer->ProcessPacket(_nSessionID, &_packetBuffer[readData]);

				packetData -= header->nSize;
				readData += header->nSize;
			}
			else {
				break;
			}
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
