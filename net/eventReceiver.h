#pragma once

#include "connection.h"
#include <functional>
#include <stdint.h>

namespace mln
{
	class Connection;
	class MessageProcedure;

	class EventReceiver
	{
	public:
		using fp_default = std::function<void(Connection::sptr)>;
		using fp_update = std::function<void(uint64_t)>;
		using fp_noHandler = std::function<void(Connection::sptr, MessageBuffer&)>;
		using fp_initHandler = std::function<void(MessageProcedure*)>;

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

		void clone(EventReceiver* target)
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

	template< typename StaticFunctionsClass >
	class EventReceiverAcceptorRegister
		: public EventReceiver
	{
	public:
		EventReceiverAcceptorRegister(StaticFunctionsClass *inst)
		{
			set_onAccept(inst);
			set_onAcceptFailed(inst);
			set_onClose(inst);
			set_onCloseFailed(inst);
			set_onExpiredSession(inst);
			set_onUpdate(inst);
			set_noHandler(inst);
			set_initHandler(inst);
		}

	private:
		void set_onAccept(StaticFunctionsClass* instance) {
			onAccept = std::bind(&StaticFunctionsClass::onAccept, instance, std::placeholders::_1);
		}
		void set_onAcceptFailed(StaticFunctionsClass* instance) {
			onAcceptFailed = std::bind(&StaticFunctionsClass::onAcceptFailed, instance, std::placeholders::_1);
		}
		void set_onClose(StaticFunctionsClass* instance) {
			onClose = std::bind(&StaticFunctionsClass::onClose, instance, std::placeholders::_1);
		}
		void set_onCloseFailed(StaticFunctionsClass* instance) {
			onCloseFailed = std::bind(&StaticFunctionsClass::onCloseFailed, instance, std::placeholders::_1); 
		}
		void set_onExpiredSession(StaticFunctionsClass* instance) {
			onExpiredSession = std::bind(&StaticFunctionsClass::onExpiredSession, instance, std::placeholders::_1);
		}
		void set_onUpdate(StaticFunctionsClass* instance) {
			onUpdate = std::bind(&StaticFunctionsClass::onUpdate, instance, std::placeholders::_1);
		}
		void set_noHandler(StaticFunctionsClass* instance){
			noHandler = std::bind(&StaticFunctionsClass::noHandler, instance, std::placeholders::_1, std::placeholders::_2);
		}
		void set_initHandler(StaticFunctionsClass* instance) {
			initHandler = std::bind(&StaticFunctionsClass::initHandler, instance, std::placeholders::_1);
		}
	};

	template< typename StaticFunctionsClass >
	class EventReceiverConnectorRegister
		: public EventReceiver
	{
	public:
		EventReceiverConnectorRegister(StaticFunctionsClass* inst)
		{
			set_onConnect(inst);
			set_onConnectFailed(inst);
			set_onClose(inst);
			set_onCloseFailed(inst);
			set_onExpiredSession(inst);
			set_onUpdate(inst);
			set_noHandler(inst);
			set_initHandler(inst);
		}

	private:
		void set_onConnect(StaticFunctionsClass* instance) {
			onConnect = std::bind(&StaticFunctionsClass::onConnect, instance, std::placeholders::_1);
		}
		void set_onConnectFailed(StaticFunctionsClass* instance) {
			onConnectFailed = std::bind(&StaticFunctionsClass::onConnectFailed, instance, std::placeholders::_1);
		}
		void set_onClose(StaticFunctionsClass* instance) {
			onClose = std::bind(&StaticFunctionsClass::onClose, instance, std::placeholders::_1);
		}
		void set_onCloseFailed(StaticFunctionsClass* instance) {
			onCloseFailed = std::bind(&StaticFunctionsClass::onCloseFailed, instance, std::placeholders::_1);
		}
		void set_onExpiredSession(StaticFunctionsClass* instance) {
			onExpiredSession = std::bind(&StaticFunctionsClass::onExpiredSession, instance, std::placeholders::_1);
		}
		void set_onUpdate(StaticFunctionsClass* instance) {
			onUpdate = std::bind(&StaticFunctionsClass::onUpdate, instance, std::placeholders::_1);
		}
		void set_noHandler(StaticFunctionsClass* instance) {
			noHandler = std::bind(&StaticFunctionsClass::noHandler, instance, std::placeholders::_1, std::placeholders::_2);
		}
		void set_initHandler(StaticFunctionsClass* instance) {
			initHandler = std::bind(&StaticFunctionsClass::initHandler, instance, std::placeholders::_1);
		}
	};
};//namespace mln