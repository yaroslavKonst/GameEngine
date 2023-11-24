#ifndef _RING_BUFFER_H
#define _RING_BUFFER_H

#include <vector>
#include <mutex>

template<typename T>
class RingBuffer
{
public:
	RingBuffer(size_t size)
	{
		_buffer.resize(size);
		_size = size;
		_begin = 0;
		_end = 0;
	}

	bool IsEmpty()
	{
		return _begin == _end;
	}

	void Insert(const T& item)
	{
		_insertMutex.lock();

		_buffer[_end] = item;

		if (_end == _size - 1) {
			_end = 0;
		} else {
			++_end;
		}

		_insertMutex.unlock();
	}

	T Get()
	{
		size_t pos = _begin;

		if (_begin == _size - 1) {
			_begin = 0;
		} else {
			++_begin;
		}

		return _buffer[pos];
	}

private:
	std::vector<T> _buffer;
	size_t _size;
	size_t _begin;
	size_t _end;

	std::mutex _insertMutex;
};

#endif
