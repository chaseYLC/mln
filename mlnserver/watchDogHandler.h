#pragma once

#include <memory>
#include <string>
#include <boost/asio.hpp>
#include <net/singleton.h>


class watchDogHandler
	: public mln::SingletonLight<watchDogHandler>
{
public:
	void registCallback(std::shared_ptr<boost::asio::io_context> ios, unsigned short port);
	std::string inputLine(const std::string &inputMsg, boost::asio::ip::tcp::socket &sock);
	void closedConsole();

private:
	std::weak_ptr<boost::asio::io_context> m_ios;
};