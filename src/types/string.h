#pragma once
#include <cstring>
#include <algorithm>

namespace guitard {
  class String {
    char* mString = nullptr;
    size_t mLength = 0;
  public:
    String(const char* source) {
      set(source);
    }

    String(const String& source) {
      set(source);
    }

    String() = default;

    String& operator= (const char* source) {
      set(source);
      return *this;
    }

    String& operator= (const String& source) {
      set(source.get());
      return *this;
    }

    String operator+ (const String& source) {
      append(source.get());
      return *this;
    }

    ~String() {
      resize(0);
    }

    static const char pathDelimiter() {
#ifdef _WIN32
      return '\\';
#else
      return '/';
#endif
    }

    void resize(const size_t size, bool keep = true) {
      if (size <= 0) { // clears the string
        mLength = 0;
        delete[] mString;
        mString = nullptr;
      }
      else {
        char* resized = new char[size + 1];
        memset(resized, 0, size + 1);
        if (mString != nullptr) { // copy over the old string and get rid of the old one
          if (keep) {
            memcpy(resized, mString, mLength);
          }
          char* old = mString;
          mString = resized;
          delete[] old;
        }
        else {
          mString = resized;
        }
        mString[size] = 0; // Make sure it's always null terminated
        mLength = size;
      }
    }

    void set(const char* source, const size_t offset = 0) {
      const size_t length = strlen(source);
      if (length > 0) {
        resize(std::max(length + offset, mLength), 0 < offset);
        memcpy(mString + offset, source, length);
      }
    }

    void set(const String& source) {
      set(source.get());
    }

    void set(const String* source) {
      set(source->get());
    }

    void append(const char* source, size_t max = 1024 * 1024) {
      set(source, mLength);
    }

    size_t getLength() const {
      return mLength;
    }

    const char* get() const {
      return mString;
    }

    const char* getExt() {
      if (mString == nullptr) {
        return nullptr;
      }
      const char* s = mString;
      const char* endp = s + mLength;
      const char* p = endp - 1;
      const char delimiter = pathDelimiter();
      while (p >= s && delimiter  != *p) {
        if (*p == '.') {
          return p;
        }
        p--;
      }
      return endp;
    }

    const char* getFilePart() {
      if (mString == nullptr) {
        return nullptr;
      }
      const char* s = mString;
      const char* p = s + mLength - 1;
      const char delimiter = pathDelimiter();
      while (p >= s && *p != delimiter) {
        p--;
      }
      return p + 1;
    }
  };
}
