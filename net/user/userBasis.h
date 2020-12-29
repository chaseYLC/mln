#pragma once

#include <optional>
#include <memory>
#include <utility>
#include "../connection.h"
#include "../packetEncType.h"

namespace mln {

	class Room;

	class UserBasis
	{
		friend class UserManagerBasis;

	public:
		using sptr = std::shared_ptr<UserBasis>;
		using wptr = std::weak_ptr<UserBasis>;

		UserBasis(mln::Connection::sptr conn);
		virtual ~UserBasis() = default;

		bool initEncKey(const mln::EncType::Type encType, void *, char *buffer, const int bufferSize, const bool writeHeader);

		int32_t Send(char* data, uint32_t size, const bool writeHeader);

		template <typename T>
		int32_t Send(T &data, const bool writeHeader = true)
		{
			return Send((char*)&data, sizeof(data), writeHeader);
		}

		int32_t Decrypt(
			const mln::EncType::Type encType
			, char *entryptedData, const uint32_t entryptedDataSize
			, char* decryptedBuffer, const uint32_t decryptedBufferSize);


		std::optional<mln::Connection::sptr> getConn();

		void CloseReserve(const size_t timeAfterMs);

		virtual std::string GetUserID() const = 0;

	public:
		inline static mln::EncType::Type m_encType = mln::EncType::GREETING;

	public:
		mln::Connection::wptr _conn;
		bool m_connected = false;

		std::weak_ptr<boost::asio::io_context::strand> _wpStrandRoom;
	};

}//namespace mln {