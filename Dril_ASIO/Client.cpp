
#include <iostream>

#include "ChatClient_Chat.h"

//using boost::asio::ip::tcp;

int main()
{
	boost::asio::io_context io_service;

	auto endpoint = boost::asio::ip::tcp::endpoint(boost::asio::ip::make_address("127.0.0.1"), PORT_NUMBER);

	ChatClient_Chat client(io_service);
	client.Connect(endpoint);

	boost::thread thread(boost::bind(&boost::asio::io_context::run, &io_service));

	char szInputMessage[MAX_MESSAGE_LEN * 2] = { 0, };

	while (std::cin.getline(szInputMessage, MAX_MESSAGE_LEN)) {
		if (strnlen_s(szInputMessage, MAX_MESSAGE_LEN) == 0) {
			break;
		}

		if (client.IsConnecting() == false) {
			std::cout << "Not connected with server" << std::endl;
			continue;
		}

		if (client.IsLogin() == false) {
			PKT_REQ_IN sendPkt;
			sendPkt.Init();
			strncpy_s(sendPkt.szName, MAX_NAME_LEN, szInputMessage, MAX_NAME_LEN - 1);

			client.PostSend(false, sendPkt.nSize, (char*)&sendPkt);
		}
		else {
			PKT_REQ_CHAT sendPkt;
			sendPkt.Init();
			strncpy_s(sendPkt.szMessage, MAX_MESSAGE_LEN, szInputMessage, MAX_MESSAGE_LEN - 1);

			client.PostSend(false, sendPkt.nSize, (char*)&sendPkt);
		}
	}

	io_service.stop();
	client.Close();
	thread.join();

	std::cout << "Client end it" << std::endl;
	

	//tcp::socket socket(io_context);	// socket 생성
	//
	//try {
	//	// 서버 연결
	//	socket.connect(tcp::endpoint(boost::asio::ip::address::from_string("127.0.0.1"), 1234));

	//	// 서버로부터 메시지 수진 및 출력
	//	char data[1024] = {};
	//	size_t length = socket.read_some(boost::asio::buffer(data));
	//	std::cout << "Server : " << data << std::endl;

	//	// 사용자 입력 받아서 서버로 전송
	//	std::string message;
	//	while (std::getline(std::cin, message)) {

	//		boost::asio::write(socket, boost::asio::buffer(message + "\n"));

	//		length = socket.read_some(boost::asio::buffer(data));
	//		std::cout << data << std::endl;
	//	}
	//}
	//catch (std::exception& e) {
	//	std::cerr << "Exception : " << e.what() << std::endl;
	//}

	return 0;
}