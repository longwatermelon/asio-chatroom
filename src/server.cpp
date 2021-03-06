#include <iostream>
#include <thread>
#include <memory>
#include <chrono>
#include <mutex>

#ifdef _WIN32
#define _WIN32_WINNT 0x0A00
#endif

#define ASIO_STANDALONE
#include <asio.hpp>

using namespace asio::ip;

std::vector<std::shared_ptr<tcp::socket>> g_users;
std::mutex g_mutex;

void send_data(const std::string msg, int ignored);


void get_data()
{
	while (true)
	{
		{
			std::lock_guard<std::mutex> lock(g_mutex);
			std::string data;

			for (int i = 0; i < g_users.size(); i++)
			{
				std::shared_ptr<tcp::socket>& sock = g_users[i];

				size_t bytes = sock->available();

				if (bytes > 0)
				{
					std::cout << "\nread " << bytes << " bytes\n";

					std::vector<char> buf(bytes);

					sock->read_some(asio::buffer(buf.data(), buf.size()));

					for (char c : buf)
					{
						data += c;
					}

					std::cout << data;

					std::stringstream ss;
					ss << "user " << i << ": " << data;
					send_data(ss.str(), i);
				}
			}
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
}


void send_data(const std::string msg, int ignored)
{
	for (int i = 0; i < g_users.size(); i++)
	{
		if (i == ignored)
			continue;

		std::shared_ptr<tcp::socket>& sock = g_users[i];

		sock->write_some(asio::buffer(msg.data(), msg.size()));
	}
}


void accept_new_users(tcp::acceptor* act, asio::io_service* service)
{
	while (true)
	{
		auto sock = std::make_shared<tcp::socket>(*service);
		act->accept(*sock);

		{
			std::lock_guard<std::mutex> lock(g_mutex);
			g_users.push_back(sock);
			std::cout << "New user connected\n";
		}
	}
}


int main()
{
	asio::io_service service;
	tcp::acceptor act(service, tcp::endpoint(tcp::v4(), 1234));

	std::thread _new(accept_new_users, &act, &service);
	std::thread get(get_data);

	while (true)
	{
		;
	}

	return 0;
}
