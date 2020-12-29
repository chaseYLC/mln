#include "stdafx.h"
#include "userBasis.h"

#include "../logManager.h"
#include "../messageBuffer.h"

namespace mln {

	UserBasis::UserBasis(mln::Connection::sptr conn)
	{
		_conn = conn;
	}

	bool UserBasis::initEncKey(const mln::EncType::Type encType, void *, char *buffer, const int bufferSize, const bool writeHeader)
	{
		switch (encType)
		{
		case mln::EncType::GREETING:
			// send
			if (auto spConn = _conn.lock(); spConn) {
				spConn->sendPacket((void*)("hello"), 5, writeHeader);
			}
			break;
		}//switch (encType)

		return true;
	}


	int32_t UserBasis::Send(char* data, uint32_t size, const bool writeHeader)
	{
		switch (m_encType)
		{
		case mln::EncType::GREETING:
		{
			if (auto spConn = _conn.lock(); spConn) {
				spConn->sendPacket(data, size, writeHeader);
				return size;
			}
			return 0;
			
		}break;
		}//switch (header.encType)

		return 0;
	}

	int32_t UserBasis::Decrypt(
		const mln::EncType::Type encType
		, char *entryptedData, const uint32_t entryptedDataSize
		, char* decryptedBuffer, const uint32_t decryptedBufferSize)
	{
		switch (m_encType)
		{
		case mln::EncType::GREETING:
			break;

		}//switch (m_encType)

		return 0;
	}

	std::optional<mln::Connection::sptr> UserBasis::getConn()
	{
		if (auto spConn = _conn.lock(); spConn) {
			return spConn;
		}
		return std::nullopt;
	}

	void UserBasis::CloseReserve(const size_t timeAfterMs)
	{
		if (auto spConn = _conn.lock(); spConn) {
			spConn->closeReserve(timeAfterMs);
		}
	}
}

