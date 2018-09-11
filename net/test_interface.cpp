#include "stdafx.h"
#include "test_interface.h"

#include <conio.h>
#include <stdarg.h> 
#include <iostream>
#include <thread>
#include "logger.h"


namespace MLN
{
	namespace Net
	{
		TestIntarface::TestIntarface()
		{
		}

		inline void setInputParam(const size_t paramIndex)
		{
		}

		void TestIntarface::waitKeyboardEvent()
		{
			std::thread keyEvntThread([&] {
				while (true) {
					std::this_thread::sleep_for((std::chrono::milliseconds(50)));

					if (0 == _kbhit()) { continue; }

					unsigned int key = ::_getch();
					if (NULL == key) {
						key = ::_getch();
						key = MapVirtualKey(key, MAPVK_VSC_TO_VK);
					}

					if (nullptr != m_keybaoardEventCallback) {
						m_keybaoardEventCallback(key);
					}
					else {
						this->keyboardEventHandler(key);
					}
				}
			});
			keyEvntThread.detach();
		}

		size_t TestIntarface::getFuncValue(const FUNC::value_type funcValue) const
		{
			return ((size_t)1) << funcValue;
		}

		bool TestIntarface::isUsingFunc(const FUNC::value_type funcValue) const
		{
			return 0 != (m_usingFunctions &getFuncValue(funcValue));
		}

		void TestIntarface::keyboardEventHandler(const unsigned int vKey)
		{
			defaultKeyboardEventHandler(vKey);
		}

		void TestIntarface::defaultKeyboardEventHandler(const unsigned int vKey)
		{
			switch (vKey)
			{
			case VK_ESCAPE:
			{
				std::cout << "pressed ESC key ..." << std::endl;

				auto ios = m_ios.lock();
				if (ios)
				{
					if (false == ios->stopped()) {
						/*LOGI << "terminating process...";*/
						ios->stop();
					}
				}
			}
			break;
			}//switch (key)
		}


		void vv()
		{

		}

		void watchDogSession(boost::asio::ip::tcp::socket &sock
			, TestIntarface::ConsoleCallbackType watchdogCallback
			, TestIntarface::ConsoleCloseCallbackType watchdogClosedCallback
		)
		{
			static const int max_length = 4096;

			try
			{
				std::string recv_cmd;
				char data[max_length];

				bool hiMsg = false;


				for (;;)
				{
					ZeroMemory(data, sizeof(data));
					boost::system::error_code error;

					if (false == hiMsg) {
						hiMsg = true;
						std::string hiMsg("Hi~ this is watchDog Interface. Type \'help\' or \'h\' and check functions\r\n");
						boost::asio::write(sock, boost::asio::buffer(hiMsg.c_str(), static_cast<int>(hiMsg.length())));
					}

					size_t length = sock.read_some(boost::asio::buffer(data), error);

					if (error == boost::asio::error::eof) {
						watchdogClosedCallback();
						break; // Connection closed cleanly by peer.
					}
					else if (error) {
						throw boost::system::system_error(error); // Some other error.
					}
						

					if (1 == length) {
						if (
							3 == data[0]			// Ctrl + c
							|| 26 == data[0]	// Ctrl + z
							)
						{
							std::string byeMsg("good-bye~");
							boost::asio::write(sock, boost::asio::buffer(byeMsg.c_str(), static_cast<int>(byeMsg.length())));

							watchdogClosedCallback();

							break;
						}
					}


					recv_cmd += data;
					const std::string::size_type pos = recv_cmd.find_first_of("\r\n");
					if (pos == std::string::npos)
						continue;

					if (0 >= pos)
					{
						recv_cmd.clear();
						continue;
					}

					std::string cmd = recv_cmd.substr(0, pos);
					recv_cmd.erase(0, pos + 2);

					std::string res_callback = watchdogCallback(cmd, sock);
					res_callback += "\r\n";
					boost::asio::write(sock, boost::asio::buffer(res_callback.c_str(), static_cast<int>(res_callback.length())));
				}
			}
			catch (std::exception& e)
			{
				LOGE << "Exception in thread: " << e.what();
			}
		}

		void TestIntarface::startWatchdogService()
		{
			using boost::asio::ip::tcp;

			std::thread watchdogThread([&] {
				auto ios = m_ios.lock();
				if (ios)
				{
					tcp::acceptor acceptor(*ios, tcp::endpoint(tcp::v4(), m_watchDogPort));

					while (true) {
						tcp::socket sock(*ios);
						acceptor.accept(sock);
						std::thread(watchDogSession, std::move(sock), m_watchDogEventCallback, m_watchDogEventClosedCallback).detach();

						/*std::thread tt(watchDogSession, sock);*/
					}//while(true)
				}
			});
			watchdogThread.detach();
		}
	};//
};//namespace MLN
