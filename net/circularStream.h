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

	class CircularStream
		: public MemoryPool< CircularStream >
	{
	public:
		using Ptr = boost::shared_ptr< CircularStream >;

		static const size_t	MAX_BUFFER_SIZE = 1024 * 32;

		CircularStream();
		CircularStream(MsgUserManip* msgManip, const bool preWriteAsHeaderSize = false);
		CircularStream(const CircularStream& e);

		void clear();
		size_t size() const;
		void facilitate();
		char* data() const;
		bool readable(char* dst, size_t count);
		bool readable(size_t count) const;
		char* enableBuffer() const;
		size_t remainWriteSize() const;
		void arrange();

		CircularStream& write(char* src, size_t count);
		CircularStream& write(size_t count);

		CircularStream& read(char* dst, size_t count);
		CircularStream& read(size_t count);
		CircularStream& readAll();

		template< typename T >
		inline CircularStream& operator >> (T& b) {
			return read((char*)&b, sizeof(b));
		}

		template< typename T >
		inline CircularStream& operator << (T& b) {
			return write((char*)&b, sizeof(b));
		}

		template< typename T >
		inline CircularStream& operator << (T&& b) {
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
