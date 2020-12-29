#include "stdafx.h"

#include <cassert>
#include <type_traits>
#include <utility>

#include "circularStream.h"
#include "logManager.h"

namespace mln
{
	CircularStream::CircularStream()
	{
		clear();
	}

	CircularStream::CircularStream(MsgUserManip* msgManip, const bool preWriteAsHeaderSize /*= false*/)
		: _manip(msgManip)
	{
		clear();

		if (true == preWriteAsHeaderSize) {
			write(std::get<0>(*msgManip)());
		}

	}

	CircularStream::CircularStream(const CircularStream& e)
	{
		clear();

		write(const_cast<CircularStream&>(e)._buffer, const_cast<CircularStream&>(e).size());
	}

	void CircularStream::clear()
	{
		_read = _buffer;
		_write = _buffer;
	}

	size_t CircularStream::size() const
	{
		return _write - _read;
	}

	void CircularStream::facilitate()
	{
		if (_manip) {
			std::get<1>(*_manip)(size(), _read);
		}
	}

	char* CircularStream::data() const
	{
		return _read;
	}

	bool CircularStream::readable(char* dst, size_t count)
	{
		if (false == readable(count)) {
			return false;
		}
		memcpy(dst, _read, count);
		return true;
	}

	bool CircularStream::readable(size_t count) const
	{
		return _read + count <= _write;
	}

	char* CircularStream::enableBuffer() const
	{
		return _write;
	}

	size_t CircularStream::remainWriteSize() const
	{
		return _buffer + MAX_BUFFER_SIZE - _write;
	}

	void CircularStream::arrange()
	{
		if (_read == _write) {
			clear();
		}
		else
		{
			size_t count = size();
			memmove(_buffer, _read, count);
			_read = _buffer;
			_write = _read + count;
		}
	}

	CircularStream& CircularStream::write(char* src, size_t count)
	{
		if (MAX_BUFFER_SIZE < (size() + count))
		{
			throw std::runtime_error("buffer overrun in CircularStream");
		}

		memcpy(_write, src, count);
		_write += count;
		return *this;
	}


	CircularStream& CircularStream::write(size_t count)
	{
		if (MAX_BUFFER_SIZE < (size() + count))
		{
			throw std::runtime_error("buffer overrun in CircularStream");
		}

		_write += count;
		return *this;
	}

	CircularStream& CircularStream::read(char* dst, size_t count)
	{
		if (false == readable(count))
		{
			throw std::runtime_error("buffer can't readable in CircularStream");
		}

		if (remainWriteSize() < MAX_BUFFER_SIZE / 3 * 2)
		{
			arrange();
		}

		memcpy(dst, _read, count);
		_read += count;

		return *this;
	}

	CircularStream& CircularStream::read(size_t count)
	{
		if (false == readable(count))
		{
			throw std::runtime_error("buffer can't readable in CircularStream");
		}

		if (remainWriteSize() < MAX_BUFFER_SIZE / 3 * 2)
		{
			arrange();
		}

		_read += count;

		return *this;
	}

	CircularStream& CircularStream::readAll()
	{
		_read = _write = _buffer;
		return *this;
	}
};//namespace mln