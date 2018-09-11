#include "stdafx.h"
#include "MessageProcedure.h"

#include "EventReceiver.h"

namespace MLN
{
	namespace Net
	{
		MessageProcedure::MessageProcedure(MessageProcedure::customMessageParser msgParser
			, MsgUserManip* manip)
			: _packetParsingFunction(msgParser)
			, _packetHeaderManip(manip)
		{
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

		bool MessageProcedure::dispatch(Connection::ptr spConn, MessageBuffer::Ptr msg)
		{
			if (Connection::status::open != spConn->get_status()) {
				return false;
			}

			bool result = false;

			//if (true == isServicePending()) {
			if (NULL != _packetParsingFunction) {
				result = _packetParsingFunction(spConn, msg, *this, _instanceElements, _staticElements);
			}
			/*}*/

			spConn->decReadHandlerPendingCount();

			return result;
		}

	};//namespace Net
};//namespace MLN
