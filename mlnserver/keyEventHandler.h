#pragma once

#include <memory>
#include <net/singleton.h>

class keyEventHandler
	: public mln::SingletonLight<keyEventHandler>
{
public:
	void registCallback(std::shared_ptr<boost::asio::io_context> ios);
	void processKey(const unsigned int vKey);

private:
	std::weak_ptr<boost::asio::io_context> m_ios;

};