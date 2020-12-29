#pragma once

#include <boost/container/vector.hpp>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <tuple>

#include "memoryPool.h"

namespace mln
{
	using MsgUserManip = std::tuple<
		boost::function<size_t()>//size_t getHeaderSize(char *buffer)
		, boost::function<void(size_t, char*)> //void facilitate(size_t, char *)
	>;

	class MessageBuffer
		: public MemoryPool< MessageBuffer >
	{
	public:
		using Ptr = boost::shared_ptr< MessageBuffer >;

		static const size_t	MAX_BUFFER_SIZE = 1024 * 32;

		MessageBuffer();
		MessageBuffer(MsgUserManip* msgManip, const bool preWriteAsHeaderSize = false);
		MessageBuffer(const MessageBuffer& e);

		void clear();
		size_t size() const;
		void facilitate();
		char* data() const;
		bool readable(char* dst, size_t count);
		bool readable(size_t count) const;
		char* enableBuffer() const;
		size_t remainWriteSize() const;
		void arrange();

		MessageBuffer& write(char* src, size_t count);
		MessageBuffer& write(size_t count);

		MessageBuffer& read(char* dst, size_t count);
		MessageBuffer& read(size_t count);
		MessageBuffer& readAll();

		template< typename T >
		inline MessageBuffer& operator >> (T& b) {
			return read((char*)&b, sizeof(b));
		}

		template< typename T >
		inline MessageBuffer& operator << (T& b) {
			return write((char*)&b, sizeof(b));
		}

		template< typename T >
		inline MessageBuffer& operator << (T&& b) {
			return write((char*)&b, sizeof(b));
		}

		void setManipulator(MsgUserManip* manip) { _manip = manip; }

	private:

		char	_buffer[MAX_BUFFER_SIZE];
		char* _read = nullptr;
		char* _write = nullptr;
		MsgUserManip* _manip = nullptr;
	};
};//namespace mln
