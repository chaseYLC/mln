#include "stdafx.h"
#include "inputManager.h"

#ifdef _WIN32
#include <conio.h>
#endif

#include <stdarg.h> 
#include <iostream>
#include <boost/chrono.hpp>
#include <boost/thread.hpp>
#include <net/logManager.h>


namespace mln
{
	InputManager::InputManager()
	{
	}

	inline void setInputParam(const size_t paramIndex)
	{
	}

	void InputManager::waitKeyboardEvent()
	{
#ifdef _WIN32
		boost::thread keyEvntThread([&] {
			while (true) {
				boost::this_thread::sleep_for((boost::chrono::milliseconds(50)));

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
#endif
	}

	size_t InputManager::getFuncValue(const FUNC::value_type funcValue) const
	{
		return ((size_t)1) << funcValue;
	}

	bool InputManager::isUsingFunc(const FUNC::value_type funcValue) const
	{
		return 0 != (m_usingFunctions & getFuncValue(funcValue));
	}

	void InputManager::keyboardEventHandler(const unsigned int vKey)
	{
		defaultKeyboardEventHandler(vKey);
	}

	void InputManager::defaultKeyboardEventHandler(const unsigned int vKey)
	{
		switch (vKey)
		{
#ifdef _WIN32
		case VK_ESCAPE:
#else
		case 0x1B:
#endif
		{
			LOGI("pressed ESC key ...");

			if (false == m_ios->stopped()) {
				LOGI("terminating process...");
				m_ios->stop();
			}
		}
		break;
		}//switch (key)
	}


	void vv()
	{

	}

	void watchDogSession(boost::shared_ptr<boost::asio::ip::tcp::socket> sockPtr
		, InputManager::ConsoleCallbackType watchdogCallback
		, InputManager::ConsoleCloseCallbackType watchdogClosedCallback
	)
	{
		static const int max_length = 4096;
		boost::asio::ip::tcp::socket& sock = *sockPtr.get();

		try
		{
			std::string recv_cmd;
			char data[max_length];

			bool hiMsg = false;


			for (;;)
			{
				memset(data, 0, sizeof(data));
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
			/*LOGE << "Exception in thread: " << e.what();*/
		}
	}

	void InputManager::startWatchdogService()
	{
		using boost::asio::ip::tcp;

		boost::thread watchdogThread([&] {
			m_watchdogAcceptor = std::make_unique<tcp::acceptor>(*m_ios);
			tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), m_watchDogPort);
			m_watchdogAcceptor->open(endpoint.protocol());
			m_watchdogAcceptor->set_option(tcp::acceptor::reuse_address(true));
			m_watchdogAcceptor->bind(endpoint);
			m_watchdogAcceptor->listen();

			while (true) {
				auto sock = boost::make_shared<tcp::socket>(*m_ios);
				m_watchdogAcceptor->accept(*sock);

				boost::thread(watchDogSession
					, sock
					, m_watchDogEventCallback
					, m_watchDogEventClosedCallback
				).detach();
			}//while(true)
			});
		watchdogThread.detach();
	}
};//namespace mln
