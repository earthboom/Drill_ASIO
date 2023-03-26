#include <sdkddkver.h>

#include "ChatServer_TCP.h"

constexpr const int MAX_SESSION_COOUNT = 100;

int main()
{
	boost::asio::io_context io_service;

	ChatServer_TCP server(io_service);
	server.Init(MAX_SESSION_COOUNT);
	server.Start();

	io_service.run();

	std::cout << "Network connection terminated" << std::endl;

	getchar();
	return 0;
	

	//boost::asio::io_context io_context;
	//
	//// TCP acceptor 생성
	//tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 1234));

	//std::cout << "Server started, listening on port 1234" << std::endl;

	//while (true) {
	//	// client의 연결 대기
	//	tcp::socket socket(io_context);
	//	acceptor.accept(socket);

	//	std::cout << " New client connected" << std::endl;

	//	boost::asio::write(socket, boost::asio::buffer("Welcome to the chat room! \n"));

	//	// client와의 메시지 송수진
	//	try {
	//		while (true) {
	//			char data[1024] = {};
	//			size_t length = socket.read_some(boost::asio::buffer(data));

	//			boost::asio::write(socket, boost::asio::buffer(data, length));
	//		}
	//	}
	//	catch (std::exception& e) {
	//		std::cerr << "Exception : " << e.what() << std::endl;
	//	}

	//}

	return 0;
}