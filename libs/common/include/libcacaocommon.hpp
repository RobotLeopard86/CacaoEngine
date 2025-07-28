#pragma once

#include <vector>
#include <istream>
#include <ostream>
#include <string>
#include <cstring>
#include <functional>
#include <stdexcept>

/**
 * @brief Quick utility to throw an exception on an error condition
 *
 * The exception will be thrown if the condition evaluates to false
 *
 * @param cond The condition to evaluate
 * @param msg The message for the exception thrown
 * @param unwindFn A function to clean up state before exception throwing should the condition be false
 */
inline void CheckException(bool cond, std::string msg, std::function<void()> unwindFn = []() {}) {
	if(!cond) {
		unwindFn();
		throw std::runtime_error(msg);
	}
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

	pos_type seekoff(off_type off, std::ios_base::seekdir dir, std::ios_base::openmode which = std::ios_base::in | std::ios_base::out) override {
		std::streamoff newpos = -1;

		//Input modification
		if(which & std::ios_base::in) {
			//Get current pointers
			char* begin = eback();
			char* at = gptr();
			char* end = egptr();

			//Move to the appropriate position based on the direction
			switch(dir) {
				case std::ios::beg:
					newpos = off;//Offset from the beginning needs no modifier
					break;
				case std::ios::cur:
					newpos = at - begin + off;//Get the index we are in and add the offset
					break;
				case std::ios::end:
					newpos = end - begin + off;//Find the end index and add the offset
					break;
				default:
					return -1;
			}

			//Bounds-check the offset
			if(newpos < 0 || newpos > (end - begin)) {
				return -1;
			}

			//Set the new get area
			setg(begin, begin + newpos, end);
		}

		//Output modification
		if(which & std::ios_base::out) {
			//Get current pointers
			char* begin = pbase();
			char* at = pptr();
			char* end = epptr();

			//Move to the appropriate position based on the direction
			switch(dir) {
				case std::ios::beg:
					newpos = off;//Offset from the beginning needs no modifier
					break;
				case std::ios::cur:
					newpos = at - begin + off;//Get the index we are in and add the offset
					break;
				case std::ios::end:
					newpos = end - begin + off;//Find the end index and add the offset
					break;
				default:
					return -1;
			}

			//Bounds-check the offset
			if(newpos < 0 || newpos > (end - begin)) {
				return -1;
			}

			//Reset put area pointers
			setp(buffer.data(), buffer.data() + buffer.size());

			//Advance put cursor
			pbump(newpos);
		}

		return newpos;
	}

	pos_type seekpos(pos_type pos, std::ios_base::openmode which = std::ios_base::in | std::ios_base::out) override {
		return seekoff(pos, std::ios::beg, which);
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
	  : std::istream(bufInit(data)) {
		rdbuf(buf);
		init(buf);
	}

	~ibytestream() {
		delete buf;
	}

  private:
	bytestreambuf* buf;

	bytestreambuf* bufInit(std::vector<char>& data) {
		buf = new bytestreambuf(data);
		return buf;
	}
};

/**
 * @brief Byte output stream utility
 */
class obytestream : public std::ostream {
  public:
	obytestream(std::vector<char>& data)
	  : std::ostream(bufInit(data)) {
		rdbuf(buf);
		init(buf);
	}

	~obytestream() {
		delete buf;
	}

  private:
	bytestreambuf* buf;

	bytestreambuf* bufInit(std::vector<char>& data) {
		buf = new bytestreambuf(data);
		return buf;
	}
};