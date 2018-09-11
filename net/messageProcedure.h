#pragma once

#include <boost/unordered_map.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <tuple>

#include "Connection.h"
#include "messageBuffer.h"

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
			enum {
				NOT_SET_CIPHER_VALUE = UINT8_MAX + 1,
			};

			using msgHandlerFn = boost::function<bool(Connection::ptr, uint32_t, MessageBuffer&)>;
			using msgMapTy = boost::unordered_map<uint32_t, msgHandlerFn >;

			using customMessageParser = boost::function<bool(Connection::ptr, MessageBuffer::Ptr
				, MessageProcedure&, msgMapTy&, msgMapTy&)>;

			using msgHandlerFn_cipherInit = boost::function<bool(const uint8_t, void *, Connection::ptr, uint32_t, MessageBuffer&)>;
			using msgHandlerFn_cipherDEC = boost::function<bool(const uint8_t, Connection::ptr, uint32_t, MessageBuffer&, OUT MessageBuffer&)>;
			using msgHandlerFn_headerBody = boost::function<bool(Connection::ptr, void *, MessageBuffer&)>;

			using encParameters = std::tuple<uint8_t	// enc type
				, msgHandlerFn_cipherInit		// Init function type
				, msgHandlerFn_cipherDEC		// decryptor function type
			>;

			MessageProcedure(
				MessageProcedure::customMessageParser msgParser
				, MsgUserManip* manip);
			~MessageProcedure();

			bool registMessage(uint32_t protocol, msgHandlerFn fn);

			template< typename __type >
			bool registMessage(
				unsigned int protocol
				, bool(__type::*fn)(Connection::ptr, uint32_t, MessageBuffer&)
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
				, bool(__type::*fnEnc)(uint8_t, void *, Connection::ptr, uint32_t, MessageBuffer&)
				, bool(__type::*fnDec)(uint8_t, Connection::ptr, uint32_t, MessageBuffer&, MessageBuffer&)
				, __type* instance)
			{
				_ciperHandlers[encType] = std::make_tuple(encType
					, boost::bind(fnEnc, instance, _1, _2, _3, _4, _5)
					, boost::bind(fnDec, instance, _1, _2, _3, _4, _5)
					);

				setUserCipherType(encType);
			}

			void setCipher(uint8_t encType, msgHandlerFn_cipherInit initFunc, msgHandlerFn_cipherDEC decFunc) {
				_ciperHandlers[encType] = std::make_tuple(encType
					, initFunc
					, decFunc);
				setUserCipherType(encType);
			}


			template< typename __type >
			void setMsgHandler_HeaderBody(
				bool(__type::*fn)(Connection::ptr, void *, MessageBuffer&)
				, __type* instance)
			{
				_msgHandlerHeaderBody = boost::bind(fn, instance, _1, _2, _3);
			}

			void setMsgHandler_HeaderBody(msgHandlerFn_headerBody func) {
				_msgHandlerHeaderBody = func;
			}

			encParameters& getCipherHandler(const uint8_t encType) {
				return _ciperHandlers[encType];
			}
			msgHandlerFn_headerBody& getMsgHandler_HeaderBody() { return _msgHandlerHeaderBody; }
			void setUserCipherType(const uint8_t cipherType) { _userCipherType = cipherType; }
			uint16_t getUserCipherType() const { return _userCipherType; }

		protected:
			bool dispatch(Connection::ptr spConn, MessageBuffer::Ptr msg);
			/*virtual bool isServicePending() const = 0;
			virtual void detatchReceiver() = 0;*/

		private:
			boost::unordered_map<uint32_t, msgHandlerFn > _instanceElements;
			boost::unordered_map<uint32_t, msgHandlerFn> _staticElements;

			uint16_t		_userCipherType = NOT_SET_CIPHER_VALUE;
			encParameters	_ciperHandlers[UINT8_MAX];
			msgHandlerFn_headerBody _msgHandlerHeaderBody = NULL;

		protected:
			//event_receiver& _receiver;
			customMessageParser _packetParsingFunction;
			MsgUserManip *_packetHeaderManip;
		};
	};//namespace Net
};//MLN::Base::