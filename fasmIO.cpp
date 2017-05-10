#include "fasmIO.h"

#include <cassert>

_fasmcpp::AssemblerIO::AssemblerIO()
{
}

_fasmcpp::AssemblerIO::AssemblerIO(const char* string)
	: _buffer(string, string + strlen(string))
{
}

_fasmcpp::AssemblerIO::AssemblerIO(const char* buffer, size_t size)
	: _buffer(buffer, buffer + size)
{
}

void _fasmcpp::AssemblerIO::open()
{
	assert(!_open);

	_open = true;
	_position = 0;
}

size_t _fasmcpp::AssemblerIO::getPosition() const
{
	assert(_open);
	return _position;
}

void _fasmcpp::AssemblerIO::setPosition(int offset, OffsetMode mode)
{
	assert(_open);

	switch (mode)
	{
	case AssemblerIO::Begin:
		_position = offset;
		break;

	case AssemblerIO::Current:
		_position += offset;
		break;

	case AssemblerIO::End:
		_position = getSize() + offset;
		break;

	default:
		assert(0);
	}

	assert(_position >= 0 && _position <= getSize());
}

const char* _fasmcpp::AssemblerIO::getData() const
{
	return _buffer.data();
}

size_t _fasmcpp::AssemblerIO::getSize() const
{
	return _buffer.size();
}

bool _fasmcpp::AssemblerIO::isOpen() const
{
	return _open;
}

void _fasmcpp::AssemblerIO::close()
{
	_open = false;
}

void _fasmcpp::AssemblerInput::read(char* buffer, size_t size) const
{
	assert(_open && _position >= 0 && _position + size <= getSize());

	const char* begin = _buffer.data() + _position;
	std::copy(begin, begin + size, buffer);
}

void _fasmcpp::AssemblerOutput::open()
{
	AssemblerIO::open();
	_buffer.clear();
}

void _fasmcpp::AssemblerOutput::write(const char* buffer, size_t size)
{
	assert(_open);

	size_t end = _position + size;
	if (end > getSize())
	{
		_buffer.resize(end);
	}
	std::copy(buffer, buffer + size, _buffer.data() + _position);
}
