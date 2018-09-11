#ifndef _MLN_BET_EVENT_RECEIVER_H_
#define _MLN_BET_EVENT_RECEIVER_H_

#include "Connection.h"
#include <functional>
#include <stdint.h>

namespace MLN
{
	namespace Net
	{
		class Connection;
		class MessageProcedure;
		class MessageProcedure;

		class EventReceiver
		{
		protected:
			virtual void set_onAccept() = 0;
			virtual void set_onAcceptFailed() = 0;

			virtual void set_onConnect() = 0;
			virtual void set_onConnectFailed() = 0;

			virtual void set_onClose() = 0;
			virtual void set_onCloseFailed() = 0;

			virtual void set_onExpiredSession() = 0;

			virtual void set_onUpdate() = 0;

			virtual void set_noHandler() = 0;

			virtual void set_initHandler() = 0;

		public:

			typedef void(*fp_default) (Connection::ptr);
			typedef void(*fp_update) (uint64_t);
			typedef void(*fp_noHandler) (Connection::ptr, MessageBuffer&);
			typedef void(*fp_initHandler) (MessageProcedure *);

			fp_default onAccept = nullptr;
			fp_default onAcceptFailed = nullptr;

			fp_default onConnect = nullptr;
			fp_default onConnectFailed = nullptr;

			fp_default onClose = nullptr;
			fp_default onCloseFailed = nullptr;

			fp_default onExpiredSession = nullptr;

			fp_update onUpdate = nullptr;

			fp_noHandler noHandler = nullptr;

			fp_initHandler initHandler = nullptr;

		public:
			EventReceiver()
			{
				static_assert(false == std::is_member_function_pointer<decltype(onAccept)>::value
					, "use static-function for handler-Function");
				static_assert(false == std::is_member_function_pointer<decltype(onAcceptFailed)>::value
					, "use static-function for handler-Function");
				static_assert(false == std::is_member_function_pointer<decltype(onConnect)>::value
					, "use static-function for handler-Function");
				static_assert(false == std::is_member_function_pointer<decltype(onConnectFailed)>::value
					, "use static-function for handler-Function");
				static_assert(false == std::is_member_function_pointer<decltype(onClose)>::value
					, "use static-function for handler-Function");
				static_assert(false == std::is_member_function_pointer<decltype(onCloseFailed)>::value
					, "use static-function for handler-Function");
				static_assert(false == std::is_member_function_pointer<decltype(onExpiredSession)>::value
					, "use static-function for handler-Function");
				static_assert(false == std::is_member_function_pointer<decltype(onUpdate)>::value
					, "use static-function for handler-Function");
				static_assert(false == std::is_member_function_pointer<decltype(noHandler)>::value
					, "use static-function for handler-Function");
				static_assert(false == std::is_member_function_pointer<decltype(initHandler)>::value
					, "use static-function for handler-Function");
			}

			void clone(EventReceiver *target)
			{
				target->onAccept = this->onAccept;
				target->onAcceptFailed = this->onAcceptFailed;
				target->onConnect = this->onConnect;
				target->onConnectFailed = this->onConnectFailed;
				target->onClose = this->onClose;
				target->onCloseFailed = this->onCloseFailed;
				target->onExpiredSession = this->onExpiredSession;
				target->onUpdate = this->onUpdate;
				target->noHandler = this->noHandler;
				target->initHandler = this->initHandler;
			}

		};

		class EventReceiverDummy
			: public EventReceiver
		{
		protected:
			void set_onAccept() override {}
			void set_onAcceptFailed() override {}

			void set_onConnect() override {}
			void set_onConnectFailed() override {}

			void set_onClose() override {}
			void set_onCloseFailed() override {}

			void set_onExpiredSession() override {}

			void set_onUpdate() override {}

			void set_noHandler() override {}

			void set_initHandler() override {}
		};


		class EventReceiverAcceptor
			: public EventReceiver
		{
		private:
			void set_onConnect() override {}
			void set_onConnectFailed() override {}
		};

		class EventReceiverConnector
			: public EventReceiver
		{
		private:
			void set_onAccept() override {}
			void set_onAcceptFailed() override {}
		};



		template< typename StaticFunctionsClass >
		class EventReceiverAcceptorRegister
			: public EventReceiverAcceptor
		{
		public:
			EventReceiverAcceptorRegister()
			{
				set_onAccept();
				set_onAcceptFailed();
				set_onClose();
				set_onCloseFailed();
				set_onExpiredSession();
				set_onUpdate();
				set_noHandler();
				set_initHandler();
			}

		private:
			void set_onAccept() override {
				onAccept = StaticFunctionsClass::onAccept;
			}
			void set_onAcceptFailed() override {
				onAcceptFailed = StaticFunctionsClass::onAcceptFailed;
			}
			void set_onClose() override {
				onClose = StaticFunctionsClass::onClose;
			}
			void set_onCloseFailed() override {
				onCloseFailed = StaticFunctionsClass::onCloseFailed;
			}
			void set_onExpiredSession() override {
				onExpiredSession = StaticFunctionsClass::onExpiredSession;
			}
			void set_onUpdate() override {
				onUpdate = StaticFunctionsClass::onUpdate;
			}
			void set_noHandler() override {
				noHandler = StaticFunctionsClass::noHandler;
			}
			void set_initHandler() override {
				initHandler = StaticFunctionsClass::initHandler;
			}
		};

		template< typename StaticFunctionsClass >
		class EventReceiverConnectorRegister
			: public EventReceiverConnector
		{
		public:
			EventReceiverConnectorRegister()
			{
				set_onConnect();
				set_onConnectFailed();
				set_onClose();
				set_onCloseFailed();
				set_onExpiredSession();
				set_onUpdate();
				set_noHandler();
				set_initHandler();
			}

		private:
			void set_onConnect() override {
				onConnect = StaticFunctionsClass::onConnect;
			}
			void set_onConnectFailed() override {
				onConnectFailed = StaticFunctionsClass::onConnectFailed;
			}
			void set_onClose() override {
				onClose = StaticFunctionsClass::onClose;
			}
			void set_onCloseFailed() override {
				onCloseFailed = StaticFunctionsClass::onCloseFailed;
			}
			void set_onExpiredSession() override {
				onExpiredSession = StaticFunctionsClass::onExpiredSession;
			}
			void set_onUpdate() override {
				onUpdate = StaticFunctionsClass::onUpdate;
			}
			void set_noHandler() override {
				noHandler = StaticFunctionsClass::noHandler;
			}
			void set_initHandler() override {
				initHandler = StaticFunctionsClass::initHandler;
			}
		};
	};//namespace Net
};//MLN

#endif//_MLN_BET_EVENT_RECEIVER_H_