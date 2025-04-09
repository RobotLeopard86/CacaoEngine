#pragma once

#include <vector>
#include <istream>
#include <ostream>
#include <exception>
#include <string>
#include <stdexcept>

/**
 * @brief Quick utility to throw an exception on an error condition
 *
 * The exception will be thrown if the condition evaluates to false
 *
 * @param cond The condition to evaluate
 * @param msg The message for the exception thrown
 */
inline void CheckException(bool cond, std::string msg) {
	if(!cond) throw std::runtime_error(msg);
}

/**
 * @brief Byte stream buffer supporting both input and output, with auto-resizing
 */
class bytestreambuf : public std::streambuf {
  public:
	/**
	 * @brief Create a bytestreambuf from a vector of data
	 *
	 * @param data The vector to use as a buffer
	 */
	bytestreambuf(std::vector<char>& data)
	  : buffer(data) {
		setg(buffer.data(), buffer.data(), buffer.data() + buffer.size());
		setp(buffer.data(), buffer.data() + buffer.size());
	}

  protected:
	int overflow(int ch) override {
		if(ch == EOF) {
			return EOF;
		}

		//Expand buffer and ensure proper size
		std::ptrdiff_t offset = pptr() - pbase();
		buffer.push_back(static_cast<char>(ch));

		//Reset stream buffer pointers
		setp(buffer.data(), buffer.data() + buffer.size());
		pbump(static_cast<int>(offset + 1));

		return ch;
	}

	std::streamsize xsputn(const char* s, std::streamsize n) override {
		std::ptrdiff_t remaining = epptr() - pptr();
		if(n > remaining) {
			std::ptrdiff_t offset = pptr() - pbase();
			buffer.insert(buffer.end(), s, s + n);

			//Reset buffer pointers
			setp(buffer.data(), buffer.data() + buffer.size());
			pbump(static_cast<int>(offset + n));
		} else {
			std::memcpy(pptr(), s, n);
			pbump(static_cast<int>(n));
		}
		return n;
	}

  private:
	std::vector<char>& buffer;
};

/**
 * @brief Byte input stream utility
 */
class ibytestream : public std::istream {
  public:
	ibytestream(std::vector<char>& data)
	  : std::istream(&buf), buf(data) {
		rdbuf(&buf);
	}

  private:
	bytestreambuf buf;
};

/**
 * @brief Byte output stream utility
 */
class obytestream : public std::ostream {
  public:
	obytestream(std::vector<char>& data)
	  : std::ostream(&buf), buf(data) {
		rdbuf(&buf);
	}

  private:
	bytestreambuf buf;
};