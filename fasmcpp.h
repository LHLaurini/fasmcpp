#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include "fasmIO.h"

class AssemblyKey;

/// Contains all public symbols
namespace fasmcpp
{
	/// Literals which can be optionally used to specify larger amounts of memory.
	namespace size_multipliers
	{
		/// Example:
		/// \code
		/// using namespace fasmcpp::size_multipliers;
		/// Assembler assembler = 100_KB;
		/// \endcode
		constexpr size_t operator""_KB(unsigned long long int size)
		{
			return static_cast<size_t>(size * 1024);
		}

		/// Example:
		/// \code
		/// using namespace fasmcpp::size_multipliers;
		/// Assembler assembler = 100.5_KB;
		/// \endcode
		constexpr size_t operator""_KB(long double size)
		{
			return static_cast<size_t>(size * 1024);
		}

		/// Example:
		/// \code
		/// using namespace fasmcpp::size_multipliers;
		/// Assembler assembler = 1_MB;
		/// \endcode
		constexpr size_t operator""_MB(unsigned long long int size)
		{
			return static_cast<size_t>(size * 1024 * 1024);
		}

		/// Example:
		/// \code
		/// using namespace fasmcpp::size_multipliers;
		/// Assembler assembler = 1.5_MB;
		/// \endcode
		constexpr size_t operator""_MB(long double size)
		{
			return static_cast<size_t>(size * 1024 * 1024);
		}

		/// Example:
		/// \code
		/// using namespace fasmcpp::size_multipliers;
		/// Assembler assembler = 1_GB;
		/// \endcode
		constexpr size_t operator""_GB(unsigned long long int size)
		{
			return static_cast<size_t>(size * 1024 * 1024 * 1024);
		}

		/// Example:
		/// \code
		/// using namespace fasmcpp::size_multipliers;
		/// Assembler assembler = 1.5_GB;
		/// \endcode
		constexpr size_t operator""_GB(long double size)
		{
			return static_cast<size_t>(size * 1024 * 1024 * 1024);
		}
	}

	using ::fasmcpp::size_multipliers::operator""_KB;

	/// Manages resources for Flat %Assembler
	class Assembler
	{
	public:
		/// \brief Constructs an empty and invalid instance.
		///
		/// You should properly initialize an instance before using it.
		Assembler();
		/// \param[in] memory Amount of memory to allocate for the assembler
		/// \param[in] predefinitionsMemory Amount of memory to allocate for storing predefinitions
		/// \note \p predefinitionsMemory is only used for 64-bit platforms and is ignored
		///       otherwise.
		/// \warning Some platforms allow only one Assembler to be constructed and will throw an
		///          exception otherwise.
		/// \throws std::runtime_error if failed to allocate memory or if the binary was not found.
		/// \throws std::length_error if the requested amount of memory cannot fit in 32-bit
		///         addressable memory.
		Assembler(size_t memory, size_t predefinitionsMemory = 10_KB);
		Assembler(const Assembler&) = delete;
		Assembler(Assembler&&);
		Assembler& operator=(const Assembler&) = delete;
		Assembler& operator=(Assembler&&);
		~Assembler();
		/// Returns a pointer to the memory allocated for Flat %Assembler
		void* getMemory() const;
		/// Returns the amount of memory in bytes allocated for Flat %Assembler
		uint32_t getMemorySize() const;
		/// \brief Returns a pointer to the memory allocated for predefinitions.
		///
		/// Only for 64-bit platforms.
		char* getPredefinitionsMemory() const;
		/// \brief Returns the amount of memory in bytes allocated for predefinitions.
		///
		/// Only for 64-bit platforms
		uint32_t getPredefinitionsMemorySize() const;

	private:
		void* _assemblerMemory;
		uint32_t _assemblerMemorySize;
		char* _predefinitionsMemory = nullptr;
		uint32_t _predefinitionsMemorySize = 0;
	};

	/// Contains assembly code and its bytecode and predefinitions
	class Assembly
	{
	public:
		/// Constructs an empty, but valid instance
		Assembly();
		/// Constructs an instance and sets the code to be built
		Assembly(const char* code);
		Assembly(const Assembly&) = delete;
		Assembly(Assembly&&);
		Assembly& operator=(const Assembly&) = delete;
		Assembly& operator=(Assembly&&);
		/// Assigns an Assembler
		void setAssembler(Assembler* assembler);
		/// Returns the assigned Assembler
		Assembler* getAssembler() const;
		/// \brief Builds the code using Flat %Assembler
		/// \throws std::invalid_argument if there is no valid Assembler bound. See setAssembler().
		/// \throws std::runtime_error on x64 platforms if out of predefinitions memory.
		/// \returns \c true if successful and \c false if failed.
		bool assemble();
		/// Runs the code previously built by assemble()
		/// \param preserveRegisters If \c true, fasmcpp backs up all non-volatile registers
		/// \returns Return code set from assembly depending on the call convention.
		/// \throws std::runtime_error if an error occurred while assembling.
		/// \throws std::logic_error if assemble() has not been called.
		int64_t run(bool preserveRegisters = true) const;
		/// Returns a string containing any errors found during the build process
		const char* getErrors() const;
		/// Returns user messages displayed by using the \c display directive
		const char* getMessages() const;
		/// \private
		void addError(const char* error, AssemblyKey);
		/// \private
		void addMessage(const char* message, AssemblyKey);
		/// \private
		_fasmcpp::AssemblerInput& getInput(AssemblyKey);
		/// \private
		_fasmcpp::AssemblerOutput& getOutput(AssemblyKey);
		/// \brief Adds a predefinition
		///
		/// Adding a predefinition through this method is equivalent to adding <tt>name=value</tt>
		/// to the assembly code
		/// \param name Name of the definition (maximum length is 255 characters)
		/// \param value Value of the definition
		/// \throws std::invalid_argument if \p name is too long
		void addPredefinition(const char* name, const char* value);
		/// \brief Adds a pointer predefinition
		///
		/// Prefer this for defining pointers. Equivalent to <tt>addPredefinition(name,
		/// std::to_string(reinterpret_cast<uintptr_t>(value)).c_str())</tt>.
		/// \param name Name of the definition (maximum length is 255 characters)
		/// \param value Value of the definition
		/// \throws std::invalid_argument if \p name is too long
		void addPredefinition(const char* name, const void* value);
		/// Removes all previously added predefinitions
		void clearPredefinitions();
		/// Returns a pointer to the output of the assembler
		const char* getOutput();
		/// Returns the size of the output of the assembler
		size_t getOutputSize();

	private:
		void allocateExecutableMemory(size_t size);
		void freeExecutableMemory();

		Assembler* _assembler;
		_fasmcpp::AssemblerInput _code;
		_fasmcpp::AssemblerOutput _bytecode;
		std::string _errors;
		std::string _messages;
		std::unordered_map<std::string, std::string> _predefinitions;
		bool _predefinitionsChanged = false;
		void* _executableMemory = nullptr;
		size_t _executableMemorySize = 0;

#ifdef _WIN64
		const char* rebuildPredefinitionsString();
#else
		std::vector<char> _predefinitionsString;
#endif
	};
}