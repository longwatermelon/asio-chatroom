#include <iostream>
#include <thread>

#ifdef _WIN32
#define _WIN32_WINNT 0x0A00
#endif

#define ASIO_STANDALONE
#include <asio.hpp>

using namespace asio::ip;


void get(tcp::socket* sock)
{
	while (true)
	{
		sock->wait(sock->wait_read);
		size_t bytes = sock->available();

		if (bytes > 0)
		{
			std::cout << "\r   \r";

			std::vector<char> buf(bytes);
			sock->read_some(asio::buffer(buf.data(), buf.size()));

			for (char c : buf)
			{
				std::cout << c;
			}
		}

		std::cout << "\n-> ";
	}
}


void send(tcp::socket* sock, const std::string msg)
{
	if (msg.size() >= 500)
	{
		std::cout << "Your message exceeds the 500 character limit\n";
		return;
	}

	sock->write_some(asio::buffer(msg.data(), msg.size()));
}


void input(tcp::socket* sock)
{
	while (true)
	{
		std::cout << "-> ";

		std::string msg;
		std::getline(std::cin, msg);

		send(sock, msg);
	}
}


int main()
{
	asio::io_service service;
	tcp::socket sock(service);

	sock.connect(tcp::endpoint(address::from_string("127.0.0.1"), 1234));

	send(&sock, "new user joined");

	std::thread read(get, &sock);
	std::thread inp(input, &sock);

	while (true)
	{
		;
	}

	return 0;
}