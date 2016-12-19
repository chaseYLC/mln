#include "stdafx.h"
#include "MessageProcedure.h"

#include "EventReceiver.h"

namespace MLN
{
	namespace Net
	{
		MessageProcedure::MessageProcedure(EventReceiver& receiver
			, MessageProcedure::customMessageParser msgParser
			, MLN::Base::customHeaderManipulator* manip)
			: _receiver(receiver)
			, _packetParsingFunction(msgParser)
			, _packetHeaderManip(manip)
		{
			memset(_ciperHandlers, 0, sizeof(_ciperHandlers));
		}

		MessageProcedure::~MessageProcedure()
		{
			_instanceElements.clear();
			_staticElements.clear();
		}

		bool MessageProcedure::registMessage(unsigned int protocol, msgHandlerFn fn) {
			auto it = _staticElements.find(protocol);

			if (_staticElements.end() != it) {
				return false;
			}

			_staticElements.insert(std::make_pair(protocol, fn));
			return true;
		}

		bool MessageProcedure::dispatch(Connection::ptr spConn, MLN::Base::MessageBuffer::ptr msg)
		{
			bool result = false;

			if (NULL != _packetParsingFunction){
				result = _packetParsingFunction(spConn, msg, *this, _instanceElements, _staticElements);
			}

			spConn->decReadHandlerPendingCount();

			return result;
		}

	};//namespace Net
};//namespace MLN
