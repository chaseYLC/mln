#pragma once

#include <boost/unordered_map.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <tuple>

#include "connection.h"
#include <Base/Base/include/messageBuffer.h>

namespace MLN
{
	namespace Net
	{
		class EventReceiver;

		class MessageProcedure
		{
			friend class Connection;
			friend class ConnectionImpl;

		public:
			using msgHandlerFn = boost::function<bool(Connection::ptr, unsigned int, MLN::Base::MessageBuffer&)>;
			using msgMapTy = boost::unordered_map<unsigned int, msgHandlerFn >;

			using customMessageParser = boost::function<bool(Connection::ptr, MLN::Base::MessageBuffer::ptr
				, MessageProcedure&, msgMapTy&, msgMapTy&)>;

			using msgHandlerFn_cipherInit = boost::function<bool(const uint8_t, void *, Connection::ptr, unsigned int, MLN::Base::MessageBuffer&)>;
			using msgHandlerFn_cipherDEC = boost::function<bool(const uint8_t, Connection::ptr, unsigned int, MLN::Base::MessageBuffer&, OUT MLN::Base::MessageBuffer&)>;
			using msgHandlerFn_headerBody = boost::function<bool(Connection::ptr, void *, MLN::Base::MessageBuffer&)>;

			using encParameters = std::tuple<uint8_t	// enc type
				, msgHandlerFn_cipherInit		// Init function type
				, msgHandlerFn_cipherDEC		// decryptor function type
			>;

			MessageProcedure(EventReceiver& receiver
				, MessageProcedure::customMessageParser msgParser
				, MLN::Base::customHeaderManipulator* manip);
			~MessageProcedure();

			bool registMessage(unsigned int protocol, msgHandlerFn fn);

			template< typename __type >
			bool registMessage(
				unsigned int protocol
				, bool(__type::*fn)(Connection::ptr, unsigned int, MLN::Base::MessageBuffer&)
				, __type* instance)
			{
				auto it = _instanceElements.find(protocol);

				if (_instanceElements.end() != it) {
					return false;
				}

				_instanceElements.insert(std::make_pair(protocol, boost::bind(fn, instance, _1, _2, _3)));
				return true;
			}

			template< typename __type >
			void setCipher(
				const uint8_t encType
				, bool(__type::*fnEnc)(uint8_t, void *, Connection::ptr, unsigned int, MLN::Base::MessageBuffer&)
				, bool(__type::*fnDec)(uint8_t, Connection::ptr, unsigned int, MLN::Base::MessageBuffer&, MLN::Base::MessageBuffer&)
				, __type* instance)
			{
				_ciperHandlers[encType] = std::make_tuple(encType
					, boost::bind(fnEnc, instance, _1, _2, _3, _4, _5)
					, boost::bind(fnDec, instance, _1, _2, _3, _4, _5)
					);
			}

			void setCipher(uint8_t encType, msgHandlerFn_cipherInit initFunc, msgHandlerFn_cipherDEC decFunc){
				_ciperHandlers[encType] = std::make_tuple(encType
					, initFunc
					, decFunc);
			}


			template< typename __type >
			void setMsgHandler_HeaderBody(
				bool(__type::*fn)(Connection::ptr, void *, MLN::Base::MessageBuffer&)
				, __type* instance)
			{
				_msgHandlerHeaderBody = boost::bind(fn, instance, _1, _2, _3);
			}

			void setMsgHandler_HeaderBody(msgHandlerFn_headerBody func){
				_msgHandlerHeaderBody = func;
			}

			EventReceiver& getReceiver() { return _receiver; }
			encParameters& getCipherHandler(const uint8_t encType){
				return _ciperHandlers[encType];
			}
			msgHandlerFn_headerBody& getMsgHandler_HeaderBody(){ return _msgHandlerHeaderBody; }

		protected:
			bool dispatch(Connection::ptr spConn, MLN::Base::MessageBuffer::ptr msg);

		private:
			boost::unordered_map<unsigned int, msgHandlerFn > _instanceElements;
			boost::unordered_map<unsigned int, msgHandlerFn> _staticElements;

			encParameters	_ciperHandlers[UINT8_MAX];
			msgHandlerFn_headerBody _msgHandlerHeaderBody = NULL;

		protected:
			EventReceiver& _receiver;
			customMessageParser _packetParsingFunction;
			MLN::Base::customHeaderManipulator *_packetHeaderManip;
		};
	};//namespace Net
};//MLN::Base::