#include "ChatServer_TCP.h"
#include "ServerSession_TCP.h"

ChatServer_TCP::ChatServer_TCP(boost::asio::io_context& io_service)
	: _acceptor(io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), PORT_NUMBER))
{
	_bIsAccepting = false;
}

ChatServer_TCP::~ChatServer_TCP()
{
	for (auto& session : _sessionList) {
		if (session->Socket().is_open()) {
			session->Socket().close();
		}

		delete session;
	}
}

void ChatServer_TCP::Init(const int nMaxSessionCount)
{
	for (int i = 0; i < nMaxSessionCount; ++i) {
		ServerSession_TCP* session = new ServerSession_TCP(i, (boost::asio::io_context&)_acceptor.get_executor().context(), this);
		_sessionList.emplace_back(session);
		_sessionQueue.emplace_back(i);
	}
}

void ChatServer_TCP::Start()
{
	std::cout << "Server start ......" << std::endl;

	PostAccept();
}

void ChatServer_TCP::CloseSession(const int nSessionID)
{
	std::cout << "Client's connection terminated. session ID : " << nSessionID << std::endl;

	_sessionList[nSessionID]->Socket().close();
	_sessionQueue.emplace_back(nSessionID);

	if (_bIsAccepting == false) {
		PostAccept();
	}
}

void ChatServer_TCP::ProcessPacket(const int nSessionID, const char* pData)
{
	PACKET_HEADER* header = (PACKET_HEADER*)pData;

	switch (header->nID) {
	case REQ_IN:
		{
			PKT_REQ_IN* packet = (PKT_REQ_IN*)pData;
			_sessionList[nSessionID]->SetName(packet->szName);

			std::cout << "Client login success! Name : " << _sessionList[nSessionID]->GetName() << std::endl;

			PKT_RES_IN sendPkt;
			sendPkt.Init();
			sendPkt.bIsSuccess = true;

			_sessionList[nSessionID]->PostSend(false, sendPkt.nSize, (char*)&sendPkt);
		}
		break;

	case REQ_CHAT:
		{
			PKT_REQ_CHAT* packet = (PKT_REQ_CHAT*)pData;

			PKT_NOTICE_CHAT sendPkt;
			sendPkt.Init();
			strncpy_s(sendPkt.szName, MAX_NAME_LEN, _sessionList[nSessionID]->GetName(), MAX_NAME_LEN - 1);
			strncpy_s(sendPkt.szMessage, MAX_MESSAGE_LEN, packet->szMessage, MAX_MESSAGE_LEN - 1);

			size_t totalSessionCount = _sessionList.size();
			for (size_t i = 0; i < totalSessionCount; ++i) {
				if (_sessionList[i]->Socket().is_open()) {
					_sessionList[i]->PostSend(false, sendPkt.nSize, (char*)&sendPkt);
				}
			}
		}
		break;
	}
}

bool ChatServer_TCP::PostAccept()
{
	if (_sessionQueue.empty()) {
		_bIsAccepting = false;
		return false;
	}

	_bIsAccepting = true;
	int sessionID = _sessionQueue.front();

	_sessionQueue.pop_front();

	_acceptor.async_accept(_sessionList[sessionID]->Socket(),
		boost::bind(&ChatServer_TCP::handle_accept, this, _sessionList[sessionID], boost::asio::placeholders::error));

	return true;
}

void ChatServer_TCP::handle_accept(ServerSession_TCP* pSession, const boost::system::error_code& error)
{
	if (!error) {
		std::cout << "Client's connection success! SessionID : " << pSession->SessionID() << std::endl;

		pSession->Init();
		pSession->PostReceive();

		PostAccept();
	}
	else {
		std::cout << "Error No : " << error.value() << " error Message : " << error.message() << std::endl;
	}
}
