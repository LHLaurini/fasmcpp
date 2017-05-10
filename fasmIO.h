#pragma once

#include <cstdint>
#include <vector>

namespace _fasmcpp
{
	class AssemblerIO
	{
	public:
		enum OffsetMode : uint8_t
		{
			Begin = 0, Current = 1, End = 2
		};

		AssemblerIO();
		AssemblerIO(const char* string);
		AssemblerIO(const char* buffer, size_t size);
		virtual void open();
		size_t getPosition() const;
		void setPosition(int offset, OffsetMode mode);
		const char* getData() const;
		size_t getSize() const;
		bool isOpen() const;
		void close();

	protected:
		std::vector<char> _buffer;
		size_t _position = 0;
		bool _open = false;
	};

	class AssemblerInput : public AssemblerIO
	{
	public:
		using AssemblerIO::AssemblerIO;
		void read(char* buffer, size_t size) const;
	};

	class AssemblerOutput : public AssemblerIO
	{
	public:
		using AssemblerIO::AssemblerIO;
		void open() override;
		void write(const char* buffer, size_t size);
	};
}
