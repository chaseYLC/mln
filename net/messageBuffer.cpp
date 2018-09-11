#include "stdafx.h"

#include <cassert>
#include <type_traits>
#include <utility>

#include "messageBuffer.h"
#include "logger.h"

namespace MLN
{
	namespace Net
	{
		MessageBuffer::MessageBuffer()
		{
			clear();
		}

		MessageBuffer::MessageBuffer(MsgUserManip *msgManip, const bool preWriteAsHeaderSize /*= false*/)
			: _manip(msgManip)
		{
			clear();

			if (true == preWriteAsHeaderSize){
				write(std::get<0>(*msgManip)());
			}
			
		}

		MessageBuffer::MessageBuffer(const MessageBuffer& e)
		{
			clear();

			write(const_cast<MessageBuffer&>(e)._buffer, const_cast<MessageBuffer&>(e).size());
		}

		void MessageBuffer::clear()
		{
			_read = _buffer;
			_write = _buffer;
		}

		size_t MessageBuffer::size() const
		{
			return _write - _read;
		}

		void MessageBuffer::facilitate()
		{
			if (_manip){
				std::get<1>(*_manip)(size(), _read);
			}
		}

		char* MessageBuffer::data() const
		{
			return _read;
		}

		bool MessageBuffer::readable(char* dst, size_t count)
		{
			if (false == readable(count)){
				return false;
			}
			memcpy(dst, _read, count);
			return true;
		}

		bool MessageBuffer::readable(size_t count) const
		{
			return _read + count <= _write;
		}

		char* MessageBuffer::enableBuffer() const
		{
			return _write;
		}

		size_t MessageBuffer::remainWriteSize() const
		{
			return _buffer + MAX_BUFFER_SIZE - _write;
		}

		void MessageBuffer::arrange()
		{
			if (_read == _write){
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

		MessageBuffer& MessageBuffer::write(char* src, size_t count)
		{
			if (MAX_BUFFER_SIZE < (size() + count))
			{
				throw std::exception("buffer overrun in MessageBuffer");
			}

			memcpy(_write, src, count);
			_write += count;
			return *this;
		}


		MessageBuffer& MessageBuffer::write(size_t count)
		{
			if (MAX_BUFFER_SIZE < (size() + count))
			{
				throw std::exception("buffer overrun in MessageBuffer");
			}

			_write += count;
			return *this;
		}

		MessageBuffer& MessageBuffer::read(char* dst, size_t count)
		{
			if (false == readable(count))
			{
				throw std::exception("buffer can't readable in MessageBuffer");
			}

			if (remainWriteSize() < MAX_BUFFER_SIZE / 3 * 2)
			{
				arrange();
			}

			memcpy(dst, _read, count);
			_read += count;

			return *this;
		}

		MessageBuffer& MessageBuffer::read(size_t count)
		{
			if (false == readable(count))
			{
				throw std::exception("buffer can't readable in MessageBuffer");
			}

			if (remainWriteSize() < MAX_BUFFER_SIZE / 3 * 2)
			{
				arrange();
			}

			_read += count;

			return *this;
		}

		MessageBuffer& MessageBuffer::readAll()
		{
			_read = _write = _buffer;
			return *this;
		}
	};//namespace Net

};//namespace MLN