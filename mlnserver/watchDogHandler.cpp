#include "stdafx.h"
#include "watchDogHandler.h"

#ifdef _WIN32
#include <conio.h>
#endif

#include <iostream>
#include <net/circularStream.h>
#include <net/testInterface.h>
#include <net/logManager.h>
#include <net/simdjson.h>

std::string watchDogHandler::inputLine(const std::string &inputMsg, boost::asio::ip::tcp::socket & /*sock*/)
{
	const int byteSize = 4;

	static const char *endline = "\r\n";

	if (inputMsg == "help"
		|| inputMsg == "h")
	{
		std::string helpString = endline;
		helpString += "<< Command List >>";
		helpString += endline;
		helpString += "status";
		helpString += endline;
		helpString += "init DB";
		helpString += endline;
		helpString += endline;
		helpString += "u can close socket by typeing \'Ctrl + C\' or \'Ctrl + Z\'";
		helpString += endline;
		helpString += endline;

		return helpString;
	}
	else if (inputMsg == "status")
	{
	}

	return inputMsg;
}

void watchDogHandler::closedConsole()
{
	LOGI("closed test-console.");
}

void watchDogHandler::registCallback(std::shared_ptr<boost::asio::io_context> ios, unsigned short port)
{
	m_ios = ios;

	mln::TestIntarface::instance()->setWatchDogEventCallback(
		std::bind(&watchDogHandler::inputLine
			, this
			, std::placeholders::_1
			, std::placeholders::_2
		)
		, std::bind(&watchDogHandler::closedConsole
			, this
		)
		, port);
}
