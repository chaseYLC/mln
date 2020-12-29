#pragma once

#include <functional>
#include <tuple>
#include <unordered_map>

#include "connection.h"
#include "messageBuffer.h"

namespace mln
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

		using msgHandlerFn = std::function<bool(Connection::sptr, uint32_t, MessageBuffer&)>;
		using msgMapTy = std::unordered_map<uint32_t, msgHandlerFn >;

		using customMessageParser = std::function<bool(Connection::sptr, MessageBuffer::Ptr
			, MessageProcedure&, msgMapTy&, msgMapTy&)>;

		using msgHandlerFn_cipherInit = std::function<bool(const uint8_t, void*, Connection::sptr, uint32_t, MessageBuffer&)>;
		using msgHandlerFn_cipherDEC = std::function<bool(const uint8_t, Connection::sptr, uint32_t, MessageBuffer&, MessageBuffer&)>;
		using msgHandlerFn_headerBody = std::function<bool(Connection::sptr, void*, MessageBuffer&)>;

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
			, bool(__type::* fn)(Connection::sptr, uint32_t, MessageBuffer&)
			, __type* instance)
		{
			auto it = _instanceElements.find(protocol);

			if (_instanceElements.end() != it) {
				return false;
			}

			_instanceElements.insert(std::make_pair(protocol, std::bind(fn, instance
				, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));
			return true;
		}

		template< typename __type >
		void setCipher(
			const uint8_t encType
			, bool(__type::* fnEnc)(uint8_t, void*, Connection::sptr, uint32_t, MessageBuffer&)
			, bool(__type::* fnDec)(uint8_t, Connection::sptr, uint32_t, MessageBuffer&, MessageBuffer&)
			, __type* instance)
		{
			_cipherHandlers[encType] = std::make_tuple(encType
				, std::bind(fnEnc, instance, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5)
				, std::bind(fnDec, instance, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5)
			);

			setUserCipherType(encType);
		}

		void setCipher(uint8_t encType, msgHandlerFn_cipherInit initFunc, msgHandlerFn_cipherDEC decFunc) {
			_cipherHandlers[encType] = std::make_tuple(encType
				, initFunc
				, decFunc);
			setUserCipherType(encType);
		}


		template< typename __type >
		void setMsgHandler_HeaderBody(
			bool(__type::* fn)(Connection::sptr, void*, MessageBuffer&)
			, __type* instance)
		{
			_msgHandlerHeaderBody = std::bind(fn, instance, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
		}

		void setMsgHandler_HeaderBody(msgHandlerFn_headerBody func) {
			_msgHandlerHeaderBody = func;
		}

		encParameters& getCipherHandler(const uint8_t encType) {
			return _cipherHandlers[encType];
		}
		msgHandlerFn_headerBody& getMsgHandler_HeaderBody() { return _msgHandlerHeaderBody; }
		void setUserCipherType(const uint8_t cipherType) { _userCipherType = cipherType; }
		uint16_t getUserCipherType() const { return _userCipherType; }

	protected:
		bool dispatch(Connection::sptr spConn, MessageBuffer::Ptr msg);
		/*virtual bool isServicePending() const = 0;
		virtual void detatchReceiver() = 0;*/

	private:
		msgMapTy _instanceElements;
		msgMapTy _staticElements;

		uint16_t		_userCipherType = NOT_SET_CIPHER_VALUE;
		encParameters	_cipherHandlers[UINT8_MAX];
		msgHandlerFn_headerBody _msgHandlerHeaderBody = NULL;

	protected:
		//event_receiver& _receiver;
		customMessageParser _packetParsingFunction;
		MsgUserManip* _packetHeaderManip;
	};
};//namespace mln