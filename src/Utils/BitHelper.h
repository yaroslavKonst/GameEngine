#ifndef _BIT_HELPER
#define _BIT_HELPER

class BitReader
{
public:
	BitReader(const uint8_t* data, size_t size)
	{
		_data = data;
		_size = size;

		_position = 0;
		_currentBytePos = 0;
	}

	bool Get(uint8_t& bit)
	{
		if (_position >= _size) {
			return false;
		}

		++_position;

		if (_currentBytePos == 0) {
			_currentByte = *_data;
			++_data;

			_currentBytePos = 8;
		}

		bit = (_currentByte & 0b10000000) >> 7;

		_currentByte <<= 1;
		--_currentBytePos;

		return true;
	}

private:
	const uint8_t* _data;
	size_t _size;
	size_t _position;

	uint8_t _currentByte;
	uint8_t _currentBytePos;
};

class BitWriter
{
public:
	BitWriter()
	{
		_size = 0;
		_currentByte = 0;
		_currentBytePos = 0;
	}

	void Put(uint8_t bit)
	{
		_currentByte <<= 1;
		_currentByte |= bit & 0b1;

		++_currentBytePos;
		++_size;

		if (_currentBytePos == 8) {
			_data.push_back(_currentByte);
			_currentBytePos = 0;
			_currentByte = 0;
		}
	}

	std::vector<uint8_t> Get(size_t& size)
	{
		if (_currentBytePos > 0) {
			_currentByte <<= 8 - _currentBytePos;
			_data.push_back(_currentByte);
		}

		std::vector<uint8_t> data = _data;
		size = _size;

		_data.clear();
		_size = 0;
		_currentByte = 0;
		_currentBytePos = 0;

		return data;
	}

private:
	std::vector<uint8_t> _data;

	size_t _size;

	uint8_t _currentByte;
	uint8_t _currentBytePos;
};

#endif
