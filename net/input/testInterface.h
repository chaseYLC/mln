#pragma once

#include <memory>
#include <boost/asio.hpp>
#include <string>

#include <net/singleton.h>

namespace mln
{
	class TestIntarface
		: public Singleton<TestIntarface>
	{
	public:
		struct FUNC {
			using value_type = size_t;

			enum {
				func_start = 1,

				//--------------- begin
				wait_keyboardEvent = func_start,
				watchdog,
				//--------------- end

				func_end
			};
		};

		TestIntarface();
		virtual ~TestIntarface() = default;

		using ConsoleCallbackType =
			std::function< std::string(
				const std::string& inputMsg
				, boost::asio::ip::tcp::socket& sock
			)
			>;

		using ConsoleCloseCallbackType =
			std::function< void(
				)
			>;

		template <typename... ARGS>
		void start(boost::asio::io_context* ios, ARGS... funcs)
		{
			// set ios.
			m_ios = ios;

			// parsing functions..
			addUsingFunc(funcs...);

			if (true == isUsingFunc(FUNC::wait_keyboardEvent)) {
				waitKeyboardEvent();
			}

			if (true == isUsingFunc(FUNC::watchdog)) {
				startWatchdogService();
			}
		}

		template <typename... ARGS>
		inline void addUsingFunc(const FUNC::value_type funcValue, ARGS&&... remainFuncList)
		{
			addUsingFunc(funcValue);
			addUsingFunc(remainFuncList...);
		}

		void addUsingFunc(const FUNC::value_type funcValue) {
			m_usingFunctions += getFuncValue(funcValue);
		}

		void setMyKeyEventCallback(std::function< void(const unsigned int) >callback) {
			m_keybaoardEventCallback = callback;
		}

		void setWatchDogEventCallback(
			ConsoleCallbackType callback
			, ConsoleCloseCallbackType callbackClosed
			, unsigned short port
		)
		{
			m_watchDogEventCallback = callback;
			m_watchDogEventClosedCallback = callbackClosed;
			m_watchDogPort = port;
		}

	protected:
		void keyboardEventHandler(const unsigned int vKey);
		void defaultKeyboardEventHandler(const unsigned int vKey);

	protected:
		size_t getFuncValue(const FUNC::value_type funcValue) const;
		bool isUsingFunc(const FUNC::value_type funcValue) const;

		void waitKeyboardEvent();
		void startWatchdogService();

	protected:
		boost::asio::io_context* m_ios;
		size_t m_usingFunctions = 0;

		std::function< void(const unsigned int) > m_keybaoardEventCallback = nullptr;
		ConsoleCallbackType m_watchDogEventCallback = nullptr;
		ConsoleCloseCallbackType m_watchDogEventClosedCallback = nullptr;
		unsigned short m_watchDogPort = 0;

		std::unique_ptr<boost::asio::ip::tcp::acceptor> m_watchdogAcceptor;
	};

};// namespace mln