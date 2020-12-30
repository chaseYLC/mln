#pragma once

#include <boost/asio.hpp>
#include <chrono>
#include <memory>
#include <thread>
#include <net/logManager.h>

#include "sampleConnector.h"

namespace mlnserver {
	class SampleClientTest
	{
	public:
		static void TestRun(std::shared_ptr<boost::asio::io_context> spIoc, const int32_t port)
		{
			SampleConnector::tryConnect(*spIoc.get(), port);
		}
	};//class PacketRecorderTest 
}//namespace mlnserver {

