#include "stdafx.h"
#include "messageProcedure.h"

#include "eventReceiver.h"

namespace mln
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

	bool MessageProcedure::registMessage(uint32_t protocol, msgHandlerFn fn) {
		return _staticElements.emplace(protocol, fn).second;
	}

	bool MessageProcedure::dispatch(Connection::sptr spConn, CircularStream::Ptr msg)
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
};//namespace mln
